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

// 6502Doc.h : interface of the CSrc6502Doc class
//
/////////////////////////////////////////////////////////////////////////////
#ifdef USE_CRYSTAL_EDIT
	#include "CCrystalTextBuffer.h"
#endif


class CSrc6502Doc : public CDocument
{
protected: // create from serialization only
  CSrc6502Doc();
  DECLARE_DYNCREATE(CSrc6502Doc)

// Attributes
public:
#ifdef USE_CRYSTAL_EDIT
	class CTextBuffer : public CCrystalTextBuffer
	{
	public:
		CDocument* m_pOwnerDoc;

		CTextBuffer() { m_pOwnerDoc = 0; };

		virtual void SetModified(BOOL bModified= TRUE)
		{ m_pOwnerDoc->SetModifiedFlag(bModified); };
	};

	CTextBuffer m_TextBuffer;
	LOGFONT m_lfNormal;

	void GetText(CString& strText)		{ m_TextBuffer.GetText(strText); }

	CCrystalTextBuffer* GetBuffer()		{ return &m_TextBuffer; }
#endif

// Operations
public:
// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CSrc6502Doc)
	public:
  virtual BOOL OnNewDocument();
  virtual void Serialize(CArchive& ar);
	virtual void DeleteContents();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	//}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CSrc6502Doc();
#ifdef _DEBUG
  virtual void AssertValid() const;
  virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
  //{{AFX_MSG(CSrc6502Doc)
	// NOTE - the ClassWizard will add and remove member functions here.
	//    DO NOT EDIT what you see in these blocks of generated code !
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////
