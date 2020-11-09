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

// MemoryInfo.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "MemoryInfo.h"
#include "ZeroPageView.h"
#include "StackView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//CRect CMemoryInfo::m_WndRect;
//bool CMemoryInfo::m_bHidden;

//-----------------------------------------------------------------------------
// Rejestracja klasy okien

bool CMemoryInfo::m_bRegistered= FALSE;
CString CMemoryInfo::m_strClass;

void CMemoryInfo::RegisterWndClass()
{
  ASSERT(!m_bRegistered);
  if (m_bRegistered)
    return;
  m_strClass = AfxRegisterWndClass(CS_VREDRAW|CS_HREDRAW/*CS_DBLCLKS*/,::LoadCursor(NULL,IDC_ARROW),0,
    AfxGetApp()->LoadIcon(IDI_MEMORY_INFO));
  m_bRegistered = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMemoryInfo

IMPLEMENT_DYNCREATE(CMemoryInfo, CMiniFrameWnd)


void CMemoryInfo::init()
{
  if (!m_bRegistered)
    RegisterWndClass();
  m_Doc.m_bAutoDelete = FALSE;
}


CMemoryInfo::CMemoryInfo()
{
  m_hWnd = 0;
  init();
  m_bHidden = false;
}

CMemoryInfo::~CMemoryInfo()
{}


//-----------------------------------------------------------------------------
// Nowe okno

bool CMemoryInfo::Create(COutputMem *pMem, UINT16 uAddr, ViewType bView)
{
  m_pMem = pMem;
  m_uAddr = uAddr;

  CString title;
  CCreateContext ctx;
  if (bView == VIEW_MEMORY)
  {
    ctx.m_pNewViewClass = RUNTIME_CLASS(CMemoryView);
    title.LoadString(IDS_MEMORY_TITLE);
  }
  else if (bView == VIEW_ZEROPAGE)
  {
    ctx.m_pNewViewClass = RUNTIME_CLASS(CZeroPageView);
    title.LoadString(IDS_ZMEMORY_TITLE);
  }
  else if (bView == VIEW_STACK)
  {
    ctx.m_pNewViewClass = RUNTIME_CLASS(CStackView);
    title.LoadString(IDS_STACK);
  }
  else
  { ASSERT(false); }

  ctx.m_pCurrentDoc = &m_Doc;	// dokument
  ctx.m_pNewDocTemplate = NULL;	// template
  ctx.m_pLastView = NULL;	// lastView
  ctx.m_pCurrentFrame = this;	// current frame

  if (!CMiniFrameWnd::Create(m_strClass, title,
    WS_POPUP | WS_CAPTION | WS_SYSMENU | MFS_THICKFRAME | MFS_SYNCACTIVE,
    m_WndRect, AfxGetMainWnd(), 0))
    return FALSE;

  if (!CreateView(&ctx))
  {
//    delete this;
    return FALSE;
  }
  RecalcLayout();
  m_Doc.SetData(m_pMem, m_uAddr);
  InitialUpdateFrame(&m_Doc, FALSE);
  return TRUE;
}

//-----------------------------------------------------------------------------

#include <AFXPRIV.H>

BEGIN_MESSAGE_MAP(CMemoryInfo, CMiniFrameWnd)
  //{{AFX_MSG_MAP(CMemoryInfo)
  ON_WM_DESTROY()
  ON_WM_SHOWWINDOW()
  ON_WM_CLOSE()
  //}}AFX_MSG_MAP
  ON_MESSAGE(CBroadcast::WM_USER_PROG_MEM_CHANGED, OnChangeCode)
  ON_MESSAGE(CBroadcast::WM_USER_START_DEBUGGER, OnStartDebug)
  ON_MESSAGE(CBroadcast::WM_USER_EXIT_DEBUGGER, OnExitDebug)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMemoryInfo message handlers


void CMemoryInfo::PostNcDestroy() 
{
// CMiniFrameWnd::PostNcDestroy();
}


void CMemoryInfo::OnDestroy() 
{
  GetWindowRect(m_WndRect);
  CMiniFrameWnd::OnDestroy();
}

//-----------------------------------------------------------------------------

afx_msg LRESULT CMemoryInfo::OnChangeCode(WPARAM wParam, LPARAM lParam)
{
  if (lParam == -1)
    SendMessage(WM_CLOSE);		// nie ma kodu - zamkniêcie okna
  else
    InvalidateRect(NULL);		// przerysowanie ca³ego okna
  return 0;
}

//=============================================================================

afx_msg LRESULT CMemoryInfo::OnStartDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
{

  if (!m_bHidden)		// okno by³o widoczne?
    if (m_hWnd)
      ShowWindow(SW_NORMAL);
//    else
//      Create();

  return 1;
}


afx_msg LRESULT CMemoryInfo::OnExitDebug(WPARAM /*wParam*/, LPARAM /* lParam */)
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

//=============================================================================


void CMemoryInfo::OnShowWindow(BOOL bShow, UINT nStatus)
{
  CMiniFrameWnd::OnShowWindow(bShow, nStatus);

  m_Doc.UpdateAllViews(0, 'show', 0);
}


void CMemoryInfo::OnClose()
{
    m_bHidden = FALSE;

	CMiniFrameWnd::OnClose();
}


void CMemoryInfo::InvalidateView(UINT16 uStackPtr/*= 0*/)
{
  m_Doc.m_uStackPtr = uStackPtr;
  m_Doc.UpdateAllViews(0, 'invl', 0);
}


void CMemoryInfo::Invalidate()
{
	if (CWnd* pView= GetActiveView())
		pView->Invalidate();
}
