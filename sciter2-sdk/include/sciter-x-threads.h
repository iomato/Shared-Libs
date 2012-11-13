#ifndef __SCITER_THREADS_H__
#define __SCITER_THREADS_H__

/*
 * The Sciter Engine
 * http://terrainformatica.com/sciter
 * 
 * Asynchronous GUI Task Queue.
 * Use these primitives when you need to run code in GUI thread.
 * 
 * The code and information provided "as-is" without
 * warranty of any kind, either expressed or implied.
 * 
 * 
 * (C) 2003-2006, Andrew Fedoniouk (andrew@terrainformatica.com)
 */

/*!\file
\brief Asynchronous GUI primitives
*/
#include <windows.h>
#include <stdio.h>
#include <assert.h>

namespace sciter { 
  
  namespace sync { 

    class mutex 
    { 
      CRITICAL_SECTION cs;
   public:
      void lock()     { EnterCriticalSection(&cs); } 
      void unlock()   { LeaveCriticalSection(&cs); } 
      mutex()         { InitializeCriticalSection(&cs); }   
      ~mutex()        { DeleteCriticalSection(&cs); }
    };
  
    class critical_section { 
      mutex& m;
    public:
      critical_section(mutex& guard) : m(guard) { m.lock(); }
      ~critical_section() { m.unlock(); }
    };

    struct event
    {
      HANDLE h;
      event()       { h = CreateEvent(NULL, FALSE, FALSE, NULL);  }
      ~event()      { CloseHandle(h); }
      void signal()                   { SetEvent(h); }
      bool wait(uint ms = INFINITE)   { return WaitForSingleObject(h, ms) == WAIT_OBJECT_0; }
      
    };

    inline void yield() { Sleep(0); }

  }

  template<typename F, typename P>
  class thread_ctx
  {
    F _f;
    P _p;
    static DWORD WINAPI ThreadProc(LPVOID lpData)
    {
        thread_ctx* self = reinterpret_cast<thread_ctx*>(lpData);
        try { 
          self->_f(self->_p);
        }
        catch(...) { assert(false); }
        delete self;
        return 0;
    }
    thread_ctx(const F& f, const P& p): _f(f),_p(p) {

      DWORD dwThreadID;
		  HANDLE hThread = ::CreateThread(NULL, 0, ThreadProc, this, 0, &dwThreadID);
		  assert(hThread != NULL); hThread;
    }
  public:
    static void start( const F& f, const P& p )
    {
      new thread_ctx<F,P>(f,p);
    }
  };

  template<typename F, typename P>
    inline void thread( F f, P p )
    {
      thread_ctx<F,P>::start(f,p);
    }

}

#endif // __SCITER_THREADS_H__
