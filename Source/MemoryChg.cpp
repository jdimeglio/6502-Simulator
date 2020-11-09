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

// MemoryChg.cpp : implementation file
//

#include "stdafx.h"
#include "MemoryChg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void AFX_CDECL DDX_HexDec(CDataExchange* pDX, int nIDC, unsigned int &num, bool bWord= true);

/////////////////////////////////////////////////////////////////////////////
// CMemoryChg dialog


CMemoryChg::CMemoryChg(COutputMem& mem, CWnd* pParent /*=NULL*/)
 : m_Mem(mem), CDialog(CMemoryChg::IDD, pParent)
{
  //{{AFX_DATA_INIT(CMemoryChg)
  m_uAddr = 0;
  m_nData = 0;
  m_nByte = 0;
  m_bSigned = FALSE;
  //}}AFX_DATA_INIT
}


void CMemoryChg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_HexDec(pDX, IDC_MEMORY_ADDR, m_uAddr);
  DDV_MinMaxUInt(pDX, m_uAddr, 0, 65535);
  DDX_HexDec(pDX, IDC_MEMORY_DATA, reinterpret_cast<unsigned int&>(m_nData));
  DDV_MinMaxInt(pDX, m_nData, -65535, 65535);
  //{{AFX_DATA_MAP(CMemoryChg)
//  DDX_Text(pDX, IDC_MEMORY_DATA, m_nData);
  DDX_Radio(pDX, IDC_MEMORY_BYTE, m_nByte);
  DDX_Check(pDX, IDC_MEMORY_SIGNED, m_bSigned);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMemoryChg, CDialog)
  //{{AFX_MSG_MAP(CMemoryChg)
  ON_EN_CHANGE(IDC_MEMORY_ADDR, OnChangeMemoryAddr)
  ON_BN_CLICKED(IDC_MEMORY_SIGNED, OnMemorySigned)
  ON_BN_CLICKED(IDC_MEMORY_BYTE, OnMemoryByte)
  ON_BN_CLICKED(IDC_MEMORY_WORD, OnMemoryWord)
  ON_BN_CLICKED(ID_MEMORY_CHG, OnMemoryChg)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMemoryChg message handlers

void CMemoryChg::OnChangeMemoryAddr()
{
  CString strText;
  GetDlgItemText(IDC_MEMORY_ADDR, strText);
  const TCHAR *pText= strText;

  bool bErr= false;
  int num;
  if (pText[0] == _T('$') && sscanf(pText + 1, _T("%X"), &num) <= 0)
    bErr = true;
  else if (pText[0] == _T('0') && (pText[1]==_T('x') || pText[1]==_T('X')) &&
    sscanf(pText + 2, _T("%X"), &num) <= 0)
    bErr = true;
  else if (sscanf(pText, _T("%u"), &num) <= 0)
    bErr = true;

  if (bErr)
  {
    SetDlgItemText(IDC_MEMORY_DATA, "");
    return;
  }

  int nData;
  bool bWord= GetCheckedRadioButton(IDC_MEMORY_BYTE,IDC_MEMORY_WORD) == IDC_MEMORY_WORD;
  if (!bWord)
    nData = m_Mem[num];
  else
    nData = m_Mem[num] + (m_Mem[num+1] << 8);

  if (IsDlgButtonChecked(IDC_MEMORY_SIGNED))	// liczba ze znakiem?
    strText.Format(_T("%d"), nData);
  else					// liczba bez znaku
    strText.Format(bWord ? _T("0x%04X") : _T("0x%02X"), nData);

  SetDlgItemText(IDC_MEMORY_DATA, strText);
}


void CMemoryChg::OnMemorySigned()
{
  OnChangeMemoryAddr();
}


void CMemoryChg::OnMemoryByte()
{
  OnChangeMemoryAddr();
}


void CMemoryChg::OnMemoryWord()
{
  OnChangeMemoryAddr();
}


void CMemoryChg::OnOK() 
{
  if (!UpdateData(TRUE))
  {
    TRACE0("UpdateData failed during dialog termination.\n");
    // the UpdateData routine will set focus to correct item
    return;
  }

  Modify();

  EndDialog(IDOK);
//  CDialog::OnOK();
}


void CMemoryChg::Modify()
{
  if (GetCheckedRadioButton(IDC_MEMORY_BYTE,IDC_MEMORY_WORD) == IDC_MEMORY_BYTE)
    m_Mem[m_uAddr] = m_nData;
  else
  {
    m_Mem[m_uAddr] = BYTE(m_nData & 0xFF);
    m_Mem[m_uAddr + 1] = BYTE((m_nData >> 8) & 0xFF);
  }
}


void CMemoryChg::OnMemoryChg() 
{
  if (!UpdateData(TRUE))
    return;

  Modify();
}
