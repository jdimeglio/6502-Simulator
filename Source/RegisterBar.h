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

// RegisterBar.h : header file
//

#include "resource.h"
#include "Asm.h"

/////////////////////////////////////////////////////////////////////////////
// CRegisterBar dialog bar

class CContext;

class CRegisterBar : public CDialogBar, public CAsm
{
  bool m_bInUpdate;

  bool UpdateItem(int itemID);

  void UpdateRegA(const CContext *pCtx, const CContext *pOld= NULL);
  void UpdateRegX(const CContext *pCtx, const CContext *pOld= NULL);
  void UpdateRegY(const CContext *pCtx, const CContext *pOld= NULL);
  void UpdateRegP(const CContext *pCtx, const CContext *pOld= NULL);
  void UpdateRegS(const CContext *pCtx, const CContext *pOld= NULL);
  void UpdateRegPC(const CContext *pCtx, const CContext *pOld= NULL);
  void UpdateCycles(ULONG uCycles);
  void ChangeRegister(int ID, int reg_no);
  void ChangeFlags(int flag_bit, bool set);	// zmiana bitu rej. flagowego

  afx_msg LRESULT OnUpdate(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnStartDebug(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnExitDebug(WPARAM wParam, LPARAM lParam);

  virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

  // Construction
public:
  static bool m_bHidden;
  void Update(const CContext *pCtx, const CString &stat, const CContext *pOld= NULL, bool bDraw= TRUE);
  bool Create(CWnd* pParentWnd, UINT nStyle, UINT nID);
  CRegisterBar();	// standard constructor

  // Dialog Data
  //{{AFX_DATA(CRegisterBar)
  enum { IDD = IDD_REGISTERBAR };
	// NOTE: the ClassWizard will add data members here
  //}}AFX_DATA

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CRegisterBar)
  //}}AFX_VIRTUAL
 
  // Implementation
protected:
 
  // Generated message map functions
  //{{AFX_MSG(CRegisterBar)
  afx_msg void OnChangeRegA();
  afx_msg void OnChangeRegX();
  afx_msg void OnChangeRegY();
  afx_msg void OnChangeRegS();
  afx_msg void OnChangeRegP();
  afx_msg void OnChangeRegPC();
  afx_msg void OnRegFlagNeg();
  afx_msg void OnRegFlagCarry();
  afx_msg void OnRegFlagDec();
  afx_msg void OnRegFlagInt();
  afx_msg void OnRegFlagOver();
  afx_msg void OnRegFlagZero();
  afx_msg void OnRegFlagBrk();
  afx_msg void OnWindowPosChanging(WINDOWPOS FAR* lpwndpos);
  afx_msg void OnRegsCyclesClr();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
private:
  CString Binary(UINT8 val);
  void SetDlgItemInf(int nID, UINT8 val);
  void SetDlgItemMem(int nID, int nBytes, UINT16 ptr, const CContext *pCtx);
  void SetDlgItemWordHex(int nID, UINT16 val);
  void SetDlgItemByteHex(int nID, UINT8 val);

};
