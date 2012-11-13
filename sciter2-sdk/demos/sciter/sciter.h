#pragma once

#include "resource.h"


#include "sciter-x.h"
#include "aux-asset.h"
#include "sciter-x-threads.h"
#include "sciter-x-dom.hpp"
#include "sciter-x-host-callback.h"
#include <deque>

extern HINSTANCE ghInstance;

namespace sciter
{
  class frame
    : public aux::asset
    , public sciter::host<frame>
    , public sciter::event_handler
  {
    HWND   _hwnd;
    frame *_next,*_prev; // linked list of all frames
    static frame *_first;
    static sync::mutex _guard;

    static LRESULT CALLBACK	wnd_proc(HWND, UINT, WPARAM, LPARAM);
    static frame* ptr(HWND hwnd);
    static bool init_class();

    void forget(bool and_release = true);

  public:
    // notification_handler traits:
    HWND      get_hwnd() { return _hwnd; }
    HINSTANCE get_resource_instance() { return ghInstance; }

    //frame(const wchar_t* url);
    virtual ~frame() { forget(false); }

    virtual bool on_close() { return false; }
            
            bool is_valid() const { return _hwnd != 0 && ::IsWindow(_hwnd); }

    virtual void close() /* thread safe*/ { ::PostMessage(_hwnd,WM_CLOSE,0,0); };

    json::string get_title();
    void         set_title(const json::string& title);

    json::value  debug(unsigned argc, const json::value* arg);

  protected:
    frame();
    bool setup(const wchar_t* url,HWND parent = NULL); // instance
    virtual void init_window() {}
    virtual void init_instance(const dom::element& root_el) {}
    virtual bool on_message(UINT, WPARAM, LPARAM, LRESULT&) { return false; }

    BEGIN_FUNCTION_MAP
      FUNCTION_V("debug", debug);
    END_FUNCTION_MAP

  };

  class main_frame: public frame
  {
    bool   _is_glassy; // true - transparent

  public:

    main_frame(const wchar_t* url = L"res:default.htm");

    void switch_glass(bool on_off);

    //virtual bool on_close() 
    //{ 
    //  ::DestroyWindow(get_hwnd());
    //  ::PostQuitMessage(0);
    //  return true;
    //}

  protected:
    virtual void init_instance(const dom::element& root_el) override; // instance

    BEGIN_FUNCTION_MAP
      FUNCTION_0("title", get_title);
      FUNCTION_1("title", set_title);
      FUNCTION_0("glass", get_glass);
      FUNCTION_1("glass", set_glass);
      FUNCTION_V("log", debug);
      FUNCTION_1("open",  open);
      FUNCTION_2("open",  open);
      FUNCTION_0("launchDebugView", launch_debug);
    END_FUNCTION_MAP

    json::value get_title();
    json::value set_title(json::value title);
    json::value get_glass();
    json::value set_glass(json::value on_off);
    json::value open(json::value url, json::value param = json::value());
    json::value launch_debug();


  };


}