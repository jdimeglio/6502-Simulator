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

#include "ColorButton.h"
#include "OptionsViewPage.h"
#include "SyntaxExample.h"

/////////////////////////////////////////////////////////////////////////////
// COptionsSymPage dialog

class COptionsSymPage : public CPropertyPage
{
	DECLARE_DYNCREATE(COptionsSymPage)

// Construction
public:
	COptionsSymPage();
	~COptionsSymPage();

// Dialog Data
	//{{AFX_DATA(COptionsSymPage)
	enum { IDD = IDD_PROPPAGE_SYMULATOR };
	UINT	m_nIOAddress;
	BOOL	m_bIOEnable;
	int	m_nFinish;
	int	m_nWndWidth;
	int	m_nWndHeight;
	BOOL	m_bProtectMemory;
	UINT	m_nProtFromAddr;
	UINT	m_nProtToAddr;
	//}}AFX_DATA

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(COptionsSymPage)
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(COptionsSymPage)
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
// COptionsEditPage dialog

class COptionsEditPage : public CPropertyPage
{
  DECLARE_DYNCREATE(COptionsEditPage)

public:
//  void OnChangeFont();

  // Construction
public:
  COptionsEditPage();
  ~COptionsEditPage();

	// Dialog Data
	//{{AFX_DATA(COptionsEditPage)
	enum { IDD = IDD_PROPPAGE_EDITOR };
	CButton	m_btnBold;
	CColorButton m_btnColor;
	CComboBox	m_wndElement;
	CSyntaxExample m_wndExample;
	BOOL	m_bAutoIndent;
	int	m_nTabStep;
	BOOL	m_bAutoSyntax;
	BOOL	m_bAutoUppercase;
	BOOL	m_bFileNew;
	int		m_nElement;
	//}}AFX_DATA
	bool m_bColorChanged;

	COLORREF* GetColorElement(int nIndex);
	bool* GetFontStyle(int nIndex);

  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsEditPage)
	public:
	virtual BOOL OnSetActive();
	protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsEditPage)
  afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeTabStep();
	afx_msg void OnColorSyntax();
	afx_msg void OnSelChangeElement();
	afx_msg void OnEditColor();
	afx_msg void OnBoldFont();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	COLORREF* GetColorElement();
	bool* GetFontStyle();
};

/////////////////////////////////////////////////////////////////////////////
// COptionsAsmPage dialog

class COptionsAsmPage : public CPropertyPage
{
  DECLARE_DYNCREATE(COptionsAsmPage)
    
    // Construction
public:
  COptionsAsmPage();
  ~COptionsAsmPage();
  
  // Dialog Data
  //{{AFX_DATA(COptionsAsmPage)
  enum { IDD = IDD_PROPPAGE_ASSEMBLY };
  int	m_nCaseSensitive;
  int	m_nAsmInstrWithDot;
  BOOL	m_bGenerateListing;
  CString m_strListingFile;
  BOOL	m_bGenerateBRKExtraByte;
  UINT	m_uBrkExtraByte;
  //}}AFX_DATA


  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsAsmPage)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsAsmPage)
  afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnOptAsmChooseFile();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// COptionsDeasmPage dialog

class COptionsDeasmPage : public CPropertyPage
{
  CColorButton m_ColorButtonAddress;
  CColorButton m_ColorButtonCode;
//  CColorButton m_ColorButtonInstr;
  bool m_bSubclassed;

  DECLARE_DYNCREATE(COptionsDeasmPage)

public:
  COLORREF m_rgbAddress;
  COLORREF m_rgbCode;
//  COLORREF m_rgbInstr;
  bool m_bColorChanged;

  // Construction
public:
  COptionsDeasmPage();
  ~COptionsDeasmPage();
  
  // Dialog Data
  //{{AFX_DATA(COptionsDeasmPage)
  enum { IDD = IDD_PROPPAGE_DEASM };
  BOOL	m_ShowCode;
  //}}AFX_DATA
  
  
  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsDeasmPage)
public:
  virtual BOOL OnSetActive();
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL
  
  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsDeasmPage)
  afx_msg void OnAddrColButton();
  afx_msg void OnCodeColButton();
  afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
    
};


/////////////////////////////////////////////////////////////////////////////
// COptionsMarksPage dialog

class COptionsMarksPage : public CPropertyPage
{
  CColorButton m_ColorButtonPointer;
  CColorButton m_ColorButtonBreakpoint;
  CColorButton m_ColorButtonError;
  bool m_bSubclassed;

  DECLARE_DYNCREATE(COptionsMarksPage)

public:
  COLORREF m_rgbPointer;
  COLORREF m_rgbBreakpoint;
  COLORREF m_rgbError;
  bool m_bColorChanged;
  bool m_bFontChanged;
  LOGFONT m_LogFont;

  // Construction
public:
  COptionsMarksPage();
  ~COptionsMarksPage();
  
  // Dialog Data
  //{{AFX_DATA(COptionsMarksPage)
  enum { IDD = IDD_PROPPAGE_GENERAL };
  int m_nProc6502;
  UINT	m_uBusWidth;
  //}}AFX_DATA


  // Overrides
  // ClassWizard generate virtual function overrides
  //{{AFX_VIRTUAL(COptionsMarksPage)
public:
  virtual BOOL OnSetActive();
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  // Generated message map functions
  //{{AFX_MSG(COptionsMarksPage)
  afx_msg void OnBrkpColButton();
  afx_msg void OnErrColButton();
  afx_msg void OnPtrColButton();
  afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  afx_msg void OnOptFontBtn();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()

};


/////////////////////////////////////////////////////////////////////////////
// COptions

class COptions : public CPropertySheet
{
  int m_nLastActivePageIndex;

  static int CALLBACK PropSheetProc(HWND hwndDlg, UINT uMsg, LPARAM lParam);
//  static LRESULT CALLBACK EditDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
//  static LRESULT (CALLBACK *m_pfnOldProc)(HWND,UINT,WPARAM,LPARAM);
//  static LRESULT CALLBACK ClassDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
//  static LRESULT (CALLBACK *m_pfnOldClassProc)(HWND,UINT,WPARAM,LPARAM);

  DECLARE_DYNAMIC(COptions)

  enum { ID = IDS_OPTIONS };
  // Construction
public:
  COptions(CWnd* pParentWnd, UINT iSelectPage= 0);
//  COptions(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
//  COptions(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

  // Attributes
public:
  static DWORD m_arrIds[];
  COptionsAsmPage m_AsmPage;
  COptionsEditPage m_EditPage;
  COptionsSymPage m_SymPage;
  COptionsDeasmPage m_DeasmPage;
  COptionsMarksPage m_MarksPage;
  COptionsViewPage m_ViewPage;

  // Operations
public:
  int GetLastActivePage();

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(COptions)
protected:
  virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
  //}}AFX_VIRTUAL

  // Implementation
public:
  virtual ~COptions();

  // Generated message map functions
protected:
  //{{AFX_MSG(COptions)
  afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
  afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
