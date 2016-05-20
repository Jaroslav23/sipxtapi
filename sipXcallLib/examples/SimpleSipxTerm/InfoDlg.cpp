// InfoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SimpleSipxTerm.h"
#include "InfoDlg.h"


 


// InfoDlg dialog

IMPLEMENT_DYNAMIC(CInfoDlg, CDialog)

CInfoDlg::CInfoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CInfoDlg::IDD, pParent)
    , m_HistoryText(_T("SimpleSipXterm was initinally made with Visual Studio 6 in order to do some "    
                       "quick tests and concept evaluations against sipXtapi and different SIP server. "
                       "It was very quickly written and so very far from 'clean/good' coding practises and meant for internal use only.\n"
                       "The project owner of sipXtapi found this usefull anyway as simple example on which "
                       "it has been added to the sipXtapi branch. After this some small adaptations have been made "
                       "by the original author (EiSl1972_at_gmail_dot_com) with respect to cleaning-up a bit and "
                       "adding this text inclusive code comments.")
                    )
    , m_Description(_T("Each SimpleSipXterm uses one sipXtapi instance and can have 1 up-to 4 lines defined.\n"
                       "A line (client/account) is represented in SimpleSipXterm as a stand-alone terminal.\n"
                       "Incoming calls are only accepted when idle and only 2 parties are allowed per line. The "
                       "reason for this (for us) was that it had to represent the traditional TDM-domain.\n\n"
                       "- Local settings\tWhen having multiple SimpleSipXterm instances, then make sure that the\n"
                       "\t\tSIP and Media ports do not overlap.\n"
                       "- 'Start DNR'\tWhen e.g. '100' and 4 lines, then 100,101,102+103 as user account\n"
                       "\t\tare created.\n\n"
                       "REMARK: Mind that there can exist some bugs, especially in the state-machine")
                    )
{

}

CInfoDlg::~CInfoDlg()
{
}

void CInfoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_INFO_HISTORY2, m_HistoryText);
    DDX_Text(pDX, IDC_INFO_DESCRIPTION, m_Description);
}


BEGIN_MESSAGE_MAP(CInfoDlg, CDialog)
    ON_BN_CLICKED(IDOK, &CInfoDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// InfoDlg message handlers

void CInfoDlg::OnEnChangeRichedit22()
{
    // TODO:  If this is a RICHEDIT control, the control will not
    // send this notification unless you override the CDialog::OnInitDialog()
    // function and call CRichEditCtrl().SetEventMask()
    // with the ENM_CHANGE flag ORed into the mask.

    // TODO:  Add your control notification handler code here
}

void CInfoDlg::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    OnOK();
}
