/*-----------------------------------------------------------------------------
	6502 Macroassembler and Simulator

Copyright (C) 1995-2003 Michal Kowalski

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
-----------------------------------------------------------------------------*/

// IdentInfoView.cpp : implementation file
//

#include "stdafx.h"
#include "IdentInfoDoc.h"
#include "IdentInfoView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIdentInfoView

IMPLEMENT_DYNCREATE(CIdentInfoView, CListView)

CIdentInfoView::CIdentInfoView()
{
  m_nSortBy = 0;
}

CIdentInfoView::~CIdentInfoView()
{
}


BEGIN_MESSAGE_MAP(CIdentInfoView, CListView)
  //{{AFX_MSG_MAP(CIdentInfoView)
  ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetDispInfo)
  ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIdentInfoView drawing

void CIdentInfoView::OnDraw(CDC* pDC)
{
  CDocument* pDoc = GetDocument();
  // TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CIdentInfoView diagnostics

#ifdef _DEBUG
void CIdentInfoView::AssertValid() const
{
  CListView::AssertValid();
}

void CIdentInfoView::Dump(CDumpContext& dc) const
{
  CListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CIdentInfoView message handlers

BOOL CIdentInfoView::PreCreateWindow(CREATESTRUCT& cs)
{
  cs.style |= LVS_REPORT | LVS_SINGLESEL | WS_CLIPSIBLINGS;
  cs.style &= ~(LVS_SORTASCENDING | LVS_SORTDESCENDING);

  return CListView::PreCreateWindow(cs);
}


void CIdentInfoView::OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult)
{
  LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

  CString str;
  CIdent inf;

  if (pDispInfo->item.mask & LVIF_TEXT)
  {
    switch (pDispInfo->item.iSubItem)
    {
      case 0:	// nazwa
      {
        ((CIdentInfoDoc *)GetDocument())->GetIdent(pDispInfo->item.lParam,str,inf);
        if (str.GetLength() > 9 && str[8] == CAsm::LOCAL_LABEL_CHAR)	// etykieta lokalna ?
	  pDispInfo->item.pszText = (TCHAR *) (const TCHAR *)str + 8;
	else
	  pDispInfo->item.pszText = (TCHAR *) (const TCHAR *)str;
	break;
      }
      case 1:	// wartoœæ
      {
        CListCtrl &listv= GetListCtrl();
        ((CIdentInfoDoc *)GetDocument())->GetIdent(
	  listv.GetItemData(pDispInfo->item.iItem), str, inf );
	static TCHAR num[16];
	switch (inf.info)
	{
	  case CIdent::I_VALUE:
	    if (inf.val < 0x100)
	      wsprintf(num,_T("$%04X (%d, '%c')"),int(inf.val),int(inf.val),
	        inf.val ? TCHAR(inf.val) : _T(' '));
	    else
	      wsprintf(num,_T("$%04X (%d)"),int(inf.val),int(inf.val));
	    break;
	  case CIdent::I_ADDRESS:
	    wsprintf(num,_T("$%04X"),int(inf.val));
	    break;
	  default:
	    _tcscpy(num,_T("?"));
	}
	pDispInfo->item.pszText = (TCHAR *) (const TCHAR *)num;
	break;
      }
      case 2:	// zakres
      {
        CListCtrl &listv= GetListCtrl();
        ((CIdentInfoDoc *)GetDocument())->GetIdent(
	  listv.GetItemData(pDispInfo->item.iItem), str, inf );
	static TCHAR area[16+1];
	const int size= sizeof(area) / sizeof(TCHAR);
        if (str.GetLength() > 9 && str[8] == CAsm::LOCAL_LABEL_CHAR)	// etykieta lokalna ?
	{
	  _tcsncpy(area,str,8);		// oznaczenie strefy etykiety lokalnej
	  area[8] = 0;
	}
	else
	{
	  CString global;
	  global.LoadString(IDS_IDENT_GLOBAL);
	  _tcsncpy(area,global,size-1);
	  area[size-1] = 0;
	}
	pDispInfo->item.pszText = area;
      }
    }
  }

//    pDispInfo->item.mask = LVIF_TEXT;
//    pDispInfo->item.iItem
//    pDispInfo->item.iSubItem = -1;
//    pDispInfo->item.state
//    pDispInfo->item.stateMask
//    pDispInfo->item.cchTextMax
//    pDispInfo->item.iImage
//    pDispInfo->item.lParam

  *pResult = 0;
}


void CIdentInfoView::OnInitialUpdate() 
{
  CListView::OnInitialUpdate();

  CListCtrl &listv= GetListCtrl();
  listv.SetRedraw(FALSE);

  CString str;
  str.LoadString(IDS_IDENT_COL_NAME);
  listv.InsertColumn(0,str,LVCFMT_LEFT,80);	// LVSCW_AUTOSIZE_USEHEADER);
  str.LoadString(IDS_IDENT_COL_VALUE);
  listv.InsertColumn(1,str,LVCFMT_LEFT,110);	//LVSCW_AUTOSIZE_USEHEADER);
  str.LoadString(IDS_IDENT_COL_AREA);
  listv.InsertColumn(2,str,LVCFMT_LEFT,80);	//LVSCW_AUTOSIZE_USEHEADER);

//  CString strItem;
  LV_ITEM lvi;
  lvi.mask = LVIF_TEXT | LVIF_PARAM;
  lvi.iSubItem = 0;

  int lim= ((CIdentInfoDoc *)GetDocument())->GetIdentCount();
  for (int nIndex = 0; nIndex < lim; nIndex++)
  {
//    strCurrent = pDoc->m_Lines[nIndex];
//    strItem = pDoc->GetField(strCurrent, 0);

    lvi.iItem = nIndex;
    lvi.lParam = nIndex;
    lvi.pszText = LPSTR_TEXTCALLBACK;

    listv.InsertItem(&lvi);
  }

/*
  CHeaderCtrl* pHeader = (CHeaderCtrl*) theCtrl.GetDlgItem(0);
  ASSERT(pHeader != NULL);
  int nColCount = pHeader->GetItemCount();
  
  TCHAR szBuffer[255];
  HD_ITEM hdi;
  hdi.mask = HDI_TEXT;
  hdi.pszText = szBuffer;
  hdi.cchTextMax = 255;
  
  for (nIndex = 0; nIndex < nColCount; nIndex++)
    pHeader->GetItem(nIndex, &hdi);
*/

  listv.SetRedraw(TRUE);
  listv.InvalidateRect(NULL);
}


int CALLBACK CIdentInfoView::CompareLVIFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
  CIdentInfoView *pView= (CIdentInfoView *)lParamSort;
  ASSERT(pView != NULL && pView->IsKindOf(RUNTIME_CLASS(CIdentInfoView)));

  CIdentInfoDoc *pDoc= (CIdentInfoDoc *)pView->GetDocument();
  ASSERT(pDoc != NULL && pDoc->IsKindOf(RUNTIME_CLASS(CIdentInfoDoc)));
  CIdent inf1,inf2;
  CString str1,str2;
  pDoc->GetIdent((int)lParam1,str1,inf1);
  pDoc->GetIdent((int)lParam2,str2,inf2);
  int neg= 1;
  int ret;

  switch (pView->m_nSortBy)	// sposób sortowania identyfikatorów
  {
    case -1:	// wg. nazwy, malej¹co
      neg = -1;
    case 1:	// wg. nazwy
      if (str1.GetLength()>8 && str1[8] == CAsm::LOCAL_LABEL_CHAR)	// etykieta lokalna?
        if (str2.GetLength()>8 && str2[8] == CAsm::LOCAL_LABEL_CHAR)
	  ret = _tcscmp((LPCSTR)str1+9,(LPCSTR)str2+9);
	else
	  return 1;	// zawsze 1 (bez wzglêdu na 'neg')
      else
        if (str2.GetLength()>8 && str2[8] == CAsm::LOCAL_LABEL_CHAR)
	  return -1;	// zawsze -1
	else
          ret = str1.Compare(str2);	// porównanie dwóch etykiet globalnych
      return ret * neg;

    case -2:
      neg = -1;
    case 2:	// wg. wartoœci
      if (inf1.info == CIdent::I_ADDRESS || inf1.info == CIdent::I_VALUE)
        if (inf2.info == CIdent::I_ADDRESS || inf2.info == CIdent::I_VALUE)
	  ret = inf1.val - inf2.val;
        else
	  ret = 1;	// drugi identyfikator niezdefiniowany
      else
	ret = -1;	// pierwszy identyfikator niezdefiniowany
      return ret * neg;

    case -3:
      neg = -1;
    case 3:	// wg. zasiêgu
      if (str1.GetLength()>8 && str1[8] == CAsm::LOCAL_LABEL_CHAR)
	if (str2.GetLength()>8 && str2[8] == CAsm::LOCAL_LABEL_CHAR)	// etykiety lokalne?
	  ret = -str1.Compare(str2);	// porównanie ca³ych etykiet
        else
	  ret = -1;	// pierwsza lokalna, druga globalna (porównywane etykiety)
      else
	if (str2.GetLength()>8 && str2[8] == CAsm::LOCAL_LABEL_CHAR)	// etykiety lokalne?
	  ret = 1;	// pierwsza globalna, druga lokalna (porównywane etykiety)
        else
	  return str1.Compare(str2);	// dwie etykiety globalne (zawsze sortuj rosn¹co)
      return ret * neg;

//    case 4:	// wg. pliku
//    case -4:
//    case 5:	// wg. nr wiersza
//    case -5:
    case 0:	// bez sortowania
      return 0;
    default:
      ASSERT(FALSE);	// z³a wart. parametr 'lParamSort' dla fn porównuj¹cej
      return 0;
  }
}


void CIdentInfoView::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
  CListCtrl& listv = GetListCtrl();

  int item= pNMListView->iSubItem + 1;
  m_nSortBy = item == abs(m_nSortBy) ? -m_nSortBy : item;
  CWaitCursor waiter;
  listv.SortItems(CompareLVIFunc,(LPARAM)this);

  *pResult = 0;
}
