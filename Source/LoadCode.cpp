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

// LoadCode.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "LoadCode.h"
#include "LoadCodeOptions.h"
#include <Dlgs.h>
#include "IntelHex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


int CLoadCode::m_nInitPos= 0;

/////////////////////////////////////////////////////////////////////////////
// CLoadCode

IMPLEMENT_DYNAMIC(CLoadCode, CFileDialog)

CLoadCode::CLoadCode(LPCTSTR lpszFileName, LPCTSTR lpszFilter, CWnd* pParentWnd) :
  CFileDialog(TRUE, _T(""), lpszFileName, 
    OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
    lpszFilter, pParentWnd)
{
  m_strTitle.LoadString(IDS_LOAD_CODE_DLG);
  m_ofn.lpstrTitle = m_strTitle;
  m_ofn.nFilterIndex = m_nInitPos+1;
}


BEGIN_MESSAGE_MAP(CLoadCode, CFileDialog)
  //{{AFX_MSG_MAP(CLoadCode)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------

void CLoadCode::LoadCode()		// wczytanie kodu 6502 do symulatora
{
	m_nInitPos = m_ofn.nFilterIndex - 1;

	CString fileName= GetPathName();
	if (fileName.GetLength() == 0)
		return;

	CFileException exception;
	CFile file;

	if (!file.Open(fileName, CFile::modeRead | CFile::shareDenyWrite, &exception))
	{
		CString msg;
		TCHAR buf[256];
		exception.GetErrorMessage(buf,255);
		AfxFormatString2(msg,IDS_LOAD_CODE_ERR_1,fileName,buf);
		AfxMessageBox(msg);
		return;
	}

	try
	{
		CString ext= GetFileExt();
		CString extensions;
		extensions.LoadString(IDS_CODE_EXTENSIONS);
		ext.MakeLower();
		int nPos= extensions.Find(ext);
		switch (nPos >= 0 ? nPos / 4 : -1)
		{	// zdeterminowanie typu na podstawie rozszerzenia pliku do zapisu
		case 0:		// 65h
		case 1:		// hex
			m_nPos = 0;
			break;
		case 2:		// 65m
		case 3:		// s9
			m_nPos = 1;
			break;
		case 4:		// 65b
			m_nPos = 2;
			break;
		case 5:		// 65p
			m_nPos = 3;
			break;
		case 6:		// com
			m_nPos = 4;
			break;
		default:
			m_nPos = m_ofn.nFilterIndex - 1;
			if (m_ofn.nFilterIndex == 6)	// wszystkie pliki (*.*) ?
				m_nPos = 2;
			break;
		}	// jeœli nierozpoznane rozszerzenie, u¿ywamy typu wybranego w pude³ku dialogowym

		CLoadCodeOptions dlg;
		if (m_nPos == 2 && dlg.DoModal() != IDOK)	// podanie adresu jeœli typ 'plik binarny'
			return;
		ASSERT(dlg.m_uStart <= 0xFFFF);

		CArchive archive(&file, CArchive::load, 1024 * 8);

		theApp.m_global.LoadCode(archive, dlg.m_uStart, 0xFFFF, m_nPos, dlg.m_bClearMem ? dlg.m_uFill : -1);

		archive.Close();

		theApp.m_global.CreateDeasm();
	}
	catch (CException *exception)
	{
		CString msg;
		TCHAR buf[256];
		exception->GetErrorMessage(buf,255);
		AfxFormatString2(msg, IDS_LOAD_CODE_ERR_2, fileName, buf);
		AfxMessageBox(msg);
		exception->Delete();
		return;
	}
	catch (CIntelHex::CIntelHexException exception)
	{
		CString msg;
		TCHAR buf[256];
		exception.GetErrorMessage(buf,sizeof(buf));
	//    AfxFormatString2(msg,IDS_SAVE_CODE_ERR_2,fileName,buf);
		AfxMessageBox(buf);
		return;
	}
}

/*
void CLoadCode::OnTypeChange()
{
  CWnd *pWnd= GetParent();
  if (pWnd == NULL)
    return;
  m_nInitPos = m_nPos = pWnd->SendDlgItemMessage(cmb1,CB_GETCURSEL);
}
*/

BOOL CLoadCode::OnInitDialog() 
{
/*
  CWnd *pWnd= GetParent();
  if (pWnd)
    m_nPos = m_nInitPos = pWnd->SendDlgItemMessage(cmb1,CB_SETCURSEL,m_nInitPos);
*/
  CFileDialog::OnInitDialog();

  return TRUE;  // return TRUE unless you set the focus to a control
              // EXCEPTION: OCX Property Pages should return FALSE
}
