///////////////////////////////////////////////////////////////////////
// DiscMaster.h
//
// Wrapper for IDiscMaster2 Interface
//
// Written by Eric Haddan
//
#pragma once

class CDiscMaster
{
private:
    IDiscMaster2*	m_discMaster;
	HRESULT			m_hResult;
	CString			m_errorMessage;

public:
	CDiscMaster(void);
	~CDiscMaster(void);

	inline HRESULT GetHresult() {return m_hResult;}
	inline CString GetErrorMessage() {return m_errorMessage;}

	bool Initialize();
	long GetTotalDevices();
	CString GetDeviceUniqueID(long index);
};
