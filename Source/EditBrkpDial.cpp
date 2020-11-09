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

// EditBrkpDial.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "EditBrkpDial.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialEditBreakpoint dialog


CDialEditBreakpoint::CDialEditBreakpoint(Breakpoint bp, CWnd* pParent /*=NULL*/)
  : CDialog(CDialEditBreakpoint::IDD, pParent)
{
  //{{AFX_DATA_INIT(CDialEditBreakpoint)
  //}}AFX_DATA_INIT
  m_Execute = (bp & BPT_EXEC) != 0;
  m_Read = (bp & BPT_READ) != 0;
  m_Write = (bp & BPT_WRITE) != 0;
  m_Disabled = (bp & BPT_DISABLED) != 0;
}


void CDialEditBreakpoint::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CDialEditBreakpoint)
  DDX_Check(pDX, IDC_EDIT_BP_DISABLED, m_Disabled);
  DDX_Check(pDX, IDC_EDIT_BP_EXEC, m_Execute);
  DDX_Check(pDX, IDC_EDIT_BP_READ, m_Read);
  DDX_Check(pDX, IDC_EDIT_BP_WRITE, m_Write);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialEditBreakpoint, CDialog)
//{{AFX_MSG_MAP(CDialEditBreakpoint)
// NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialEditBreakpoint message handlers

