// BaseObject.cpp : implementation file
//

#include "stdafx.h"
#include "BaseObject.h"


// CBaseObject
IMPLEMENT_DYNAMIC(CBaseObject, CObject)

CBaseObject::CBaseObject(const CString& path)
: m_path(path)
{
}

CBaseObject::~CBaseObject()
{
}


// CBaseObject member functions
CString CBaseObject::GetName()
{
	int findIndex = m_path.ReverseFind(_T('\\'));
	return /*L"AAAAAAAAAAAA";*/m_path.Mid(findIndex+1);
}
