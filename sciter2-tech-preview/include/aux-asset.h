#pragma once

#if defined(_WINDOWS)

  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  //#include <assert.h>

#endif

namespace aux { 
  
  namespace atomic {

    #if defined(_WINDOWS)

      typedef volatile long counter_t;
      inline long _inc(counter_t& v)               {    return InterlockedIncrement((LPLONG)&v);  }
      inline long _dec(counter_t& v)               {    return InterlockedDecrement((LPLONG)&v);  }
      inline long _set(counter_t& v, long nv)      {    return InterlockedExchange((LPLONG)&v, nv);  }

    #else

      typedef long counter_t;
      inline long _inc(counter_t& v)               {    return ++v;  }
      inline long _dec(counter_t& v)               {    return --v;  }
      inline long _set(counter_t& v, long nv)      {    long t = v; v = nv;  return t;  }

    #endif

      struct counter
      {
        counter_t cv;
        counter():cv(0) {}
        counter(long iv):cv(iv) {}
        counter& operator=(long nv) { _set(cv,nv); return *this; }
        operator long() const { return cv; }
        long operator++() { return _inc(cv); } 
        long operator--() { return _dec(cv); } 
      };
  } // atomic

  // intrusive add_ref/release counter
  class asset
  {
    atomic::counter _ref_cntr;
  public:
    asset ():_ref_cntr(0) {}
    asset (const asset&/*r*/):_ref_cntr(0) {}

    virtual ~asset ()
    {
      assert ( _ref_cntr == 0 );
    }
    
    virtual atomic::counter_t  release()
    {
        assert(_ref_cntr > 0);
        long t = --_ref_cntr;
        if(t == 0)
          finalize();
        return t;
    }
    virtual atomic::counter_t  add_ref() { return ++_ref_cntr; }

    virtual void finalize() 
    {  
      delete this;
    }
  };

  //asset - yet another shared_ptr
  //        R here is something derived from the asset above  
  template <class R>
  class asset_ptr
  {
  protected:
	  R* p;

  public:
	  typedef R asset_t;

	  asset_ptr():p(0)                        {}
	  asset_ptr(R* lp):p(0)                   { if (lp) (p = lp)->add_ref();	}
    asset_ptr(const asset_ptr<R>& cp):p(0)  { if (cp.p) (p = cp.p)->add_ref();	}

	  ~asset_ptr()                      { if (p)	p->release();}
	  operator R*() const	          {	return p;	}
    R* operator->() const         { assert(p != 0); return p; }

    bool operator!() const        {	return p == 0;	}
    operator bool() const         { return  p != 0;	}
	  bool operator!=(R* pR) const  {	return p != pR;	}
	  bool operator==(R* pR) const  {	return p == pR;	}

	  // release the interface and set it to NULL
	  void release()                {	if (p)	{ R* pt = p; p = 0; pt->release(); }}
	
    // attach to an existing interface (does not AddRef)
	  void attach(R* p2)            { release();	p = p2;	}
	  // detach the interface (does not Release)
	  R* detach()                   {	R* pt = p; p = 0;	return pt; }

    static R* assign(R* &pp, R* lp)
    {
	    if (lp != 0) lp->add_ref();
	    if (pp) pp->release();
	    pp = lp;
	    return lp;
    }

    R* operator=(R* lp)                   { if(p != lp) return assign(p, lp); return p;	}
    R* operator=(const asset_ptr<R>& lp)	{ if(p != lp) return assign(p, lp.p); return p;	}	

  };




}