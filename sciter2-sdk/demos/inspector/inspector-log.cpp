#include "stdafx.h"
#include "inspector-log.h"
#include "inspector-window.h"
#include <deque>

namespace sciter
{
  struct log_item
  {
    //long long        snum;     // serial number
    OUTPUT_SUBSYTEMS subsystem;
    OUTPUT_SEVERITY  severity;
    json::string     text;
  };

  sync::mutex          lock;
  std::deque<log_item> log; 

  VOID CALLBACK debug_output_proc(LPVOID param, UINT subsystem /*OUTPUT_SUBSYTEMS*/, UINT severity, LPCWSTR text, UINT text_length)
  {
    {
      sync::critical_section _(lock);
      log_item it;
      it.severity = (OUTPUT_SEVERITY)severity;
      it.subsystem = (OUTPUT_SUBSYTEMS)subsystem;
      it.text = json::string(text,text_length);
      log.push_back(it);
    }
    inspector_window::ht_post_notification(WPARAM_HOST_LOG_ARRIVED);
  }

  bool fetch_log( 
      OUTPUT_SUBSYTEMS& subsystem,
      OUTPUT_SEVERITY&  severity,
      json::string&     text)
  {
     sync::critical_section _(lock);
     if(log.size() == 0)
       return false;
     const log_item& it = log.front();
     subsystem = it.subsystem;
     severity = it.severity;
     text = it.text;
     log.pop_front();
     return true;
  }
 
  void attach_log(HWND hwndSciter)
  {
    SciterSetupDebugOutput(hwndSciter,NULL,debug_output_proc);
  }
  void detach_log(HWND hwndSciter)
  {
    SciterSetupDebugOutput(hwndSciter,NULL,NULL);
  }
  
}
