#pragma once

#include "sciter-x.h"
#include "sciter-x-threads.h"

namespace sciter
{

  bool fetch_log( 
      OUTPUT_SUBSYTEMS& subsystem,
      OUTPUT_SEVERITY&  severity,
      json::string&     text);

  bool push_log( 
      OUTPUT_SUBSYTEMS subsystem,
      OUTPUT_SEVERITY  severity,
      json::string     text);

  void attach_log(HWND hwndSciter);
  void detach_log(HWND hwndSciter);


}