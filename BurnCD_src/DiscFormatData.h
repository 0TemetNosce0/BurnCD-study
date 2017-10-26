///////////////////////////////////////////////////////////////////////
// DiscFormatData.h
//
// Wrapper for IDiscFormat2Data Interface
//
// Written by Eric Haddan
//
#pragma once


#define WM_IMAPI_UPDATE		WM_APP+842

typedef struct _IMAPI_STATUS {
	// IDiscFormat2DataEventArgs Interface
    LONG elapsedTime;		// Elapsed time in seconds
    LONG remainingTime;		// Remaining time in seconds
    LONG totalTime;			// total estimated time in seconds
	// IWriteEngine2EventArgs Interface
    LONG startLba;			// the starting lba of the current operation
    LONG sectorCount;		// the total sectors to write in the current operation
    LONG lastReadLba;		// the last read lba address
    LONG lastWrittenLba;	// the last written lba address
    LONG totalSystemBuffer;	// total size of the system buffer
    LONG usedSystemBuffer;	// size of used system buffer
    LONG freeSystemBuffer;	// size of the free system buffer
} IMAPI_STATUS, *PIMAPI_STATUS;

#define RETURN_CANCEL_WRITE	0
#define RETURN_CONTINUE		1


class CDiscRecorder;

class CDiscFormatData
{
private:
	IDiscFormat2Data*	m_discFormatData;
	SAFEARRAY*			m_mediaTypesArray;//所支持的媒体类型
	HRESULT				m_hResult;
	CString				m_errorMessage;
	HWND				m_hNotificationWnd;
	bool				m_closeMedia;


public:
	CDiscFormatData(void);
	~CDiscFormatData(void);

	inline	HRESULT GetHresult() {return m_hResult;}
	inline	CString GetErrorMessage() {return m_errorMessage;}

	inline	IDiscFormat2Data* GetInterface() {return m_discFormatData;}
	bool	Initialize(CDiscRecorder* pDiscRecorder, const CString& clientName);

	bool	Burn(HWND hWnd, IStream* data);

	ULONG	GetTotalSupportedMediaTypes();
	int		GetSupportedMediaType(ULONG index);

	inline	void SetCloseMedia(bool closeMedia){m_closeMedia = closeMedia;}

protected:
	IStream*	m_streamData;
};
