#include "stdafx.h"
#include "sciter.h"
#include "sciter-x-dom.hpp"


HINSTANCE ghInstance = 0;


int APIENTRY _tWinMain(HINSTANCE hInstance,
                      HINSTANCE hPrevInstance,
                      LPTSTR    lpCmdLine,
                      int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

  OleInitialize(0); // for system drag-n-drop

  ghInstance = hInstance;

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

  {
    aux::asset_ptr<sciter::main_frame> wnd = new sciter::main_frame(L"res:default.htm");

	  if (!wnd || !wnd->is_valid())
    {
		  return FALSE;
    }

	  hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCITER));

	  // Main message loop:
	  while (wnd->is_valid() && GetMessage(&msg, NULL, 0, 0))
	  {
		  if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		  {
			  TranslateMessage(&msg);
			  DispatchMessage(&msg);
		  }
	  }
  }

  OleUninitialize();

	return (int) msg.wParam;
}
