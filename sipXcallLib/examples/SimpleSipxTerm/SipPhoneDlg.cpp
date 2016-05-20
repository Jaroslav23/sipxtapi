// SipPhoneDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SimpleSipxTerm.h"
#include "SipPhoneDlg.h"
#include "SimpleSipxTermDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSipPhoneDlg dialog


CSipPhoneDlg::CSipPhoneDlg(CWnd* pParent /*=NULL*/,
                           CSimpleSipxTermDlg* fi_pToCtrl)
                           : CDialog(CSipPhoneDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSipPhoneDlg)
    m_DestinationDnr = _T("");
    //}}AFX_DATA_INIT

    m_pCtrl = fi_pToCtrl;
}


void CSipPhoneDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSipPhoneDlg)
    DDX_Control(pDX, IDC_EB_DISPLAY_U, m_UpperDisplay);
    DDX_Control(pDX, IDC_EB_DISPLAY_L, m_LowerDisplay);
    DDX_Control(pDX, IDC_EB_CONNSTATE, m_CurrConnState);
    DDX_Text(pDX, IDC_EB_DEST, m_DestinationDnr);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSipPhoneDlg, CDialog)
    //{{AFX_MSG_MAP(CSipPhoneDlg)
    ON_BN_CLICKED(IDC_BUT_MAKECALL, OnButMakeCall)
    ON_BN_CLICKED(IDC_BUT_ANSWERCALL, OnButAnswerCall)
    ON_BN_CLICKED(IDC_BUT_CLEARCALL, OnButClearCall)
    ON_BN_CLICKED(IDC_BUT_HOLDCALL, OnButHold_UnholdCall)
    ON_BN_CLICKED(IDC_BUT_TRANSFERCALL, OnButTransferCall)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


void CSipPhoneDlg::SetConfig( const int fi_PhoneIndex, const int fi_OwnDNR )
{
    char lv_TmpStr[16];

    if ( fi_OwnDNR < 65535 )
    {
        SetConfig( fi_PhoneIndex, _itoa( fi_OwnDNR, lv_TmpStr, 10 ));
    }
}
void CSipPhoneDlg::SetConfig( const int fi_PhoneIndex, const CString& fi_OwnDNR )
{
    m_OwnDnr     = fi_OwnDNR;
    m_PhoneIndex = fi_PhoneIndex;
}

void CSipPhoneDlg::UpdateConnState( const CString& fi_strConnState )
{
    m_CurrConnState.SetWindowText(fi_strConnState);
}

void CSipPhoneDlg::ChangeButtonStates( const DWORD fi_EnableBitmap, const DWORD fi_DisableBitmap)
{

}

void CSipPhoneDlg::ChangeUpperDisplay( const CString& fi_String )
{
    m_UpperDisplay.SetWindowText(fi_String);
}
void CSipPhoneDlg::ChangeLowerDisplay( const CString& fi_String )
{
    m_LowerDisplay.SetWindowText(fi_String);
}

/////////////////////////////////////////////////////////////////////////////
// CSipPhoneDlg message handlers

void CSipPhoneDlg::OnButMakeCall() 
{
    CString lv_String;

    // TODO: Add your control notification handler code here
    UpdateData(TRUE);

    if ( !m_DestinationDnr.IsEmpty() )
    {
        m_pCtrl->MakeCall( m_PhoneIndex, m_DestinationDnr );
    }	
}

void CSipPhoneDlg::OnButAnswerCall() 
{
    CString lv_String;

    // TODO: Add your control notification handler code here
    m_pCtrl->AnswerCall( m_PhoneIndex );
}

void CSipPhoneDlg::OnButClearCall() 
{
    CString lv_String;

    // TODO: Add your control notification handler code here
    m_pCtrl->ClearCall( m_PhoneIndex );
}

void CSipPhoneDlg::OnButHold_UnholdCall() 
{
    CString lv_String;

    // TODO: Add your control notification handler code here
    m_pCtrl->HoldUnholdCall( m_PhoneIndex );	
}


void CSipPhoneDlg::OnButTransferCall() 
{
    CString lv_String;

    // TODO: Add your control notification handler code here
    m_pCtrl->TransferCall( m_PhoneIndex );	
}
