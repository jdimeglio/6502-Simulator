////////////////////////////////////////////////////////////////
// CFlatToolBar 1997 Microsoft Systems Journal. 
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
#include "StdAfx.h"
#include "FlatBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

////////////////////////////////////////////////////////////////
// CFlatToolBar--does flat tool bar in MFC.
//
IMPLEMENT_DYNAMIC(CFlatToolBar, CToolBar)

BEGIN_MESSAGE_MAP(CFlatToolBar, CToolBar)
  ON_WM_WINDOWPOSCHANGING()
  ON_WM_WINDOWPOSCHANGED()
END_MESSAGE_MAP()

////////////////
// Load override modifies the style after loading toolbar.
//
BOOL CFlatToolBar::LoadToolBar(LPCTSTR lpszResourceName)
{
  if (!CToolBar::LoadToolBar(lpszResourceName))
    return FALSE;
  ModifyStyle(0, TBSTYLE_FLAT); // make it flat
  return TRUE;
}

//#define ILLUSTRATE_DISPLAY_BUG			 // remove comment to see the bug

//////////////////
// MFC doesn't handle moving a TBSTYLE_FLAT toolbar correctly.
// The simplest way to fix it is to repaint the old rectangle and
// toolbar itself whenever the toolbar moves.
// 
void CFlatToolBar::OnWindowPosChanging(LPWINDOWPOS lpwp)
{
  CToolBar::OnWindowPosChanging(lpwp);

#ifndef ILLUSTRATE_DISPLAY_BUG

  if (!(lpwp->flags & SWP_NOMOVE))        // if moved:
  {
    CRect rc;                             //   Fill rectangle with..
    GetWindowRect(&rc);                   //   ..my (toolbar) rectangle.
    CWnd* pParent = GetParent();          //   get parent (dock bar/frame) win..
    pParent->ScreenToClient(&rc);         //   .. and convert to parent coords

    // Ask parent window to paint the area beneath my old location.
    // Typically, this is just solid grey. The area won't get painted until
    // I send WM_NCPAINT after the move, in OnWindowPosChanged below.
    //
    pParent->InvalidateRect(&rc);         // paint old rectangle
  }
#endif
}

//////////////////
// Now toolbar has moved: repaint old area
//
void CFlatToolBar::OnWindowPosChanged(LPWINDOWPOS lpwp)
{
  CToolBar::OnWindowPosChanged(lpwp);

#ifndef ILLUSTRATE_DISPLAY_BUG
  if (!(lpwp->flags & SWP_NOMOVE))        // if moved:
  {
    // Now paint my non-client area at the new location.
    // This is the extra bit of border space surrounding the buttons.
    // Without this, you will still have a partial display bug (try it!)
    //
    SendMessage(WM_NCPAINT);
  }
#endif
}

////////////////////////////////////////////////////////////////
// The following stuff is to make the command update UI mechanism
// work properly for flat tool bars. The main idea is to convert
// a "checked" button state into a "pressed" button state. Changed 
// lines marked with "PD"

////////////////
// The following class was copied from BARTOOL.CPP in the MFC source.
// All I changed was SetCheck--PD.
//
class CFlatOrCoolBarCmdUI : public CCmdUI // class private to this file !
{
public: // re-implementations only
  virtual void Enable(BOOL bOn);
  virtual void SetCheck(int nCheck);
  virtual void SetText(LPCTSTR lpszText);
};


void CFlatOrCoolBarCmdUI::Enable(BOOL bOn)
{
  m_bEnableChanged = TRUE;
  CToolBar* pToolBar = (CToolBar*)m_pOther;
  ASSERT(pToolBar != NULL);
  ASSERT_KINDOF(CToolBar, pToolBar);
  ASSERT(m_nIndex < m_nIndexMax);

  TBBUTTON btn;
  pToolBar->GetToolBarCtrl().GetButton(m_nIndex, &btn);
  BYTE byNewStyle= bOn ? btn.fsState | TBSTATE_ENABLED : btn.fsState & ~TBSTATE_ENABLED;
  if (byNewStyle != btn.fsState)
  {
    pToolBar->GetToolBarCtrl().EnableButton(btn.idCommand, bOn);
    if (!bOn)
      pToolBar->GetToolBarCtrl().Indeterminate(btn.idCommand, false);
  }
}

// Take your pick:
//#define MYTBBS_CHECKED TBBS_CHECKED			// use "checked" state
//#define MYTBBS_CHECKED TBBS_PRESSED			// use pressed state
//#define MYTBBS_CHECKED (TBBS_PRESSED | TBBS_CHECKED)

//////////////////
// This is the only function that has changed: instead of TBBS_CHECKED,
// I use TBBS_PRESSED--PD
//
void CFlatOrCoolBarCmdUI::SetCheck(int nCheck)
{
  ASSERT(nCheck >= 0 && nCheck <= 2); // 0=>off, 1=>on, 2=>indeterminate
  CToolBar* pToolBar = (CToolBar*)m_pOther;
  ASSERT(pToolBar != NULL);
  ASSERT_KINDOF(CToolBar, pToolBar);
  ASSERT(m_nIndex < m_nIndexMax);

  TBBUTTON btn;
  pToolBar->GetToolBarCtrl().GetButton(m_nIndex, &btn);
  BYTE byNewStyle= btn.fsState;
  switch (nCheck)
  {
  case 0:   // off
    byNewStyle &= ~(TBSTATE_CHECKED);
    break;
  case 1:   // on
    byNewStyle |= TBSTATE_CHECKED;
    break;
  case 2:   // indeterminate
    byNewStyle |= TBSTATE_INDETERMINATE;
    break;
  }
  if (byNewStyle != btn.fsState)
      pToolBar->GetToolBarCtrl().SetState(btn.idCommand, byNewStyle);
/*    if (nCheck != 2)
    {
      btn.fsState &= ~TBSTATE_INDETERMINATE;
      pToolBar->GetToolBarCtrl().SetState(btn.idCommand,
        nCheck == 0 ? btn.fsState & ~(TBSTATE_PRESSED | TBSTATE_CHECKED)
                    : btn.fsState | TBSTATE_PRESSED | TBSTATE_CHECKED);
    }
    else
    {
      if ((btn.fsState & TBSTATE_INDETERMINATE) == 0)
      {
        pToolBar->GetToolBarCtrl().SetState(btn.idCommand,
          (btn.fsState & ~(TBSTATE_PRESSED | TBSTATE_CHECKED)) | TBSTATE_INDETERMINATE);
      }
    } */

/*
  UINT nOldStyle = pToolBar->GetButtonStyle(m_nIndex); // PD
  UINT nNewStyle = nOldStyle &
				~(MYTBBS_CHECKED | TBBS_INDETERMINATE); // PD
  if (nCheck == 1)
    nNewStyle |= MYTBBS_CHECKED; // PD
  else if (nCheck == 2)
    nNewStyle |= TBBS_INDETERMINATE;

  // Following is to fix display bug for TBBS_CHECKED:
  // If new state is unchecked, repaint--but only if style actually changing.
  // (Otherwise will end up with flicker)
  // 
  if (nNewStyle != nOldStyle)
  {
    ASSERT(!(nNewStyle & TBBS_SEPARATOR));
    pToolBar->SetButtonStyle(m_nIndex, nNewStyle);
    pToolBar->Invalidate();
  }
*/
}

void CFlatOrCoolBarCmdUI::SetText(LPCTSTR)
{
	// ignore for now, but you should really set the text
}

//////////////////
// This function is mostly copied from CToolBar/BARTOOL.CPP. The only thing
// that's different is I instantiated a CFlatOrCoolBarCmdUI instead of
// CToolCmdUI.
//

void CFlatToolBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
	CFlatOrCoolBarCmdUI state; // <<<< This is the only line that's different--PD
	state.m_pOther = this;

	state.m_nIndexMax = (UINT)DefWindowProc(TB_BUTTONCOUNT, 0, 0);
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++)
	{
		// get button state
		TBBUTTON button;
		VERIFY(DefWindowProc(TB_GETBUTTON, state.m_nIndex, (LPARAM)&button));
		// TBSTATE_ENABLED == TBBS_DISABLED so invert it
		button.fsState ^= TBSTATE_ENABLED;

		state.m_nID = button.idCommand;

		// ignore separators
		if (!(button.fsStyle & TBSTYLE_SEP))
		{
			// allow the toolbar itself to have update handlers
			if (CWnd::OnCmdMsg(state.m_nID, CN_UPDATE_COMMAND_UI, &state, NULL))
				continue;

			// allow the owner to process the update
			state.DoUpdate(pTarget, bDisableIfNoHndler);
		}
	}

	// update the dialog controls added to the toolbar
	UpdateDialogControls(pTarget, bDisableIfNoHndler);
}
