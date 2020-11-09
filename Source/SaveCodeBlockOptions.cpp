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

// SaveCodeBlockOptions.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "SaveCodeBlockOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern void AFX_CDECL DDX_HexDec(CDataExchange* pDX, int nIDC, unsigned int &num, bool bWord= true);

/////////////////////////////////////////////////////////////////////////////
// CSaveCodeBlockOptions dialog


CSaveCodeBlockOptions::CSaveCodeBlockOptions(CWnd* pParent /*=NULL*/)
: CDialog(CSaveCodeBlockOptions::IDD, pParent)
{
  //{{AFX_DATA_INIT(CSaveCodeBlockOptions)
  m_uEnd = 0;
  m_uLength = 0;
  m_uStart = 0;
  //}}AFX_DATA_INIT
}


void CSaveCodeBlockOptions::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CSaveCodeBlockOptions)
  DDX_HexDec(pDX, IDC_SAVE_CODE_OPT_2_END, m_uEnd);
  DDV_MinMaxUInt(pDX, m_uEnd, 0, 65535);
  DDX_HexDec(pDX, IDC_SAVE_CODE_OPT_2_LENGTH, m_uLength);
  DDV_MinMaxUInt(pDX, m_uLength, 0, 65536);
  DDX_HexDec(pDX, IDC_SAVE_CODE_OPT_2_START, m_uStart);
  DDV_MinMaxUInt(pDX, m_uStart, 0, 65535);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSaveCodeBlockOptions, CDialog)
  //{{AFX_MSG_MAP(CSaveCodeBlockOptions)
  ON_NOTIFY(UDN_DELTAPOS, IDC_SAVE_CODE_OPT_2_SPIN_START, OnDeltaposSpinStart)
  ON_NOTIFY(UDN_DELTAPOS, IDC_SAVE_CODE_OPT_2_SPIN_END, OnDeltaposSpinEnd)
  ON_NOTIFY(UDN_DELTAPOS, IDC_SAVE_CODE_OPT_2_SPIN_LENGTH, OnDeltaposSpinLength)
  ON_EN_CHANGE(IDC_SAVE_CODE_OPT_2_START, OnChangeFieldStart)
  ON_EN_CHANGE(IDC_SAVE_CODE_OPT_2_END, OnChangeFieldEnd)
  ON_EN_CHANGE(IDC_SAVE_CODE_OPT_2_LENGTH, OnChangeFieldLength)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSaveCodeBlockOptions message handlers

BOOL CSaveCodeBlockOptions::OnInitDialog()
{
  CDialog::OnInitDialog();
  static UDACCEL accel[]= {0,1, 2,0x10, 5,0x100, 10,0x400};

  CSpinButtonCtrl *pSpin= (CSpinButtonCtrl *)GetDlgItem(IDC_SAVE_CODE_OPT_2_SPIN_START);
  if (pSpin)
  {
    pSpin->SetBase(16);
    pSpin->SetRange(0,6000);
    pSpin->SetAccel(sizeof(accel)/sizeof(accel[0]),accel);
  }
  pSpin = (CSpinButtonCtrl *)GetDlgItem(IDC_SAVE_CODE_OPT_2_SPIN_END);
  if (pSpin)
  {
    pSpin->SetBase(16);
    pSpin->SetRange(0,6000);
    pSpin->SetAccel(sizeof(accel)/sizeof(accel[0]),accel);
  }
  pSpin = (CSpinButtonCtrl *)GetDlgItem(IDC_SAVE_CODE_OPT_2_SPIN_LENGTH);
  if (pSpin)
  {
    pSpin->SetBase(16);
    pSpin->SetRange(0,6000);
    pSpin->SetAccel(sizeof(accel)/sizeof(accel[0]),accel);
  }

  m_bModify = FALSE;

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}

//-----------------------------------------------------------------------------

void CSaveCodeBlockOptions::OnDeltaposSpinStart(NMHDR* pNMHDR, LRESULT* pResult)
{
  NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

  pNMUpDown->iPos = 3000;
  if (pNMUpDown->iDelta)
    IncEditField(GetDlgItem(IDC_SAVE_CODE_OPT_2_START), pNMUpDown->iDelta,0,0xFFFF);

  *pResult = 0;
}


void CSaveCodeBlockOptions::OnDeltaposSpinEnd(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

  pNMUpDown->iPos = 3000;
  if (pNMUpDown->iDelta)
    IncEditField(GetDlgItem(IDC_SAVE_CODE_OPT_2_END), pNMUpDown->iDelta,0,0xFFFF);

  *pResult = 0;
}


void CSaveCodeBlockOptions::OnDeltaposSpinLength(NMHDR* pNMHDR, LRESULT* pResult) 
{
  NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;

  pNMUpDown->iPos = 3000;
  if (pNMUpDown->iDelta)
    IncEditField(GetDlgItem(IDC_SAVE_CODE_OPT_2_LENGTH), pNMUpDown->iDelta,1,0x10000);

  *pResult = 0;
}

//-----------------------------------------------------------------------------

void CSaveCodeBlockOptions::CalculateNums(int pos)
{
  NumFmt fmt1;
  int start= ReadNumber(GetDlgItem(IDC_SAVE_CODE_OPT_2_START),fmt1);
  NumFmt fmt2;
  int end= ReadNumber(GetDlgItem(IDC_SAVE_CODE_OPT_2_END),fmt2);
  NumFmt fmt3;
  int len= ReadNumber(GetDlgItem(IDC_SAVE_CODE_OPT_2_LENGTH),fmt3);

  if (start > end)
    return;
  if (pos==3)		// zmieniæ pole d³ugoœæ?
  {
    if (end-start+1 != len && end-start+1 <= 0x10000)
      SetNumber(GetDlgItem(IDC_SAVE_CODE_OPT_2_LENGTH),end-start+1,fmt3);
  }
  else if (pos==2)	// zmieniæ pole koniec?
  {
    if (start+len-1 != end && start+len-1 <= 0xFFFF && len > 0)
      SetNumber(GetDlgItem(IDC_SAVE_CODE_OPT_2_END),start+len-1,fmt2);
  }
}


void CSaveCodeBlockOptions::OnChangeFieldStart() 
{
  if (m_bModify)
    return;
  m_bModify = TRUE;
  CalculateNums(3);
  m_bModify = FALSE;
}

void CSaveCodeBlockOptions::OnChangeFieldEnd() 
{
  if (m_bModify)
    return;
  m_bModify = TRUE;
  CalculateNums(3);
  m_bModify = FALSE;
}

void CSaveCodeBlockOptions::OnChangeFieldLength() 
{
  if (m_bModify)
    return;
  m_bModify = TRUE;
  CalculateNums(2);
  m_bModify = FALSE;
}
