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

// IntRequestGeneratorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "IntRequestGeneratorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIntRequestGeneratorDlg dialog


CIntRequestGeneratorDlg::CIntRequestGeneratorDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIntRequestGeneratorDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIntRequestGeneratorDlg)
	m_bGenerateIRQ = FALSE;
	m_bGenerateNMI = FALSE;
	m_uIRQTimeLapse = 0;
	m_uNMITimeLapse = 0;
	//}}AFX_DATA_INIT
}


void CIntRequestGeneratorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIntRequestGeneratorDlg)
	DDX_Check(pDX, IDC_GEN_IRQ, m_bGenerateIRQ);
	DDX_Check(pDX, IDC_GEN_NMI, m_bGenerateNMI);
	DDX_Text(pDX, IDC_IRQ_LAPSE, m_uIRQTimeLapse);
	DDV_MinMaxUInt(pDX, m_uIRQTimeLapse, 1, 2147483647);
	DDX_Text(pDX, IDC_NMI_LAPSE, m_uNMITimeLapse);
	DDV_MinMaxUInt(pDX, m_uNMITimeLapse, 1, 2147483647);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CIntRequestGeneratorDlg, CDialog)
	//{{AFX_MSG_MAP(CIntRequestGeneratorDlg)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIntRequestGeneratorDlg message handlers
