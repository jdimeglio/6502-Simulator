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

// LogWindow.cpp : implementation file
//

#include "stdafx.h"
#include "LogWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogWindow

IMPLEMENT_DYNCREATE(CLogWindow, CMiniFrameWnd)

CLogWindow::CLogWindow()
{
	m_bHidden = false;
	m_WndRect.SetRect(100, 100, 400, 600);
}

CLogWindow::~CLogWindow()
{
}


BEGIN_MESSAGE_MAP(CLogWindow, CMiniFrameWnd)
	//{{AFX_MSG_MAP(CLogWindow)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
//	ON_NOTIFY(EN_CHANGE, OnChange())
	ON_MESSAGE(CBroadcast::WM_USER_PROG_MEM_CHANGED, OnChangeCode)
	ON_MESSAGE(CBroadcast::WM_USER_START_DEBUGGER, OnStartDebug)
	ON_MESSAGE(CBroadcast::WM_USER_EXIT_DEBUGGER, OnExitDebug)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLogWindow message handlers


bool CLogWindow::Create()
{
	m_brBackground.DeleteObject();
	m_brBackground.CreateSolidBrush(::GetSysColor(COLOR_WINDOW));

	CString strClass= AfxRegisterWndClass(CS_VREDRAW | CS_HREDRAW/*CS_DBLCLKS*/, ::LoadCursor(NULL,IDC_ARROW), 0,
			AfxGetApp()->LoadIcon(IDI_MEMORY_INFO));

	if (!CMiniFrameWnd::Create(strClass, "Command Log",
		WS_POPUP | WS_CAPTION | WS_SYSMENU | MFS_THICKFRAME | MFS_SYNCACTIVE,
		m_WndRect, AfxGetMainWnd(), 0))
		return false;

	CCreateContext ctx;
	ctx.m_pNewViewClass = RUNTIME_CLASS(CEditView);
	ctx.m_pCurrentDoc = 0;			// document
	ctx.m_pNewDocTemplate = NULL;	// template
	ctx.m_pLastView = NULL;			// lastView
	ctx.m_pCurrentFrame = this;		// current frame

	CEditView* pView= static_cast<CEditView*>(CreateView(&ctx));

	if (pView == 0)
		return false;

	pView->ModifyStyle(ES_AUTOHSCROLL | WS_HSCROLL,
		ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE, SWP_FRAMECHANGED);

	pView->GetEditCtrl().SetReadOnly();

	if (m_fntMono.m_hObject == 0)
	{
		HFONT hFont= static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
		LOGFONT lf;
		::GetObject(hFont, sizeof(lf), &lf);
		lf.lfPitchAndFamily = FIXED_PITCH;
		strcpy(lf.lfFaceName, "FixedSys");
		m_fntMono.CreateFontIndirect(&lf);
	}
	pView->SetFont(&m_fntMono);

	RecalcLayout();

	InitialUpdateFrame(0, FALSE);

	return true;
}


void CLogWindow::PostNcDestroy()
{
	// skip default: deletes this
}


HBRUSH CLogWindow::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));
	pDC->SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
	return m_brBackground;
}


void CLogWindow::SetText(const CommandLog& log)
{
	int nCount= log.GetCount();

	if (nCount == 0)
	{
		if (CWnd* pView= GetActiveView())
			pView->SetWindowText("");
		return;
	}

	CString strBuf;
	strBuf.GetBuffer(nCount * 16);	// estimate
	strBuf.ReleaseBuffer(0);

	for (int i= 0; i < nCount; ++i)
		strBuf += log[i].Asm() + "\r\n";

	if (CWnd* pView= GetActiveView())
	{
		CEdit* pEdit= static_cast<CEdit*>(pView);
		pEdit->SetWindowText(strBuf);
		int nLen= strBuf.GetLength();
		pEdit->SetSel(nLen, nLen);
	}
}


void CLogWindow::Invalidate()
{
	if (CWnd* pView= GetActiveView())
		pView->Invalidate();
}

//=============================================================================

afx_msg LRESULT CLogWindow::OnChangeCode(WPARAM wParam, LPARAM lParam)
{
	if (lParam == -1)
		SendMessage(WM_CLOSE);		// nie ma kodu - zamkniêcie okna
	else
		InvalidateRect(NULL);		// przerysowanie ca³ego okna
	return 0;
}


afx_msg LRESULT CLogWindow::OnStartDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{

	if (!m_bHidden)		// okno by³o widoczne?
		if (m_hWnd)
			ShowWindow(SW_NORMAL);
		//    else
		//      Create();
		
	return 1;
}


afx_msg LRESULT CLogWindow::OnExitDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{
	if (m_hWnd && (GetStyle() & WS_VISIBLE))	// okno aktualnie wyœwietlone?
	{
		m_bHidden = FALSE;				// info - okno by³o wyœwietlane
		ShowWindow(SW_HIDE);			// ukrycie okna
	}
	else
		m_bHidden = TRUE;				// info - okno by³o ukryte

	return 1;
}


void CLogWindow::OnDestroy()
{
	GetWindowRect(m_WndRect);
	CMiniFrameWnd::OnDestroy();
}


void CLogWindow::OnClose()
{
	ShowWindow(SW_HIDE);		// ukrycie okna
//	CMiniFrameWnd::OnClose();
}
