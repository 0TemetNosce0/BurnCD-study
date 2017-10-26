///////////////////////////////////////////////////////////////////////
// DiscRecorder.h
//
// Wrapper for IDiscRecorder2 Interface
//
// Written by Eric Haddan
//
#pragma once
class CDiscRecorder
{
private:
	IDiscRecorder2* m_discRecorder;
	SAFEARRAY*		m_volumePathNames;
	HRESULT			m_hResult;
	CString			m_recorderUniqueId;


public:
	CDiscRecorder(void);
	~CDiscRecorder(void);

	inline HRESULT GetHresult() {return m_hResult;}
	inline IDiscRecorder2* GetInterface() {return m_discRecorder;}
	inline CString GetUniqueId() {return m_recorderUniqueId;}

	bool Initialize(const CString& recorderUniqueId);

	bool AcquireExclusiveAccess(bool force, const CString& clientName);	
	bool ReleaseExclusiveAccess();
	CString ExclusiveAccessOwner();

	bool EjectMedia();
	bool CloseTray();

	bool EnableMcn();
	bool DisableMcn();

	LONG	GetLegacyDeviceNumber();
	CString GetProductID();
	CString GetProductRevision();
	CString GetVendorId();
	CString GetVolumeName();

	ULONG GetTotalVolumePaths();
	CString GetVolumePath(ULONG volumePathIndex);

};
