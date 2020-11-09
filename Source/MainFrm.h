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

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "DialBar.h"
#include "ToolBox.h"
#include "RegisterBar.h"
#include "IdentInfo.h"
#include "MemoryInfo.h"
#include "IOWindow.h"
#include "ConfigSettings.h"
#include "Broadcast.h"
#include "FlatBar.h"
#include "LogWindow.h"
#include "DynamicHelp.h"

class CSrc6502Doc;
class CSrc6502View;

class CMainFrame : public CMDIFrameWnd, virtual CBroadcast, CConfigSettings
{
	static const TCHAR REG_ENTRY_LAYOUT[];
	static const TCHAR REG_ENTRY_MAINFRM[];
	static const TCHAR REG_POSX[], REG_POSY[], REG_SIZX[], REG_SIZY[], REG_STATE[];

	static WNDPROC m_pfnOldProc;
	static LRESULT CALLBACK StatusBarWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static CBitmap m_bmpCode;
	static CBitmap m_bmpDebug;

	CString m_strFormat;	// znaki formatuj¹ce do ust. wiersza/kolumny w pasku stanu

//  enum { WND_REG, WND_IO, WND_MEM, WND_ID, WND_ZPG } Windows;

	afx_msg LRESULT OnUpdateState(WPARAM wParam, LPARAM lParam);

	int RedrawAllViews(int chgHint= 0);
	int Options(int page);
	int m_nLastPage;

	void ConfigSettings(bool load);
	void ExitDebugMode();

	DECLARE_DYNAMIC(CMainFrame)

	UINT m_uTimer;
public:
//  virtual HMENU GetWindowMenuPopup(HMENU hMenuBar);

//  void ShowRegisterBar(bool bShow= TRUE);
	void SetPositionText(int row, int col);
//  void SetRowColumn(CEdit &edit);
	CSrc6502View *GetCurrentView();
	CSrc6502Doc *GetCurrentDocument();
	CMainFrame();

// Attributes
public:
	CRegisterBar m_wndRegisterBar;
	CIOWindow m_IOWindow;
	CMemoryInfo m_Memory;
	CMemoryInfo m_ZeroPage;
	CMemoryInfo m_Stack;
	CIdentInfo m_Idents;
	CLogWindow m_wndLog;
	CDynamicHelp m_wndHelpBar;

// Operations
public:
	static const HWND * /*const*/ m_hWindows[];

	void UpdateAll();
	void DelayedUpdateAll();

	void ShowDynamicHelp(const CString& strLine, int nWordStart, int nWordEnd);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

public:
	CStatusBar m_wndStatusBar;
protected:  // control bar embedded members
//  CDialBar m_wndZeroPageBar;
//  CToolBar m_wndToolBar;
	CFlatToolBar m_wndToolBar;

	void SymGenInterrupt(CSym6502::IntType eInt);
	void UpdateSymGenInterrupt(CCmdUI* pCmdUI);
	void StopIntGenerator();
	void StartIntGenerator();

// Generated message map functions
protected:
  //{{AFX_MSG(CMainFrame)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnClose();
  afx_msg void OnAssemble();
  afx_msg void OnUpdateAssemble(CCmdUI* pCmdUI);
  afx_msg void OnUpdateSymDebug(CCmdUI* pCmdUI);
  afx_msg void OnSymDebug();
  afx_msg void OnSymStepInto();
  afx_msg void OnUpdateSymStepInto(CCmdUI* pCmdUI);
  afx_msg void OnSymSkipInstr();
  afx_msg void OnUpdateSymSkipInstr(CCmdUI* pCmdUI);
  afx_msg void OnSymBreakpoint();
  afx_msg void OnUpdateSymBreakpoint(CCmdUI* pCmdUI);
  afx_msg void OnSymBreak();
  afx_msg void OnUpdateSymBreak(CCmdUI* pCmdUI);
  afx_msg void OnSymGo();
  afx_msg void OnUpdateSymGo(CCmdUI* pCmdUI);
  afx_msg void OnOptions();
  afx_msg void OnUpdateOptions(CCmdUI* pCmdUI);
  afx_msg void OnSymGoToLine();
  afx_msg void OnUpdateSymGoToLine(CCmdUI* pCmdUI);
  afx_msg void OnSymSkipToLine();
  afx_msg void OnUpdateSymSkipToLine(CCmdUI* pCmdUI);
  afx_msg void OnSymGoToRts();
  afx_msg void OnUpdateSymGoToRts(CCmdUI* pCmdUI);
  afx_msg void OnSymStepOver();
  afx_msg void OnUpdateSymStepOver(CCmdUI* pCmdUI);
  afx_msg void OnSymEditBreakpoint();
  afx_msg void OnUpdateSymEditBreakpoint(CCmdUI* pCmdUI);
  afx_msg void OnSymRestart();
  afx_msg void OnUpdateSymRestart(CCmdUI* pCmdUI);
  afx_msg void OnSymAnimate();
  afx_msg void OnUpdateSymAnimate(CCmdUI* pCmdUI);
  afx_msg void OnUpdateIdViewRegisterbar(CCmdUI* pCmdUI);
  afx_msg void OnFileSaveCode();
  afx_msg void OnUpdateFileSaveCode(CCmdUI* pCmdUI);
  afx_msg void OnViewDeasm();
  afx_msg void OnUpdateViewDeasm(CCmdUI* pCmdUI);
  afx_msg void OnViewIdents();
  afx_msg void OnUpdateViewIdents(CCmdUI* pCmdUI);
  afx_msg void OnViewMemory();
  afx_msg void OnUpdateViewMemory(CCmdUI* pCmdUI);
  afx_msg void OnEditorOpt();
  afx_msg void OnUpdateEditorOpt(CCmdUI* pCmdUI);
  afx_msg void OnViewIOWindow();
  afx_msg void OnUpdateViewIOWindow(CCmdUI* pCmdUI);
  afx_msg void OnDestroy();
  afx_msg void OnFileLoadCode();
  afx_msg void OnUpdateFileLoadCode(CCmdUI* pCmdUI);
  afx_msg void OnDeasmOptions();
  afx_msg void OnUpdateDeasmOptions(CCmdUI* pCmdUI);
  afx_msg void OnViewRegisterWnd();
  afx_msg void OnSysColorChange();
  afx_msg void OnUpdateViewZeropage(CCmdUI* pCmdUI);
  afx_msg void OnViewZeropage();
  afx_msg void OnUpdateMemoryOptions(CCmdUI* pCmdUI);
  afx_msg void OnMemoryOptions();
  afx_msg void OnTimer(UINT nIDEvent);
  afx_msg void OnViewStack();
  afx_msg void OnUpdateViewStack(CCmdUI* pCmdUI);
	afx_msg void OnSymGenIRQ();
	afx_msg void OnUpdateSymGenIRG(CCmdUI* pCmdUI);
	afx_msg void OnSymGenNMI();
	afx_msg void OnUpdateSymGenNMI(CCmdUI* pCmdUI);
	afx_msg void OnSymGenReset();
	afx_msg void OnUpdateSymGenReset(CCmdUI* pCmdUI);
	afx_msg void OnSymGenIntDlg();
	afx_msg void OnUpdateSymGenIntDlg(CCmdUI* pCmdUI);
	afx_msg void OnViewLog();
	afx_msg void OnUpdateViewLog(CCmdUI* pCmdUI);
	afx_msg void OnHelpDynamic();
	afx_msg void OnUpdateHelpDynamic(CCmdUI* pCmdUI);
	//}}AFX_MSG
  afx_msg void OnSymDebugStop();
  afx_msg LRESULT OnStartDebugger(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnExitDebugger(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnChangeCode(WPARAM wParam, LPARAM lParam);
  DECLARE_MESSAGE_MAP()

private:
	void EnableDockingEx(DWORD dwDockStyle);
	static const DWORD dwDockBarMapEx[4][2];

	void AddBreakpoint(CSrc6502View* pView, int nLine, CAsm::Breakpoint bp);
	void RemoveBreakpoint(CSrc6502View* pView, int nLine);
};

/////////////////////////////////////////////////////////////////////////////
