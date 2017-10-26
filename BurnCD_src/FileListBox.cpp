// FileListBox.cpp : implementation file
//

#include "stdafx.h"
#include "BurnCD.h"
#include "FileListBox.h"
#include "FileObject.h"
#include "DirObject.h"


// CFileListBox

IMPLEMENT_DYNAMIC(CFileListBox, CListBox)

CFileListBox::CFileListBox()
{

}

CFileListBox::~CFileListBox()
{
}


BEGIN_MESSAGE_MAP(CFileListBox, CListBox)
	ON_WM_MEASUREITEM()
END_MESSAGE_MAP()



// CFileListBox message handlers



void CFileListBox::PreSubclassWindow()
{
	//ModifyStyle(0,LBS_OWNERDRAWFIXED);

	CListBox::PreSubclassWindow();
}

BOOL CFileListBox::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style |= LBS_OWNERDRAWFIXED;

	return CListBox::PreCreateWindow(cs);
}

void CFileListBox::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMIS)
{
	lpMIS->itemHeight = 24;

	CListBox::OnMeasureItem(nIDCtl, lpMIS);
}

void CFileListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	if (lpDIS->itemID == -1)
		return;

	CString strFile;
	CObject* pObject = (CObject*)GetItemData(lpDIS->itemID);
	ASSERT(pObject != NULL);
	if (pObject == NULL)
		return;

	if (!pObject->IsKindOf(RUNTIME_CLASS(CBaseObject)))
		return;

	CBaseObject* pBaseObject = (CBaseObject*)pObject;
	strFile = pBaseObject->GetPath();


	SHFILEINFO shFileInfo = {0};
	::SHGetFileInfo(strFile, 0, &shFileInfo, sizeof(SHFILEINFO),
		SHGFI_DISPLAYNAME|SHGFI_ICON|SHGFI_SMALLICON);

	CRect rect(lpDIS->rcItem);
	HBRUSH hBrush;
	COLORREF clrText;
	if (lpDIS->itemState & ODS_SELECTED)
	{
		hBrush = GetSysColorBrush(COLOR_HIGHLIGHT);
		clrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
	}
	else
	{
		hBrush = GetSysColorBrush(COLOR_WINDOW);
		clrText = GetSysColor(COLOR_WINDOWTEXT);
	}
	::FillRect(lpDIS->hDC, &lpDIS->rcItem, hBrush);

	if (lpDIS->itemState & ODS_FOCUS)
		::DrawFocusRect(lpDIS->hDC, &rect);

	DrawIconEx(lpDIS->hDC, rect.left + 4, rect.top+4, shFileInfo.hIcon,
		16, 16, 0, NULL, DI_NORMAL);

	COLORREF oldColor = ::SetTextColor(lpDIS->hDC, clrText);
	int nOldMode = ::SetBkMode(lpDIS->hDC, TRANSPARENT);

	rect.left += 24;
	DrawText(lpDIS->hDC, shFileInfo.szDisplayName, 
		(int)_tcslen(shFileInfo.szDisplayName), &rect, 
		DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_END_ELLIPSIS);

	::SetBkMode(lpDIS->hDC, nOldMode);
	::SetTextColor(lpDIS->hDC, oldColor);
}

void CFileListBox::MeasureItem(LPMEASUREITEMSTRUCT lpMIS)
{
	lpMIS->itemHeight = 24;
}


