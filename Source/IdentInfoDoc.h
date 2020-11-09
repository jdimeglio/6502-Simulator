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

// IdentInfoDoc.h : header file
//
#include "DebugInfo.h"

/////////////////////////////////////////////////////////////////////////////
// CIdentInfoDoc document

class CIdentInfoDoc : public CDocument
{
  CDebugInfo *m_pDebug;

public:
  void GetIdent(int index, CString &str, CIdent &info)
  {
    ASSERT(m_pDebug != NULL);
    m_pDebug->GetIdent(index,str,info);
  }
  int GetIdentCount()
  {
    ASSERT(m_pDebug != NULL);
    return m_pDebug->GetIdentCount();
  }
//protected:
//  CIdentInfoDoc();           // protected constructor used by dynamic creation
  DECLARE_DYNCREATE(CIdentInfoDoc)

public:
//  CIdentInfoDoc(CDebugInfo *pDebug);
  void SetDebugInfo(CDebugInfo *pDebug)
  { m_pDebug = pDebug; }

  // Attributes
public:

  // Operations
public:

  // Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CIdentInfoDoc)
public:
  virtual void Serialize(CArchive& ar);   // overridden for document i/o
protected:
  virtual BOOL OnNewDocument();
  //}}AFX_VIRTUAL

  // Implementation
public:
  CIdentInfoDoc();
  virtual ~CIdentInfoDoc();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

  // Generated message map functions
protected:
  //{{AFX_MSG(CIdentInfoDoc)
    // NOTE - the ClassWizard will add and remove member functions here.
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};
