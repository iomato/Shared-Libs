
#include "stdafx.h"
#include "sciter-x-dom.hpp"
#include "inspector-window.h"
#include "inspector-host-behavior.h"
#include "inspector-log.h"
#include <process.h>

extern HINSTANCE ghInstance;

namespace sciter
{
  
  #define WINDOW_CLASS_NAME L"sciter-inspector"			// the main frame class name

  //debug_output_console dbg_out;

  //
  //  FUNCTION: frame::init()
  //
  //  PURPOSE: Registers the frame class.
  //
  bool inspector_window::init_class()
  {
    
    static ATOM cls = 0;
    if( cls ) 
      return true;

	  WNDCLASSEX wcex;

	  wcex.cbSize = sizeof(WNDCLASSEX);

	  wcex.style			    = CS_HREDRAW | CS_VREDRAW;
	  wcex.lpfnWndProc	  = wnd_proc;
	  wcex.cbClsExtra		  = 0;
	  wcex.cbWndExtra		  = 0;
	  wcex.hInstance		  = ghInstance;
	  wcex.hIcon			    = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SCITER_INSPECTOR));
	  wcex.hCursor		    = LoadCursor(NULL, IDC_ARROW);
	  wcex.hbrBackground	= NULL;//(HBRUSH)(COLOR_WINDOW+1);
	  wcex.lpszMenuName	  = 0;//MAKEINTRESOURCE(IDC_PLAINWIN);
	  wcex.lpszClassName	= WINDOW_CLASS_NAME;
	  wcex.hIconSm		    = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SCITER_INSPECTOR));

	  cls = RegisterClassEx(&wcex);
    return cls != 0;
  }

  inspector_window::inspector_window():_hwnd(0)
  {
    add_ref();
  }

  inspector_window* inspector_window::create(const wchar_t* url)
  {
    if(!init_class())
      return 0;

    inspector_window *pw = new inspector_window();

    pw->_hwnd = CreateWindowEx(0,WINDOW_CLASS_NAME, L"Sciter Inspector", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, ghInstance, pw);
    
    if (!pw->_hwnd)
    {
      delete pw;
      return false;
    }
     
    SetWindowLongPtr(pw->_hwnd, GWLP_USERDATA, LONG_PTR(pw));
    pw->setup_callback(); // to receive SC_LOAD_DATA, SC_DATA_LOADED, etc. notification
    attach_dom_event_handler(pw->_hwnd,pw); // to receive DOM events

    pw->load_file(url);

    dom::element root_el = pw->root();
    assert(root_el.is_valid());
    if(root_el)
    {
      //init_instance(root_el);
      dom::element title_el = root_el.find_first(":root>head>title");
      pw->set_title(title_el.text());
    }     
    
    ShowWindow(pw->_hwnd, SW_SHOW);
    UpdateWindow(pw->_hwnd);
    
    return pw;
  }

  bool inspector_window::ht_post_notification(WPARAM_HOST_NOTIFICATION wp, LPARAM lp)
  {
    inspector_window* iw = instance;
    if(iw) { PostMessage(iw->_hwnd,WM_HOST_NOTIFICATION,wp,lp); return true; }
    return false;
  }

  bool inspector_window::ht_send_notification(WPARAM_HOST_NOTIFICATION wp, LPARAM lp)
  {
    inspector_window* iw = instance;
    DWORD_PTR lr = 0;
    if(iw) { return 0 != ::SendMessageTimeout(iw->_hwnd,WM_HOST_NOTIFICATION,wp,lp,SMTO_BLOCK,2000,&lr); }
    return false;
  }


  void inspector_window::open() {

    sync::critical_section _(lock);
    if(!instance)
      _beginthread(&inpector_thread,0,0);
    else
      ShowWindow(instance->_hwnd, SW_SHOW);
  }

  void inspector_window::forget(bool and_release)
  {
    _hwnd = 0;
    if(and_release)
      release();
  }

  inspector_window* inspector_window::ptr(HWND hwnd)
  {
    return reinterpret_cast<inspector_window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  json::value inspector_window::debug(unsigned argc, const json::value* argv)
  {
    for(unsigned n = 0; n < argc; ++n)
    {
      if(n) OutputDebugStringW(L",");
      OutputDebugStringW(argv[n].to_string(CVT_JSON_LITERAL).c_str());
    }
    OutputDebugStringW(L"\n");
    return json::value();
  }
  
  //
  //  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
  //
  //  PURPOSE:  Processes messages for the main frame.
  //

  LRESULT CALLBACK inspector_window::wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {

  //SCITER integration starts
    BOOL handled = FALSE;
    LRESULT lr = SciterProcND(hWnd,message,wParam,lParam, &handled);
    if( handled )
      return lr;
  //SCITER integration ends
  
    aux::asset_ptr<inspector_window> self = ptr(hWnd);
    if(self)
  	  switch (message)
	  {
	  case WM_DESTROY:
      debug_broker::instance()->$inspector_window_closed();
      SetWindowLongPtr(hWnd, GWLP_USERDATA,0);
      self->forget(true);
      instance = 0;
      PostQuitMessage(0);
		  break;
    default:
      if( message == WM_HOST_NOTIFICATION )
      {
        switch(wParam)
        {
          case WPARAM_HOST_ATTACHED:
            self->on_host_attached();
            return 0;
          case WPARAM_HOST_DETACHED:
            self->on_host_detached();
            return 0;
          case WPARAM_HOST_LOG_ARRIVED:
            self->on_host_log_arrived();
            return 0;
          case WPARAM_HOST_CONTENT_CHANGED:
            self->on_host_content_changed(lParam);
            return 0;
          case WPARAM_HOST_HIGHLIGHTED_CHANGED:
            self->on_host_highlighted_changed(lParam);
            return 0;

        }
        return 0;
      }

      //if(self && self->on_message(message,wParam,lParam,lr))
      //  return lr;
	  }
    
	  return DefWindowProc(hWnd, message, wParam, lParam);
  }

  json::string inspector_window::get_title()
  {
    TCHAR buf[256] = {0};
    ::GetWindowText(_hwnd, buf, 255);
    return json::string(buf);
  }
  void inspector_window::set_title(const json::string& title)
  {
    ::SetWindowText(_hwnd, title.c_str() );
  }

  void  inspector_window::on_host_attached() 
  {
    call_function("onHostAttached");
    //debug_broker::instance()->host_exec([](HELEMENT root,HELEMENT current){
    // 
    //});
    //sciter::eval()
    
  }
  void  inspector_window::on_host_detached() 
  {
    call_function("onHostDetached");
    ::DestroyWindow(_hwnd);
  }
  void  inspector_window::on_host_highlighted_changed(unsigned int uid) 
  {
    call_function("onHostHighlightedChanged", uid? json::value(int(uid)): json::value());
  }
  void  inspector_window::on_host_log_arrived() 
  {
    call_function("onLogItemArrived");
  }
  void  inspector_window::on_host_content_changed(unsigned int uid)
  {
    call_function("onHostContentChanged",uid? json::value(int(uid)): json::value());
  }

  sync::mutex                      inspector_window::lock;
  aux::asset_ptr<inspector_window> inspector_window::instance;
  
  void inspector_window::inpector_thread( void* ) 
  {
    OleInitialize(0); // for system drag-n-drop
    {
      sync::critical_section _(inspector_window::lock);
      if(instance)
        return;
      instance = create(L"res:facade.htm");
    }

	  if (!instance || !instance->is_valid())
		  return;

	  MSG msg;

    ht_post_notification(WPARAM_HOST_ATTACHED);
	  
	  // Main message loop:
	  while (instance && instance->is_valid() && GetMessage(&msg, NULL, 0, 0))
	  {
  	  TranslateMessage(&msg);
		  DispatchMessage(&msg);
	  }
    OleUninitialize();
    instance = 0;
  }

  bool inspector_window::on_script_call(HELEMENT he, LPCSTR name, UINT argc, json::value* argv, json::value& retval)
  {
    aux::chars fname = aux::chars_of(name);
#define FUNCTION(ARGC, NAME) if( argc == ARGC && fname == const_chars(#NAME))

    FUNCTION(0,nextLogMessage)
    {
      OUTPUT_SUBSYTEMS subsystem;
      OUTPUT_SEVERITY  severity;
      json::string     text;

      if(!fetch_log(subsystem,severity,text))
      {
        retval = json::value::null();
        return true;
      }

      retval.set_item(0,json::value(int(subsystem)));
      retval.set_item(1,json::value(int(severity)));
      retval.set_item(2,json::value(text));
      return true;
    }
    FUNCTION(1,navigateTo)
    {
      retval = debug_broker::instance()->$element_navigate_to(argv[0]);
      return true;
    }
    FUNCTION(0,navigateToRoot)
    {
      retval = debug_broker::instance()->$element_navigate_to(json::value());
      return true;
    }
    FUNCTION(0,highlightElement)
    {
      retval = debug_broker::instance()->$element_highlight(json::value());
      return true;
    }
    FUNCTION(1,highlightElement)
    {
      retval = debug_broker::instance()->$element_highlight(argv[0]);
      return true;
    }
    FUNCTION(1,getChildren)
    {
      retval = debug_broker::instance()->$element_children(argv[0]);
      return true;
    }
    FUNCTION(0,getRootChildren)
    {
      retval = debug_broker::instance()->$element_children(json::value());
      return true;
    }
    FUNCTION(1,getElementDetails)
    {
      retval = debug_broker::instance()->$element_details(argv[0]);
      return true;
    }
    FUNCTION(1,getElementStack)
    {
      retval = debug_broker::instance()->$element_stack(argv[0]);
      return true;
    }
    FUNCTION(1,runHostScript)
    {
      retval = debug_broker::instance()->$run_host_script(argv[0]);
      return true;
    }
    FUNCTION(1,exec)
    {
      json::value cmd = argv[0];
      if( !cmd.is_string() )
        return false;

      STARTUPINFO si;
      memset(&si,0,sizeof(si));
      si.cb = sizeof(si);
      PROCESS_INFORMATION pi;
      memset(&pi,0,sizeof(pi));

      json::string scmd = cmd.get(L"");
      LPWSTR lpcmd = const_cast<LPWSTR>(scmd.c_str());

      BOOL r = CreateProcess(NULL,lpcmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi);

      if(r) 
      {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
      }
      retval = json::value(r != 0);
      return true;
    }


    return false;
  }



}

