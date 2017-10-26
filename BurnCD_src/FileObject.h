#pragma once

#include "BaseObject.h"

// CFileObject command target

class CFileObject : public CBaseObject
{
	DECLARE_DYNAMIC(CFileObject)

protected:
	IStream* m_pStream;

public:
	CFileObject(const CString& filePath);
	virtual ~CFileObject();

	virtual ULONGLONG	GetSizeOnDisc();
	IStream*	GetStream();
};


