// FileObject.cpp : implementation file
//

#include "stdafx.h"
#include "BurnCD.h"
#include "FileObject.h"

#define SECTOR_SIZE	2048
#define MAX_READ_SIZE   (4*1024*1024)   // 4MB

// CFileObject
IMPLEMENT_DYNAMIC(CFileObject, CBaseObject)

CFileObject::CFileObject(const CString& filePath)
: CBaseObject(filePath)
, m_pStream(NULL)
{
}

CFileObject::~CFileObject()
{
    if (m_pStream != NULL)
        m_pStream->Release();
}


// CFileObject member functions

ULONGLONG CFileObject::GetSizeOnDisc()
{
    CFileStatus status;
    if (CFile::GetStatus(m_path, status))
    {
        if (status.m_size > 0)
        {
            return ((status.m_size / SECTOR_SIZE) + 1) * SECTOR_SIZE;
        }
    }

    return 0;
}

//获取写入流
IStream* CFileObject::GetStream()
{
    if (m_pStream == NULL)
    {
		//打开或创建一个文件，并获取一条流来读取或写入该文件。mpath:要读取的文件
        SHCreateStreamOnFileEx(m_path, 
            STGM_READ|STGM_SHARE_DENY_NONE|STGM_DELETEONRELEASE,
            FILE_ATTRIBUTE_NORMAL, 
            FALSE, 
            NULL, 
            &m_pStream);//m_pStream读取的流
        //CFile file;
        //if (file.Open(m_path, CFile::modeRead))
        //{
        //    ULONGLONG totalBytesRead = 0;
        //    ULONGLONG fileLength = file.GetLength();
        //    ULONG readSize = (ULONG)min(fileLength, MAX_READ_SIZE);
        //    HGLOBAL hGlobal = GlobalAlloc(GHND, (ULONG)fileLength);
        //    if (hGlobal == NULL)
        //    {
        //        return NULL;
        //    }

        //    if (CreateStreamOnHGlobal(hGlobal, TRUE, &m_pStream) == S_OK)
        //    {
        //        BYTE *pBuffer = new BYTE[readSize];
        //        if (pBuffer != NULL)
        //        {
        //            while (totalBytesRead < fileLength)
        //            {
        //                ULONG bytesRead = file.Read(pBuffer, readSize);
        //                if (bytesRead > 0)
        //                {
        //                    HRESULT hr = m_pStream->Write(pBuffer, bytesRead, NULL);
        //                    if (SUCCEEDED(hr))
        //                    {
        //                        totalBytesRead += bytesRead;
        //                    }
        //                    else
        //                    {
        //                        //
        //                        // We failed to write to the IStream, so cleanup
        //                        // and return NULL
        //                        //
        //                        if (m_pStream)
        //                        {
        //                            m_pStream->Release();
        //                            m_pStream = NULL;
        //                        }
        //                        break;
        //                    }
        //                }
        //            }

        //            delete [] pBuffer;
        //        }
        //    }
        //    else
        //    {
        //        GlobalFree(hGlobal);
        //    }
        //}
    }

    return m_pStream;
}

