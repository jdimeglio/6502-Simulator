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

// SaveCode.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSaveCode dialog

class CSaveCode : public CFileDialog
{
  static UINT m_uStart;
  static UINT m_uEnd;
  CString m_strTitle;
  int m_nPos;
  static int m_nInitPos;

  void EnableOptions(bool bRedraw= TRUE);

  DECLARE_DYNAMIC(CSaveCode)

public:
  CSaveCode(LPCTSTR lpszFileName = NULL,
    LPCTSTR lpszFilter = NULL,
    CWnd* pParentWnd = NULL);
  void SaveCode();

protected:
  //{{AFX_MSG(CSaveCode)
  virtual BOOL OnInitDialog();
  //}}AFX_MSG
//  afx_msg LRESULT OnNotify(WPARAM wParam, LPARAM lParam);
  void OnOptions();

  virtual void OnTypeChange();

  DECLARE_MESSAGE_MAP()
};
