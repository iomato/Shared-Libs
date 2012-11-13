#pragma once

//#include "resource.h"

#include "sciter-x.h"
#include "aux-asset.h"
#include "sciter-x-threads.h"
#include "sciter-x-dom.hpp"
#include "sciter-x-behavior.h"
//#include "inspector-window.h"
#include <functional>
#include <deque>

namespace sciter
{
  class inspector_window;

  #define POSTED_EVENT (unsigned(-1) - 0xaf)

  // this event handler to be attached to the root element of document that needs to be inspected.
  struct debug_broker : public event_handler, aux::asset
  {
    typedef std::function<void()> fcb_t;
    bool          attached_to_window;
    HWND          hwnd; // inpectable HWND
    dom::element  root; // inpectable root
    dom::element  highlighted;
    dom::element  current;
    sync::event   e_done;
    fcb_t         fcb; 
    
    debug_broker():root(0),current(0),hwnd(0),attached_to_window() {}

    virtual void attached  (HELEMENT he ) override;
    virtual void detached  (HELEMENT he ) override;
    virtual bool handle_mouse  (HELEMENT he, MOUSE_PARAMS& params ) override;
    virtual bool handle_event  (HELEMENT he, BEHAVIOR_EVENT_PARAMS& params ) override;

    static  debug_broker* instance();
    
    // execute cb in host thread, blocks calling thread until cb executed
    bool    host_exec(fcb_t cb);
    
    // methods marked by $ are called from inspector thread
    void    $inspector_window_closed();
    json::value  $element_navigate_to(json::value uid/*to root if undefined*/);
    json::value  $element_highlight(json::value uid /*if undefined then current*/);
    json::value  $element_children(json::value uid /*if undefined then current*/);
    json::value  $element_attributes(json::value uid /*if undefined then current*/);
    json::value  $element_details(json::value uid);
    json::value  $element_stack(json::value uid);
    json::value  $run_host_script(json::value str);


    void attach_to(HWND hwnd);
    void attach_to(HELEMENT he);

  };
}