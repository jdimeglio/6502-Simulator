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

// ToolBox.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "ToolBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CToolBox dialog


CToolBox::CToolBox(CWnd* pParent /*=NULL*/)
: CDialog(CToolBox::IDD, pParent), created(FALSE)
{
  //{{AFX_DATA_INIT(CToolBox)
    // NOTE: the ClassWizard will add member initialization here
  //}}AFX_DATA_INIT
}


void CToolBox::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CToolBox)
    // NOTE: the ClassWizard will add DDX and DDV calls here
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CToolBox, CDialog)
//{{AFX_MSG_MAP(CToolBox)
  // NOTE: the ClassWizard will add message map macros here
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolBox message handlers

BOOL CToolBox::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{

  return CDialog::Create(IDD, pParentWnd);
}


BOOL CToolBox::Create(int rsc_id, CWnd* pParentWnd)
{
  this->rsc_id = rsc_id;

  int ret= CDialog::Create(IDD, pParentWnd);
  if (!ret)
    return ret;

  CString regs;
  if (regs.LoadString(rsc_id))
    SetWindowText(regs);	// nazwa (tytu³) okna

  created = true;

  return true;
}


BOOL CToolBox::OnCommand(WPARAM wParam, LPARAM lParam) 
{
  if (LOWORD(wParam) == rsc_id)
  {
    

    return true;
  }
  else
    return CDialog::OnCommand(wParam, lParam);
}

CToolBox::~CToolBox()
{
  if (created)
    DestroyWindow();
}
