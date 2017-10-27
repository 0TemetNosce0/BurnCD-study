// BurnCDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "BurnCD.h"
#include "BurnCDDlg.h"

#include "DiscMaster.h"
#include "FileObject.h"
#include "DirObject.h"
#include "DiscFormatData.h"
#include "DiscRecorder.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CD_MEDIA		0
#define DVD_MEDIA		1
#define DL_DVD_MEDIA	2

#define WM_BURN_STATUS_MESSAGE	WM_APP+300
#define WM_BURN_FINISHED		WM_APP+301

#define CLIENT_NAME		_T("BurnCD")


// CBurnCDDlg dialog



//������
CBurnCDDlg::CBurnCDDlg(CWnd* pParent /*=NULL*/)
: CDialog(CBurnCDDlg::IDD, pParent)
, m_closeMedia(TRUE)
, m_cancelBurn(false)
, m_selectedMediaType(-1)
, m_isBurning(false)
, m_ejectWhenFinished(TRUE)
{
    m_volumeLabel = CTime::GetCurrentTime().Format(_T("%Y_%m_%d"));
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CBurnCDDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_DEVICE_COMBO, m_deviceComboBox);
    DDX_Control(pDX, IDC_BURN_FILE_LIST, m_fileListbox);
    DDX_Control(pDX, IDC_PROGRESS_TEXT, m_progressText);
    DDX_Control(pDX, IDC_ESTIMATED_TIME, m_estimatedTime);
    DDX_Control(pDX, IDC_TIME_LEFT, m_timeLeft);
    DDX_Control(pDX, IDC_PROGRESS, m_progressCtrl);
    DDX_Control(pDX, IDC_BUFFER_TEXT, m_bufferText);
    DDX_Control(pDX, IDC_BUFFER_PROG, m_bufferCtrl);
    DDX_Control(pDX, IDC_SUPPORTED_MEDIA_TYPES, m_supportedMediaTypes);
    DDX_Control(pDX, IDC_CAPACITY, m_capacityProgress);
    DDX_Control(pDX, IDC_MAX_TEXT, m_maxText);
    DDX_Control(pDX, IDC_MEDIA_TYPE_COMBO, m_mediaTypeCombo);
    DDX_Text(pDX, IDC_VOLUME, m_volumeLabel);
    DDX_Check(pDX, IDC_CLOSE_MEDIA_CHK, m_closeMedia);
    DDX_Check(pDX, IDC_EJECT_WHEN_FINISHED, m_ejectWhenFinished);
}

BEGIN_MESSAGE_MAP(CBurnCDDlg, CDialog)
    ON_CBN_SELCHANGE(IDC_DEVICE_COMBO, &CBurnCDDlg::OnCbnSelchangeDeviceCombo)// device combo
    ON_LBN_SELCHANGE(IDC_BURN_FILE_LIST, &CBurnCDDlg::OnLbnSelchangeBurnFileList)
    ON_BN_CLICKED(IDC_ADD_FILES_BUTTON, &CBurnCDDlg::OnBnClickedAddFilesButton)//����ļ���ť
    ON_BN_CLICKED(IDC_ADD_FOLDER_BUTTON, &CBurnCDDlg::OnBnClickedAddFolderButton)
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BURN_BUTTON, &CBurnCDDlg::OnBnClickedBurnButton)
    ON_MESSAGE(WM_IMAPI_UPDATE, OnImapiUpdate)
    ON_MESSAGE(WM_BURN_STATUS_MESSAGE, OnBurnStatusMessage)
    ON_MESSAGE(WM_BURN_FINISHED, OnBurnFinished)
    ON_CBN_SELCHANGE(IDC_MEDIA_TYPE_COMBO, &CBurnCDDlg::OnCbnSelchangeMediaTypeCombo)
    ON_BN_CLICKED(IDC_REMOVE_FILES_BUTTON, &CBurnCDDlg::OnBnClickedRemoveFilesButton)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


// CBurnCDDlg message handlers

BOOL CBurnCDDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_progressText.SetWindowText(_T(""));
    m_bufferText.SetWindowText(_T(""));
    m_supportedMediaTypes.SetWindowText(_T(""));
    m_maxText.SetWindowText(_T(""));
    m_capacityProgress.SetRange(0,100);

    AddRecordersToComboBox();//����豸����Ͽ�
    OnLbnSelchangeBurnFileList();
    EnableBurnButton();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon

    return TRUE;  // return TRUE  unless you set the focus to a control
}

//����豸����Ͽ�
void CBurnCDDlg::AddRecordersToComboBox()
{
    CDiscMaster			discMaster;

    //
    // Cleanup old data on combobox
    //
	//�����ǰ������
    int itemCount = m_deviceComboBox.GetCount();
    for (int itemIndex = 0; itemIndex < itemCount; itemIndex++)
    {
        delete (CDiscRecorder*)m_deviceComboBox.GetItemDataPtr(itemIndex);
    }
    m_deviceComboBox.ResetContent();


    if (!discMaster.Initialize())
    {
        AfxMessageBox(discMaster.GetErrorMessage(), MB_OK|MB_ICONERROR);
        EndDialog(IDOK);
        return;
    }

    //
    // Add Devices to ComboBox
    //
    long totalDevices = discMaster.GetTotalDevices();
    if (totalDevices == 0 && FAILED(discMaster.GetHresult()))
    {
        AfxMessageBox(discMaster.GetErrorMessage(), MB_OK|MB_ICONERROR);
    }

    for (long deviceIndex = 0; deviceIndex < totalDevices; deviceIndex++)
    {
        CString recorderUniqueID = discMaster.GetDeviceUniqueID(deviceIndex);
        if (recorderUniqueID.IsEmpty())
        {
            CString errorMessage(discMaster.GetErrorMessage());
            if (!errorMessage.IsEmpty())
            {
                AfxMessageBox(errorMessage, MB_OK|MB_ICONERROR);
                continue;
            }
        }

        //
        // Create an IDiscRecorder2
        //
        CDiscRecorder* pDiscRecorder = new CDiscRecorder();
        ASSERT(pDiscRecorder != NULL);
        if (pDiscRecorder == NULL)
            continue;

        if (!pDiscRecorder->Initialize(recorderUniqueID))//��¼��ʼ��
        {
            if (totalDevices == 1 && FAILED(pDiscRecorder->GetHresult()))//����
            {
                CString errorMessage;
                errorMessage.Format(_T("Failed to initialize recorder - Error:0x%08x\n\nRecorder Unique ID:%s"),
                    pDiscRecorder->GetHresult(), (LPCTSTR)recorderUniqueID);
                AfxMessageBox(errorMessage, MB_OK|MB_ICONERROR);
            }
            delete pDiscRecorder;
            continue;
        }

        //
        // Get the volume path(s). usually just 1
        //
        CString volumeList;
        ULONG totalVolumePaths = pDiscRecorder->GetTotalVolumePaths();
        for (ULONG volIndex = 0; volIndex < totalVolumePaths; volIndex++)
        {
            if (volIndex)
                volumeList += _T(",");
            volumeList += pDiscRecorder->GetVolumePath(volIndex);
        }

        //
        // Add Drive to combo and IDiscRecorder as data
        //
		CString productId = pDiscRecorder->GetProductID();//L"xxxxxxxxx";//�豸�Ĺ�Ӧ��ID��
        CString strName;
        strName.Format(_T("%s [%s]"), (LPCTSTR)volumeList, (LPCTSTR)productId);
        int comboBoxIndex = m_deviceComboBox.AddString(strName);
        m_deviceComboBox.SetItemDataPtr(comboBoxIndex, pDiscRecorder);//��ӵ���ʾ�б�
    }

    if (totalDevices > 0)
    {
        m_deviceComboBox.SetCurSel(0);
        OnCbnSelchangeDeviceCombo();
    }
}

//
// CBurnCDDlg::OnCbnSelchangeDeviceCombo
//
// Selected a New Device
//
void CBurnCDDlg::OnCbnSelchangeDeviceCombo()//ѡ������ı�����
{
    m_isCdromSupported = false;
    m_isDvdSupported = false;
    m_isDualLayerDvdSupported = false;

    m_mediaTypeCombo.ResetContent();

    int selectedIndex = m_deviceComboBox.GetCurSel();
    ASSERT(selectedIndex >= 0);
    if (selectedIndex < 0)
    {
        return;
    }

    CDiscRecorder* discRecorder = 
        (CDiscRecorder*)m_deviceComboBox.GetItemDataPtr(selectedIndex);
    if (discRecorder != NULL)
    {
        CDiscFormatData discFormatData;
        if  (!discFormatData.Initialize(discRecorder, CLIENT_NAME))
        {
            return;
        }

        //
        // Display Supported Media Types
        //
        CString supportedMediaTypes;//֧�ֵ�ý�����ͣ�����cd��dvd��dvd dual-layer˫��ʽ
        ULONG totalMediaTypes = discFormatData.GetTotalSupportedMediaTypes();//��ȡ��֧�ֵĶ�ý�����͸���
        for (ULONG volIndex = 0; volIndex < totalMediaTypes; volIndex++)
        {
            int mediaType = discFormatData.GetSupportedMediaType(volIndex);
            if (volIndex > 0)
                supportedMediaTypes += _T(", ");
            supportedMediaTypes += GetMediaTypeString(mediaType);//���Ͷ�Ӧ���ַ���
        }
        m_supportedMediaTypes.SetWindowText(/*L"xxxxxxxxxxxxx"*/supportedMediaTypes);// text

        //
        // Add Media Selection
        //
        if (m_isCdromSupported)//cd rom ֧��
        {
            int stringIndex = m_mediaTypeCombo.AddString(_T("700MB CD Media"));
            m_mediaTypeCombo.SetItemData(stringIndex, CD_MEDIA);
        }
        if (m_isDvdSupported)//dvd֧��
        {
            int stringIndex = m_mediaTypeCombo.AddString(_T("4.7GB DVD Media"));
            m_mediaTypeCombo.SetItemData(stringIndex, DVD_MEDIA);
        }
        if (m_isDualLayerDvdSupported)//dvd dual-layer֧��
        {
            int stringIndex = m_mediaTypeCombo.AddString(_T("8.5GB Dual-Layer DVD"));
            m_mediaTypeCombo.SetItemData(stringIndex, DL_DVD_MEDIA);
        }
        m_mediaTypeCombo.SetCurSel(0);
        OnCbnSelchangeMediaTypeCombo();
    }
}

//��ȡý������mediaType���ַ���
CString	CBurnCDDlg::GetMediaTypeString(int mediaType)
{
    switch (mediaType)
    {
    case IMAPI_MEDIA_TYPE_UNKNOWN:
    default:
        return _T("Unknown Media Type");

    case IMAPI_MEDIA_TYPE_CDROM:
        m_isCdromSupported = true;
        return _T("CD-ROM or CD-R/RW media");

    case IMAPI_MEDIA_TYPE_CDR:
        m_isCdromSupported = true;
        return _T("CD-R");

    case IMAPI_MEDIA_TYPE_CDRW:
        m_isCdromSupported = true;
        return _T("CD-RW");

    case IMAPI_MEDIA_TYPE_DVDROM:
        m_isDvdSupported = true;
        return _T("DVD ROM");

    case IMAPI_MEDIA_TYPE_DVDRAM:
        m_isDvdSupported = true;
        return _T("DVD-RAM");

    case IMAPI_MEDIA_TYPE_DVDPLUSR:
        m_isDvdSupported = true;
        return _T("DVD+R");

    case IMAPI_MEDIA_TYPE_DVDPLUSRW:
        m_isDvdSupported = true;
        return _T("DVD+RW");

    case IMAPI_MEDIA_TYPE_DVDPLUSR_DUALLAYER:
        m_isDualLayerDvdSupported = true;
        return _T("DVD+R Dual Layer");

    case IMAPI_MEDIA_TYPE_DVDDASHR:
        m_isDvdSupported = true;
        return _T("DVD-R");

    case IMAPI_MEDIA_TYPE_DVDDASHRW:
        m_isDvdSupported = true;
        return _T("DVD-RW");

    case IMAPI_MEDIA_TYPE_DVDDASHR_DUALLAYER:
        m_isDualLayerDvdSupported = true;
        return _T("DVD-R Dual Layer");

    case IMAPI_MEDIA_TYPE_DISK:
        return _T("random-access writes");

    case IMAPI_MEDIA_TYPE_DVDPLUSRW_DUALLAYER:
        m_isDualLayerDvdSupported = true;
        return _T("DVD+RW DL");

    case IMAPI_MEDIA_TYPE_HDDVDROM:
        return _T("HD DVD-ROM");

    case IMAPI_MEDIA_TYPE_HDDVDR:
        return _T("HD DVD-R");

    case IMAPI_MEDIA_TYPE_HDDVDRAM:
        return _T("HD DVD-RAM");

    case IMAPI_MEDIA_TYPE_BDROM:
        return _T("Blu-ray DVD (BD-ROM)");

    case IMAPI_MEDIA_TYPE_BDR:
        return _T("Blu-ray media");

    case IMAPI_MEDIA_TYPE_BDRE:
        return _T("Blu-ray Rewritable media");
    }

}


void CBurnCDDlg::OnLbnSelchangeBurnFileList()
{
    GetDlgItem(IDC_REMOVE_FILES_BUTTON)->EnableWindow(m_fileListbox.GetCurSel()!=-1);
}

//����ļ���ť click
void CBurnCDDlg::OnBnClickedAddFilesButton()
{   
    CFileDialog fileDialog(TRUE, NULL, NULL, OFN_FILEMUSTEXIST, _T("All Files (*.*)|*.*||"), NULL, 0);
    if (fileDialog.DoModal() == IDOK)
    {
        CFileObject* pFileObject = new CFileObject(fileDialog.GetPathName());
        int addIndex = m_fileListbox.AddString(pFileObject->GetName());
        m_fileListbox.SetItemDataPtr(addIndex, pFileObject);
        UpdateCapacity();
        EnableBurnButton();
    }
}
//����ļ���
void CBurnCDDlg::OnBnClickedAddFolderButton()
{
    BROWSEINFO bi = {0};
    bi.hwndOwner = m_hWnd;
    bi.ulFlags = BIF_RETURNONLYFSDIRS;//|BIF_USENEWUI;
    LPITEMIDLIST lpidl = SHBrowseForFolder(&bi);

    if (!lpidl)
        return;

    TCHAR selectedPath[_MAX_PATH] = {0};
    if (SHGetPathFromIDList(lpidl, selectedPath))
    {
        CDirObject* pDirObject = new CDirObject(selectedPath);
        int addIndex = m_fileListbox.AddString(pDirObject->GetName());
        m_fileListbox.SetItemDataPtr(addIndex, pDirObject);
        UpdateCapacity();
        EnableBurnButton();
    }
}

void CBurnCDDlg::OnDestroy()
{
    int itemCount = m_fileListbox.GetCount();
    for (int itemIndex = 0; itemIndex < itemCount; itemIndex++)
    {
        delete (CBaseObject*)m_fileListbox.GetItemDataPtr(itemIndex);
    }

    itemCount = m_deviceComboBox.GetCount();
    for (int itemIndex = 0; itemIndex < itemCount; itemIndex++)
    {
        delete (CDiscRecorder*)m_deviceComboBox.GetItemDataPtr(itemIndex);
    }


    CDialog::OnDestroy();
}

//��¼��ť
void CBurnCDDlg::OnBnClickedBurnButton()
{
    if (m_isBurning)//¼��
    {
        SetCancelBurning(true);//����ȡ����¼��ť
    }
    else
    {
        SetCancelBurning(false);//ȥ��ȡ����¼��ť
        m_isBurning = true;
        UpdateData();
        EnableUI(false);//ui ʹ�ܡ�

        AfxBeginThread(BurnThread, this, THREAD_PRIORITY_NORMAL);//�̴߳���
    }
}

//��¼�߳�
UINT CBurnCDDlg::BurnThread(LPVOID pParam)
{
    CBurnCDDlg* pThis = (CBurnCDDlg*)pParam;

    //
    // Get the selected recording device from the combobox
    //
    int selectedIndex = pThis->m_deviceComboBox.GetCurSel();
    ASSERT(selectedIndex >= 0);
    if (selectedIndex < 0)
    {
        pThis->SendMessage(WM_BURN_FINISHED, 0, (LPARAM)_T("Error: No Device Selected"));
        return 0;
    }

    CDiscRecorder* pOrigDiscRecorder = 
        (CDiscRecorder*)pThis->m_deviceComboBox.GetItemDataPtr(selectedIndex);
    if (pOrigDiscRecorder == NULL)
    {
        //
        // This should never happen
        //
        pThis->SendMessage(WM_BURN_FINISHED, 0, (LPARAM)_T("Error: No Data for selected device"));
        return 0;
    }

    //
    // Did user cancel?
    //
    if (pThis->GetCancelBurning())//�Ƿ�ȡ��
    {
        pThis->SendMessage(WM_BURN_FINISHED, 0, (LPARAM)_T("User Canceled!"));
        return 0;
    }

    pThis->SendMessage(WM_BURN_STATUS_MESSAGE, 0, (LPARAM)_T("Initializing Disc Recorder..."));

    //
    // Create another disc recorder because we're in a different thread
    //
    CDiscRecorder discRecorder;//

    CString errorMessage;
    if (discRecorder.Initialize(pOrigDiscRecorder->GetUniqueId()))
    {
        //
        // 
        //
        if (discRecorder.AcquireExclusiveAccess(true, CLIENT_NAME))
        {
            CDiscFormatData discFormatData;
            if (discFormatData.Initialize(&discRecorder, CLIENT_NAME))
            {
                //
                // Get the media type currently in the recording device
                //
                IMAPI_MEDIA_PHYSICAL_TYPE mediaType = IMAPI_MEDIA_TYPE_UNKNOWN;
                discFormatData.GetInterface()->get_CurrentPhysicalMediaType(&mediaType);

                //
                // Create the file system
                //
                IStream* dataStream = NULL;
                if (!CreateMediaFileSystem(pThis, mediaType, &dataStream))
                {	// CreateMediaFileSystem reported error to UI
                    return false;
                }

                discFormatData.SetCloseMedia(pThis->m_closeMedia ? true : false);

                //
                // Burn the data, this does all the work
                //
                discFormatData.Burn(pThis->m_hWnd, dataStream);//��¼

                //
                // Eject Media if they chose
                //
                if (pThis->m_ejectWhenFinished)
                {
                    discRecorder.EjectMedia();
                }

            }

            discRecorder.ReleaseExclusiveAccess();

            //
            // Finished Burning, GetHresult will determine if it was successful or not
            //
            pThis->SendMessage(WM_BURN_FINISHED, discFormatData.GetHresult(), 
                (LPARAM)(LPCTSTR)discFormatData.GetErrorMessage());
        }
        else
        {
            errorMessage.Format(_T("Failed: %s is exclusive owner"),
                (LPCTSTR)discRecorder.ExclusiveAccessOwner());
            pThis->SendMessage(WM_BURN_FINISHED, discRecorder.GetHresult(), 
                (LPARAM)(LPCTSTR)errorMessage);
        }
    }
    else
    {
        errorMessage.Format(_T("Failed to initialize recorder - Unique ID:%s"),
            (LPCTSTR)pOrigDiscRecorder->GetUniqueId());
        pThis->SendMessage(WM_BURN_FINISHED, discRecorder.GetHresult(), 
            (LPARAM)(LPCTSTR)errorMessage);
    }

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
//
// CBurnCDDlg::CreateMediaFileSystem
//
// Description
//		Creates an IStream used to 
//
bool CBurnCDDlg::CreateMediaFileSystem(CBurnCDDlg* pThis, IMAPI_MEDIA_PHYSICAL_TYPE mediaType, IStream** ppDataStream)
{
    IFileSystemImage*		image = NULL;
    IFileSystemImageResult*	imageResult = NULL;
    IFsiDirectoryItem*		rootItem = NULL;
    CString					message;
    bool					returnVal = false;

	//ʹ�ô˽ӿڹ����ļ�ϵͳӳ�����ûỰ������������򵼳�image��
	//�ļ�ϵͳĿ¼��νṹ��ͨ����Ŀ¼���ļ���ӵ���Ŀ¼����Ŀ¼��������
	//CLSID�������֤һ�����Ǹ����Ψһ��ʶ
    HRESULT hr = CoCreateInstance(CLSID_MsftFileSystemImage,
        NULL, CLSCTX_ALL, __uuidof(IFileSystemImage), (void**)&image);
    if (FAILED(hr) || (image == NULL))
    {
        pThis->SendMessage(WM_BURN_FINISHED, hr, 
            (LPARAM)_T("Failed to create IFileSystemImage Interface"));
        return false;
    }

    pThis->SendMessage(WM_BURN_STATUS_MESSAGE, 0, (LPARAM)_T("Creating File System..."));

    image->put_FileSystemsToCreate((FsiFileSystems)(FsiFileSystemJoliet|FsiFileSystemISO9660));//�������ɽ����ʱ�������ļ�ϵͳ��
    image->put_VolumeName(pThis->m_volumeLabel.AllocSysString());//Ϊ���ļ�ϵͳӳ�����þ����ơ�
    image->ChooseImageDefaultsForMediaType(mediaType);//����ָ����ý����������Ĭ�ϵ��ļ�ϵͳ���ͺ�image��С

    //
    // Get the image root
    //
    hr = image->get_Root(&rootItem);//������Ŀ¼�
    if (SUCCEEDED(hr))
    {
        //
        // Add Files and Directories to File System Image
        //
        int itemCount = pThis->m_fileListbox.GetCount();
        for (int itemIndex = 0; itemIndex < itemCount; itemIndex++)
        {
            CBaseObject* pObject = (CBaseObject*)pThis->m_fileListbox.GetItemDataPtr(itemIndex);
            ASSERT(pObject != NULL);
            if (pObject == NULL)
                continue;

            CString fileName = pObject->GetName();//�ļ���������·��
            message.Format(_T("Adding \"%s\" to file system..."), (LPCTSTR)fileName);
            pThis->SendMessage(WM_BURN_STATUS_MESSAGE, 0, (LPARAM)(LPCTSTR)message);

            if (pObject->IsKindOf(RUNTIME_CLASS(CFileObject)))// �ļ�
            {
                CFileObject* pFileObject = (CFileObject*)pObject;
                IStream* fileStream = pFileObject->GetStream();//��ȡ��ȡ����
                if (fileStream != NULL)
                {
                    hr = rootItem->AddFile(pFileObject->GetName().AllocSysString(), fileStream);//���ļ���ӵ��ļ�ϵͳӳ���С�
                    if (FAILED(hr))
                    {
                        // IMAPI_E_IMAGE_SIZE_LIMIT 0xc0aab120
                        message.Format(_T("Failed IFsiDirectoryItem->AddFile(%s)!"), 
                            (LPCTSTR)pFileObject->GetName());
                        pThis->SendMessage(WM_BURN_FINISHED, hr, (LPARAM)(LPCTSTR)message);
                        break;
                    }
                }
            }
            else if (pObject->IsKindOf(RUNTIME_CLASS(CDirObject)))//��������Ӧ�ڸ��࣬�򷵻ط���ֵ������Ϊ0��
            {
                CDirObject* pDirObject = (CDirObject*)pObject;
                hr = rootItem->AddTree(pDirObject->GetPath().AllocSysString(), VARIANT_TRUE);//��Ŀ¼����������ӵ��ļ�ϵͳӳ���С�

                if (FAILED(hr))
                {
                    // IMAPI_E_IMAGE_SIZE_LIMIT 0xc0aab120
                    message.Format(_T("Failed IFsiDirectoryItem->AddTree(%s)!"), 
                        (LPCTSTR)pDirObject->GetName());
                    pThis->SendMessage(WM_BURN_FINISHED, hr, (LPARAM)(LPCTSTR)message);
                    break;
                }
            }

            //
            // Did user cancel?
            //
            if (pThis->GetCancelBurning())
            {
                pThis->SendMessage(WM_BURN_FINISHED, 0, (LPARAM)_T("User Canceled!"));
                hr = IMAPI_E_FSI_INTERNAL_ERROR;
            }
        }

        if (SUCCEEDED(hr))
        {
            // Create the result image
            hr = image->CreateResultImage(&imageResult);//���������ļ�ϵͳ���ļ����ݵĽ��image
            if (SUCCEEDED(hr))
            {
                //
                // Get the stream
                //
                hr = imageResult->get_ImageStream(ppDataStream);//������¼��image stream
                if (SUCCEEDED(hr))
                {
                    returnVal = true;
                }
                else
                {
                    pThis->SendMessage(WM_BURN_FINISHED, hr, 
                        (LPARAM)_T("Failed IFileSystemImageResult->get_ImageStream!"));
                }

            }
            else
            {
                pThis->SendMessage(WM_BURN_FINISHED, hr, 
                    (LPARAM)_T("Failed IFileSystemImage->CreateResultImage!"));
            }
        }
    }
    else
    {
        pThis->SendMessage(WM_BURN_FINISHED, hr, (LPARAM)_T("Failed IFileSystemImage->getRoot"));
    }

    //
    // Cleanup
    //
    if (image != NULL)
        image->Release();
    if (imageResult != NULL)
        imageResult->Release();
    if (rootItem != NULL)
        rootItem->Release();

    return returnVal;
}


LRESULT CBurnCDDlg::OnBurnStatusMessage(WPARAM, LPARAM lpMessage)
{
    if (lpMessage != NULL)
    {
        m_progressText.SetWindowText(L"qqqqqqqqqqqq"/*(LPCTSTR)lpMessage*/);
    }
    return 0;
}

//��¼��� ����
LRESULT CBurnCDDlg::OnBurnFinished(WPARAM hResult, LPARAM lpMessage)
{
    if (lpMessage != NULL)
    {
        if (SUCCEEDED((HRESULT)hResult))
        {
            m_progressText.SetWindowText((LPCTSTR)lpMessage);
        }
        else
        {
            CString text;
            text.Format(_T("%s - Error:0x%08X"), (LPCTSTR)lpMessage, hResult);
            m_progressText.SetWindowText(text);
        }
    }
    else
    {
        if (SUCCEEDED((HRESULT)hResult))
        {
            m_progressText.SetWindowText(_T("Burn completed successfully!"));
        }
        else
        {
            CString message;
            message.Format(_T("Burn failed! Error: 0x%08x"), hResult);
            m_progressText.SetWindowText(message);
        }
    }

    EnableUI(TRUE);
    return 0;
}

//��¼��ʱ��һЩ��ʾ
LRESULT CBurnCDDlg::OnImapiUpdate(WPARAM wParam, LPARAM lParam)
{
    IMAPI_FORMAT2_DATA_WRITE_ACTION currentAction = 
        (IMAPI_FORMAT2_DATA_WRITE_ACTION)wParam;
    PIMAPI_STATUS pImapiStatus = (PIMAPI_STATUS)lParam;

    switch (currentAction)
    {
    case IMAPI_FORMAT2_DATA_WRITE_ACTION_VALIDATING_MEDIA:
        m_progressText.SetWindowText(_T("Validating current media..."));
        break;

    case IMAPI_FORMAT2_DATA_WRITE_ACTION_FORMATTING_MEDIA:
        m_progressText.SetWindowText(_T("Formatting media..."));
        break;

    case IMAPI_FORMAT2_DATA_WRITE_ACTION_INITIALIZING_HARDWARE:
        m_progressText.SetWindowText(_T("Initializing hardware..."));
        break;

    case IMAPI_FORMAT2_DATA_WRITE_ACTION_CALIBRATING_POWER:
        m_progressText.SetWindowText(_T("Optimizing laser intensity..."));
        break;

    case IMAPI_FORMAT2_DATA_WRITE_ACTION_WRITING_DATA:
        UpdateTimes(pImapiStatus->totalTime, pImapiStatus->remainingTime);
        UpdateBuffer(pImapiStatus->usedSystemBuffer, pImapiStatus->totalSystemBuffer);
        UpdateProgress(pImapiStatus->lastWrittenLba-pImapiStatus->startLba, pImapiStatus->sectorCount);
        break;

    case IMAPI_FORMAT2_DATA_WRITE_ACTION_FINALIZATION:
        m_progressText.SetWindowText(_T("Finalizing writing..."));
        break;

    case IMAPI_FORMAT2_DATA_WRITE_ACTION_COMPLETED:
        m_progressText.SetWindowText(_T("Completed!"));
        break;
    }

    return GetCancelBurning() ? RETURN_CANCEL_WRITE : RETURN_CONTINUE;
}
//ʱ��
void CBurnCDDlg::UpdateTimes(LONG totalTime, LONG remainingTime)
{
    //
    // Set the estimated total time
    //
    CString strText;
    if (totalTime > 0)
    {
        strText.Format(_T("%d:%02d"), totalTime / 60, totalTime % 60);
    }
    else
    {
        strText = _T("0:00");
    }
    m_estimatedTime.SetWindowText(strText);

    //
    // Set the estimated remaining time
    //
    if (remainingTime > 0)
    {
        strText.Format(_T("%d:%02d"), remainingTime / 60, remainingTime % 60);
    }
    else
    {
        strText = _T("0:00");
    }
    m_timeLeft.SetWindowText(strText);
}

//���������
void CBurnCDDlg::UpdateBuffer(LONG usedSystemBuffer, LONG totalSystemBuffer)
{
    CString text;

    if (usedSystemBuffer && totalSystemBuffer)
    {
        m_bufferCtrl.SetRange32(0, totalSystemBuffer);
        m_bufferCtrl.SetPos(usedSystemBuffer);
        text.Format(_T("Buffer111111: %d%%"), (100*usedSystemBuffer) / totalSystemBuffer);
    }
    else
    {
        text = _T("Buffer Empty");
        m_bufferCtrl.SetPos(0);
    }

    m_bufferText.SetWindowText(text);
}

//���ȸ���
void CBurnCDDlg::UpdateProgress(LONG writtenSectors, LONG totalSectors)
{
    static LONG prevTotalSector = 0;
    CString text;

    if (totalSectors && (totalSectors != prevTotalSector))
    {
        prevTotalSector = totalSectors;
        m_progressCtrl.SetRange32(0, totalSectors);
    }
    m_progressCtrl.SetPos(writtenSectors);

    if (writtenSectors && totalSectors)
    {
        text.Format(_T("Progress: %d%%"), (100*writtenSectors) / totalSectors);
    }
    else
    {
        text = _T("Progress");
    }
    m_progressText.SetWindowText(text);
}

void CBurnCDDlg::OnCbnSelchangeMediaTypeCombo()//comboѡ��ı�����
{
    int selectedIndex = m_mediaTypeCombo.GetCurSel();//��ǰѡ��
    if (selectedIndex == -1)
    {
        m_selectedMediaType = -1;
    }
    else
    {
        m_selectedMediaType = (int)m_mediaTypeCombo.GetItemData(selectedIndex);
    }

    UpdateCapacity();
}

//��������
void CBurnCDDlg::UpdateCapacity()
{
    //
    // Set the selected media type data
    //
    ULONGLONG totalSize = 0;
    CString maxText;
    if (m_selectedMediaType == CD_MEDIA)
    {
        maxText = _T("700MB");
        totalSize = 700000000;
    }
    else if (m_selectedMediaType == DVD_MEDIA)
    {
        maxText = _T("4.7GB");
        totalSize = 4700000000;
    }
    else if (m_selectedMediaType == DL_DVD_MEDIA)
    {
        maxText = _T("8.5GB");
        totalSize = 8500000000;
    }
    m_maxText.SetWindowText(maxText);

    //
    // Calculate the size of the files
    //
    ULONGLONG mediaSize = 0;
    int itemCount = m_fileListbox.GetCount();
    for (int itemIndex = 0; itemIndex < itemCount; itemIndex++)
    {
        CBaseObject* pObject = (CBaseObject*)m_fileListbox.GetItemDataPtr(itemIndex);
        mediaSize += pObject->GetSizeOnDisc();
    }

    m_capacityProgress.SetRange(0,100);//������
    if (mediaSize == 0)
    {
        m_capacityProgress.SetPos(0);
#if _MFC_VER >= 0x0900
        m_capacityProgress.SetState(PBST_NORMAL);
#endif
    }
    else
    {
        int percent = (int)((mediaSize*100)/totalSize);
        if (percent > 100)
        {
            m_capacityProgress.SetPos(100);
#if _MFC_VER >= 0x0900
            m_capacityProgress.SetState(PBST_ERROR);
#endif
        }
        else
        {
            m_capacityProgress.SetPos(percent);
#if _MFC_VER >= 0x0900
            m_capacityProgress.SetState(PBST_NORMAL);
#endif
        }

    }
}

// burnʹ��
void CBurnCDDlg::EnableBurnButton()
{
    GetDlgItem(IDC_BURN_BUTTON)->EnableWindow(m_fileListbox.GetCount()>0);
}

//remove file click ����
void CBurnCDDlg::OnBnClickedRemoveFilesButton()
{
    int currentSelection = m_fileListbox.GetCurSel();
    ASSERT(currentSelection != -1);
    if (currentSelection == -1)
        return;

    CBaseObject* pBaseObject = (CBaseObject*)m_fileListbox.GetItemDataPtr(currentSelection);
    if (pBaseObject == NULL)
        return;

    CString message;
    message.Format(_T("Are you sure you want to remove \"%s\"?"), (LPCTSTR)pBaseObject->GetName());

    if (AfxMessageBox(message, MB_YESNO|MB_ICONQUESTION) == IDYES)
    {
        m_fileListbox.DeleteString(currentSelection);
        delete pBaseObject;

        OnLbnSelchangeBurnFileList();
        EnableBurnButton();
        UpdateCapacity();
    }
}

void CBurnCDDlg::EnableUI(BOOL bEnable)
{
    m_deviceComboBox.EnableWindow(bEnable);
    m_fileListbox.EnableWindow(bEnable);
    m_mediaTypeCombo.EnableWindow(bEnable);
    GetDlgItem(IDC_ADD_FILES_BUTTON)->EnableWindow(bEnable);
    GetDlgItem(IDC_ADD_FOLDER_BUTTON)->EnableWindow(bEnable);
    GetDlgItem(IDC_REMOVE_FILES_BUTTON)->EnableWindow(bEnable);
    GetDlgItem(IDCANCEL)->EnableWindow(bEnable);
    GetDlgItem(IDC_EJECT_WHEN_FINISHED)->EnableWindow(bEnable);
    GetDlgItem(IDC_CLOSE_MEDIA_CHK)->EnableWindow(bEnable);
    GetDlgItem(IDC_VOLUME)->EnableWindow(bEnable);

    GetDlgItem(IDC_BURN_BUTTON)->SetWindowText(bEnable ? 
        _T("Burn") : _T("Cancel"));
}

//ȡ��burn
void CBurnCDDlg::SetCancelBurning(bool bCancel)
{
    CSingleLock singleLock(&m_critSection);//����
    m_cancelBurn = bCancel;
}
//ȡ��burn
bool CBurnCDDlg::GetCancelBurning()
{
    CSingleLock singleLock(&m_critSection);
    return m_cancelBurn;
}
