// DirObject.cpp : implementation file
//

#include "stdafx.h"
#include "DirObject.h"
#include "FileObject.h"


// CDirObject
IMPLEMENT_DYNAMIC(CDirObject, CBaseObject)

CDirObject::CDirObject(const CString& dirPath)//添加目录时调用
: CBaseObject(dirPath)
{
	CFileFind fileFind;//本地文件查找
	BOOL bWorking = fileFind.FindFile(dirPath+_T("\\*.*"));
	while (bWorking)
	{
	   bWorking = fileFind.FindNextFile();
	   if (fileFind.IsDots())
		   continue;

	   if (fileFind.IsDirectory())
	   {
		   CDirObject* pDirObject = new CDirObject(fileFind.GetFilePath());
		   m_dirArray.Add(pDirObject);
	   }
	   else
	   {
		   CFileObject* pFileObject = new CFileObject(fileFind.GetFilePath());
		   m_fileArray.Add(pFileObject);
	   }
	} 
}

CDirObject::~CDirObject()
{
	INT_PTR dirCount = m_dirArray.GetCount();
	for (INT_PTR dirIndex = 0; dirIndex < dirCount; dirIndex++)
		delete m_dirArray.GetAt(dirIndex);

	INT_PTR fileCount = m_fileArray.GetCount();
	for (INT_PTR fileIndex = 0; fileIndex < fileCount; fileIndex++)
		delete m_fileArray.GetAt(fileIndex);
}


// CDirObject member functions
//返回木录所占大小
ULONGLONG CDirObject::GetSizeOnDisc()
{
	ULONGLONG sectorSize = 0;

	INT_PTR dirCount = m_dirArray.GetCount();
	for (INT_PTR dirIndex = 0; dirIndex < dirCount; dirIndex++)
		sectorSize += m_dirArray.GetAt(dirIndex)->GetSizeOnDisc();

	INT_PTR fileCount = m_fileArray.GetCount();
	for (INT_PTR fileIndex = 0; fileIndex < fileCount; fileIndex++)
		sectorSize += m_fileArray.GetAt(fileIndex)->GetSizeOnDisc();

	return sectorSize;
}