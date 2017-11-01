///////////////////////////////////////////////////////////////////////
// DiscFormatDataEvent.h
//
// Wrapper for DDiscFormat2DataEvents Interface Events
//ddiscformat2dataevent接口事件的包装器
// Written by Eric Haddan
//

#pragma once

class CDiscFormatData;

// CDiscFormatDataEvent command target

class CDiscFormatDataEvent : public CCmdTarget
{
	DECLARE_DYNAMIC(CDiscFormatDataEvent)
private:
	LPTYPEINFO  m_ptinfo;           // ITest type information
	DWORD		m_dwCookie;
	LPUNKNOWN	m_pUnkSink;
	LPUNKNOWN	m_pUnkSrc;

	HWND		m_hNotifyWnd;

public:
	CDiscFormatDataEvent();
	virtual ~CDiscFormatDataEvent();

	static CDiscFormatDataEvent* CreateEventSink();

	bool ConnectDiscFormatData(CDiscFormatData*);
	inline void SetHwnd(HWND hWnd){m_hNotifyWnd = hWnd;}

    DECLARE_INTERFACE_MAP()

	//一个类XFormatDataEvents继承DDiscFormat2DataEvents，BEGIN_INTERFACE_PART是个宏
    BEGIN_INTERFACE_PART(FormatDataEvents, DDiscFormat2DataEvents)
		//
		// IDispatch Methods
		//
        STDMETHOD(GetTypeInfoCount)(UINT FAR* pctinfo);
        STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo FAR* FAR* pptinfo);
        STDMETHOD(GetIDsOfNames)(REFIID riid, OLECHAR FAR* FAR* rgszNames,
			UINT cNames, LCID lcid, DISPID FAR* rgdispid);
        STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid,
			WORD wFlags, DISPPARAMS FAR* pdispparams, VARIANT FAR* pvarResult,
			EXCEPINFO FAR* pexcepinfo, UINT FAR* puArgErr);
		//
		// DDiscFormat2DataEvents Methods
		//
		STDMETHOD_(HRESULT, Update)(LPDISPATCH, LPDISPATCH);
    END_INTERFACE_PART(FormatDataEvents)

protected:
	DECLARE_MESSAGE_MAP()
};


