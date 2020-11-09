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

// DockBarEx.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "DockBarEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDockBarEx

CDockBarEx::CDockBarEx()
{
	m_bResize = false;
	m_bResizing = false;
	m_nDeltaWidth = 0;
}

CDockBarEx::~CDockBarEx()
{}


BEGIN_MESSAGE_MAP(CDockBarEx, CDockBar)
	//{{AFX_MSG_MAP(CDockBarEx)
	ON_WM_SETCURSOR()
	ON_WM_NCHITTEST()
	ON_WM_SIZE()
	ON_MESSAGE(WM_ENTERSIZEMOVE, OnEnterSizeMove)
	ON_MESSAGE(WM_EXITSIZEMOVE, OnExitSizeMove)
	ON_WM_WINDOWPOSCHANGING()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CDockBarEx message handlers

/*CSize CDockBarEx::CalcFixedLayout(BOOL bStretch, BOOL bHorz)
{
	CSize sizeRaw= CDockBar::CalcFixedLayout(bStretch, bHorz);
	return GetDockedCount() > 0 ? sizeRaw + CSize(8, 0) : sizeRaw;
}*/


BOOL CDockBarEx::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (GetDockedVisibleCount() == 0)
		return CDockBar::OnSetCursor(pWnd, nHitTest, message);

	switch (nHitTest)
	{
	case HTLEFT:
	case HTRIGHT:
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
		return true;
/*	case HTTOP:
	case HTBOTTOM:
		::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
		return true; */
	}

	::SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
	return true;

//	return CDockBar::OnSetCursor(pWnd, nHitTest, message);
}


const int nMARGIN= 5;

// Return screen coord rect of dockbar resizing area
//
CRect CDockBarEx::ResizeArea()
{
	CRect rect;
	GetWindowRect(rect);

	switch (GetDlgCtrlID())
	{
	case AFX_IDW_DOCKBAR_LEFT:
		rect.left = rect.right - nMARGIN;
		break;
	case AFX_IDW_DOCKBAR_RIGHT:
		rect.right = rect.left + nMARGIN;
		break;
	}

	return rect;
}

// Resize docked bars to requested width
//
void CDockBarEx::ResizeBars(int nWidth)
{
	for (int i = 0; i < m_arrBars.GetSize(); i++)
	{
		if (CControlBar* pBar= GetDockedControlBar(i))
			pBar->CalcDynamicLayout(nWidth, LM_HORZ | LM_HORZDOCK | 0x8000);
	}

	if (CFrameWnd* pFrame= static_cast<CFrameWnd*>(GetParent()))
	{
		pFrame->RecalcLayout();
		for (int i = 0; i < m_arrBars.GetSize(); i++)
		{
			if (CControlBar* pBar= GetDockedControlBar(i))
			{
				pBar->RedrawWindow(0, 0, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW | RDW_ALLCHILDREN);
			}
		}
		pFrame->UpdateWindow();
	}
}


BOOL CDockBarEx::PreCreateWindow(CREATESTRUCT& cs)
{
	// turn on resizing edge
	if (cs.style & CBRS_ALIGN_LEFT)
		m_cyBottomBorder = nMARGIN;
	else if (cs.style & CBRS_ALIGN_RIGHT)
		m_cyTopBorder = nMARGIN;
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	return CDockBar::PreCreateWindow(cs);
}


// Report back resizing edge position
//
LRESULT CDockBarEx::OnNcHitTest(CPoint ptPos)
{
//	CPoint ptPos(0, 0);
//	::GetCursorPos(&ptPos);

	if (ResizeArea().PtInRect(ptPos))
	{
		switch (GetDlgCtrlID())
		{
		case AFX_IDW_DOCKBAR_LEFT:
			return HTRIGHT;
		case AFX_IDW_DOCKBAR_RIGHT:
			return HTLEFT;
		case AFX_IDW_DOCKBAR_TOP:
			return HTBOTTOM;
		case AFX_IDW_DOCKBAR_BOTTOM:
			return HTTOP;
		}
	}

	return CDockBar::OnNcHitTest(ptPos);
}

// Resizing
//
void CDockBarEx::OnSize(UINT nType, int cx, int cy)
{
	CDockBar::OnSize(nType, cx, cy);

	if (m_bResizing)
	{
		CRect rect;
		GetClientRect(rect);
		m_bResizing = false;
		ResizeBars(rect.Width() + m_nDeltaWidth);
		m_bResizing = true;
	}
}

// Entering user initiated resizing
//
LRESULT CDockBarEx::OnEnterSizeMove(WPARAM wParam, LPARAM lParam)
{
	m_nDeltaWidth = 4;
	// Calc difference between dockbar client area and docked bar width
/*	for (int i = 0; i < m_arrBars.GetSize(); i++)
	{
		if (CControlBar* pBar= GetDockedControlBar(i))
			if (pBar->IsWindowVisible())
			{
				CRect rect;
				GetClientRect(rect);
				m_nDeltaWidth = rect.Width() - CalcDynamicLayout(0, LM_HORZ | LM_HORZDOCK).cx;
				break;
			}
	}*/
	m_bResizing = true;
	return Default();
}

LRESULT CDockBarEx::OnExitSizeMove(WPARAM wParam, LPARAM lParam)
{
	m_bResizing = false;
	return Default();
}

// Suppress z-order changing (important for RecalcLayout)
//
void CDockBarEx::OnWindowPosChanging(WINDOWPOS FAR* pWndPos)
{
	if (m_bResizing)
		pWndPos->flags |= SWP_NOZORDER;
	CDockBar::OnWindowPosChanging(pWndPos);
}
