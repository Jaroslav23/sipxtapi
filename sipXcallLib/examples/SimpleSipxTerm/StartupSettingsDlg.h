#if !defined(AFX_STARTUPSETTINGSDLG_H__E16EE878_222F_49D2_A770_4665826D3DBC__INCLUDED_)
#define AFX_STARTUPSETTINGSDLG_H__E16EE878_222F_49D2_A770_4665826D3DBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StartupSettingsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStartupSettingsDlg dialog

class CStartupSettingsDlg : public CDialog
{
// Construction
public:
	CStartupSettingsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CStartupSettingsDlg)
	enum { IDD = IDD_STARTUP };
    CString m_Interface;
	int		m_RtpPort;
	int		m_SipPort;
	int		m_NrOfAccounts;
	CString	m_SipServerIP;
	int   	m_SipServerPort;
	int		m_StartDnr;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStartupSettingsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CStartupSettingsDlg)
	afx_msg void OnStart();
	afx_msg void OnExit();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedInfo();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STARTUPSETTINGSDLG_H__E16EE878_222F_49D2_A770_4665826D3DBC__INCLUDED_)
