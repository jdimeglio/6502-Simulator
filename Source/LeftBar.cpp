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

// LeftBar.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "LeftBar.h"
#include "MemoryDC.h"
#include "6502View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLeftBar

CLeftBar::CLeftBar()
{
  m_nBarWidth = 0;
  m_pEditView = 0;
}

CLeftBar::~CLeftBar()
{}


BEGIN_MESSAGE_MAP(CLeftBar, CControlBar)
  //{{AFX_MSG_MAP(CLeftBar)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLeftBar message handlers

CSize CLeftBar::MySize()
{
  CRect rect;
  GetParent()->GetClientRect(rect);
  return CSize(m_nBarWidth, rect.Height());
}


bool CLeftBar::Create(CWnd* pParent, CSrc6502View* pView)
{
  CRect rect(0,0,0,0);
  bool bRet= !!CWnd::CreateEx(0, "STATIC", "", WS_CHILD | WS_VISIBLE,
    rect, pParent, AFX_IDW_CONTROLBAR_LAST);

  m_dwStyle = CBRS_ALIGN_LEFT;

  m_pEditView = pView;

  return bRet;
}


static const int LEFT_MARGIN= 1;		// margin for markers
static const int LEFT_ERR_MARGIN= 4;	// margin for error marker


void CLeftBar::DoPaint(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

//  pDC->DrawFrameControl(rect, DFC_SCROLL, DFCS_SCROLLCOMBOBOX);

  COLORREF rgbLight= GetBkColor();

  CMemoryDC dcMem(*pDC, this, rgbLight);

  if (m_pEditView != 0)
  {
    int nTopLine= 0, nLineCount= 0, nLineHeight= 0;
    m_pEditView->GetDispInfo(nTopLine, nLineCount, nLineHeight);
    if (nLineHeight > 0)
    {
//      int nCtrlBottom= (rect.Height() + nLineHeight - 1) / nLineHeight;

      int nPointerLine= m_pEditView->GetPointerLine();
      int nErrMarkLine= m_pEditView->GetErrorMarkLine();

      int nLine= nTopLine;
      for (int y= 0; y < rect.Height(); y += nLineHeight, ++nLine)
      {
        if (nLine > nLineCount)
          break;

        if (BYTE bp= m_pEditView->GetBreakpoint(nLine))
          draw_breakpoint(dcMem, LEFT_MARGIN, y, nLineHeight, !(bp & CAsm::BPT_DISABLED));

        if (nPointerLine == nLine)
          draw_pointer(dcMem, LEFT_MARGIN, y, nLineHeight);

        if (nErrMarkLine == nLine)
          draw_mark(dcMem, LEFT_ERR_MARGIN, y, nLineHeight);
      }

    }
  }

  dcMem.BitBlt();

  //pDC->FillSolidRect(rect, rgbLight);
}


void CLeftBar::DrawMark()
{
}


void CLeftBar::SetWidth(int nWidth)
{
  m_nBarWidth = nWidth + 1;
  Invalidate();
}


void CLeftBar::RedrawLine(int nLine)
{
  CClientDC dc(this);
  DoPaint(&dc);
}


COLORREF CLeftBar::GetBkColor()
{
  COLORREF rgbGray= ::GetSysColor(COLOR_3DFACE);
  COLORREF rgbLight= RGB(0x80 + GetRValue(rgbGray) / 2, 0x80 + GetGValue(rgbGray) / 2, 0x80 + GetBValue(rgbGray) / 2);
  return rgbLight;
}


BOOL CLeftBar::OnEraseBkgnd(CDC* pDC)
{
  return true;
}
