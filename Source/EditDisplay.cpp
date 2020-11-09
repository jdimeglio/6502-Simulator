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

// EditDisplay.cpp : implementation file
//

#include "StdAfx.h"
//#include "6502.h"
//#include "EditDisplay.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditDisplay

CEditDisplay::CEditDisplay() : m_pView(NULL)
{
}

CEditDisplay::~CEditDisplay()
{
}


BEGIN_MESSAGE_MAP(CEditDisplay, CEdit)
  //{{AFX_MSG_MAP(CEditDisplay)
  ON_WM_PAINT()
  ON_WM_CHAR()
  ON_WM_KEYUP()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CEditDisplay::Subclass(CSrc6502View *pView)
{
  m_pView = pView;
  SubclassWindow(pView->GetEditCtrl().m_hWnd);
}


/////////////////////////////////////////////////////////////////////////////
// CEditDisplay message handlers

void CEditDisplay::OnPaint() 
{
//  CPaintDC dc(this); // device context for painting
  
  // TODO: Add your message handler code here
  
  // Do not call CEdit::OnPaint() for painting messages
}

void CEditDisplay::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  // TODO: Add your message handler code here and/or call default
  
//  CEdit::OnChar(nChar, nRepCnt, nFlags);
}

void CEditDisplay::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
  // TODO: Add your message handler code here and/or call default
  
//  CEdit::OnKeyUp(nChar, nRepCnt, nFlags);
}

//-----------------------------------------------------------------------------

