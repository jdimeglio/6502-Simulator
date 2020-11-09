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

// SaveCode.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "SaveCode.h"
#include <Dlgs.h>
#include "SaveCodeOptions.h"
#include "SaveCodeBlockOptions.h"
#include "IntelHex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

UINT CSaveCode::m_uStart= 0;
UINT CSaveCode::m_uEnd= 0xFFFF;
int CSaveCode::m_nInitPos= 0;

/////////////////////////////////////////////////////////////////////////////

extern void AFX_CDECL DDX_HexDec(CDataExchange* pDX, int nIDC, unsigned int &num, bool bWord= true)
{
  HWND hWndCtrl = pDX->PrepareEditCtrl(nIDC);
  TCHAR szT[64];
  if (pDX->m_bSaveAndValidate)
  {
    ::GetWindowText(hWndCtrl, szT, sizeof(szT)/sizeof(szT[0]));
    TCHAR *pText= szT;
    bool bNegative= false;
    if (pText[0] == _T('-'))	// liczba ujemna?
    {
      pText++;
      bNegative = true;
    }
    if (pText[0] == _T('$'))
    {
      if (sscanf(pText + 1, _T("%X"), &num) <= 0)
      {
	AfxMessageBox(IDS_MSG_BAD_DEC_HEX_NUM);
	pDX->Fail();		// throws exception
      }
    }
    else if (pText[0] == _T('0') && (pText[1]==_T('x') || pText[1]==_T('X')))
    {
      if (sscanf(pText + 2, _T("%X"), &num) <= 0)
      {
	AfxMessageBox(IDS_MSG_BAD_DEC_HEX_NUM);
	pDX->Fail();		// throws exception
      }
    }
    else if (sscanf(pText, _T("%u"), &num) <= 0)
    {
      AfxMessageBox(IDS_MSG_BAD_DEC_HEX_NUM);
      pDX->Fail();		// throws exception
    }
    if (bNegative)
      num = 0 - num;
  }
  else
  {
    wsprintf(szT,bWord ? _T("0x%04X") : _T("0x%02X"),num);
    ::SetWindowText(hWndCtrl, szT);
  }
}

/////////////////////////////////////////////////////////////////////////////
// CSaveCode

IMPLEMENT_DYNAMIC(CSaveCode, CFileDialog)

CSaveCode::CSaveCode(LPCTSTR lpszFileName,
		     LPCTSTR lpszFilter, CWnd* pParentWnd) :
  CFileDialog(FALSE, _T(""), lpszFileName,
    OFN_ENABLETEMPLATE | OFN_ENABLEHOOK | OFN_EXPLORER | OFN_NOREADONLYRETURN |
    OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, lpszFilter, pParentWnd)
{
  m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_SAVE_CODE);
  m_ofn.hInstance = AfxGetResourceHandle();
  m_strTitle.LoadString(IDS_SAVE_CODE_DLG);
  m_ofn.lpstrTitle = m_strTitle;
  m_ofn.nFilterIndex = m_nInitPos+1;
}


BEGIN_MESSAGE_MAP(CSaveCode, CFileDialog)
  //{{AFX_MSG_MAP(CSaveCode)
  //}}AFX_MSG_MAP
  ON_COMMAND(IDC_SAVE_CODE_OPT, OnOptions)
END_MESSAGE_MAP()



void CSaveCode::OnOptions()
{
  switch (m_ofn.nFilterIndex - 1)
  {
    case 0:		// format Intel-HEX kodu wynikowego (*.65h/*.hex)
    {
      break;
    }
    case 1:		// format s-rekord Motoroli kodu wynikowego (*.65m/*.s9)
    {
      break;
    }
    case 2:		// obraz binarny kodu wynikowego (*.65b)
    {
      CSaveCodeOptions optDial;
      optDial.m_uStart = m_uStart;
      optDial.m_uEnd = m_uEnd;
      optDial.m_uLength = m_uEnd-m_uStart+1;
      optDial.DoModal();
      m_uStart = optDial.m_uStart;
      m_uEnd = optDial.m_uEnd;
      break;
    }
    case 3:		// program wynikowy (*.65p)
    {
      CSaveCodeBlockOptions optDial;
      optDial.m_uStart = m_uStart;
      optDial.m_uEnd = m_uEnd;
      optDial.m_uLength = m_uEnd-m_uStart+1;
      optDial.DoModal();
      m_uStart = optDial.m_uStart;
      m_uEnd = optDial.m_uEnd;
      break;
    }
  }
}

//-----------------------------------------------------------------------------

void CSaveCode::SaveCode()		// zapisanie kodu wynikowego
{
  m_nInitPos = m_ofn.nFilterIndex - 1;

  CString fileName= GetPathName();
  if (fileName.GetLength() == 0)
    return;
  CFileException exception;
  CFile file;

  if( !file.Open(fileName, CFile::modeCreate | CFile::modeWrite, &exception) )
  {
    CString msg;
    TCHAR buf[256];
    exception.GetErrorMessage(buf,255);
    AfxFormatString2(msg,IDS_SAVE_CODE_ERR_1,fileName,buf);
    AfxMessageBox(msg);
    return;
  }

  m_nPos = m_ofn.nFilterIndex - 1;

  try
  {
//    file.SetLength(0);	// obcinamy koniec, jeœli jest

    CString ext= GetFileExt();
    CString extensions;
    extensions.LoadString(IDS_CODE_EXTENSIONS);
    ext.MakeLower();
    switch (extensions.Find(ext))
    {	// zdeterminowanie typu na podstawie rozszerzenia pliku do zapisu
      case 0:	// 65h
      case 3:	// hex
	m_nPos = 0;
	break;
      case 6:	// 65m
      case 9:	// s9
	m_nPos = 1;
	break;
      case 12:	// 65b
	m_nPos = 2;
	break;
      case 15:	// 65p
	m_nPos = 3;
	break;
    }	// jeœli nierozpoznane rozszerzenie, u¿ywamy typu wybranego w pude³ku dialogowym

    CArchive archive(&file,CArchive::store,1024*8);

    switch (m_nPos)
    {
      case 0:		// format Intel-HEX kodu wynikowego (*.65h/*.hex)
	theApp.m_global.SaveCode(archive,0,0,0);
	break;
      case 1:		// format s-rekord Motoroli kodu wynikowego (*.65m/*.s9)
	theApp.m_global.SaveCode(archive,0,0,1);
	break;
      case 2:		// obraz binarny kodu wynikowego (*.65b)
	theApp.m_global.SaveCode(archive,m_uStart,m_uEnd,2);
	break;
      case 3:		// program wynikowy (*.65p)
	theApp.m_global.SaveCode(archive,m_uStart,m_uEnd,3);
	break;
    }

    archive.Close();
  }
  catch (CException *exception)
  {
    CString msg;
    TCHAR buf[256];
    exception->GetErrorMessage(buf,sizeof(buf));
    AfxFormatString2(msg,IDS_SAVE_CODE_ERR_2,fileName,buf);
    AfxMessageBox(msg);
    try
    {
      CFile::Remove(fileName);
    }
    catch (CFileException *)
    {
    }
    return;
  }
  catch (CIntelHex::CIntelHexException exception)
  {
    CString msg;
    TCHAR buf[256];
    exception.GetErrorMessage(buf,sizeof(buf));
//    AfxFormatString2(msg,IDS_SAVE_CODE_ERR_2,fileName,buf);
    AfxMessageBox(buf);
    try
    {
      CFile::Remove(fileName);
    }
    catch (CFileException *)
    {
    }
    return;
  }
}


void CSaveCode::OnTypeChange()
{
  EnableOptions();
}


void CSaveCode::EnableOptions(bool bRedraw /*= TRUE*/)
{
  CWnd *pBtn= GetDlgItem(IDC_SAVE_CODE_OPT);
  m_nPos = m_ofn.nFilterIndex - 1;
  if (m_nPos < 2)
  {
    if (pBtn->ModifyStyle(0,WS_DISABLED) && bRedraw)
    {
      pBtn->Invalidate();
      pBtn->UpdateWindow();
    }
  }
  else
    if (pBtn->ModifyStyle(WS_DISABLED,0) && bRedraw)
    {
      pBtn->Invalidate();
      pBtn->UpdateWindow();
    }
}


BOOL CSaveCode::OnInitDialog()
{
  CFileDialog::OnInitDialog();

  EnableOptions(FALSE);

  return TRUE;  // return TRUE unless you set the focus to a control
              // EXCEPTION: OCX Property Pages should return FALSE
}
