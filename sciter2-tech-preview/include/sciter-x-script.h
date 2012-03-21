/** \mainpage Terra Informatica Sciter engine.
 *
 * \section legal_sec In legalese
 *
 * The code and information provided "as-is" without
 * warranty of any kind, either expressed or implied.
 *
 * <a href="http://terrainformatica.com/Sciter">Sciter Home</a>
 *
 * (C) 2003-2006, Terra Informatica Software, Inc. and Andrew Fedoniouk
 *
 * \section structure_sec Structure of the documentation
 *
 * See <a href="files.html">Files</a> section.
 **/

/**\file
 * \brief Application defined scripting classes.
 **/

#ifndef __sciter_x_scripting_h__
#define __sciter_x_scripting_h__

#include "sciter-x.h"
#include "sciter-x-value.h"
#include "tiscript.h"

typedef tiscript_VM* HVM;

// Returns scripting VM assosiated with Sciter HWND.
EXTERN_C HVM   SCAPI SciterGetVM( HWND hwnd );

// Calls method 'function' of the object 'obj' with parameters given as VALUEs rather tiscript_value. 
// This method can be used in inter-thread/inter-vm communications. 
EXTERN_C BOOL  SCAPI SciterCall_V( HVM vm, tiscript_value obj, const char* methodName, 
                                  const SCITER_VALUE* argv, unsigned int argn, SCITER_VALUE* pretval);

// Converts tiscript value handle to the json::value (VALUE) with optional isolation.
// tiscript::value handle is a handle that specific for particular VM
// json::value can be passed across different VMs, threads and can be stored outside VM heap.
// isolate==true - will create value that can be passed across thread and VM boundaries.
EXTERN_C BOOL  SCAPI Sciter_v2V(HVM vm, tiscript_value tisv, SCITER_VALUE* jsonv, BOOL isolate);

// Backward vonversion
EXTERN_C BOOL  SCAPI Sciter_V2v(HVM vm, const SCITER_VALUE* jsonv, tiscript_value* tisv);

#if defined(__cplusplus) && !defined( PLAIN_API_ONLY )
  #include "tiscript.hpp"
  namespace sciter
  {
    inline SCITER_VALUE v2v(tiscript::VM* vm, tiscript::value val, bool isolate = true)
    {
      SCITER_VALUE v;
      BOOL r = Sciter_v2V(vm,val,&v, BOOL(isolate));
      assert(r); r;
      return v;
    }
    inline tiscript::value v2v(tiscript::VM* vm, const SCITER_VALUE& val)
    {
      tiscript::value v;
      BOOL r = Sciter_V2v(vm,&val,&v);
      assert(r); r;
      return v;
    }
  }
#endif

#endif
