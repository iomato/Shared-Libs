#include "stdafx.h"
#include "aux-cvt.h"
#include "inspector-host-behavior.h"
#include "inspector-window.h"
#include "inspector-log.h"

#include <vector>

namespace sciter 
{

    void debug_broker::attach_to(HWND hwnd)
    {
      if(this->hwnd && (hwnd != this->hwnd))
      {
        sciter::detach_dom_event_handler(this->hwnd, this );
        this->hwnd = 0;
      }
      if(!this->hwnd || !sciter::inspector_window::activate())
      {
        this->hwnd = hwnd;
        this->root = dom::element::root_element(hwnd);
        sciter::attach_dom_event_handler(hwnd, this );
      }
    }

    void debug_broker::attach_to(HELEMENT root)
    {
      if( root != this->root || !sciter::inspector_window::activate())
      {
        if(this->root)
          this->root.detach_event_handler( this );
        this->root = root;
        this->hwnd = this->root.get_element_hwnd(true);
        this->root.attach_event_handler( this );
      }
    }

    void debug_broker::attached  (HELEMENT he ) 
    { 
      root = he;
      attach_log(hwnd);
      inspector_window::open();
      //inspector_window::ht_post_notification(WPARAM_HOST_ATTACHED);
    }
    void debug_broker::detached  (HELEMENT he ) 
    { 
      root = 0;
      detach_log(hwnd);
      inspector_window::ht_send_notification(WPARAM_HOST_DETACHED);
    }
    
    bool debug_broker::handle_mouse  (HELEMENT he, MOUSE_PARAMS& params )
    {
      if(  params.cmd == (MOUSE_DOWN | SINKING)
        && params.alt_state == (CONTROL_KEY_PRESSED | SHIFT_KEY_PRESSED) ) 
      {
        highlighted = params.target;
        highlighted.highlight();
        inspector_window::ht_post_notification(WPARAM_HOST_HIGHLIGHTED_CHANGED,highlighted.get_element_uid()); // highlighted has changed
        return true;
      }
      return false;
    }

    bool debug_broker::handle_event (HELEMENT he, BEHAVIOR_EVENT_PARAMS& params )
    {
      switch(params.cmd)
      {
        case POSTED_EVENT:
        {
          if(fcb)
            fcb();
          e_done.signal();
          return true;
        }
        case CONTENT_CHANGED:

          if(!he) 
          { 
            root = dom::element::root_element(hwnd);
            highlighted = 0;
            current = 0;
            inspector_window::ht_post_notification(WPARAM_HOST_HIGHLIGHTED_CHANGED,0); // highlighted has gone
            inspector_window::ht_post_notification(WPARAM_HOST_CONTENT_CHANGED);
          }
          else 
          {
            if( highlighted )
            {
              if(!highlighted.get_element_hwnd(true))
              {
                highlighted = 0;
                inspector_window::ht_post_notification(WPARAM_HOST_HIGHLIGHTED_CHANGED,0); // highlighted has gone
              }
            }
            inspector_window::ht_post_notification(WPARAM_HOST_ELEMENT_CONTENT_CHANGED,dom::element(params.heTarget).get_element_uid());
          }
          
          break;
      }
      return false;
    }
    
    debug_broker* debug_broker::instance()
    {
      static aux::asset_ptr<debug_broker> _instance = new debug_broker();
      return _instance;
    }

    bool debug_broker::host_exec(fcb_t cb)
    {
      if( !root ) 
        return false;
      if( fcb )
        return false; // previous rq is not finished yet.

      fcb = cb;
      dom::element(root).post_event(POSTED_EVENT);
      bool r = e_done.wait(5000); // 5 sec max to avoid deadlocks.
      fcb = 0;
      return r; // true if the function was executed without timeout.
    }

    #define START_HOST_CODE auto _t_ = [&]() {
    #define END_HOST_CODE }; this->host_exec(_t_);
    
    void debug_broker::$inspector_window_closed()
    {
    START_HOST_CODE
        if(highlighted)
          SciterSetHighlightedElement(hwnd,0);
        if(attached_to_window)
          detach_dom_event_handler(hwnd,this);
        else if(root)
          root.detach_event_handler(this);
        root = 0;
        highlighted = 0;
        current = 0;
    END_HOST_CODE
    }

    json::value element_def(dom::element el)
    {
      json::value eldef;
      dom::node n = el;
      json::value atts;
      for(uint i = 0; i < el.get_attribute_count(); ++i)
      {
        json::value kv[2]; 
        kv[0] = json::string(aux::a2w(el.get_attribute_name(i).chars()).chars());
        kv[1] = el.get_attribute(i);
        atts.append(json::value(kv,2));
      }
      if( atts.is_array() )
        eldef.set_item(json::value("atts"),atts);
      eldef.set_item(json::value("tag"),json::value(el.get_element_type()));
      eldef.set_item(json::value("children"),json::value((int)el.children_count()));
      eldef.set_item(json::value("nodes"),json::value((int)n.children_count()));
      eldef.set_item(json::value("uid"),json::value((int)el.get_element_uid()));
      eldef.set_item(json::value("index"),json::value((int)el.index()));
      if(el.children_count() == 0)
      { // if element has no children but text:
        json::string t = el.text();
        if( aux::trim(t.chars()).length )
          eldef.set_item(json::value("text"),json::value(t));
      }

      return eldef;
    }

    json::value debug_broker::$element_navigate_to(json::value uid)
    {
      json::value r;
    START_HOST_CODE
        current = uid.is_int()
          ? dom::element::element_by_uid(hwnd,uid.get(0))
          : root; //dom::element::root_element(hwnd);
        if(current)
          r = element_def(current);
    END_HOST_CODE
      return r;
    }

    json::value debug_broker::$element_highlight(json::value uid)
    {
      bool r = false;
    START_HOST_CODE
      if(uid.is_int())
        current = dom::element::element_by_uid(hwnd,uid.get(0));

    if(current.is_valid())
      {
        current.set_highlighted(hwnd);
        highlighted = current;
        r = true;
      }

    END_HOST_CODE
      return json::value(r);
    }

    json::value debug_broker::$element_children(json::value uid)
    {
      json::value out;
    START_HOST_CODE
      if(uid.is_int())
        current = dom::element::element_by_uid(hwnd,uid.get(0));
      if(!current.is_valid())
        return;

      dom::node n = current;
      json::value r;
      
      for(n = n.first_child(); n.is_valid(); n = n.next_sibling())
      {
        if( n.is_text() )
        {
          json::string t = n.text();
          r.append(t); // string 
        }
        else if( n.is_element() )
          r.append(element_def(dom::element(n)));
        //else 
        // continue; // we are not showing comments
      }
      out = r;
   END_HOST_CODE
      return out;
    }

    json::value debug_broker::$element_attributes(json::value uid)
    {
      json::value r;
    START_HOST_CODE
      if(uid.is_int())
        current = dom::element::element_by_uid(hwnd,uid.get(0));
      if(!current.is_valid())
        return;

      for(uint i = 0; i < current.get_attribute_count(); ++i)
      {
        json::string name = json::string(aux::a2w(current.get_attribute_name(i).chars()).chars());
        json::string val = current.get_attribute(i);
        r.set_item(json::value(name),json::value(val));
      }

   END_HOST_CODE
      return r;
    }

    json::value debug_broker::$element_details(json::value uid)
    {
      json::value r;
    START_HOST_CODE
      if(uid.is_int())
        current = dom::element::element_by_uid(hwnd,uid.get(0));
      if(!current.is_valid())
        return;

      json::value v = current.call_method("_applied_style_rules_");
      r.set_item(json::value("appliedStyleRules"),v);

      v = current.call_method("_used_style_properties_");
      r.set_item(json::value("usedStyleProperties"),v);

      json::value a;

      for(uint i = 0; i < current.get_attribute_count(); ++i)
      {
        json::string name = json::string(aux::a2w(current.get_attribute_name(i).chars()).chars());
        json::string val = current.get_attribute(i);
        a.set_item(json::value(name),json::value(val));
      }
      r.set_item(json::value("attributes"),a);

      int states = current.get_state();
      r.set_item(json::value("states"),json::value(states));

   END_HOST_CODE
      return r;
    }

    json::value debug_broker::$element_stack(json::value uid)
    {
      json::value r;
    START_HOST_CODE
      if(uid.is_int())
        current = dom::element::element_by_uid(hwnd,uid.get(0));
      if(!current.is_valid())
        return;
            
      dom::element el = current;
      std::deque<dom::element> stack;

      while(el.is_valid())
      {
        stack.push_front(el);
        if(aux::streqi(el.get_element_type(),"html"))
          break;
        el = el.parent();
      }

      for(auto it = stack.cbegin(); it != stack.cend(); ++it)
      {
        json::value si;
        si.set_item(json::value("tag"),json::value(it->get_element_type()));
        si.set_item(json::value("class"),json::value(it->get_attribute("class")));
        si.set_item(json::value("id"),json::value(it->get_attribute("id")));
        si.set_item(json::value("type"),json::value(it->get_attribute("type")));
        si.set_item(json::value("uid"),json::value((int)it->get_element_uid()));
        r.append(si);
      }

   END_HOST_CODE
      return r;
    }

    json::value debug_broker::$run_host_script(json::value str)
    {
      json::string s = str.get(L"");
      json::value r;
START_HOST_CODE
      if(highlighted)
        r = highlighted.eval(s);
      else if(root) 
        r = root.eval(s);
END_HOST_CODE
      return r;
    }

}