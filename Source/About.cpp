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

#include "StdAfx.h"
#include "About.h"



CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
  //{{AFX_DATA_INIT(CAboutDlg)
  m_strVersion = _T("");
  //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CAboutDlg)
  DDX_Control(pDX, IDC_6502, m_wndTitle);
  DDX_Text(pDX, IDC_ABOUT_VER, m_strVersion);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
  //{{AFX_MSG_MAP(CAboutDlg)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()



BOOL CAboutDlg::OnInitDialog()
{
  HRSRC hRsrc= ::FindResource(AfxGetInstanceHandle(),MAKEINTRESOURCE(VS_VERSION_INFO),RT_VERSION);
  HGLOBAL hGlobal;
  if ( hRsrc && (hGlobal = ::LoadResource(AfxGetInstanceHandle(),hRsrc)) != NULL )
  {
    VS_FIXEDFILEINFO *pVer= (VS_FIXEDFILEINFO *)( (char *)::LockResource(hGlobal) + 0x28 );
    if (pVer->dwSignature == 0xfeef04bd)
      m_strVersion.Format( IDS_ABOUT_VER,
        (int)HIWORD(pVer->dwProductVersionMS),
	(int)LOWORD(pVer->dwProductVersionMS),
	(int)HIWORD(pVer->dwProductVersionLS) );
//	(int)LOWORD(pVer->dwProductVersionLS) );

    ::FreeResource(hGlobal);
  }

  CDialog::OnInitDialog();

  LOGFONT lf;
  m_wndTitle.GetFont()->GetLogFont(&lf);

  GetFont()->GetLogFont(&lf);
//  CClientDC dc(this);
//    lf.lfHeight = -MulDiv(9, dc.GetDeviceCaps(LOGPIXELSY), 96);
  lf.lfWeight = 700;      // bold
  lf.lfHeight -= 2;
  m_TitleFont.CreateFontIndirect(&lf);
  m_wndTitle.SetFont(&m_TitleFont);

  m_StaticLink.SubclassDlgItem(IDC_LINK, this, "http://home.pacbell.net/michal_k/");

  return true;	// return true unless you set the focus to a control
		// EXCEPTION: OCX Property Pages should return false
}

/*
void CAboutDlg::OnPaint() 
{
  CPaintDC dc(this); // device context for painting

  RECT rectWindow;
  POINT org;
  DWORD dbu= ::GetDialogBaseUnits();
  CWnd *pFrm = GetDlgItem(IDC_TEST);
  pFrm->GetWindowRect(&rectWindow);
  ScreenToClient(&rectWindow);
  org.x = (rectWindow.left * LOWORD(dbu)) / 4;
  org.y = (rectWindow.top * HIWORD(dbu)) / 8;

  CBitmap bmp, mask;
  bmp.LoadBitmap(IDB_BITMAP1);
  mask.LoadBitmap(IDB_BITMAP2);

  CDC dcComp;
  dcComp.CreateCompatibleDC(&dc);


  dcComp.SelectObject(&mask);
  BITMAP info;
  bmp.GetObject(sizeof(info),&info);
  dc.BitBlt(1,1,info.bmWidth,info.bmHeight,&dcComp,0,0,SRCAND);
  dcComp.SelectObject(&bmp);
  dc.BitBlt(1,1,info.bmWidth,info.bmHeight,&dcComp,0,0,SRCPAINT);

  // Do not call CDialog::OnPaint() for painting messages
}
*/
