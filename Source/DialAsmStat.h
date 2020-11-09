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

// DialAsmStat.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDialAsmStat dialog

class CDialAsmStat : public CDialog, public CAsm, CBroadcast
{
  static UINT start_asm_thread(LPVOID pDial);
  UINT StartAsm();
  void SetProgressRange(int max_line);
  void ProgressStep();
  void SetCtrlText(int id, int val);
  void SetLineNo(int val);
  void SetPassNo(int val);

  DWORD m_dwTimer;
//  int m_nUpdateInit;
//  int m_nUpdateDelay;		// opóŸnienie odœwie¿enia inf. o asemblowanym wierszu
//  int m_nUpdateLineDelay;

  Stat m_stAsmRetCode;
  CAsm6502 *m_pAsm6502;
  int m_nPassNo;
  int m_nCurrLine;
  int m_nLines;
  CString m_strRow;
  CString m_strPassNo;
  CSrc6502View *m_pView;
  bool m_bFinished;
  CString m_strText;		// ca³y tekst do asemblacji
  const TCHAR *m_pText;

  CString GetLine(int nLine);
  void GetLine(int nLine, TCHAR *buf, int max_len);
  afx_msg LRESULT OnAbortAsm(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnGetNextLine(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnGetLineNo(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnGetDocTitle(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnNextPass(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnFinished(WPARAM wParam, LPARAM /* lParam */);

  // Construction
public:
  ~CDialAsmStat();
//  CDialAsmStat(CWnd* pParent = NULL);   // standard constructor
  CDialAsmStat(CSrc6502View *pView);	// aktywny dokument (przez 'pView')
  bool Create();
  void SetValues(int row, int pass);

  // Dialog Data
  //{{AFX_DATA(CDialAsmStat)
  enum { IDD = IDD_ASSEMBLE };
  CString   m_strCtrlRow;
  CString   m_strCtrlPassNo;
  //}}AFX_DATA


  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CDialAsmStat)
protected:
  virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
  //}}AFX_VIRTUAL

  // Implementation
protected:
  
  // Generated message map functions
  //{{AFX_MSG(CDialAsmStat)
  virtual BOOL OnInitDialog();
  virtual void OnCancel();
  afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
