///////////////////////////////////////////////////////////////////////
// DiscFormatDataEvent.cpp
//
// Wrapper for DDiscFormat2DataEvents Interface Events
//
// Written by Eric Haddan
//

#include "stdafx.h"
#include "DiscFormatDataEvent.h"

#include "DiscFormatData.h"

// CDiscFormatDataEvent

IMPLEMENT_DYNAMIC(CDiscFormatDataEvent, CCmdTarget)

BEGIN_INTERFACE_MAP(CDiscFormatDataEvent, CCmdTarget)
	INTERFACE_PART(CDiscFormatDataEvent, IID_IDispatch, FormatDataEvents)
	INTERFACE_PART(CDiscFormatDataEvent, IID_DDiscFormat2DataEvents, FormatDataEvents)
END_INTERFACE_MAP()

CDiscFormatDataEvent::CDiscFormatDataEvent()
: m_hNotifyWnd(NULL)
, m_ptinfo(NULL)
, m_dwCookie(0)
, m_pUnkSink(0)
, m_pUnkSrc(0)
{
}

CDiscFormatDataEvent::~CDiscFormatDataEvent()
{
	if (m_dwCookie && (m_pUnkSrc != NULL) && (m_pUnkSink != NULL))
	{
		AfxConnectionUnadvise(m_pUnkSrc, IID_DDiscFormat2DataEvents, m_pUnkSink,
			TRUE, m_dwCookie);
	}
}


BEGIN_MESSAGE_MAP(CDiscFormatDataEvent, CCmdTarget)
END_MESSAGE_MAP()



// CDiscFormatDataEvent message handlers

//�ڲ���
ULONG FAR EXPORT CDiscFormatDataEvent::XFormatDataEvents::AddRef()
{
	METHOD_PROLOGUE(CDiscFormatDataEvent, FormatDataEvents)
	return pThis->ExternalAddRef();
}
ULONG FAR EXPORT CDiscFormatDataEvent::XFormatDataEvents::Release()
{
	METHOD_PROLOGUE(CDiscFormatDataEvent, FormatDataEvents)
	return pThis->ExternalRelease();
}
STDMETHODIMP CDiscFormatDataEvent::XFormatDataEvents::QueryInterface(REFIID riid,
												   LPVOID FAR* ppvObj)
{
	METHOD_PROLOGUE(CDiscFormatDataEvent, FormatDataEvents)
	return (HRESULT)pThis->ExternalQueryInterface(&riid, ppvObj);
}
STDMETHODIMP
CDiscFormatDataEvent::XFormatDataEvents::GetTypeInfoCount(UINT FAR* pctinfo)
{
	METHOD_PROLOGUE(CDiscFormatDataEvent, FormatDataEvents)
	*pctinfo = 1;
	return NOERROR;
}
STDMETHODIMP CDiscFormatDataEvent::XFormatDataEvents::GetTypeInfo(
	UINT itinfo,
	LCID lcid,
	ITypeInfo FAR* FAR* pptinfo)
{
	METHOD_PROLOGUE(CDiscFormatDataEvent, FormatDataEvents)
	*pptinfo = NULL;

	if(itinfo != 0)
		return ResultFromScode(DISP_E_BADINDEX);
	pThis->m_ptinfo->AddRef();
	*pptinfo = pThis->m_ptinfo;
	return NOERROR;
}
STDMETHODIMP CDiscFormatDataEvent::XFormatDataEvents::GetIDsOfNames(
	REFIID riid,
	OLECHAR FAR* FAR* rgszNames,
	UINT cNames,
	LCID lcid,
	DISPID FAR* rgdispid)
{
	METHOD_PROLOGUE(CDiscFormatDataEvent, FormatDataEvents)
	return DispGetIDsOfNames(pThis->m_ptinfo, rgszNames, cNames, rgdispid);
}
STDMETHODIMP CDiscFormatDataEvent::XFormatDataEvents::Invoke(
	DISPID dispidMember,
	REFIID riid,
	LCID lcid,
	WORD wFlags,
	DISPPARAMS FAR* pdispparams,
	VARIANT FAR* pvarResult,
	EXCEPINFO FAR* pexcepinfo,
	UINT FAR* puArgErr)
{
	METHOD_PROLOGUE(CDiscFormatDataEvent, FormatDataEvents)
		//return S_OK;
	return DispInvoke(&pThis->m_xFormatDataEvents, pThis->m_ptinfo,
		dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}


///////////////////////////////////////////////////////////////////////
//
// CDiscFormatDataEvent::CreateEventSink
//
// Description:
//			Establishes a link between the CDiscFormatData(IDiscFormat2Data)
//			and the CDiscFormatDataEvent(DDiscFormat2DataEvents) so 
//			CDiscFormatDataEvent can receive Update messages
//
CDiscFormatDataEvent* CDiscFormatDataEvent::CreateEventSink()
{
	// Create the event sink
	CDiscFormatDataEvent* pDiscFormatDataEvent = new CDiscFormatDataEvent();

	pDiscFormatDataEvent->EnableAutomation();
	pDiscFormatDataEvent->ExternalAddRef();

	return pDiscFormatDataEvent;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscFormatDataEvent::ConnectDiscFormatData
//
// Description:
//			Establishes a link between the CDiscFormatData(IDiscFormat2Data)
//			and the CDiscFormatDataEvent(DDiscFormat2DataEvents) so 
//			CDiscFormatDataEvent can receive Update messages
//
//����CDiscFormatData(IDiscFormat2Data)֮�������
//��CDiscFormatDataEvent(DDiscFormat2DataEvents)
//CDiscFormatDataEvent���Խ��ո�����Ϣ


//connect CDiscFormatData, so CDiscFormatDataEvent���Դ�����Ϣ
bool CDiscFormatDataEvent::ConnectDiscFormatData(CDiscFormatData* pDiscFormatData)
{
	////ʹ��IConnectionPointContainer����
	//m_pUnkSink = GetIDispatch(FALSE);
	m_pUnkSrc = pDiscFormatData->GetInterface();//��ȡIDiscFormat2Data

	LPTYPELIB ptlib = NULL;
	HRESULT hr = LoadRegTypeLib(LIBID_IMAPILib2, 
		IMAPILib2_MajorVersion, IMAPILib2_MinorVersion, 
		LOCALE_SYSTEM_DEFAULT, &ptlib);//com�����ʹ�����Ϳ�ĵ�һ����װ������LoadRegTypeLib������ͼ��Windows��ע�����װ��ָ�������Ϳ�
	if (FAILED(hr))
	{
		return false;
	}
	hr = ptlib->GetTypeInfoOfGuid(IID_DDiscFormat2DataEvents, &m_ptinfo);//��ȡ��Ӧ��ָ����GUID��������
		ptlib->Release();
	if (FAILED(hr))
	{
		return false;
	}

	IConnectionPointContainer  *lpConnectionPointContainer = NULL;
	m_pUnkSrc->QueryInterface(IID_IConnectionPointContainer, (void **)&lpConnectionPointContainer);
	if (lpConnectionPointContainer)
	{
		IConnectionPoint    *lpConnectionPoint = NULL;
		lpConnectionPointContainer->FindConnectionPoint(IID_DDiscFormat2DataEvents, &lpConnectionPoint);

		lpConnectionPointContainer->Release();
		lpConnectionPointContainer = NULL;

		if (lpConnectionPoint)
		{
			DWORD  ldwCookie = 0;
			// ��ӵ�COM�е���������  
			lpConnectionPoint->Advise(&m_xFormatDataEvents, &ldwCookie);
			return true;
		}
		return false;
	}
	
	//ʹ��MFC��AfxConnectionAdvise��������
	//m_pUnkSink = GetIDispatch(FALSE);
	//m_pUnkSrc = pDiscFormatData->GetInterface();//��ȡIDiscFormat2Data
	//LPTYPELIB ptlib = NULL;
	//HRESULT hr = LoadRegTypeLib(LIBID_IMAPILib2, 
	//	IMAPILib2_MajorVersion, IMAPILib2_MinorVersion, 
	//	LOCALE_SYSTEM_DEFAULT, &ptlib);//com�����ʹ�����Ϳ�ĵ�һ����װ������LoadRegTypeLib������ͼ��Windows��ע�����װ��ָ�������Ϳ�
	//if (FAILED(hr))
	//{
	//	return false;
	//}
	//hr = ptlib->GetTypeInfoOfGuid(IID_DDiscFormat2DataEvents, &m_ptinfo);//��ȡ��Ӧ��ָ����GUID��������
 //   ptlib->Release();
	//if (FAILED(hr))
	//{
	//    return false;
	//}

	//BOOL bRet = AfxConnectionAdvise(m_pUnkSrc, IID_DDiscFormat2DataEvents, m_pUnkSink,
	//	TRUE, &m_dwCookie);//���ô˺�����pUnkSrcָ����Դ��pUnkSinkָ���Ľ�����֮�佨�����ӡ�

	//if (bRet)
	//{
	//	return true;
	//}

	//return false;
}


///////////////////////////////////////////////////////////////////////
//
// CDiscFormatDataEvent::Update
//
// Description:
//			Receives update notifications from IDiscFormat2Data
//
STDMETHODIMP_(HRESULT) CDiscFormatDataEvent::XFormatDataEvents::Update(IDispatch* objectDispatch, IDispatch* progressDispatch)
{//��¼��ʱ��᲻�ϵĵ���
	METHOD_PROLOGUE(CDiscFormatDataEvent, FormatDataEvents)

    IDiscFormat2DataEventArgs* progress = NULL;
    HRESULT hr = progressDispatch->QueryInterface(IID_PPV_ARGS(&progress));

	IDiscFormat2Data* discFormatData = NULL;
    hr = objectDispatch->QueryInterface(IID_PPV_ARGS(&discFormatData));

    IMAPI_FORMAT2_DATA_WRITE_ACTION currentAction = IMAPI_FORMAT2_DATA_WRITE_ACTION_VALIDATING_MEDIA;
	hr = progress->get_CurrentAction(&currentAction);
	ASSERT(SUCCEEDED(hr));
	if (FAILED(hr))
	{
		return S_OK;
	}

	IMAPI_STATUS imapiStatus = {0};

	if ((currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_WRITING_DATA) ||
		(currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_FINALIZATION))
	{
		progress->get_ElapsedTime(&imapiStatus.elapsedTime);
		progress->get_RemainingTime(&imapiStatus.remainingTime);
		progress->get_TotalTime(&imapiStatus.totalTime);

		if (currentAction == IMAPI_FORMAT2_DATA_WRITE_ACTION_WRITING_DATA)
		{
			progress->get_StartLba(&imapiStatus.startLba);
			progress->get_SectorCount(&imapiStatus.sectorCount);
			progress->get_LastReadLba(&imapiStatus.lastReadLba);
			progress->get_LastWrittenLba(&imapiStatus.lastWrittenLba);
			progress->get_TotalSystemBuffer(&imapiStatus.totalSystemBuffer);
			progress->get_UsedSystemBuffer(&imapiStatus.usedSystemBuffer);
			progress->get_FreeSystemBuffer(&imapiStatus.freeSystemBuffer);
		}
	}

	//������Ϣ����
	LRESULT ret = ::SendMessage(pThis->m_hNotifyWnd, WM_IMAPI_UPDATE, currentAction, (LPARAM)(LPVOID)&imapiStatus);//WM_IMAPI_UPDATE��Ϣ��������ǲ���
	if (ret == RETURN_CANCEL_WRITE)
	{
		discFormatData->CancelWrite();//ȡ��д
	}

	return S_OK;
}
