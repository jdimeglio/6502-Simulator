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

// MemoryGoto.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "MemoryGoto.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void AFX_CDECL DDX_HexDec(CDataExchange* pDX, int nIDC, unsigned int &num, bool bWord= true);

/////////////////////////////////////////////////////////////////////////////
// CMemoryGoto dialog


CMemoryGoto::CMemoryGoto(CWnd* pParent /*=NULL*/)
	: CDialog(CMemoryGoto::IDD, pParent)
{
  //{{AFX_DATA_INIT(CMemoryGoto)
  m_uAddr = 0;
  //}}AFX_DATA_INIT
}


void CMemoryGoto::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
//  DDX_Text(pDX, IDC_MEMORY_ADDR, m_uAddr);
  DDX_HexDec(pDX, IDC_MEMORY_ADDR, m_uAddr);
  //{{AFX_DATA_MAP(CMemoryGoto)
  DDV_MinMaxUInt(pDX, m_uAddr, 0, 65535);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMemoryGoto, CDialog)
  //{{AFX_MSG_MAP(CMemoryGoto)
	  // NOTE: the ClassWizard will add message map macros here
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMemoryGoto message handlers
