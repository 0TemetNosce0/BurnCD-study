///////////////////////////////////////////////////////////////////////
// DiscMaster.cpp
//
// Wrapper for IDiscMaster2 Interface
//
// Written by Eric Haddan
//
#include "StdAfx.h"
#include "DiscMaster.h"

CDiscMaster::CDiscMaster(void)
: m_discMaster(NULL)
, m_hResult(0)
{
}

CDiscMaster::~CDiscMaster(void)
{
	if (m_discMaster)
		m_discMaster->Release();
}

///////////////////////////////////////////////////////////////////////
//
// CDiscMaster::Initialize()
//
// Description:
//		Creates and initializes the IDiscMaster2 interface
//
bool CDiscMaster::Initialize()
{
	ASSERT(m_discMaster == NULL);

	//
	// Initialize the IDiscMaster2 Interface
	//
	if (m_discMaster == NULL)
	{
		m_hResult = CoCreateInstance(__uuidof(MsftDiscMaster2), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IDiscMaster2), (void**)&m_discMaster);
		if (!SUCCEEDED(m_hResult))
		{
			m_errorMessage.Format(_T("Unable to Initialize IDiscMaster2 - Error:0x%08x"), m_hResult);
			return false;
		}
	}

	//
	// Verify that we have some device that uses this interface
	//
	VARIANT_BOOL isSupported = VARIANT_FALSE;
	m_hResult = m_discMaster->get_IsSupportedEnvironment(&isSupported);
	if (!SUCCEEDED(m_hResult))
	{
		m_errorMessage.Format(_T("IDiscMaster2->get_IsSupportedEnvironment failed! - Error:0x%08x"), m_hResult);
		return false;
	}
	if (isSupported == VARIANT_FALSE)
	{
		m_errorMessage = _T("There were no writable devices detected!");
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscMaster::GetTotalDevices()
//
// Description:
//		Returns the total number of installed CD/DVD devices
//
long CDiscMaster::GetTotalDevices()
{
	ASSERT(m_discMaster != NULL);
	if (m_discMaster == NULL)
		return 0;

	long totalDevices = 0;
	m_hResult = m_discMaster->get_Count(&totalDevices);
	if (FAILED(m_hResult))
	{
		m_errorMessage.Format(_T("IDiscMaster2->get_Count failed! - Error:0x%08x"), m_hResult);
		return 0;
	}

	return totalDevices;
}

///////////////////////////////////////////////////////////////////////
//
// CDiscMaster::GetDeviceUniqueID()
//
// Description:
//		Returns the unique id of the device
//
CString CDiscMaster::GetDeviceUniqueID(long index)
{
	ASSERT(m_discMaster != NULL);
	ASSERT(index < GetTotalDevices());

	BSTR	uniqueID = NULL;
	m_hResult = m_discMaster->get_Item(index, &uniqueID);
	if (FAILED(m_hResult))
	{
		m_errorMessage.Format(_T("IDiscMaster2->get_Item(%d) failed! - Error:0x%08x"),
			index, m_hResult);
		return _T("");
	}

	return uniqueID;
}
