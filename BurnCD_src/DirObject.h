#pragma once

#include "BaseObject.h"

class CFileObject;

// CDirObject command target

class CDirObject : public CBaseObject
{
	DECLARE_DYNAMIC(CDirObject)
protected:
	CTypedPtrArray<CObArray, CDirObject*>	m_dirArray;
	CTypedPtrArray<CObArray, CFileObject*>	m_fileArray;

public:
	CDirObject(const CString& dirPath);
	virtual ~CDirObject();

	virtual ULONGLONG	GetSizeOnDisc();
};


