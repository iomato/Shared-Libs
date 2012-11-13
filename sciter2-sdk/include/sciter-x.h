#ifndef __SCITER_X__
#define __SCITER_X__

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

/**\file sciter-x.h
 * Main include file.
 **/

#include <windows.h>
#include <oaidl.h>
#include <d2d1.h>
#include <dwrite.h>

#include "value.h"
#ifdef __cplusplus
  #include "aux-cvt.h"
#endif

#if !defined(OBSOLETE)
  /* obsolete API marker*/ 
  #if defined(__GNUC__)
    #define OBSOLETE __attribute__((deprecated))
  #elif defined(_MSC_VER) && (_MSC_VER >= 1300)
    #define OBSOLETE __declspec(deprecated)
  #else
    #define OBSOLETE
  #endif
#endif  

#ifndef LPUINT
  typedef UINT FAR *LPUINT;
#endif

#ifndef LPCBYTE
  typedef const BYTE* LPCBYTE;
#endif

#define STDCALL __stdcall

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C extern
#endif /* __cplusplus **/

#ifndef STATIC_LIB
  #if defined(SCITER_EXPORTS)
    #define SCAPI __declspec(dllexport) __stdcall
  #else
    #define SCAPI __declspec(dllimport) __stdcall
  #endif
#else
  #define SCAPI __stdcall
  void SciterInit( HINSTANCE hModule, bool start);
#endif

#define HAS_TISCRIPT

/** Resource data type.
 *  Used by SciterDataReadyAsync() function.
 **/
enum SciterResourceType
{
  RT_DATA_HTML = 0,
  RT_DATA_IMAGE = 1,
  RT_DATA_STYLE = 2,
  RT_DATA_CURSOR = 3,
  RT_DATA_SCRIPT = 4,
  RT_DATA_RAW = 5,
};

#include "sciter-x-value.h"
#include "sciter-x-dom.h"

/**Get name of Sciter window class.
 *
 * \return \b LPCWSTR, name of Sciter window class.
 *         \b NULL if initialization of the engine failed, Direct2D or DirectWrite are not supported on the OS. 
 *
 * Use this function if you wish to create unicode version of Sciter.
 * The returned name can be used in CreateWindow(Ex)W function.
 * You can use #SciterClassNameT macro.
 **/
EXTERN_C LPCWSTR SCAPI SciterClassName();

/**Returns major and minor version of Sciter engine.
 **/
EXTERN_C UINT  SCAPI SciterVersion(BOOL major);

enum
{
  LOAD_OK = 0,      // do default loading if data not set
  LOAD_DISCARD = 1, // discard request completely
  LOAD_DELAYED = 2, // data will be delivered later by the host
};

/**Notifies that HtmLayout is about to download a referred resource.
 *
 * \param lParam #LPSCN_LOAD_DATA.
 * \return #LOAD_OK or #LOAD_DISCARD
 *
 * This notification gives application a chance to override built-in loader and
 * implement loading of resources in its own way (for example images can be loaded from
 * database or other resource). To do this set #SCN_LOAD_DATA::outData and
 * #SCN_LOAD_DATA::outDataSize members of SCN_LOAD_DATA. Sciter does not
 * store pointer to this data. You can call #SciterDataReady() function instead
 * of filling these fields. This allows you to free your outData buffer
 * immediately.
**/
#define SC_LOAD_DATA       0x01

/**This notification indicates that external data (for example image) download process
 * completed.
 *
 * \param lParam #LPSCN_DATA_LOADED
 *
 * This notifiaction is sent for each external resource used by document when
 * this resource has been completely downloaded. Sciter will send this
 * notification asynchronously.
 **/
#define SC_DATA_LOADED     0x02

/**This notification is sent when all external data (for example image) has been downloaded.
 *
 * This notification is sent when all external resources required by document
 * have been completely downloaded. Sciter will send this notification
 * asynchronously.
 **/
/* obsolete #define SC_DOCUMENT_COMPLETE 0x03 
   use DOCUMENT_COMPLETE DOM event.
  */


/**This notification is sent on parsing the document and while processing
 * elements having non empty style.behavior attribute value.
 *
 * \param lParam #LPSCN_ATTACH_BEHAVIOR
 *
 * Application has to provide implementation of #sciter::behavior interface.
 * Set #SCN_ATTACH_BEHAVIOR::impl to address of this implementation.
 **/
#define SC_ATTACH_BEHAVIOR 0x04


/**This notification is sent on
 * 1) stdin, stdout and stderr operations and
 * 2) view.hostCallback(p1,p2) calls from script
 *
 * \param lParam #LPSCN_CALLBACK_HOST
 *
 **/
#define SC_CALLBACK_HOST 0x05


/**Notification callback structure.
 **/
struct SCITER_CALLBACK_NOTIFICATION
{
  UINT code; /**< [in] one of the codes above.*/
  HWND hwnd; /**< [in] HWND of the window this callback was attached to.*/
};
typedef SCITER_CALLBACK_NOTIFICATION * LPSCITER_CALLBACK_NOTIFICATION;

typedef UINT CALLBACK SciterHostCallback( LPSCITER_CALLBACK_NOTIFICATION pns, LPVOID callbackParam );

typedef SciterHostCallback * LPSciterHostCallback;


/**This structure is used by #SCN_LOAD_DATA notification.
 *\copydoc SCN_LOAD_DATA
 **/

struct SCN_LOAD_DATA: SCITER_CALLBACK_NOTIFICATION
{
    LPCWSTR  uri;              /**< [in] Zero terminated string, fully qualified uri, for example "http://server/folder/file.ext".*/

    LPCBYTE  outData;          /**< [in,out] pointer to loaded data to return. if data exists in the cache then this field contain pointer to it*/
    UINT     outDataSize;      /**< [in,out] loaded data size to return.*/
    UINT     dataType;         /**< [in] SciterResourceType */

    LPVOID   requestId;        /**< [in] request id that needs to be passed as is to the SciterDataReadyAsync call */

    HELEMENT principal;
    HELEMENT initiator;
};

typedef SCN_LOAD_DATA FAR * LPSCN_LOAD_DATA;

/**This structure is used by #SCN_DATA_LOADED notification.
 *\copydoc SCN_DATA_LOADED
 **/
struct SCN_DATA_LOADED: SCITER_CALLBACK_NOTIFICATION
{
    LPCWSTR  uri;              /**< [in] zero terminated string, fully qualified uri, for example "http://server/folder/file.ext".*/
    LPCBYTE  data;             /**< [in] pointer to loaded data.*/
    DWORD    dataSize;         /**< [in] loaded data size (in bytes).*/
    UINT     dataType;         /**< [in] SciterResourceType */
    UINT     status;           /**< [in] 
                                         status = 0 (dataSize == 0) - unknown error. 
                                         status = 100..505 - http response status, Note: 200 - OK! 
                                         status > 12000 - wininet error code, see ERROR_INTERNET_*** in wininet.h
                                 */   
};
typedef SCN_DATA_LOADED FAR * LPSCN_DATA_LOADED;

/**This structure is used by #SCN_ATTACH_BEHAVIOR notification.
 *\copydoc SCN_ATTACH_BEHAVIOR **/
struct SCN_ATTACH_BEHAVIOR: SCITER_CALLBACK_NOTIFICATION
{
    HELEMENT element;          /**< [in] target DOM element handle*/
    LPCSTR   behaviorName;     /**< [in] zero terminated string, string appears as value of CSS behavior:"???" attribute.*/

    ElementEventProc* elementProc;    /**< [out] pointer to ElementEventProc function.*/
    LPVOID            elementTag;     /**< [out] tag value, passed as is into pointer ElementEventProc function.*/

};
typedef SCN_ATTACH_BEHAVIOR FAR * LPSCN_ATTACH_BEHAVIOR;


/**This structure is used by #SC_CALLBACK_HOST notification.
 *\copydoc SC_CALLBACK_HOST **/
struct SCN_CALLBACK_HOST: SCITER_CALLBACK_NOTIFICATION
{
   UINT channel; // 0 - stdin, 1 - stdout, 2 - stderr
   SCITER_VALUE p1; // in, parameter #1
   SCITER_VALUE p2; // in, parameter #2
   SCITER_VALUE r;  // out, retval
};
typedef SCN_CALLBACK_HOST FAR * LPSCN_CALLBACK_HOST;

#include "sciter-x-behavior.h"

/**This function is used in response to SCN_LOAD_DATA request.
 *
 * \param[in] hwnd \b HWND, Sciter window handle.
 * \param[in] uri \b LPCWSTR, URI of the data requested by Sciter.
 * \param[in] data \b LPBYTE, pointer to data buffer.
 * \param[in] dataLength \b DWORD, length of the data in bytes.
 * \return \b BOOL, TRUE if Sciter accepts the data or \c FALSE if error occured
 * (for example this function was called outside of #SCN_LOAD_DATA request).
 *
 * \warning If used, call of this function MUST be done ONLY while handling
 * SCN_LOAD_DATA request and in the same thread. For asynchronous resource loading
 * use SciterDataReadyAsync
 **/
EXTERN_C BOOL SCAPI SciterDataReady(HWND hwnd,LPCWSTR uri,LPCBYTE data, DWORD dataLength);

/**Use this function outside of SCN_LOAD_DATA request. This function is needed when you
 * you have your own http client implemented in your application.
 *
 * \param[in] hwnd \b HWND, Sciter window handle.
 * \param[in] uri \b LPCWSTR, URI of the data requested by Sciter.
 * \param[in] data \b LPBYTE, pointer to data buffer.
 * \param[in] dataLength \b DWORD, length of the data in bytes.
 * \param[in] requestId \b LPVOID, SCN_LOAD_DATA requestId.
 * \return \b BOOL, TRUE if Sciter accepts the data or \c FALSE if error occured
 **/

EXTERN_C BOOL SCAPI SciterDataReadyAsync(HWND hwnd,LPCWSTR uri, LPCBYTE data, DWORD dataLength,
                                         LPVOID requestId);


/**Sciter Window Proc.*/
EXTERN_C LRESULT SCAPI SciterProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**Sciter Window Proc without call of DefWindowProc.*/
EXTERN_C LRESULT SCAPI SciterProcND(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, BOOL* pbHandled);


/**Load HTML file.
 *
 * \param[in] hWndSciter \b HWND, Sciter window handle.
 * \param[in] filename \b LPCWSTR, File name of an HTML file.
 * \return \b BOOL, \c TRUE if the text was parsed and loaded successfully, \c FALSE otherwise.
 **/
EXTERN_C BOOL SCAPI     SciterLoadFile(HWND hWndSciter, LPCWSTR filename);

/**Load HTML from in memory buffer with base.
 *
 * \param[in] hWndSciter \b HWND, Sciter window handle.
 * \param[in] html \b LPCBYTE, Address of HTML to load.
 * \param[in] htmlSize \b UINT, Length of the array pointed by html parameter.
 * \param[in] baseUrl \b LPCWSTR, base URL. All relative links will be resolved against
 *                                this URL.
 * \return \b BOOL, \c TRUE if the text was parsed and loaded successfully, FALSE otherwise.
 **/
EXTERN_C BOOL SCAPI     SciterLoadHtml(HWND hWndSciter, LPCBYTE html, UINT htmlSize, LPCWSTR baseUrl);

/**Set \link #SCITER_NOTIFY() notification callback function \endlink.
 *
 * \param[in] hWndSciter \b HWND, Sciter window handle.
 * \param[in] cb \b SCITER_NOTIFY*, \link #SCITER_NOTIFY() callback function \endlink.
 * \param[in] cbParam \b LPVOID, parameter that will be passed to \link #SCITER_NOTIFY() callback function \endlink as vParam paramter.
 **/
EXTERN_C VOID SCAPI     SciterSetCallback(HWND hWndSciter, LPSciterHostCallback cb, LPVOID cbParam);

/**Set Master style sheet.
  See: http://www.terrainformatica.com/sciter/master_ss.css.txt
  Or resource section of the library for "master-css" HTML resource.
 *
 * \param[in] utf8 \b LPCBYTE, start of CSS buffer.
 * \param[in] numBytes \b UINT, number of bytes in utf8.
 **/

EXTERN_C BOOL SCAPI     SciterSetMasterCSS(LPCBYTE utf8, UINT numBytes);

/**Append Master style sheet.
  This function appends intrinsic style sheet by custom styles.
  See: http://www.terrainformatica.com/wiki/h-smile:built-in-behaviors:master_style_sheet
 *
 * \param[in] utf8 \b LPCBYTE, start of CSS buffer.
 * \param[in] numBytes \b UINT, number of bytes in utf8.
 *
 **/

EXTERN_C BOOL SCAPI     SciterAppendMasterCSS(LPCBYTE utf8, UINT numBytes);

/**Set (reset) style sheet of current document.
 Will reset styles for all elements according to given CSS (utf8)
 *
 * \param[in] hWndSciter \b HWND, Sciter window handle.
 * \param[in] utf8 \b LPCBYTE, start of CSS buffer.
 * \param[in] numBytes \b UINT, number of bytes in utf8.
 **/

EXTERN_C BOOL SCAPI     SciterSetCSS(HWND hWndSciter, LPCBYTE utf8, UINT numBytes, LPCWSTR baseUrl, LPCWSTR mediaType);

/**Set media type of this sciter instance.
 *
 * \param[in] hWndSciter \b HWND, Sciter window handle.
 * \param[in] mediaType \b LPCWSTR, media type name.
 *
 * For example media type can be "handheld", "projection", "screen", "screen-hires", etc.
 * By default sciter window has "screen" media type.
 *
 * Media type name is used while loading and parsing style sheets in the engine so
 * you should call this function *before* loading document in it.
 *
 **/

EXTERN_C BOOL SCAPI     SciterSetMediaType(HWND hWndSciter, LPCWSTR mediaType);

/**Set media variables of this sciter instance.
 *
 * \param[in] hWndSciter \b HWND, Sciter window handle.
 * \param[in] mediaVars \b SCITER_VALUE, map that contains name/value pairs - media variables to be set.
 *
 * For example media type can be "handheld:true", "projection:true", "screen:true", etc.
 * By default sciter window has "screen:true" and "desktop:true"/"handheld:true" media variables.
 *
 * Media variables can be changed in runtime. This will cause styles of the document to be reset.
 *
 **/

EXTERN_C BOOL SCAPI     SciterSetMediaVars(HWND hWndSciter, const SCITER_VALUE *mediaVars);

EXTERN_C UINT SCAPI     SciterGetMinWidth(HWND hWndSciter);
EXTERN_C UINT SCAPI     SciterGetMinHeight(HWND hWndSciter, UINT width);

EXTERN_C BOOL SCAPI     SciterCall(HWND hWnd, LPCSTR functionName, UINT argc, const SCITER_VALUE* argv, SCITER_VALUE* retval);
// evalue script in context of current document
EXTERN_C BOOL SCAPI     SciterEval( HWND hwnd, LPCWSTR script, UINT scriptLength, SCITER_VALUE* pretval);

/**Update pending changes in Sciter window.
 *
 * \param[in] hwnd \b HWND, Sciter window handle.
 *
 **/
EXTERN_C VOID SCAPI     SciterUpdateWindow(HWND hwnd);

/** Try to translate message that sciter window is interested in.
 *
 * \param[in,out] lpMsg \b MSG*, address of message structure that was passed before to ::DispatchMessage(), ::PeekMessage().
 *
 * SciterTranslateMessage has the same meaning as ::TranslateMessage() and should be called immediately before it.
 * Example:
 *
 *   if( !SciterTranslateMessage(&msg) ) 
 *      TranslateMessage(&msg);         
 *
 * ATTENTION!: SciterTranslateMessage call is critical for popup elements in MoSciter.
 *             On Desktop versions of the Sciter this function does nothing so can be ommited.
 *
 **/

EXTERN_C BOOL SCAPI SciterTranslateMessage(MSG* lpMsg);

/**Set various options.
 *
 * \param[in] hWnd \b HWND, Sciter window handle.
 * \param[in] option \b UINT, id of the option, one of SCITER_RT_OPTIONS
 * \param[in] option \b UINT, value of the option.
 *
 **/

enum SCITER_RT_OPTIONS
{
   SCITER_SMOOTH_SCROLL = 1,      // value:TRUE - enable, value:FALSE - disable, enabled by default
   SCITER_CONNECTION_TIMEOUT = 2, // value: milliseconds, connection timeout of http client
   SCITER_HTTPS_ERROR = 3,        // value: 0 - drop connection, 1 - use builtin dialog, 2 - accept connection silently
   SCITER_FONT_SMOOTHING = 4,     // value: 0 - system default, 1 - no smoothing, 2 - std smoothing, 3 - clear type

   SCITER_TRANSPARENT_WINDOW = 6, // Windows Aero support, value: 
                                  // 0 - normal drawing, 
                                  // 1 - window has transparent background after calls DwmExtendFrameIntoClientArea() or DwmEnableBlurBehindWindow().
   SCITER_SET_GPU_BLACKLIST  = 7  // hWnd = NULL,
                                  // value = LPCBYTE, json - GPU black list, see: gpu-blacklist.json resource.
};

EXTERN_C BOOL SCAPI SciterSetOption(HWND hWnd, UINT option, UINT_PTR value );

/**Render document to ID2D1RenderTarget 
 *
 * \param[in] hWndSciter \b HWND, HTMLayout window handle.
 * \param[in] ID2D1RenderTarget \b prt, Direct2D render target.
 * \return \b BOOL, \c TRUE if hBmp is 24bpp or 32bpp, FALSE otherwise.
 *
 **/

EXTERN_C BOOL SCAPI SciterRenderD2D(HWND hWndSciter, ID2D1RenderTarget* prt);


/** Obtain pointer to ID2D1Factory instance used by the engine:
 *
 * \param[in] ID2D1Factory \b **ppf, address of variable receiving pointer of ID2D1Factory.
 * \return \b BOOL, \c TRUE if parameters are valid and *ppf was set by valid pointer.
 *
 * NOTE 1: ID2D1Factory returned by the function is "add-refed" - it is your responsibility to call Release() on it. 
 * NOTE 2: *ppf variable shall be initialized to NULL before calling the function. 
 *
 **/

EXTERN_C BOOL SCAPI     SciterD2DFactory(ID2D1Factory ** ppf);

/** Obtain pointer to IDWriteFactory instance used by the engine:
 *
 * \param[in] IDWriteFactory \b **ppf, address of variable receiving pointer of IDWriteFactory.
 * \return \b BOOL, \c TRUE if parameters are valid and *ppf was set by valid pointer.
 *
 * NOTE 1: IDWriteFactory returned by the function is "add-refed" - it is your responsibility to call Release() on it. 
 * NOTE 2: *ppf variable shall be initialized to NULL before calling the function. 
 *
 **/

EXTERN_C BOOL SCAPI     SciterDWFactory(IDWriteFactory ** ppf);

/** Get graphics capabilities of the system
 *
 * \pcaps[in] LPUINT \b pcaps, address of variable receiving: 
 *                             0 - no compatible graphics found;
 *                             1 - compatible graphics found but Direct2D will use WARP driver (software emulation);
 *                             2 - Direct2D will use hardware backend (best performance);
 * \return \b BOOL, \c TRUE if pcaps is valid pointer.
 *
 **/

EXTERN_C BOOL SCAPI     SciterGraphicsCaps(LPUINT pcaps);


/** Set sciter home url.
 *  home url is used for resolving sciter: urls
 *  If you will set it like SciterSetHomeURL(hwnd,"http://sciter.com/modules/")
 *  then <script src="sciter:lib/root-extender.tis"> will load
 *  root-extender.tis from http://sciter.com/modules/lib/root-extender.tis
 *
 * \param[in] hWndSciter \b HWND, Sciter window handle.
 * \param[in] baseUrl \b LPCWSTR, URL of sciter home.
 *
 **/

EXTERN_C BOOL SCAPI     SciterSetHomeURL(HWND hWndSciter, LPCWSTR baseUrl);

/** SciterSetupDebugOutput - setup debug output function.
 *
 *  This output function will be used for reprting problems
 *  found while loading html and css documents.
 *
 **/

enum OUTPUT_SUBSYTEMS
{
   OT_DOM = 0,       // html parser & runtime
   OT_CSSS,          // csss! parser & runtime
   OT_CSS,           // css parser
   OT_TIS,           // TIS parser & runtime
};
enum OUTPUT_SEVERITY
{
  OS_INFO,
  OS_WARNING,  
  OS_ERROR,
};

typedef VOID (CALLBACK* DEBUG_OUTPUT_PROC)(LPVOID param, UINT subsystem /*OUTPUT_SUBSYTEMS*/, UINT severity, LPCWSTR text, UINT text_length);

EXTERN_C VOID SCAPI SciterSetupDebugOutput(
                HWND                  hwndOrNull,// HWND or null if this is global output handler
                LPVOID                param,     // param to be passed "as is" to the pfOutput
                DEBUG_OUTPUT_PROC     pfOutput   // output function, output stream alike thing.
                );

typedef VOID (WINAPI* SciterInspectorPF)(HELEMENT root); // the inspector entry function
typedef VOID (WINAPI* SciterWindowInspectorPF)(HWND hwnd); // the inspector entry function

#if !defined(SCITER_EXTENDER_MODULE)

#if defined(__cplusplus) && !defined( PLAIN_API_ONLY )

  namespace sciter
  {

    struct debug_output
    {
      debug_output(HWND hwnd = 0)
      {
        setup_on(hwnd);
      }

      void setup_on(HWND hwnd = 0)
      {
        ::SciterSetupDebugOutput(hwnd,this,_output_debug);
      }

      static VOID CALLBACK _output_debug(LPVOID param, UINT subsystem, UINT severity, LPCWSTR text, UINT text_length)
      {
        static_cast<debug_output*>(param)->output((OUTPUT_SUBSYTEMS)subsystem,(OUTPUT_SEVERITY)severity, text,text_length);
      }
      virtual void output( OUTPUT_SUBSYTEMS subsystem, OUTPUT_SEVERITY severity, const wchar_t* text, unsigned int text_length )
      {
        switch(severity)
        {
           case OS_INFO     : print("info:"); break;
           case OS_WARNING  : print("warning:"); break;
           case OS_ERROR    : print("error:"); break;
        } 
        switch(subsystem)
        {
           case OT_DOM: print("DOM:"); break;
           case OT_CSSS: print("csss!:"); break;
           case OT_CSS:  print("css:"); break;
           case OT_TIS:  print("script:"); break;
        }
        if(text[text_length])
        {
          unsigned n = wcslen(text);
          assert(false);
        }
        else 
          print(text);
      }
      virtual void print(const wchar_t* text) { OutputDebugStringW(text); }
      virtual void print(const char* text) { OutputDebugStringA(text); }
      void printf( const char* fmt, ... )
      {
        char buffer [ 2049 ];
        va_list args;
        va_start ( args, fmt );
        int len = _vsnprintf( buffer, 2048, fmt, args );
        va_end ( args );
        buffer [ len ] = 0;
        buffer [ 2048 ] = 0;
        print(buffer);
      }

    };

    struct debug_output_console: public debug_output
    {

#ifdef _WIN32_WCE
      FILE *f;
      ~debug_output_console()
      {
        fclose(f);
      }
#endif
      debug_output_console():debug_output()
      {
#ifdef _WINCE32_WCE
        f = 0;
#endif
      }
#if !defined(_WIN32_WCE)
      virtual void print(const wchar_t* text) /*override*/
      {
        print(aux::w2oem(text));
      }
      virtual void print(const char* text) /*override*/
      {
        static bool initialized = false;
        if(!initialized)
        {
          AllocConsole();
          freopen("conin$", "r", stdin);
          freopen("conout$", "w", stdout);
          freopen("conout$", "w", stderr);
          initialized = true;
        }
        while(text && *text)
          putchar(*text++);
      }
#else
      virtual void print(const wchar_t* text) /*override*/
      {
        if( !f )
          f = fopen("\\mosciter.log", "wb");
        fputws(text, f);
      }
      virtual void print(const char* text) /*override*/
      {
        if( !f )
          f = fopen("\\mosciter.log", "wb");
        fputs(text, f);
      }
#endif
    };

    // standard implementation of SCN_ATTACH_BEHAVIOR notification
    inline bool create_behavior( LPSCN_ATTACH_BEHAVIOR lpab )
    {
      event_handler *pb = behavior_factory::create(lpab->behaviorName, lpab->element);
      if(pb)
      {
        lpab->elementTag  = pb;
        lpab->elementProc = event_handler::element_proc;
        return true;
      }
      return false;
    }
        

    inline bool inspect(HELEMENT root)
    {
      static SciterInspectorPF pSciterInspect = 0;

      if(!pSciterInspect) {
        HMODULE hinsp = LoadLibrary(
          #if defined(_WIN64)
            TEXT("inspector64.dll")
          #else 
            TEXT("inspector32.dll")
          #endif
            );
        if(!hinsp) 
          return false;
        pSciterInspect = (SciterInspectorPF)GetProcAddress(hinsp,"SciterInspector");
      }
      if(pSciterInspect)
      {
        pSciterInspect(root);
        return true;
      }
      return false;
    }
    inline bool inspect(HWND hwndView)
    {
      static SciterWindowInspectorPF pSciterInspect = 0;

      if(!pSciterInspect) {
        HMODULE hinsp = LoadLibrary(
          #if defined(_WIN64)
            TEXT("inspector64.dll")
          #else 
            TEXT("inspector32.dll")
          #endif
            );
        if(!hinsp) 
          return false;
        pSciterInspect = (SciterWindowInspectorPF)GetProcAddress(hinsp,"SciterWindowInspector");
      }
      if(pSciterInspect)
      {
        pSciterInspect(hwndView);
        return true;
      }
      return false;
    }


  }

#endif

// link against xsciter library

#if !defined(DO_NOT_USE_EXPORT_LIB)
  #if defined(UNDER_CE)
    #pragma comment(lib, "mosciter.lib")
  #elif defined(_WIN64)
    #pragma comment(lib, "sciter64.lib")
  #else 
    #pragma comment(lib, "sciter32.lib")
  #endif
#endif

#endif // defined(SCITER_EXTENDER_MODULE)

#endif

