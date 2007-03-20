// URLexamDlg.h : header file
//

#if !defined(AFX_URLEXAMDLG_H__801D7FD7_C36D_11D3_9A79_00105A6B4B2A__INCLUDED_)
#define AFX_URLEXAMDLG_H__801D7FD7_C36D_11D3_9A79_00105A6B4B2A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CURLexamDlg dialog

class CURLexamDlg : public CDialog
{
// Construction
public:
	CURLexamDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CURLexamDlg)
	enum { IDD = IDD_URLEXAM_DIALOG };
	CString	m_edit1;
	CString	m_edit2;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CURLexamDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CURLexamDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnChangeEdit1();
	afx_msg void OnRadio1();
	afx_msg void OnRadio2();
	afx_msg void encode();
	afx_msg void decode();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_URLEXAMDLG_H__801D7FD7_C36D_11D3_9A79_00105A6B4B2A__INCLUDED_)
