// plain-win.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "sciter.h"
#include "sciter-x-dom.hpp"
#include "sciter-debug.h"
#include <process.h>


extern HINSTANCE ghInstance;

namespace sciter
{
  
  #define WINDOW_CLASS_NAME L"sciter-frame"			// the main frame class name

  debug_output_console dbg_out;

  //
  //  FUNCTION: frame::init()
  //
  //  PURPOSE: Registers the frame class.
  //
  bool frame::init_class()
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
	  wcex.hIcon			    = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_SCITER));
	  wcex.hCursor		    = LoadCursor(NULL, IDC_ARROW);
	  wcex.hbrBackground	= NULL;//(HBRUSH)(COLOR_WINDOW+1);
	  wcex.lpszMenuName	  = 0;//MAKEINTRESOURCE(IDC_PLAINWIN);
	  wcex.lpszClassName	= WINDOW_CLASS_NAME;
	  wcex.hIconSm		    = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	  cls = RegisterClassEx(&wcex);
    return cls != 0;
  }

  frame* frame::_first = 0;
  sync::mutex frame::_guard;

  frame::frame():_hwnd(0),_next(0),_prev(0)
  {
    sync::critical_section _(_guard);
    init_class();
    if(_first) 
      _first->_prev = this;
    _next = _first;
    _first = this;
    add_ref();
  }

  bool frame::setup(const wchar_t* url, HWND parent)
  {
    _hwnd = CreateWindowEx(0,WINDOW_CLASS_NAME, L"sciter-frame", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, parent, NULL, ghInstance, this);
    
    if (!_hwnd)
      return false;
     
    SetWindowLongPtr(_hwnd, GWLP_USERDATA, LONG_PTR(this));
    setup_callback(); // to receive SC_LOAD_DATA, SC_DATA_LOADED, etc. notification
    attach_dom_event_handler(_hwnd,this); // to receive DOM events

    init_window();

    load_file(url);

    dom::element root_el = root();
    assert(root_el.is_valid());
    if(root_el)
    {
      init_instance(root_el);
      dom::element title_el = root_el.find_first(":root>head>title");
      set_title(title_el.text());
    }     
    
    ShowWindow(_hwnd, SW_SHOW);
    UpdateWindow(_hwnd);
    
    return true;
  }

  void frame::forget(bool and_release)
  {
    {
      sync::critical_section _(_guard);
      if( this == _first )
        _first = _next;
      if(_next) _next->_prev = _prev;
      if(_prev) _prev->_next = _next;
    }
    _hwnd = 0;
    if(and_release)
      release();
  }

  frame* frame::ptr(HWND hwnd)
  {
    return reinterpret_cast<frame*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }


  json::value frame::debug(unsigned argc, const json::value* argv)
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

  LRESULT CALLBACK frame::wnd_proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
  {

  //SCITER integration starts
    BOOL handled = FALSE;
    LRESULT lr = SciterProcND(hWnd,message,wParam,lParam, &handled);
    if( handled )
      return lr;
  //SCITER integration ends
  
    frame* self = ptr(hWnd);
  
	  switch (message)
	  {
	  case WM_DESTROY:
      SetWindowLongPtr(hWnd, GWLP_USERDATA,0);
      self->forget();
      if( !_first ) // last window closed
		     PostQuitMessage(0);
		  break;
    default:
      if(self && self->on_message(message,wParam,lParam,lr))
        return lr;
	  }
    
	  return DefWindowProc(hWnd, message, wParam, lParam);
  }

  json::string frame::get_title()
  {
    TCHAR buf[256] = {0};
    ::GetWindowText(_hwnd, buf, 255);
    return json::string(buf);
  }
  void frame::set_title(const json::string& title)
  {
    ::SetWindowText(_hwnd, title.c_str() );
  }

// main_frame stuff

  main_frame::main_frame(const wchar_t* url)
    :_is_glassy(false) 
  { 
    setup(url); 
  }

  void main_frame::init_instance(const dom::element& root)
  {
    debug_output::setup_on(get_hwnd());
    if(root.get_style_attribute("background-color") == L"transparent")
       switch_glass(true);
  }


  json::value main_frame::open(json::value url, json::value param)
  {
    if( !url.is_string() )
      return json::value(false);
    sciter::main_frame* wnd = new sciter::main_frame(url.to_string());
    return json::value(wnd != 0);
  }

  void main_frame::switch_glass(bool on_off)
  {
    _is_glassy = on_off;
    static MARGINS unlimited_margins = {-1};
    static MARGINS zero_margins = {0};
    HRESULT hr = DwmExtendFrameIntoClientArea(get_hwnd(),_is_glassy?&unlimited_margins:&zero_margins); // Extend frame across entire window.
    assert(SUCCEEDED(hr));

    ::SciterSetOption(get_hwnd(),SCITER_TRANSPARENT_WINDOW,_is_glassy);
    RECT rc;
    ::GetWindowRect(get_hwnd(), &rc);
    ::SetWindowPos(get_hwnd(), NULL,
                 rc.left, rc.top,
                 rc.right - rc.left, rc.bottom - rc.top,
                 SWP_FRAMECHANGED);    
  }

  json::value main_frame::get_title()
  {
    return json::value(frame::get_title());
  }
  json::value main_frame::set_title(json::value title)
  {
    frame::set_title(title.to_string());
    return json::value();
  }

  json::value main_frame::get_glass()
  {
    return json::value(_is_glassy);
  }
  json::value main_frame::set_glass(json::value on_off)
  {
     json::value pv = get_glass();
     switch_glass(on_off.get(false));
     return pv;
  }

  json::value main_frame::launch_debug()
  {
    if(!_debug_observer || !_debug_observer->has_debug_window())
      sciter::thread(debug_thread,get_debug_observer()); 
    else
    {
      get_debug_observer();
      ::SetForegroundWindow(_debug_observer->_debug->get_hwnd());
    }
    return json::value(true);
  }

  bool main_frame::on_message(UINT m, WPARAM wp, LPARAM lp, LRESULT& lr) 
  {
    if(_debug_observer && m == debug_observer::get_remote_thread_invocation_message_id() )
    {
      lr = _debug_observer->invoke(lp);
      return true;
    }
    else if(_debug_observer && m == debug_observer::get_debug_notification_message_id() )
    {
      switch(wp)
      {
        case debug_observer::DEBUG_VIEW_CLOSED:
        {
          remove_debug_observer();
          call_function("debugDetached");

        } break;
        case debug_observer::DEBUG_VIEW_READY:
        {
           call_function("debugAttached");
        } break;
      }
      return true;
    }
    return false;
  }

  debug_observer* main_frame::get_debug_observer()
  {
    if(!_debug_observer) 
    {
      _debug_observer = new debug_observer();
      _debug_observer->_host = this;
    }
    if(_debug_observer->is_detached())
    {
      dom::element root = get_root();
      dom::element frame = root.find_first("frame#content");
      assert(frame.is_valid());
      _debug_observer->_host = this;
      _debug_observer->_root = frame.child(0);
      assert(_debug_observer->_root.is_valid());
      if(_debug_observer->_root)
        _debug_observer->_root.attach_event_handler(_debug_observer);
    }
    return _debug_observer;
  }

  void main_frame::remove_debug_observer()
  {
    if(_debug_observer && _debug_observer->_root)
    {
      _debug_observer->_root.detach_event_handler(_debug_observer);
    }
  }


  void main_frame::output( OUTPUT_SUBSYTEMS subsystem, OUTPUT_SEVERITY severity, const wchar_t* text, unsigned int text_length )
  {
    if(!_debug_observer) 
    {
      _debug_observer = new debug_observer();
      _debug_observer->_host = this;
    }
    if(_debug_observer)
    {
      _debug_observer->write_log(subsystem, severity, json::string(text,text_length));
      if(!_debug_observer->has_debug_window())
      {
        dom::element root = get_root();
        if( root.is_valid() )
        {
          dom::element inspector = root.find_first("button#inspector");
          if(inspector.is_valid())
          {
            inspector.set_state(STATE_BUSY); // use :busy to indicate "need attention" case
            return;
          }
        }
      }
      else {
        ::PostMessage(_debug_observer->_debug->get_hwnd()
                      , debug_observer::get_debug_notification_message_id()
                      , debug_observer::OUTPUT_READY
                      , 0);
        return;
      }


    }
    
 
    switch(severity)
    {
        case OS_INFO     : print("info:"); break;
        case OS_WARNING  : print("warning:"); break;
        case OS_ERROR    : print("error:"); break;
    } 
    switch(subsystem)
    {
        case OT_DOM:  print("DOM:"); break;
        case OT_CSSS: print("csss!:"); break;
        case OT_CSS:  print("css:"); break;
        case OT_TIS:  print("script:"); break;
    }
    if(text[text_length])
    {
      unsigned n = wcslen(text);
      assert(false);
    }
    else 
      print(text);
  }


  debug_frame::debug_frame(debug_observer* pdo): _dbo(pdo)
  {
    pdo->_debug = this;
    _dbo->fetch_log_items(_log);
    setup(L"res:debug.htm"/*,pdo->_host->get_hwnd()*/);

    if(_dbo->_host)
      ::PostMessage(_dbo->_host->get_hwnd(), _dbo->get_debug_notification_message_id(),debug_observer::DEBUG_VIEW_READY,0); 

  }

  void debug_frame::init_window()
  {
    // but errors in this window will go to the console
    debug_output::setup_on(get_hwnd());
  }
  
  bool debug_frame::on_message(UINT m, WPARAM wp, LPARAM lp, LRESULT& lr)
  {
    if(m == WM_CLOSE)
    {
      if(_dbo && _dbo->_host)
        ::PostMessage(_dbo->_host->get_hwnd(), _dbo->get_debug_notification_message_id(),debug_observer::DEBUG_VIEW_CLOSED,0); 
      if(_dbo)
        _dbo->_debug = 0;
      ::DestroyWindow(get_hwnd());
      ::PostQuitMessage(0);
      return true;
    }
    
    if( m != debug_observer::get_debug_notification_message_id())
      return false;
    if( wp == debug_observer::OUTPUT_READY )
    {
      _dbo->fetch_log_items(this->_log);
      this->call_function("onLogMessage");
      FLASHWINFO fi;
      fi.cbSize = sizeof(fi);
      fi.dwFlags = FLASHW_TRAY;
      fi.uCount = 3;
      fi.hwnd = get_hwnd();
      fi.dwTimeout = 0;
      FlashWindowEx(&fi);
    }
    else if( wp == debug_observer::DOCUMENT_ATACHED )
    {
      _dbo->fetch_log_items(this->_log);
      this->call_function("onDocumentAtached");
    }
    else if( wp == debug_observer::DOCUMENT_DETACHED )
    {
      _dbo->fetch_log_items(this->_log);
      this->call_function("onDocumentDetached");
    }
    else if( wp == debug_observer::CURRRENT_ELEMENT_HAS_CHANGED  )
    {
      this->call_function("onElementChange");
    }
    else 
      assert(false);
    return true;
  }

  json::value debug_frame::get_next_log_message()
  {
    if(_log.empty()) 
      return json::value();
    const log_item& item = *_log.cbegin();
    json::value rv;// = json::value
    rv.set_item(0,json::value(int(item.subsystem)));
    rv.set_item(1,json::value(int(item.severity)));
    rv.set_item(2,json::value(item.text));
    _log.pop_front();
    return rv;
  }

  // run command line 
  json::value debug_frame::run(json::value cmd)
  {
    STARTUPINFO si;
    memset(&si,0,sizeof(si));
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi;
    memset(&pi,0,sizeof(pi));

    json::string scmd = cmd.get(L"");
    LPWSTR lpcmd = const_cast<LPWSTR>(scmd.c_str());

    BOOL r = CreateProcess(NULL,
          lpcmd,
          NULL,
          NULL,
          FALSE,
          0,
          NULL,
          NULL,
          &si,
          &pi);

    if(r) {
      CloseHandle(pi.hProcess);
      CloseHandle(pi.hThread);
    }
    return json::value(r != 0);
  }

  json::value debug_frame::get_highlighted_stack()
  {
    json::value stack;
    auto host_code = [&]() {
      _dbo->get_stack_items(stack);
    };
    _dbo->exec_in_hosts_thread(host_code);
    return stack;
  }

  json::value debug_frame::highlight_stack_element(json::value num)
  {
    int  n = num.get(0);
    bool r = false;
    auto host_code = [&]() {
      r = _dbo->select_stack_item(n);
    };
    _dbo->exec_in_hosts_thread(host_code);
    return json::value(r);
  }

  json::value debug_frame::styles_of_stack_element(json::value num)
  {
    int n = num.get(0);
    json::value r;
    auto host_code = [&]() {
      _dbo->style_rules_of_stack_item(n,r);
    };
    _dbo->exec_in_hosts_thread(host_code);
    return r;
  }

  json::value debug_frame::used_style_of_stack_element(json::value num)
  {
    int n = num.get(0);
    json::value r;
    auto host_code = [&]() {
      _dbo->used_style_of_stack_element(n,r);
    };
    _dbo->exec_in_hosts_thread(host_code);
    return r;
  }

  json::value debug_frame::states_and_attributes(json::value num)
  {
    int n = num.get(0);
    json::value r;
    auto host_code = [&]() {
      _dbo->states_and_attributes(n,r);
    };
    _dbo->exec_in_hosts_thread(host_code);
    return r;
  }

  json::value debug_frame::run_host_script(json::value num, json::value str)
  {
    int n = num.get(0);
    json::string s = str.get(L"");
    json::value r;
    auto host_code = [&]() {
      _dbo->run_host_script(n,s);
    };
    _dbo->exec_in_hosts_thread(host_code);
    return r;
  }

  


  void debug_thread( aux::asset_ptr<debug_observer> ob ) 
  {
    OleInitialize(0); // for system drag-n-drop
    aux::asset_ptr<debug_frame> wnd = new debug_frame(ob);

	  if (!wnd || !wnd->is_valid())
		  return;

	  MSG msg;
	  
	  // Main message loop:
	  while (wnd->is_valid() && GetMessage(&msg, NULL, 0, 0))
	  {
  	  TranslateMessage(&msg);
		  DispatchMessage(&msg);
	  }
    OleUninitialize();
  }

   



}

