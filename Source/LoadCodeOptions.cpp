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

// LoadCodeOptions.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "LoadCodeOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


UINT CLoadCodeOptions::m_uStart= 0;
BOOL CLoadCodeOptions::m_bClearMem= true;
UINT CLoadCodeOptions::m_uFill= 0x00;

/////////////////////////////////////////////////////////////////////////////
// CLoadCodeOptions dialog


CLoadCodeOptions::CLoadCodeOptions(CWnd* pParent /*=NULL*/)
: CDialog(CLoadCodeOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoadCodeOptions)
	//}}AFX_DATA_INIT
}


extern void AFX_CDECL DDX_HexDec(CDataExchange* pDX, int nIDC, unsigned int &num, bool bWord= true);

void CLoadCodeOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoadCodeOptions)
	DDX_HexDec(pDX, IDC_LOAD_CODE_START, m_uStart);
	DDV_MinMaxUInt(pDX, m_uStart, 0, 65535);
	DDX_Check(pDX, IDC_LOAD_CODE_CLR, m_bClearMem);
	DDX_HexDec(pDX, IDC_LOAD_CODE_FILL_VALUE, m_uFill, false);
	DDV_MinMaxUInt(pDX, m_uFill, 0, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoadCodeOptions, CDialog)
  //{{AFX_MSG_MAP(CLoadCodeOptions)
  // NOTE: the ClassWizard will add message map macros here
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoadCodeOptions message handlers
