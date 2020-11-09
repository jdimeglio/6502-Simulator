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

// 6502.h : main header file for the 6502 application
//

#include "resource.h"
#include "Global.h"

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

//#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// C6502App:
// See 6502.cpp for the implementation of this class
//

class C6502App : public CWinApp
{
  static const TCHAR REGISTRY_KEY[];
  static const TCHAR PROFILE_NAME[];
  HINSTANCE m_hInstRes;
  HMODULE m_hRichEdit;

public:
  static bool m_bMaximize;	// flaga - maksymalne wymiary okna przy starcie;
  static bool m_bFileNew;	// flaga - owieranie pustego dokumentu przy starcie
  CGlobal m_global;		// zmienne globalne aplikacji
  bool m_bDoNotAddToRecentFileList;
  CDocTemplate *m_pDocDeasmTemplate;

  C6502App();

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(C6502App)
public:
  virtual BOOL InitInstance();
  virtual void AddToRecentFileList(LPCTSTR lpszPathName);
  virtual int ExitInstance();
  //}}AFX_VIRTUAL

// Implementation

  //{{AFX_MSG(C6502App)
  afx_msg void OnAppAbout();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};


extern C6502App theApp;

/////////////////////////////////////////////////////////////////////////////
