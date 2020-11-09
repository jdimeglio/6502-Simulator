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

// 6502Doc.cpp : implementation of the CSrc6502Doc class
//

#include "stdafx.h"
//#include "6502.h"
#include "6502Doc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSrc6502Doc

IMPLEMENT_DYNCREATE(CSrc6502Doc, CDocument)

BEGIN_MESSAGE_MAP(CSrc6502Doc, CDocument)
  //{{AFX_MSG_MAP(CSrc6502Doc)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSrc6502Doc construction/destruction

CSrc6502Doc::CSrc6502Doc()
{
	m_TextBuffer.m_pOwnerDoc = this;
}

CSrc6502Doc::~CSrc6502Doc()
{
#ifdef USE_CRYSTAL_EDIT
//	m_TextBuffer.FreeAll();
#endif
}

BOOL CSrc6502Doc::OnNewDocument()
{
  if (!CDocument::OnNewDocument())
    return FALSE;

#ifdef USE_CRYSTAL_EDIT
	m_TextBuffer.InitNew();
#endif

  static UINT no= 1;
  CString name;
  name.Format(_T("NewFile %u"),no++);
  SetPathName(name,false);

  return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSrc6502Doc serialization

void CSrc6502Doc::Serialize(CArchive& ar)
{
#ifdef USE_CRYSTAL_EDIT
	//no-op
#else
  // CEditView contains an edit control which handles all serialization
  ((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);
#endif
}

/////////////////////////////////////////////////////////////////////////////
// CSrc6502Doc diagnostics

#ifdef _DEBUG
void CSrc6502Doc::AssertValid() const
{
  CDocument::AssertValid();
}

void CSrc6502Doc::Dump(CDumpContext& dc) const
{
  CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////

#ifdef USE_CRYSTAL_EDIT

void CSrc6502Doc::DeleteContents()
{
	CDocument::DeleteContents();
	m_TextBuffer.FreeAll();
}


BOOL CSrc6502Doc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	return m_TextBuffer.LoadFromFile(lpszPathName);
}


BOOL CSrc6502Doc::OnSaveDocument(LPCTSTR lpszPathName)
{
	m_TextBuffer.SaveToFile(lpszPathName);
	return true;
}

#else

void CSrc6502Doc::DeleteContents()
{
	CDocument::DeleteContents();
}


BOOL CSrc6502Doc::OnOpenDocument(LPCTSTR lpszPathName)
{
	return CDocument::OnOpenDocument(lpszPathName);
}


BOOL CSrc6502Doc::OnSaveDocument(LPCTSTR lpszPathName)
{
	return CDocument::OnSaveDocument(lpszPathName);
}

#endif
