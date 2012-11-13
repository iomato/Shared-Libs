#pragma once

//#include "resource.h"

#include "sciter-x.h"
#include "aux-asset.h"
#include "sciter-x-threads.h"
#include "sciter-x-dom.hpp"
#include "sciter-x-host-callback.h"
#include "inspector-host-behavior.h"
#include <deque>

extern HINSTANCE ghInstance;

namespace sciter
{
  #define WM_HOST_NOTIFICATION (WM_APP + 0xAF)
  enum WPARAM_HOST_NOTIFICATION {
    WPARAM_HOST_ATTACHED,
    WPARAM_HOST_DETACHED,
    WPARAM_HOST_CONTENT_CHANGED,
    WPARAM_HOST_LOG_ARRIVED,
    WPARAM_HOST_ELEMENT_CONTENT_CHANGED, // lparam - UID
    WPARAM_HOST_HIGHLIGHTED_CHANGED , // lparam - UID or 0
  };

  class inspector_window
    : public aux::asset
    , public sciter::host<inspector_window>
    , public sciter::event_handler
  {
    HWND         _hwnd;
    
    //dom::element _o_root;     // observing root 
    //dom::element _o_current;  // observing current element

    static LRESULT CALLBACK	wnd_proc(HWND, UINT, WPARAM, LPARAM);
    static inspector_window* ptr(HWND hwnd);
    static bool init_class();

    void forget(bool and_release = true);

    inspector_window();

    static inspector_window* create(const wchar_t* url = L"res:facade.htm");
    static void inpector_thread(void*);

    static sync::mutex                      lock;
    static aux::asset_ptr<inspector_window> instance;

  public:
    
    // notification_handler traits:
    HWND      get_hwnd() { return _hwnd; }
    HINSTANCE get_resource_instance() { return ghInstance; }

    static bool activate()
    {
      if(instance && ::IsWindow(instance->_hwnd))
      {
        ::ShowWindow(instance->_hwnd,SW_SHOW);
        ::SetForegroundWindow(instance->_hwnd);
        return true;
      }
      return false;
    }


        
    virtual ~inspector_window() { forget(false); }
            
            bool is_valid() const { return _hwnd != 0 && ::IsWindow(_hwnd); }

    virtual void close() /* thread safe*/ { ::PostMessage(_hwnd,WM_CLOSE,0,0); };

    json::string get_title();
    void         set_title(const json::string& title);

    json::value  debug(unsigned argc, const json::value* arg);

    // ht_*** functions - called from host thread 
    static bool ht_post_notification(WPARAM_HOST_NOTIFICATION wp, LPARAM lp = 0);
    static bool ht_send_notification(WPARAM_HOST_NOTIFICATION wp, LPARAM lp = 0);
    static void open();

  protected:

    void  on_host_attached();
    void  on_host_detached();
    void  on_host_highlighted_changed(unsigned int uid);
    void  on_host_log_arrived();
    void  on_host_content_changed(unsigned int uid);
    
/*
    BEGIN_FUNCTION_MAP
      FUNCTION_0("nextLogMessage", get_next_log_message);
      //FUNCTION_V("log", debug);
      //FUNCTION_1("run", run);
      //FUNCTION_0("getHighlightedStack", get_highlighted_stack);
      //FUNCTION_1("highlightStackElement", highlight_stack_element);
      //FUNCTION_1("styleRulesOfStackElement", styles_of_stack_element);
      //FUNCTION_1("usedStylePropsOfStackElement", used_style_of_stack_element);
      //FUNCTION_1("statesAndAttributesOfStackElement", states_and_attributes);
      //FUNCTION_2("runHostScript", run_host_script);
    END_FUNCTION_MAP */

    virtual bool on_script_call(HELEMENT he, LPCSTR name, UINT argc, json::value* argv, json::value& retval);



    json::value get_next_log_message();
    
    //| element in "inspector-focus"
    json::value element_navigate_to(json::value uid)                   { return debug_broker::instance()->$element_navigate_to(uid); }
    json::value element_highlight(json::value uid = json::value())     { return debug_broker::instance()->$element_highlight(uid); }
    json::value element_used_styles(json::value uid = json::value());
    json::value element_states_and_attributes(json::value uid = json::value());
    json::value element_get_content(json::value uid = json::value());
    json::value element_get_stack(json::value uid = json::value());

    //json::value get_highlighted_stack();
    //json::value highlight_stack_element(json::value num);
    //json::value styles_of_stack_element(json::value num);
    //json::value used_style_of_stack_element(json::value num);
    //json::value states_and_attributes(json::value num);
    //json::value run_host_script(json::value num, json::value str);
    //json::value run(json::value cmd);

  };

  extern void debug_thread();

}
