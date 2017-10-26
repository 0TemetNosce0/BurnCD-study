// BurnCDDlg.h : header file
//

#pragma once

#include "FileListBox.h"

// CBurnCDDlg dialog
class CBurnCDDlg : public CDialog
{
// Construction
public:
	CBurnCDDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_BURNCD_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	bool	m_isCdromSupported;
	bool	m_isDvdSupported;
	bool	m_isDualLayerDvdSupported;
	int		m_selectedMediaType;//选择的媒体类型：比如cd dvd dvd双层
	bool	m_isBurning;
	bool	m_cancelBurn;
	CCriticalSection m_critSection;


	void	SetCancelBurning(bool bCancel);
	bool	GetCancelBurning();


	CString	GetMediaTypeString(int mediaType);
	void	AddRecordersToComboBox();
	void	ClearListControl();
	void	UpdateCapacity();
	void	EnableBurnButton();
	void	EnableUI(BOOL bEnable);

	static UINT __cdecl BurnThread(LPVOID pParam);
	static bool CreateMediaFileSystem(CBurnCDDlg* pThis, IMAPI_MEDIA_PHYSICAL_TYPE mediaType, IStream** dataStream);

	void UpdateTimes(LONG totalTime, LONG remainingTime);
	void UpdateBuffer(LONG usedSystemBuffer, LONG totalSystemBuffer);
	void UpdateProgress(LONG writtenSectors, LONG totalSectors);

    // Generated message map functions
	virtual BOOL OnInitDialog();
	CComboBox m_deviceComboBox;
	afx_msg void OnCbnSelchangeDeviceCombo();
	afx_msg void OnLbnSelchangeBurnFileList();
	afx_msg void OnBnClickedAddFilesButton();
	afx_msg void OnBnClickedAddFolderButton();
	CFileListBox m_fileListbox;
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBurnButton();
	afx_msg LRESULT OnImapiUpdate(WPARAM, LPARAM);
	afx_msg LRESULT OnBurnStatusMessage(WPARAM, LPARAM);
	afx_msg LRESULT OnBurnFinished(WPARAM, LPARAM);
	CStatic m_progressText;
	CStatic m_estimatedTime;
	CStatic m_timeLeft;
	CProgressCtrl m_progressCtrl;
	CStatic m_bufferText;
	CProgressCtrl m_bufferCtrl;
	CStatic m_supportedMediaTypes;
	CProgressCtrl m_capacityProgress;
	CStatic m_maxText;
	CComboBox m_mediaTypeCombo;
	afx_msg void OnCbnSelchangeMediaTypeCombo();
	afx_msg void OnBnClickedRemoveFilesButton();
	CString m_volumeLabel;
	BOOL m_closeMedia;
	BOOL m_ejectWhenFinished;
	DECLARE_MESSAGE_MAP()
};
