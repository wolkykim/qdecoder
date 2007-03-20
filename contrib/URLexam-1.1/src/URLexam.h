// URLexam.h : main header file for the URLEXAM application
//

#if !defined(AFX_URLEXAM_H__801D7FD5_C36D_11D3_9A79_00105A6B4B2A__INCLUDED_)
#define AFX_URLEXAM_H__801D7FD5_C36D_11D3_9A79_00105A6B4B2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CURLexamApp:
// See URLexam.cpp for the implementation of this class
//

class CURLexamApp : public CWinApp
{
public:
	CURLexamApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CURLexamApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CURLexamApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_URLEXAM_H__801D7FD5_C36D_11D3_9A79_00105A6B4B2A__INCLUDED_)
