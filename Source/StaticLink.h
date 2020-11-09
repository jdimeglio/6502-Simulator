////////////////////////////////////////////////////////////////
// 1998 Microsoft Systems Journal
// If this code works, it was written by Paul DiLascia.
// If not, I don't know who wrote it.
//
//
// CStaticLink implements a static control that's a hyperlink
// to any file on your desktop or web. You can use it in dialog boxes
// to create hyperlinks to web sites. When clicked, opens the file/URL
//
#ifndef _STATLINK_H
#define _STATLINK_H

#include "HyperLink.h"

class CStaticLink : public CStatic
{
public:
	DECLARE_DYNAMIC(CStaticLink)
	CStaticLink(LPCTSTR lpText = NULL, BOOL bDeleteOnDestroy= FALSE);
	~CStaticLink() { }

	// Use this if you want to subclass and also set different URL
	BOOL SubclassDlgItem(UINT nID, CWnd* pParent, LPCTSTR lpszLink= NULL)
	{
		m_link = lpszLink;
		return CStatic::SubclassDlgItem(nID, pParent);
	}

	// Hyperlink contains URL/filename. If NULL, I will use the window text.
	// (GetWindowText) to get the target.
	CHyperlink  m_link;
	COLORREF    m_color;

	// Default colors you can change
	// These are global, so they're the same for all links.
	static COLORREF g_colorUnvisited;
	static COLORREF g_colorVisited;

	// Cursor used when mouse is on a link--you can set, or
	// it will default to the standard hand with pointing finger.
	// This is global, so it's the same for all links.
	static HCURSOR   g_hCursorLink;

protected:
	CFont     m_font;         // underline font for text control
	BOOL      m_bDeleteOnDestroy; // delete object when window destroyed?

	virtual void PostNcDestroy();

	// message handlers
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void  OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL  OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};

#endif _STATLINK_H
