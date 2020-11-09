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

// 6502.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
//#include "6502.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "ChildFrmDeAsm.h"
#include "6502Doc.h"
#include "6502View.h"
#include "Deasm6502Doc.h"
#include "Deasm6502View.h"
#include "Splash.h"
#include "CXMultiDocTemplate.h"
#include "winver.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const TCHAR C6502App::REGISTRY_KEY[]= _T("MiKSoft");
const TCHAR C6502App::PROFILE_NAME[]= _T("6502 Simulator\\1.2");

/////////////////////////////////////////////////////////////////////////////
// C6502App

BEGIN_MESSAGE_MAP(C6502App, CWinApp)
  //{{AFX_MSG_MAP(C6502App)
  ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
  //}}AFX_MSG_MAP
  // Standard file based document commands
  ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
  ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
  // Standard print setup command
  ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// C6502App construction

C6502App::C6502App()
{
  // TODO: add construction code here,
  // Place all significant initialization in InitInstance
  m_bDoNotAddToRecentFileList = false;
  m_hInstRes = NULL;
  m_hRichEdit = 0;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only C6502App object

C6502App theApp;

/////////////////////////////////////////////////////////////////////////////
// C6502App initialization

bool C6502App::m_bMaximize= false;	// flaga - maksymalne wymiary okna przy starcie;
bool C6502App::m_bFileNew= true;	// flaga - owieranie pustego dokumentu przy starcie

BOOL C6502App::InitInstance()
{
  // wczytanie zasobów
  m_hInstRes = LoadLibrary("Res.dll");
  if (m_hInstRes == NULL)
  {
    AfxMessageBox("Can't find resource DLL 'Res.dll'.", MB_OK | MB_ICONERROR);
    return false;
  }
  AfxSetResourceHandle(m_hInstRes);

  m_hRichEdit = ::LoadLibrary(_T("riched20.dll"));
  AfxInitRichEdit();

  // CG: The following block was added by the Splash Screen component.
  CCommandLineInfo cmdInfo;
  ParseCommandLine(cmdInfo);
  CSplashWnd::EnableSplashScreen(cmdInfo.m_bShowSplash);

  CSplashWnd::ShowSplashScreen();	// przeniesione z CMainFrame::OnCreate

  // Standard initialization
  // If you are not using these features and wish to reduce the size
  //  of your final executable, you should remove from the following
  //  the specific initialization routines you do not need.

	INITCOMMONCONTROLSEX init;
	init.dwSize = sizeof init;
	init.dwICC = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_USEREX_CLASSES;
	::InitCommonControlsEx(&init);

#ifdef _AFXDLL
  Enable3dControls();			// Call this when using MFC in a shared DLL
#else
//  Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

  SetRegistryKey(REGISTRY_KEY);
  //First free the string allocated by MFC at CWinApp startup.
  //The string is allocated before InitInstance is called.
  free((void*)m_pszProfileName);
  //Change the name of the .INI file.
  //The CWinApp destructor will free the memory.
  m_pszProfileName=_tcsdup(PROFILE_NAME);
  //m_pszProfileName = PROFILE_NAME;

  LoadStdProfileSettings(_AFX_MRU_MAX_COUNT);  // Load standard INI file options (including MRU)

  // Register the application's document templates.  Document templates
  //  serve as the connection between documents, frame windows and views.

	CXMultiDocTemplate* pDocTemplate;
	pDocTemplate = new CXMultiDocTemplate(
		IDR_SRC65TYPE,
		RUNTIME_CLASS(CSrc6502Doc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(CSrc6502View));

	AddDocTemplate(pDocTemplate);

	pDocTemplate = new CXMultiDocTemplate(
		IDR_DEASM65TYPE,
		RUNTIME_CLASS(CDeasm6502Doc),
		RUNTIME_CLASS(CChildFrameDeAsm), // custom MDI child frame
		RUNTIME_CLASS(CDeasm6502View),
		false);

	AddDocTemplate(pDocTemplate);
	m_pDocDeasmTemplate = pDocTemplate;

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return false;
	m_pMainWnd = pMainFrame;

	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();

	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(true);

	CXMultiDocTemplate::s_bRegistrationExt = false;

  // Parse command line for standard shell commands, DDE, file open
//  CCommandLineInfo cmdInfo;
//  ParseCommandLine(cmdInfo);

  if (!m_bFileNew && cmdInfo.m_nShellCommand == CCommandLineInfo::FileNew)
    cmdInfo.m_nShellCommand = CCommandLineInfo::FileNothing;
  // Dispatch commands specified on the command line
  if (!ProcessShellCommand(cmdInfo))
    return false;

  if (m_bMaximize && m_nCmdShow==SW_SHOWNORMAL)
    m_nCmdShow = SW_MAXIMIZE;

  // The main window has been initialized, so show and update it.
  pMainFrame->ShowWindow(m_nCmdShow);
  pMainFrame->UpdateWindow();
  pMainFrame->SetFocus();	// przywracamy fokus - zamiast na DialBar, do g³ównego okna

  return true;
}

/////////////////////////////////////////////////////////////////////////////
// C6502App commands

void C6502App::AddToRecentFileList(LPCTSTR lpszPathName)
{
  if (m_bDoNotAddToRecentFileList)
    return;

  CWinApp::AddToRecentFileList(lpszPathName);
}


#include "About.h"

// App command to run the dialog
void C6502App::OnAppAbout()
{
  CAboutDlg aboutDlg;
  aboutDlg.DoModal();
}


int C6502App::ExitInstance() 
{
	if (m_hInstRes != NULL)
		::FreeLibrary(m_hInstRes);

	if (m_hRichEdit)
		::FreeLibrary(m_hRichEdit);

	return CWinApp::ExitInstance();
}
