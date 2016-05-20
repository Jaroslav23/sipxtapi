// StartupSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SimpleSipxTerm.h"
#include "SimpleSipxTermDlg.h"
#include "StartupSettingsDlg.h"
#include "InfoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStartupSettingsDlg dialog


CStartupSettingsDlg::CStartupSettingsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStartupSettingsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStartupSettingsDlg)
	//}}AFX_DATA_INIT
}


void CStartupSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CStartupSettingsDlg)
    DDX_Text(pDX, IDC_EB_INTERFACE, m_Interface);
    DDX_Text(pDX, IDC_EB_RTPPORT, m_RtpPort);
    DDX_Text(pDX, IDC_EB_SIPPORT, m_SipPort);
    DDX_Text(pDX, IDC_EB_NRACC, m_NrOfAccounts);
    DDX_Text(pDX, IDC_EB_STARTDNR, m_StartDnr);
    DDX_Text(pDX, IDC_EB_SIPS_IP, m_SipServerIP);
    DDX_Text(pDX, IDC_EB_SIPS_PORT, m_SipServerPort);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStartupSettingsDlg, CDialog)
	//{{AFX_MSG_MAP(CStartupSettingsDlg)
	ON_BN_CLICKED(IDC_START, OnStart)
	ON_BN_CLICKED(IDC_EXIT, OnExit)
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_INFO, &CStartupSettingsDlg::OnBnClickedInfo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStartupSettingsDlg message handlers

void CStartupSettingsDlg::OnStart() 
{
	// TODO: Add your control notification handler code here
  OnOK();	
}

void CStartupSettingsDlg::OnExit() 
{
	// TODO: Add your control notification handler code here
  OnCancel();	
}

void CStartupSettingsDlg::OnBnClickedInfo()
{
    // TODO: Add your control notification handler code here
    CInfoDlg lv_InfoDlg;

    lv_InfoDlg.DoModal();
}
