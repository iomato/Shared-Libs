#pragma once

#include "sciter.h"
#include "aux-asset.h"
#include "sciter-x-threads.h"
#include "sciter-x-dom.hpp"
#include "sciter-x-host-callback.h"
#include "sciter-x-behavior.h"
#include <deque>
#include <functional>
#include <algorithm>

namespace sciter
{

  // attached to the root of document being debugged

  struct debug_observer
    :  public aux::asset 
    ,  public event_handler
  {
    enum $ {
      LOG_CAP = 1000, // max number of items in the log
    };

    enum NOTIFICATIONS // from host to debug view, passed in WPARAM
    {
      CURRRENT_ELEMENT_HAS_CHANGED,
      OUTPUT_READY,
      DOCUMENT_DETACHED,
      DOCUMENT_ATACHED,
    };
    enum HOST_NOTIFICATIONS // from debug view to host, passed in WPARAM
    {
      DEBUG_VIEW_CLOSED,
      DEBUG_VIEW_READY,
    };

    aux::asset_ptr<main_frame>  _host;     // host frame
    dom::element                _root;     // root of debugging element
    std::deque<dom::element>    _stack;    // stack of current elements to be observed

    aux::asset_ptr<debug_frame> _debug; // debug view
    std::deque<log_item>        _log;
    sync::mutex                 _lock; 
           
    virtual void attached  (HELEMENT he ) override
    {
      _root = he;
      if(_debug && _debug->is_valid())
        ::PostMessage(_debug->get_hwnd(),get_debug_notification_message_id(),DOCUMENT_ATACHED,0);
    }
    virtual void detached  (HELEMENT he ) override
    { 
      if(_host)
        dom::element::remove_highlightion(_host->get_hwnd());
      _host = 0;
      _root = 0;
      //_current = 0;
      if(_debug && _debug->is_valid())
        ::PostMessage(_debug->get_hwnd(),get_debug_notification_message_id(),DOCUMENT_DETACHED,0);
    }

    bool is_detached() const { return !_root; }
    bool has_debug_window() const { return !!_debug; }

    virtual bool handle_mouse  (HELEMENT he, MOUSE_PARAMS& params ) override
    {
      if( params.cmd == (MOUSE_DOWN | SINKING)
      &&  params.alt_state == (CONTROL_KEY_PRESSED | SHIFT_KEY_PRESSED)
      &&  params.target )
      {
        dom::element el = params.target;
        el.set_highlighted(_host->get_hwnd());
        _stack.clear();
        while(el.is_valid())
        {
          _stack.push_front(el);
          if(aux::streqi(el.get_element_type(),"html"))
            break;
          el = el.parent();
        }
        if(_debug) // notify debug view
          ::PostMessage(_debug->get_hwnd(), get_debug_notification_message_id(),CURRRENT_ELEMENT_HAS_CHANGED,0);
        return true;
      }
      return false;
    }

    void write_log(OUTPUT_SUBSYTEMS subsystem, OUTPUT_SEVERITY severity, const json::string& text )
    {
      {
        sync::critical_section _(_lock);
        static long long _snum = 0;
        log_item li = {++_snum,subsystem,severity,text};
        _log.push_back(li);
        if(_log.size() > LOG_CAP)
          _log.pop_front();
      }
      //if(_debug)
      //  ::PostMessage(_debug->get_hwnd(),get_debug_notification_message_id(),OUTPUT_READY,0);
    }

    static UINT get_remote_thread_invocation_message_id()
    {
      static UINT _msg_id = 0;
      if( !_msg_id )
        _msg_id = ::RegisterWindowMessage(TEXT("sciter::remote_thread_invocation_message_id"));
      return _msg_id;
    }
    static UINT get_debug_notification_message_id()
    {
      static UINT _msg_id = 0;
      if( !_msg_id )
        _msg_id = ::RegisterWindowMessage(TEXT("sciter::debug_notification_message_id"));
      return _msg_id;
    }

    typedef std::function<void()> remote_func;

    // called when window receives get_remote_thread_invocation_message_id  
    LRESULT invoke(LPARAM lp)
    {
      remote_func *pfunc = reinterpret_cast< remote_func* >(lp);
      if( pfunc ) 
         (*pfunc)();
      return 0xAFED;
    }

    //|
    //| methods to be called by debug view sync
    //|

    void get_stack_items( json::value& stack )
    {
      for(auto it = _stack.cbegin(); it != _stack.cend(); ++it)
      {
        json::value si;
        si.set_item(json::value("tag"),json::value(it->get_element_type()));
        si.set_item(json::value("class"),json::value(it->get_attribute("class")));
        si.set_item(json::value("id"),json::value(it->get_attribute("id")));
        si.set_item(json::value("type"),json::value(it->get_attribute("type")));
        stack.append(si);
      }
    }


    bool select_stack_item( uint n )
    {
      if(n < 0 || n >= _stack.size())
        return false;
      if(!_host)
        return false;
      _stack[n].set_highlighted(_host->get_hwnd());
      return true;
    }

    bool style_rules_of_stack_item(uint n, json::value& r)
    {
      if(n < 0 || n >= _stack.size())
        return false;
      if(!_host)
        return false;
      r = _stack[n].call_method("_applied_style_rules_");
      return true;
    }

    bool used_style_of_stack_element(uint n, json::value& r)
    {
      if(n < 0 || n >= _stack.size())
        return false;
      if(!_host)
        return false;
      r = _stack[n].call_method("_used_style_properties_");
      return true;
    }

    bool states_and_attributes(uint n, json::value& r)
    {
      if(n < 0 || n >= _stack.size())
        return false;
      if(!_host)
        return false;
      
      int states = _stack[n].get_state();
      r.set_item(json::value("states"),json::value(states));

      json::value atts;
      for(int i = _stack[n].get_attribute_count() - 1; i >= 0; --i)
      {
        json::astring name = _stack[n].get_attribute_name(unsigned(i));
        json::string val = _stack[n].get_attribute(unsigned(i));
        atts.set_item(json::value(name),json::value(val));
      }
      r.set_item(json::value("attributes"),atts);
      return true;
    }

    void run_host_script(int n, const json::string& str)
    {
      if(!_root)
        return;
      if(n < 0 || n >= _stack.size())
        _root.eval(str);
      else
        _stack[n].eval(str);
    }
     
    void fetch_log_items( std::deque<log_item>& log )
    {
      sync::critical_section _(_lock);
      while( !_log.empty() )
      {
        log.push_back(*_log.cbegin());
        _log.pop_front();
      }
    }
    bool exec_in_hosts_thread(remote_func func)
    {
      return 0xAFED == ::SendMessageTimeout(
        _host->get_hwnd(),
        get_remote_thread_invocation_message_id(),
        0,
        LPARAM(&func),
        SMTO_BLOCK | SMTO_ERRORONEXIT,
        50*1000, // 50 second delay. Is it enough?
        0); 
    }

    //virtual bool handle_mouse  (HELEMENT he, MOUSE_PARAMS& params ) 

    //void


  };
}