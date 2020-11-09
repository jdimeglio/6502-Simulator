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
#include "IdentInfo.h"
#include "resource.h"



BEGIN_MESSAGE_MAP(CIdentInfo, CIdentInfoFrame)
  //{{AFX_MSG_MAP(CIdentInfo)
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


CRect CIdentInfo::m_WndRect;
//-----------------------------------------------------------------------------
// Rejestracja klasy okien

bool CIdentInfo::m_bRegistered= false;
CString CIdentInfo::m_strClass;


void CIdentInfo::RegisterWndClass()
{
  ASSERT(!m_bRegistered);
  if (m_bRegistered)
    return;
  m_strClass = AfxRegisterWndClass(CS_DBLCLKS,::LoadCursor(NULL,IDC_ARROW),0,
    AfxGetApp()->LoadIcon(IDR_IDENT_INFO));
  m_bRegistered = TRUE;
}

//-----------------------------------------------------------------------------
// Konstrukcja i zakoñczenie

void CIdentInfo::init()
{
  if (!m_bRegistered)
    RegisterWndClass();
  m_doc.m_bAutoDelete = FALSE;
}


CIdentInfo::CIdentInfo(CDebugInfo *pDebugInfo)
{
  m_pDebugInfo = pDebugInfo;
  init();
}

CIdentInfo::~CIdentInfo()
{
}

void CIdentInfo::PostNcDestroy()
{
  m_hWnd = NULL;
}

//-----------------------------------------------------------------------------
// Nowe okno

bool CIdentInfo::Create(CDebugInfo *pDebugInfo/*= NULL*/)
{
  if (pDebugInfo)
    m_pDebugInfo = pDebugInfo;
  ASSERT(m_pDebugInfo);
  CString title;
  title.LoadString(IDS_IDENT_TITLE);
  CCreateContext ctx;
  ctx.m_pNewViewClass = RUNTIME_CLASS(CIdentInfoView);
  ctx.m_pCurrentDoc = &m_doc;	// dokument
  ctx.m_pNewDocTemplate = NULL;	// template
  ctx.m_pLastView = NULL;	// lastView
  ctx.m_pCurrentFrame = this;	// current frame

  if (!CIdentInfoFrame::Create(m_strClass,title,
    WS_POPUP | WS_CAPTION | WS_SYSMENU | MFS_THICKFRAME | MFS_SYNCACTIVE,
    m_WndRect,AfxGetMainWnd()))
    return FALSE;
  if (!CreateView(&ctx))
  {
    delete this;
    return FALSE;
  }
  RecalcLayout();
  m_doc.SetDebugInfo(m_pDebugInfo);
  InitialUpdateFrame(&m_doc,TRUE);

  return TRUE;

}


void CIdentInfo::OnDestroy() 
{
  GetWindowRect(m_WndRect);
  CIdentInfoFrame::OnDestroy();
}
