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

// Klasa wspomagaj¹ce wyœwietlanie okna (CListView)
// wyœwietlaj¹cego zdefiniowane etykiety (po asemblacji)

#include "IdentInfoDoc.h"
#include "IdentInfoFrame.h"
#include "IdentInfoView.h"
#include "resource.h"

class CIdentInfo : public CIdentInfoFrame
{
  CDebugInfo *m_pDebugInfo;

  static bool m_bRegistered;
  static CString m_strClass;
  static void RegisterWndClass();
  void init();
  enum { ID_TITLE = IDS_IDENT_TITLE };

  CIdentInfoDoc m_doc;
public:
  static CRect m_WndRect;

  CIdentInfo() : m_pDebugInfo(NULL)
  { init(); }
  CIdentInfo(CDebugInfo *pDebugInfo);
  ~CIdentInfo();

  void SetDebugInfo(CDebugInfo *pDebugInfo)
  { m_doc.SetDebugInfo(pDebugInfo); }

  bool Create(CDebugInfo *pDebugInfo= NULL);

  virtual void PostNcDestroy();
  // Generated message map functions
protected:
  //{{AFX_MSG(CIdentInfo)
  afx_msg void OnDestroy();
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
