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

// DialAsmStat.cpp : implementation file
//

#include "stdafx.h"
//#include "6502.h"
#include "MainFrm.h"
#include "DialAsmStat.h"
#include "Broadcast.h"
#include "6502View.h"
#include "6502Doc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDialAsmStat dialog


CDialAsmStat::CDialAsmStat(CSrc6502View *pView) :
  CDialog(CDialAsmStat::IDD), m_pView(pView)
{
  ASSERT(pView != NULL);
  ASSERT(pView->GetDocument());		  // musi byæ do³¹czony dokument
  //{{AFX_DATA_INIT(CDialAsmStat)
  m_strCtrlRow = _T("");
  m_strCtrlPassNo = _T("");
  //}}AFX_DATA_INIT

  m_nLines = m_pView->GetLineCount();
  m_nCurrLine = 0;
  m_nPassNo = 0;
  m_pAsm6502 = NULL;
}


void CDialAsmStat::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CDialAsmStat)
  DDX_Text(pDX, IDC_CURR_ROW, m_strCtrlRow);
  DDX_Text(pDX, IDC_PASS_NO, m_strCtrlPassNo);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialAsmStat, CDialog)
  //{{AFX_MSG_MAP(CDialAsmStat)
  ON_WM_SETCURSOR()
  //}}AFX_MSG_MAP
  ON_MESSAGE(WM_USER_ABORT_ASM, OnAbortAsm)
  ON_MESSAGE(WM_USER_GET_NEXT_LINE, OnGetNextLine)
  ON_MESSAGE(WM_USER_GET_LINE_NO, OnGetLineNo)
  ON_MESSAGE(WM_USER_GET_TITLE, OnGetDocTitle)
  ON_MESSAGE(WM_USER_NEXT_PASS, OnNextPass)
  ON_MESSAGE(WM_USER_FIN, OnFinished)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDialAsmStat message handlers

void CDialAsmStat::SetValues(int row, int pass)
{
  m_strCtrlRow.Format(_T("%d"),row);
  m_strCtrlPassNo.Format(_T("%d"),pass);
  UpdateData();
  GetDlgItem(IDC_CURR_ROW)->Invalidate();
  GetDlgItem(IDC_PASS_NO)->Invalidate();
}


bool CDialAsmStat::Create()
{
  return CDialog::Create(IDD);
}


CDialAsmStat::~CDialAsmStat()
{
  DestroyWindow();
  if (m_pAsm6502)
    delete m_pAsm6502;
}


BOOL CDialAsmStat::OnInitDialog() 
{
  CDialog::OnInitDialog();

  try
  {
    m_pView->GetText(m_strText);
    m_pText = m_strText;

    SetProgressRange(m_nLines);
    m_dwTimer= ::GetTickCount();

    m_stAsmRetCode = CAsm::OK;
    m_bFinished = FALSE;

    m_pAsm6502 = new CAsm6502(this, theApp.m_global.GetMemForAsm(),
      theApp.m_global.GetDebug(), theApp.m_global.GetMarkArea(),
      theApp.m_global.GetProcType(),
      theApp.m_global.m_bGenerateListing && !theApp.m_global.m_strListingFile.IsEmpty() ?
      (const TCHAR *)(theApp.m_global.m_strListingFile) : NULL);
    if (AfxBeginThread(CDialAsmStat::start_asm_thread,this) == NULL)
    {
      AfxMessageBox(IDS_ERR_ASM_THREAD);
      EndDialog(-1);
      return TRUE;
    }
  }
  catch (CException *ex)
  {
    ex->ReportError();
  }

  return TRUE;  // return TRUE unless you set the focus to a control
  // EXCEPTION: OCX Property Pages should return FALSE
}


CString CDialAsmStat::GetLine(int nLine)
{
#ifdef USE_CRYSTAL_EDIT
	//TODO
	return "";
#else
  CString buff;
  int chr_index= m_pView->GetEditCtrl().LineIndex(nLine);
  if (chr_index == -1)
    return buff;
  int len= m_pView->GetEditCtrl().LineLength(chr_index) + 1;
  TCHAR *ptr= buff.GetBuffer(len+1);		// przydzielenie miejsca na 'len' znaków w 'buff'
  m_pView->GetEditCtrl().GetLine(nLine,ptr,len);
  ptr[len-1] = _T('\n');			// do³¹czenie znaku koñca wiersza
  buff.ReleaseBuffer(len);
  return buff;
#endif
}


void CDialAsmStat::GetLine(int nLine, TCHAR *buf, int max_len)
{
  if (m_pText == NULL)
  {
    buf[0] = 0;
    return;
  }
  const TCHAR *ptr= _tcschr(m_pText,0xD);
  if (ptr >= m_pText)
  {
    int len= min(ptr+1 - m_pText, max_len-1);
    memcpy(buf,m_pText,len*sizeof(TCHAR));
    buf[len] = 0;
    m_pText = ptr+1+1;		// ominiêcie 0D 0A
  }
  else		// ostatni wiersz
  {
    int len= min(int(_tcslen(m_pText)), max_len-1);
    memcpy(buf,m_pText,len*sizeof(TCHAR));
    buf[len++] = _T('\n');
    buf[len] = 0;
    m_pText = NULL;
  }
/*
  int chr_index= m_edit.LineIndex(nLine);
  if (chr_index == -1)
  {
    *buf = _T('\0');
    return;
  }
  int len= m_edit.GetLine(nLine,buf,max_len);
  len = min(len,max_len);
  buf[len++] = _T('\n');
  buf[len] = _T('\0');
*/
}


afx_msg LRESULT CDialAsmStat::OnAbortAsm(WPARAM wParam, LPARAM /* lParam */)
{
  EndDialog((int)wParam);
  return LRESULT(0);
}


afx_msg LRESULT CDialAsmStat::OnGetNextLine(WPARAM wParam, LPARAM lParam)
{
//  CString test;
//  m_edit.GetWindowText(test);

  ASSERT( (TCHAR *)lParam != NULL );	// wymagany adres bufora zwrotnego
  ASSERT( wParam > 0 );			// wymagane podanie minimalnej d³. bufora
  if (m_nCurrLine >= m_nLines)
  {
    *(TCHAR *)lParam = _T('\0');
    return LRESULT(-1);
  }
//  const CString &line= GetLine(m_nCurrLine++);
//  int len= min((int)wParam-1,line.GetLength());
//  _tcsncpy((TCHAR *)lParam, (const TCHAR *)line, len);
//  *((TCHAR *)lParam + len) = _T('\0');
  GetLine(m_nCurrLine++, (TCHAR *)lParam, (int)wParam-1);

  DWORD timer= ::GetTickCount();
  if (timer - m_dwTimer >= 100)
  {
    m_dwTimer = timer;
    SendDlgItemMessage(IDC_ASM_PROGRESS,PBM_SETPOS,(m_nPassNo - 1) * m_nLines + m_nCurrLine);
    SetLineNo(m_nCurrLine);		// wyœwietlenie numeru aktualnego wiersza
  }

  return LRESULT(1);
}


void CDialAsmStat::SetPassNo(int val)	// wyœwietlenie numeru przejœcia asemblacji
{
  SetCtrlText(IDC_PASS_NO,val);
}


void CDialAsmStat::SetLineNo(int val)	// wyœwietlenie numeru wiersza
{
  SetCtrlText(IDC_CURR_ROW,val);
}


void CDialAsmStat::SetCtrlText(int id, int val)
{
  SetDlgItemInt(id,val,FALSE);
}


void CDialAsmStat::ProgressStep()
{
  CProgressCtrl *pProgress = (CProgressCtrl*) GetDlgItem(IDC_ASM_PROGRESS);
  if (pProgress == NULL)
  {
    ASSERT(FALSE);		// z³y numer obiektu w oknie dialogowym
    return;
  }
  pProgress->StepIt();
}


void CDialAsmStat::SetProgressRange(int max_line)
{
  CProgressCtrl *pProgress = (CProgressCtrl*) GetDlgItem(IDC_ASM_PROGRESS);
  if (pProgress == NULL)
  {
    ASSERT(FALSE);		// z³y numer obiektu w oknie dialogowym
    return;
  }
  pProgress->SetRange(1,2*max_line);
}


afx_msg LRESULT CDialAsmStat::OnGetLineNo(WPARAM wParam, LPARAM lParam)
{
  ASSERT((int *)lParam != NULL);
  *(int *)lParam = m_nCurrLine - 1;	  // bie¿¹cy wiersz (numery od 0)
  return LRESULT(1);
}


afx_msg LRESULT CDialAsmStat::OnGetDocTitle(WPARAM wParam, LPARAM lParam)
{
  ASSERT( (TCHAR *)lParam != NULL );	// wymagany adres bufora zwrotnego
  ASSERT( wParam > 0 );			// wymagane podanie minimalnej d³. bufora
  CSrc6502Doc* pDoc= dynamic_cast<CSrc6502Doc*>(m_pView->GetDocument());
//  if (pDoc && pDoc->m_strPath.IsEmpty())
//    pDoc->m_strPath = pDoc->GetPathName();
  const CString &title= pDoc ? pDoc->GetPathName()/*pDoc->m_strPath*/ : CString(_T(""));
  int len= min((int)wParam-1,title.GetLength());	// nazwa dokumentu (ze œcie¿k¹)
  _tcsncpy((TCHAR *)lParam, (const TCHAR *)title, len);
  *((TCHAR *)lParam + len) = _T('\0');
  return LRESULT(1);
}


afx_msg LRESULT CDialAsmStat::OnNextPass(WPARAM wParam, LPARAM /* lParam */)
{
//  ASSERT( wParam > 0 );			// wymagane podanie numeru przejœcia asemblacji
  SetPassNo(++m_nPassNo);		// wyœwietlenie numeru przejœcia asemblacji
  m_nCurrLine = 0;			// odczyt wraca na pocz¹tek
  m_pText = m_strText;
  return LRESULT(1);
}


afx_msg LRESULT CDialAsmStat::OnFinished(WPARAM wParam, LPARAM /* lParam */)
{
  m_bFinished = TRUE;			// asemblacja zakoñczona

  CString ok;
  if (ok.LoadString(IDS_DIAL_ASM_OK))	// zmiana napisu na przycisku z 'Przerwij' na 'OK'
    SetDlgItemText(IDCANCEL,ok);

  CWnd *prg= GetDlgItem(IDC_ASM_PROGRESS);
  ASSERT(prg != NULL);			// z³y numer obiektu w oknie dialogowym
  if (prg)
    prg->ShowWindow(SW_HIDE);
  CWnd *icn= GetDlgItem(IDC_DIAL_ASM_ICN1);
  ASSERT(icn != NULL);			// z³y numer obiektu w oknie dialogowym
  if (icn)
    icn->ShowWindow(SW_HIDE);
  icn = GetDlgItem(IDC_DIAL_ASM_ICN2);
  ASSERT(icn != NULL);			// z³y numer obiektu w oknie dialogowym
  if (icn)
    icn->ShowWindow(SW_HIDE);
  icn = GetDlgItem(IDC_DIAL_ASM_ICN3);
  ASSERT(icn != NULL);			// z³y numer obiektu w oknie dialogowym
  if (icn)
    icn->ShowWindow(SW_HIDE);

  theApp.m_global.SetCodePresence((Stat)wParam==OK);
  theApp.m_global.SetStart(m_pAsm6502->GetProgramStart());
  SendMessageToViews(WM_USER_PROG_MEM_CHANGED,(WPARAM)-1,(Stat)wParam==OK ? 0 : -1);
  SendMessageToPopups(WM_USER_PROG_MEM_CHANGED,(WPARAM)-1,(Stat)wParam==OK ? 0 : -1);

  if (wParam)	    // b³¹d asemblacji?
  {
    SetLineNo(m_nCurrLine);		// wyœwietlenie numeru aktualnego wiersza
    const CString err= m_pAsm6502->GetErrMsg((Stat)wParam);
    CWnd *ctrl= GetDlgItem(IDC_DIAL_ASM_ERR);
    ASSERT(ctrl != NULL);		// z³y numer obiektu w oknie dialogowym
    if (ctrl)
    {
      ctrl->SetWindowText(err);
      ctrl->ShowWindow(SW_SHOWNORMAL);
    }
  }
  else
    EndDialog(0);


  return LRESULT(1);
}


void CDialAsmStat::OnCancel() 
{
  if (m_bFinished)		// asemblacja zakoñczy³a siê?
  {
    if (m_stAsmRetCode)		// wyst¹pi³ b³¹d?
    {
      m_pView->SetErrMark(m_nCurrLine-1);	// zaznaczenie wiersza zawieraj¹cego b³¹d
      CMainFrame *pMain = (CMainFrame*) AfxGetApp()->m_pMainWnd;
      pMain->m_wndStatusBar.SetPaneText(0,m_pAsm6502->GetErrMsg(m_stAsmRetCode));
    }
    EndDialog(0);
  }
  else				// przerwanie asemblacji
  {
    m_pAsm6502->Abort();
    CDialog::OnCancel();
  }
}


UINT CDialAsmStat::start_asm_thread(LPVOID pDial)
{
  return ((CDialAsmStat *)pDial)->StartAsm();
}


UINT CDialAsmStat::StartAsm()
{
  m_stAsmRetCode = (Stat)(m_pAsm6502->Assemble());
  PostMessage(WM_USER_FIN, WPARAM(m_stAsmRetCode));
  return 0;
}


BOOL CDialAsmStat::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
  if (!m_bFinished && nHitTest==HTCLIENT)
  {
    CWnd *btn= GetDlgItem(IDCANCEL);
    ASSERT(btn != NULL);			// z³y numer obiektu w oknie dialogowym
    if (btn)
    {
	  // convert cursor position to client co-ordinates
      CPoint point;
      GetCursorPos(&point);
//      pWnd->ScreenToClient(&point);
      CRect rect;
      btn->GetWindowRect(rect);		// wymiary przycisku
      if (rect.PtInRect(point))		// mysz nad przyciskiem?
        return CDialog::OnSetCursor(pWnd, nHitTest, message);
    }

    ::SetCursor( AfxGetApp()->LoadStandardCursor(IDC_WAIT) );
    return true;
  }

  return CDialog::OnSetCursor(pWnd, nHitTest, message);
}
