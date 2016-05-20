#pragma once


// InfoDlg dialog

class CInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CInfoDlg)

public:
	CInfoDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CInfoDlg();

// Dialog Data
	enum { IDD = IDD_INFODLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnEnChangeRichedit22();
protected:
    CString m_HistoryText;
    CString m_Description;
public:
    afx_msg void OnBnClickedOk();
};
