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

#include "StdAfx.h"
#include "DialBar.h"
#include "AFXPRIV.H"


BEGIN_MESSAGE_MAP(CDialBar, CDialogBar)
	//{{AFX_MSG_MAP(CDialBar)
	ON_WM_GETMINMAXINFO()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


IMPLEMENT_DYNAMIC(CDialBar, CDialogBar)


CDialBar::CDialBar() : m_lastSize(50,50)
{
}


CDialBar::~CDialBar()
{
}


void CDialBar::OnGetMinMaxInfo(MINMAXINFO *lpMMI) 
{
  CDialogBar::OnGetMinMaxInfo(lpMMI);

  lpMMI->ptMaxSize = CPoint(200,600);
  lpMMI->ptMaxPosition = CPoint(20,20);
  lpMMI->ptMaxTrackSize = CPoint(200,600);
  lpMMI->ptMinTrackSize = CPoint(100,100);
}


CSize CDialBar::CalcDynamicLayout(int nLength, DWORD dwMode)
{
//  TRACE2("length = %d \tmode = %x\n", nLength, (int)dwMode);

  if ((nLength == -1) && !(dwMode & LM_MRUWIDTH) && !(dwMode & LM_COMMIT) &&
    ((dwMode & LM_HORZDOCK) || (dwMode & LM_VERTDOCK)))
  {
    return CalcFixedLayout(dwMode & LM_STRETCH, dwMode & LM_HORZDOCK);
  }
  return CalcLayout(nLength,dwMode);
}


CSize CDialBar::CalcLayout(int nLength, DWORD dwMode)
{
  if (dwMode & LM_HORZDOCK)
    return m_lastSize;
/*  {
    CRect rect;
    AfxGetMainWnd()->GetClientRect(rect);
    return CSize( rect.Width(), m_lastSize.cy );
  }*/
//    return m_lastSize;
//    return CSize( GetSystemMetrics(SM_CXSCREEN), m_lastSize.cy );
  else if (dwMode & LM_VERTDOCK)
    return m_lastSize;
/*  {
    CRect rect;
    AfxGetMainWnd()->GetClientRect(rect);
    return CSize( m_lastSize.cx, rect.Height() );
  }*/
//    return m_lastSize;
//    return CSize( m_lastSize.cx, GetSystemMetrics(SM_CYSCREEN) );
  else if (nLength!=-1 && !(dwMode & LM_MRUWIDTH) && !(dwMode & LM_COMMIT))
  {
    nLength &= ~0xF;
    if (nLength < 50)
      nLength = 50;

    if (dwMode & LM_LENGTHY)
      m_lastSize.cy = nLength;
    else
      m_lastSize.cx = nLength;
  }

  return m_lastSize;
}


CSize CDialBar::CalcFixedLayout(bool bStretch, bool bHorz)
{
  return m_lastSize;
/*
  DWORD dwMode = bStretch ? LM_STRETCH : 0;
  dwMode |= bHorz ? LM_HORZ : 0;

  return CalcLayout(-1,dwMode);
*/
/*
  CRect rect;
  GetWindowRect(rect);
  CSize size(rect.Size());
  if (size.cx == 0)
    size.cx = 50;
  if (size.cy == 0)
    size.cy = 50;
  if (bStretch)	   // if not docked stretch to fit
    return CSize(bHorz ? 32767 : size.cx, bHorz ? size.cy : 32767);
  else
    return size;
*/
}

BOOL CDialBar::Create(CWnd* pParentWnd, UINT nIDTemplate, UINT nStyle, UINT nID)
{
  bool ret= CDialogBar::Create(pParentWnd, nIDTemplate, nStyle, nID);

  if (!ret)
    return false;

  CString title;
  if (title.LoadString(nIDTemplate))
    SetWindowText(title);

  return true;
}

// style &= ~MFS_4THICKFRAME;

void CDialBar::OnLButtonDown(UINT nFlags, CPoint point)
{
  CWnd *pWnd= GetParent();
  if (pWnd && (pWnd=pWnd->GetParent()))
    pWnd->ModifyStyle(MFS_4THICKFRAME,MFS_THICKFRAME);

  CDialogBar::OnLButtonDown(nFlags, point);
}
