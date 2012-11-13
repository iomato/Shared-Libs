// inspector.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "inspector.h"
#include "inspector-log.h"
#include "inspector-window.h"
#include "inspector-host-behavior.h"

INSPECTOR_API VOID WINAPI SciterInspector(HELEMENT root)
{
  sciter::debug_broker* pdbg = sciter::debug_broker::instance();
  if( root != pdbg->root || !sciter::inspector_window::activate())
  {
    if(pdbg->root)
      pdbg->root.detach_event_handler( pdbg );
    pdbg->root = root;
    ::SciterGetElementHwnd(root,&pdbg->hwnd,TRUE);
    sciter::dom::element(root).attach_event_handler( pdbg );
  }
}

INSPECTOR_API VOID WINAPI SciterWindowInspector(HWND hwndSciter)
{
  if(!sciter::inspector_window::activate())
  {
    sciter::debug_broker* pdbg = sciter::debug_broker::instance();
    pdbg->hwnd = hwndSciter;
    sciter::attach_dom_event_handler(hwndSciter, pdbg );
  }
}


/*INSPECTOR_API VOID WINAPI SciterLog( OUTPUT_SUBSYTEMS subsystem, OUTPUT_SEVERITY severity, LPCWSTR text, UINT length)
{
}*/
