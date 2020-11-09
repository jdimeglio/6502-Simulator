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

// CG: This file was added by the Splash Screen component.
// Splash.cpp : implementation file
//

#include "stdafx.h"  // e. g. stdafx.h
#include "resource.h"  // e.g. resource.h

#include "Splash.h"  // e.g. splash.h

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
//   Splash Screen class

bool CSplashWnd::c_bShowSplashWnd;
CSplashWnd* CSplashWnd::c_pSplashWnd;
CSplashWnd::CSplashWnd()
{
  m_hdib = NULL;
}


CSplashWnd::~CSplashWnd()
{
  // Clear the static window pointer.
  ASSERT(c_pSplashWnd == this);
  c_pSplashWnd = NULL;
  if (m_hdib)
    DeleteObject(m_hdib);
}


BEGIN_MESSAGE_MAP(CSplashWnd, CWnd)
//{{AFX_MSG_MAP(CSplashWnd)
ON_WM_CREATE()
ON_WM_PAINT()
ON_WM_TIMER()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CSplashWnd::EnableSplashScreen(bool bEnable /*= TRUE*/)
{
  c_bShowSplashWnd = bEnable;
}


void CSplashWnd::ShowSplashScreen(CWnd* pParentWnd /*= NULL*/)
{
  if (!c_bShowSplashWnd || c_pSplashWnd != NULL)
    return;
  
  // Allocate a new splash screen, and create the window.
  c_pSplashWnd = new CSplashWnd;
  if (!c_pSplashWnd->Create(pParentWnd))
    delete c_pSplashWnd;
  else
    c_pSplashWnd->UpdateWindow();
}


void CSplashWnd::PreTranslateAppMessage(MSG* pMsg)
{
  if (c_pSplashWnd == NULL)
    return;
  
  // If we get a keyboard or mouse message, hide the splash screen.
  if (pMsg->message == WM_KEYDOWN ||
    pMsg->message == WM_SYSKEYDOWN ||
    pMsg->message == WM_LBUTTONDOWN ||
    pMsg->message == WM_RBUTTONDOWN ||
    pMsg->message == WM_MBUTTONDOWN ||
    pMsg->message == WM_NCLBUTTONDOWN ||
    pMsg->message == WM_NCRBUTTONDOWN ||
    pMsg->message == WM_NCMBUTTONDOWN)
  {
    c_pSplashWnd->HideSplashScreen();
  }
}


bool CSplashWnd::Create(CWnd* pParentWnd /*= NULL*/)
{
#ifdef _DEBUG
	return false;
#endif
  HDC hdc= ::GetWindowDC(HWND_DESKTOP);
  int nColors= ::GetDeviceCaps(hdc,NUMCOLORS);
  ::ReleaseDC(HWND_DESKTOP,hdc);

  m_hdib = LoadImage(		// za³adowanie obrazka i palety kolorów
    AfxGetResourceHandle(),	// handle of the instance that contains the image
    (LPCTSTR)MAKEINTRESOURCE(nColors==-1 ? IDB_SPLASH : IDB_SPLASH16),
    IMAGE_BITMAP,		// type of image
    0,				// desired width
    0,				// desired height
    LR_CREATEDIBSECTION		// load flags (tutaj: format DIB)
   );

  if (m_hdib == NULL)
    return FALSE;
//  if (!m_bitmap.LoadBitmap(nColors==-1 ? IDB_SPLASH : IDB_SPLASH16))
//    return FALSE;

  BITMAP bm;
  ::GetObject(m_hdib, sizeof(BITMAP), &bm);
//  m_bitmap.GetBitmap(&bm);
  
  return CreateEx(WS_EX_TOPMOST,
    AfxRegisterWndClass(0, AfxGetApp()->LoadStandardCursor(IDC_ARROW)),
    NULL, WS_POPUP | WS_VISIBLE, 0, 0, bm.bmWidth, bm.bmHeight,
    pParentWnd ? pParentWnd->GetSafeHwnd() : HWND_DESKTOP, NULL);
}


void CSplashWnd::HideSplashScreen()
{
  // Destroy the window, and update the mainframe.
  DestroyWindow();
  CWnd *pWnd= AfxGetMainWnd();
  if (pWnd)
    pWnd->UpdateWindow();
}


void CSplashWnd::PostNcDestroy()
{
  // Free the C++ class.
  delete this;
}


int CSplashWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CWnd::OnCreate(lpCreateStruct) == -1)
    return -1;

  // Center the window.
  CenterWindow();

  // Set a timer to destroy the splash screen.
  SetTimer(1, 750, NULL);	// 0.75 sekundy

  return 0;
}


void CSplashWnd::OnPaint()
{
  CPaintDC dc(this);

  CDC dcImage;
  if (!dcImage.CreateCompatibleDC(&dc))
    return;

  BITMAP bm;
  ::GetObject(m_hdib, sizeof(BITMAP), &bm);

  // Paint the image.
  HGDIOBJ hOldBitmap = SelectObject(dcImage.GetSafeHdc(),(HGDIOBJ)m_hdib);

  int nColors= dc.GetDeviceCaps(NUMCOLORS);	// iloœæ wpisów w palecie kolorów
  if (nColors != -1)
  {
    RGBQUAD rgbColors[16];
    BYTE buf[sizeof(LOGPALETTE)+16*sizeof(PALETTEENTRY)];
    LOGPALETTE *pLogPalette= (LOGPALETTE *)buf;
    pLogPalette->palVersion = 0x300;
    pLogPalette->palNumEntries = 16;
    PALETTEENTRY *pEntry= (PALETTEENTRY *)(pLogPalette+1);
    CPalette Palette;
    UINT ret= GetDIBColorTable(dcImage.GetSafeHdc(),0,16,rgbColors);
    if (ret)
    {
      for (int i=0; i<16; i++)
      {
	pEntry[i].peRed = rgbColors[i].rgbRed;
	pEntry[i].peGreen = rgbColors[i].rgbGreen;
	pEntry[i].peBlue = rgbColors[i].rgbBlue;
	pEntry[i].peFlags = 0; //PC_NOCOLLAPSE;
      }
      Palette.CreatePalette(pLogPalette);
      dc.SelectPalette(&Palette,FALSE);
      dc.RealizePalette();
    }
  }

  dc.BitBlt(0, 0, bm.bmWidth, bm.bmHeight, &dcImage, 0, 0, SRCCOPY);
  SelectObject(dcImage.GetSafeHdc(),hOldBitmap);
}


void CSplashWnd::OnTimer(UINT nIDEvent)
{
  // Destroy the splash screen window.
  HideSplashScreen();
}
