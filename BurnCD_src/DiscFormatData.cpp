///////////////////////////////////////////////////////////////////////
// DiscFormatData.cpp
//
// Wrapper for IDiscFormat2Data Interface
//
// Written by Eric Haddan
//

#include "StdAfx.h"
#include "DiscFormatData.h"
#include "DiscRecorder.h"
#include "DiscFormatDataEvent.h"

CDiscFormatData::CDiscFormatData(void)
: m_discFormatData(NULL)
, m_mediaTypesArray(NULL)
, m_hResult(0)
, m_hNotificationWnd(NULL)
, m_closeMedia(true)
{
}

CDiscFormatData::~CDiscFormatData(void)
{
	if (m_discFormatData != NULL)
	{
		m_discFormatData->Release();
	}
}

///////////////////////////////////////////////////////////////////////
//
// CDiscFormatData::Initialize()
//
// Description:
//		Creates and initializes the IDiscFormat2Data interface
//
bool CDiscFormatData::Initialize(CDiscRecorder* pDiscRecorder, const CString& clientName)
{
	ASSERT(m_discFormatData == NULL);
	ASSERT(pDiscRecorder != NULL);
	if (pDiscRecorder == NULL)
	{
		m_errorMessage = _T("Error - CDiscFormatData::Initialize - pDiscRecorder is NULL");
		return false;
	}

	//
	// Initialize the IDiscFormat2Data Interface
	//IDiscFormat2Data使用此接口将数据流写入磁盘。
	m_hResult = CoCreateInstance(__uuidof(MsftDiscFormat2Data), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IDiscFormat2Data), (void**)&m_discFormatData);
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage.Format(_T("Unable to Initialize IDiscFormat2Data - Error:0x%08x"), m_hResult);
		return false;
	}

	//
	// Setup the Disc Format Information
	//
	VARIANT_BOOL isSupported = VARIANT_FALSE;
	m_hResult = m_discFormatData->IsRecorderSupported(pDiscRecorder->GetInterface(), &isSupported);//确定记录器是否支持给定格式。
	if (isSupported == VARIANT_FALSE)
	{
		m_errorMessage = _T("Recorder not supported");
		return false;
	}

	m_hResult = m_discFormatData->put_Recorder(pDiscRecorder->GetInterface());//设置用于写操作的记录设备。
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage.Format(_T("IDiscFormat2Data->put_Recorder Failed - Error:0x%08x"), m_hResult);
		return false;
	}

	m_hResult = m_discFormatData->put_ClientName(clientName.AllocSysString());//设置客户端的友好名称。
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage.Format(_T("IDiscFormat2Data->put_ClientName Failed - Error:0x%08x"), m_hResult);
		return false;
	}

	m_hResult = m_discFormatData->get_SupportedMediaTypes(&m_mediaTypesArray);//检索记录器所支持的媒体类型。
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage.Format(_T("IDiscFormat2Data->get_SupportedMediaTypes Failed - Error:0x%08x"), m_hResult);
		return false;
	}

	return true;
}

ULONG CDiscFormatData::GetTotalSupportedMediaTypes()
{
	if (m_mediaTypesArray == NULL)
		return 0;

	return m_mediaTypesArray->rgsabound[0].cElements;
}

int CDiscFormatData::GetSupportedMediaType(ULONG index)
{
	ASSERT(index < GetTotalSupportedMediaTypes());
	if (index < GetTotalSupportedMediaTypes())
	{
		if (m_mediaTypesArray)
		{
			return ((VARIANT*)(m_mediaTypesArray->pvData))[index].intVal;
		}
	}

	return 0;
}

//烧录
bool CDiscFormatData::Burn(HWND hNotificationWnd, IStream* streamData)
{
	if (m_discFormatData == NULL)
		return false;

	if (hNotificationWnd == NULL)
		return false;

	if (streamData == NULL)
		return false;

	m_streamData = streamData;
	m_hNotificationWnd = hNotificationWnd;

	// Create the event sink
	CDiscFormatDataEvent* eventSink = CDiscFormatDataEvent::CreateEventSink();
	if (eventSink == NULL)
	{
		m_errorMessage = _T("Unable to create event sink");
		return false;
	}

	if (!eventSink->ConnectDiscFormatData(this))//connect
	{
		m_errorMessage = _T("Unable to connect event sink with interface");
		return false;
	}

	eventSink->SetHwnd(m_hNotificationWnd);

	m_discFormatData->put_ForceMediaToBeClosed(m_closeMedia ? VARIANT_TRUE : VARIANT_FALSE);//设置用于写操作的记录设备。


	m_hResult = m_discFormatData->Write(m_streamData);//写，一直在写，直到完成。

	delete eventSink;
	//m_errorMessage.Format(_T("IDiscFormat2Data11111111111111111111111111"));
	if (SUCCEEDED(m_hResult))
	{
		return true;
	}

	m_errorMessage.Format(_T("IDiscFormat2Data->Write Failed! Error:0x%08x"),
		m_hResult);

	return true;

}

