// SimpleSipxTermDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SimpleSipxTerm.h"
#include "SimpleSipxTermDlg.h"
#include "SipPhoneDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSimpleSipxTermDlg::LineInfos::LineInfos()
{
    m_PhoneDlg    = NULL;
    m_SipLine     = 0;
    m_ActiveCall  = SIPX_CALL_NULL;
    m_HeldCall    = SIPX_CALL_NULL;
    m_Activated   = false;  
    m_State       = TS_INIT;
}
CSimpleSipxTermDlg::LineInfos::~LineInfos()
{
}

/////////////////////////////////////////////////////////////////////////////
// CSimpleSipxTermDlg dialog

CSimpleSipxTermDlg::CSimpleSipxTermDlg(CWnd* pParent /*=NULL*/)
: CDialog(CSimpleSipxTermDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CSimpleSipxTermDlg)
    // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSimpleSipxTermDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSimpleSipxTermDlg)
    DDX_Control(pDX, IDC_LIST_LOG, m_LogList);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSimpleSipxTermDlg, CDialog)
    //{{AFX_MSG_MAP(CSimpleSipxTermDlg)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDCANCEL, OnExit)
    ON_BN_CLICKED(IDOK, &CSimpleSipxTermDlg::OnBnClickedOk)
    ON_WM_DESTROY()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimpleSipxTermDlg message handlers

BOOL CSimpleSipxTermDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    // TODO: Add extra initialization here
    // -----------------------------------

    SIPX_RESULT lv_Result;
    bool        lv_Abort = false;
    CString     lv_String;
    CString     lv_LogStr;
    RECT        lv_ViewRect, lv_DlgRect ;

    // Get initial sizings of this dialog
    GetWindowRect( &lv_ViewRect );

    lv_LogStr.Format("Creating sipXtapi instance (local SIP-port=%d, RTP-port=%d on interface=%s)",
        m_SipPort, m_RtpPort, m_Interface);
    m_LogList.AddString(lv_LogStr);

    lv_Result = sipxInitialize(&m_sipXInstance,            
                                m_SipPort,          // UDP
                                m_SipPort,          // TCP
                                SIPX_PORT_DISABLE,  // TLS
                                m_RtpPort,
                                DEFAULT_CONNECTIONS,
                                DEFAULT_IDENTITY,
                                m_Interface);

    if ( SIPX_RESULT_SUCCESS == lv_Result )
    {
        sipxConfigSetUserAgentName( m_sipXInstance, "Simple Agent (sipXtapi)", false );

        m_LogList.AddString("-> OK. Register our callback");

        // Register our callback handler so we can receive events from sipXtapi stack
        sipxEventListenerAdd( m_sipXInstance, GenericSipXCallbackHdlr, this );

        lv_LogStr.Format("Start creation + registration of %d terminals...", m_NrOfAccounts);
        m_LogList.AddString(lv_LogStr);

        // ------------------------------------------------------------------------
        // Make sure no more than 4 SIP lines can be created...
        if ( m_NrOfAccounts > 4 )
            m_NrOfAccounts = 4;

        for ( int i = 0 ; i < m_NrOfAccounts ; i++ )
        {
            m_Lines[i].m_Dnr.Format("%d", m_StartDnr);

            // First do the GUI stuff
            // ----------------------
            // Create the window
            m_Lines[i].m_PhoneDlg = new CSipPhoneDlg( NULL, this );
            m_Lines[i].m_PhoneDlg->Create( IDD_SIPPHONE );
            m_Lines[i].m_PhoneDlg->SetParentHwnd(this->m_hWnd );

            if ( 0 == i ) // 0 == i is defensive programming...
            {
                // Get the dialog's sizes (only here for the first time). 
                // We're using the right+bottom for width and heigth
                m_Lines[i].m_PhoneDlg->GetWindowRect( &lv_DlgRect );
                lv_DlgRect.right  = lv_DlgRect.right  - lv_DlgRect.left;
                lv_DlgRect.bottom = lv_DlgRect.bottom - lv_DlgRect.top;

                lv_ViewRect.left += 10;
                m_Lines[i].m_PhoneDlg->MoveWindow( lv_ViewRect.left, 
                    0,
                    lv_DlgRect.right,
                    lv_DlgRect.bottom );

                // Pin-point the main window below the first SIPphone
                MoveWindow( lv_ViewRect.left,
                    0 + lv_DlgRect.bottom + 10,
                    lv_ViewRect.right,
                    lv_ViewRect.bottom );
            }
            else
            {
                lv_ViewRect.left += (lv_DlgRect.right - lv_DlgRect.left + 10);
                m_Lines[i].m_PhoneDlg->MoveWindow( lv_ViewRect.left , 
                                                   0,
                                                   lv_DlgRect.right,
                                                   lv_DlgRect.bottom );
            }

            // Now create and register a SIP account
            // -------------------------------------
            // Create the SIP line
            lv_Abort = true;
            lv_String.Format("sip:%d@%s:%d",
                m_StartDnr, m_SipServerIP, m_SipServerPort);

            lv_LogStr.Format("- sipxLineAdd: %s", lv_String);
            m_LogList.AddString(lv_LogStr);

            lv_Result = sipxLineAdd( m_sipXInstance,
                lv_String,
                &m_Lines[i].m_SipLine );

            if ( SIPX_RESULT_SUCCESS == lv_Result )
            {
                // We expect for this that the SIP-server does not use
                // passwords. If it does, than this requires adaptation.
                lv_String.Format("%d", m_StartDnr);
                sipxLineAddCredential( m_Lines[i].m_SipLine , lv_String, "", "" );

                lv_LogStr.Format("  --> sipxLineRegister (%d)", m_Lines[i].m_SipLine);
                m_LogList.AddString(lv_LogStr);

                // And finally try to register this to the SIP Server.
                // In this scenario below we're first de-registering to overcome a 
                // 'hanging-line' bug in some SIP-enabled TDM PBX.
                lv_Result = sipxLineRegister( m_Lines[i].m_SipLine  , false);
                Sleep(250);
                lv_Result = sipxLineRegister( m_Lines[i].m_SipLine  , true);

                if ( SIPX_RESULT_SUCCESS == lv_Result )
                {
                    // All success (with sending!!! Maybe registring fails).
                    // Now copy info to Phone dialog and show it
                    m_Lines[i].m_PhoneDlg->SetConfig( i, m_StartDnr );
                    m_Lines[i].m_Activated = true;

                    lv_String.Format("DNR: %s (Registring...)",m_Lines[i].m_Dnr);
                    m_Lines[i].m_PhoneDlg->SetWindowText(lv_String);
                    m_Lines[i].m_PhoneDlg->UpdateConnState(_T("Not initialized / Registering"));

                    if ( NULL != m_Lines[i].m_PhoneDlg ) 
                        m_Lines[i].m_PhoneDlg->ShowWindow( SW_SHOW );

                    lv_Abort = false;
                }
            }

            // If anything went wrong, remove the line again
            if ( true == lv_Abort )
            {
                try
                {
                    if ( m_Lines[i].m_SipLine != 0 )
                        sipxLineRemove( m_Lines[i].m_SipLine );
                }
                catch(...)
                {
                }
            }

            // Now update the m_StartDnr for next loop
            m_StartDnr++;
        }
    }
    else
    {
        m_sipXInstance = NULL;

        lv_String.Format("Cannot startup SIP Phones because of error during initialization.\nThe sipXtapi return error = %d",
            lv_Result);

        AfxMessageBox(lv_String, IDOK | MB_ICONEXCLAMATION   );
    }

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSimpleSipxTermDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
  CDialog::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSimpleSipxTermDlg::OnPaint() 
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSimpleSipxTermDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

void CSimpleSipxTermDlg::OnExit() 
{
    // TODO: Add your control notification handler code here

}

void CSimpleSipxTermDlg::OnSipPhoneEvent( void )
{
}

void CSimpleSipxTermDlg::OnDestroy() 
{
    CDialog::OnDestroy();
    // TODO: Add your message handler code here

    // First detach our callback handler, else we can get 
    // very funny deadlocks if you are realy unlucky.
    sipxEventListenerRemove( m_sipXInstance, GenericSipXCallbackHdlr, this );

    // Now nicely de-register our lines from SIP-server and have this line /
    // account removed from stack (this last action is also automatically 
    // performed by uninitializing of the stack.
    // Finally destroy terminals GUI instance.
    for ( int i = 0 ; i < m_NrOfAccounts ; i++ )
    {
        if ( m_Lines[i].m_PhoneDlg != NULL )
        {
            try
            {
                sipxLineRegister( m_Lines[i].m_SipLine, false);
                m_Lines[i].m_PhoneDlg->DestroyWindow();
                delete( m_Lines[i].m_PhoneDlg );
            }
            catch(...)
            {
            }
        }	
    }

    // Now the stack itself
    try
    {
        sipxUnInitialize(m_sipXInstance, true);
    }
    catch(...)
    {
    }
}

void CSimpleSipxTermDlg::OnShowTerminals() 
{
    // TODO: Add your control notification handler code here

}

void CSimpleSipxTermDlg::CopySettings( CStartupSettingsDlg* fi_SettingsDlg )
{
    m_Interface     = fi_SettingsDlg->m_Interface;
    m_SipPort       = fi_SettingsDlg->m_SipPort;
    m_RtpPort       = fi_SettingsDlg->m_RtpPort;
    m_NrOfAccounts  = fi_SettingsDlg->m_NrOfAccounts;
    m_StartDnr      = fi_SettingsDlg->m_StartDnr;
    m_SipServerIP   = fi_SettingsDlg->m_SipServerIP;
    m_SipServerPort = fi_SettingsDlg->m_SipServerPort;
}
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void CSimpleSipxTermDlg::MakeCall( int             fi_PhoneIndex, 
                                   const CString&  fi_DestDnr )
{
    CString     lv_String;
    CString     lv_Destination;
    SIPX_CALL   lv_MyCall;
    SIPX_RESULT lv_Result;
    LineInfos&  lv_LineInfo = m_Lines[fi_PhoneIndex];

    // Making a call is only allowed when no active call exist
    // (one held call is allowed).
    if ( lv_LineInfo.m_ActiveCall == SIPX_CALL_NULL )
    {
        lv_String.Format("-> sipxCallCreate for line=%d", lv_LineInfo.m_SipLine);
        m_LogList.AddString(lv_String);

        lv_Result = sipxCallCreate( m_sipXInstance, lv_LineInfo.m_SipLine, &lv_MyCall );

        if ( SIPX_RESULT_SUCCESS == lv_Result )
        {
            // Build destination address
            lv_Destination.Format("sip:%s@%s",
                fi_DestDnr, m_SipServerIP);

            lv_String.Format("-> sipxCallConnect (L=%d,C=%d) to dest=%s", lv_LineInfo.m_SipLine, lv_MyCall, lv_Destination);
            m_LogList.AddString(lv_String);

            lv_Result = sipxCallConnect( lv_MyCall, 
                lv_Destination );

            if ( SIPX_RESULT_SUCCESS == lv_Result )
            {
                lv_String.Format("%s => %s",
                    lv_LineInfo.m_Dnr, fi_DestDnr);

                lv_LineInfo.m_PhoneDlg->ChangeUpperDisplay( lv_String );

                if ( SIPX_CALL_NULL != lv_MyCall )
                {
                    lv_LineInfo.m_ActiveCall = lv_MyCall;
                }
            }
            else
            {
                lv_String.Format("Error during sipxCallConnect (%d)", lv_Result);
                AfxMessageBox(lv_String);

                lv_LineInfo.m_PhoneDlg->ChangeUpperDisplay( "" );
            }
        }
        else
        {
            lv_String.Format("Error during sipxCallCreate (%d)", lv_Result);
            AfxMessageBox(lv_String);
            lv_LineInfo.m_PhoneDlg->ChangeUpperDisplay( "" );
        }
    }
    m_LogList.SetTopIndex(m_LogList.GetCount() - 1);
}

void CSimpleSipxTermDlg::ClearCall( int fi_PhoneIndex )
{
    LineInfos& lv_LineInfo = m_Lines[fi_PhoneIndex];
    CString    lv_String;
    SIPX_CALL  lv_Call;

    if ( SIPX_CALL_NULL != lv_LineInfo.m_ActiveCall )
    {
        lv_String.Format("-> sipxCallDestroy (L=%d,C=%d)", lv_LineInfo.m_SipLine, lv_LineInfo.m_ActiveCall);
        lv_Call = lv_LineInfo.m_ActiveCall;
        sipxCallDestroy( &lv_Call );
    }

    m_LogList.SetTopIndex(m_LogList.GetCount() - 1);
}

void CSimpleSipxTermDlg::AnswerCall ( int fi_PhoneIndex )
{
    LineInfos& lv_LineInfo = m_Lines[fi_PhoneIndex];
    CString    lv_String;

    if ( ( TS_ALERTING        == lv_LineInfo.m_State ) ||
        ( TS_WAIT_FOR_ANSWER == lv_LineInfo.m_State )
        )
    {  
        if ( SIPX_CALL_NULL != lv_LineInfo.m_ActiveCall )
        {
            lv_String.Format("-> sipxCallAnswer (L=%d,C=%d)", lv_LineInfo.m_SipLine, lv_LineInfo.m_ActiveCall);
            m_LogList.AddString(lv_String);

            sipxCallAnswer( lv_LineInfo.m_ActiveCall, false );
        }
    }
    else
    {
        AfxMessageBox("Terminal is not alerting");
    }

    m_LogList.SetTopIndex(m_LogList.GetCount() - 1);
}

void CSimpleSipxTermDlg::HoldUnholdCall ( int  fi_PhoneIndex )
{
    LineInfos&  lv_LineInfo = m_Lines[fi_PhoneIndex];
    CString     lv_String;
    CString     lv_LogStr;
    SIPX_CALL   lv_Call1;
    SIPX_CALL   lv_Call2;
    SIPX_RESULT lv_Result;

    // No held call yet...
    if ( ( SIPX_CALL_NULL != lv_LineInfo.m_ActiveCall ) &&
        ( SIPX_CALL_NULL == lv_LineInfo.m_HeldCall   )
        )
    {
        lv_String.Format("-> sipxCallHold (L=%d,C=%d)", lv_LineInfo.m_SipLine, lv_LineInfo.m_ActiveCall);
        m_LogList.AddString(lv_String);

        lv_Call1 = lv_LineInfo.m_ActiveCall;
        lv_Result = sipxCallHold(lv_Call1);

        if ( SIPX_RESULT_SUCCESS == lv_Result )
        {
            // Swap info...
            lv_LineInfo.m_HeldCall = lv_LineInfo.m_ActiveCall;
            lv_LineInfo.m_ActiveCall = SIPX_CALL_NULL;

            // And update displays
            lv_LineInfo.m_PhoneDlg->m_UpperDisplay.GetWindowText( lv_String );
            lv_LineInfo.m_PhoneDlg->ChangeUpperDisplay("");
            lv_String.Insert(0, "HELD: ");
            lv_LineInfo.m_PhoneDlg->ChangeLowerDisplay(lv_String);
        }
        else
        {
            AfxMessageBox("sipxCallHold failed");
        }
    }
    else
    {
        // In case of both Active & Held => Shuttle
        if ( ( SIPX_CALL_NULL != lv_LineInfo.m_ActiveCall ) &&
            ( SIPX_CALL_NULL != lv_LineInfo.m_HeldCall   )
            )
        {
            m_LogList.AddString("-> Shuttle Active + Held call");

            lv_String.Format("  -> sipxCallHold for Active Call (L=%d,C=%d)", lv_LineInfo.m_SipLine, lv_LineInfo.m_ActiveCall);
            m_LogList.AddString(lv_String);

            // get Call IDs
            lv_Call1 = lv_LineInfo.m_ActiveCall;
            lv_Call2 = lv_LineInfo.m_HeldCall;

            // Start putting active on hold and then the held to active
            lv_Result = sipxCallHold(lv_Call1);

            if ( SIPX_RESULT_SUCCESS == lv_Result )
            {
                // And unhold the held party
                lv_String.Format("  -> sipxCallUnhold for held Call (L=%d,C=%d)", lv_LineInfo.m_SipLine, lv_LineInfo.m_HeldCall);
                m_LogList.AddString(lv_String);

                lv_Result = sipxCallUnhold(lv_Call2);

                if ( SIPX_RESULT_SUCCESS == lv_Result )
                {
                    // Now both are swapped. Update vars+displays
                    lv_LineInfo.m_ActiveCall = lv_Call2;
                    lv_LineInfo.m_HeldCall   = lv_Call1;

                    CString lv_LowerStr;

                    // Get both display content
                    lv_LineInfo.m_PhoneDlg->m_UpperDisplay.GetWindowText( lv_String );
                    lv_LineInfo.m_PhoneDlg->m_LowerDisplay.GetWindowText( lv_LowerStr );

                    // ...move active to held
                    lv_String.Insert(0, "HELD: ");
                    lv_LineInfo.m_PhoneDlg->ChangeLowerDisplay(lv_String);

                    // ... move held to active (removing HELD)
                    lv_LowerStr.Delete(0, 6); // Remove the HELD:
                    lv_LineInfo.m_PhoneDlg->ChangeUpperDisplay(lv_LowerStr);
                }
                else
                {
                    AfxMessageBox("sipxCallUnHold failed for held party. Clearing both...");

                    sipxCallDestroy( &lv_Call1 );
                    sipxCallDestroy( &lv_Call2 );
                }
            }
            else
            {
                AfxMessageBox("sipxCallHold failed for active call -> clearing both");
                sipxCallDestroy( &lv_Call1 );
                sipxCallDestroy( &lv_Call2 );
            }
        }
        else
        {
            // No Active party.. only Held. -> retrieve this one
            if ( ( SIPX_CALL_NULL == lv_LineInfo.m_ActiveCall ) &&
                ( SIPX_CALL_NULL != lv_LineInfo.m_HeldCall   )
                )
            {
                lv_String.Format("-> sipxCallUnHold (L=%d,C=%d)", lv_LineInfo.m_SipLine, lv_LineInfo.m_HeldCall);
                m_LogList.AddString(lv_String);

                lv_Call1 = lv_LineInfo.m_HeldCall;
                lv_Result = sipxCallUnhold(lv_Call1);

                if ( SIPX_RESULT_SUCCESS == lv_Result )
                {
                    // Swap info...
                    lv_LineInfo.m_ActiveCall = lv_LineInfo.m_HeldCall;
                    lv_LineInfo.m_HeldCall   = SIPX_CALL_NULL;

                    // And update displays (remove HELD)
                    lv_LineInfo.m_PhoneDlg->m_LowerDisplay.GetWindowText( lv_String );
                    lv_LineInfo.m_PhoneDlg->ChangeLowerDisplay("");
                    lv_String.Delete(0, 6);
                    lv_LineInfo.m_PhoneDlg->ChangeUpperDisplay(lv_String);
                }
                else
                {
                    AfxMessageBox("sipxCallUnHold failed");
                }
            }
        }
    }

    m_LogList.SetTopIndex(m_LogList.GetCount() - 1);
}


void CSimpleSipxTermDlg::TransferCall ( int  fi_PhoneIndex )
{
    LineInfos&  lv_LineInfo = m_Lines[fi_PhoneIndex];
    CString     lv_String;
    SIPX_CALL   lv_Call;
    SIPX_RESULT lv_Result;

    if ( ( SIPX_CALL_NULL != lv_LineInfo.m_ActiveCall ) && 
        ( SIPX_CALL_NULL != lv_LineInfo.m_HeldCall   )
        )
    {
        lv_String.Format("-> sipxCallTransfer for held/active Call (L=%d,HC=%d,AC=%d)", 
            lv_LineInfo.m_SipLine, lv_LineInfo.m_HeldCall, lv_LineInfo.m_ActiveCall);
        m_LogList.AddString(lv_String);

        lv_Result = sipxCallTransfer( lv_LineInfo.m_HeldCall, lv_LineInfo.m_ActiveCall );

        if ( SIPX_RESULT_SUCCESS != lv_Result )
        {
            AfxMessageBox("sipxCallTransfer failed. Clear both parties..");
            lv_Call = lv_LineInfo.m_ActiveCall;
            sipxCallDestroy( &lv_Call );
            lv_Call = lv_LineInfo.m_HeldCall;
            sipxCallDestroy( &lv_Call );
        }
    }

    m_LogList.SetTopIndex(m_LogList.GetCount() - 1);
}



/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// This is our callback to register to sipXtapi instance.
// It needs to be a static callback.
bool CSimpleSipxTermDlg::GenericSipXCallbackHdlr
( 
 SIPX_EVENT_CATEGORY  fi_Category,
 void*                fi_pInfo,
 void*                fi_pUserData
 )
{
    CSimpleSipxTermDlg* lv_ThisPtr = (CSimpleSipxTermDlg*)fi_pUserData;
    SIPX_LINE           lv_Line;
    char                lv_EventString[255];
    CString             lv_LogStr;

    // Parse the event to a readable string for our logger
    sipxEventToString( fi_Category, fi_pInfo, lv_EventString, sizeof(lv_EventString));

    if ( fi_Category == EVENT_CATEGORY_CALLSTATE )
    {
        SIPX_CALLSTATE_INFO* lv_pCallStateInfo = (SIPX_CALLSTATE_INFO*)fi_pInfo;

        lv_LogStr.Format("(E) L=%d;C=%d; String=%s",
            lv_pCallStateInfo->hLine,
            lv_pCallStateInfo->hCall,
            lv_EventString );
    }
    else if ( fi_Category == EVENT_CATEGORY_LINESTATE )
    {
        SIPX_LINESTATE_INFO* lv_pLineStateInfo = (SIPX_LINESTATE_INFO*) fi_pInfo;

        lv_LogStr.Format("(E) L=%d; String=%s",
            lv_pLineStateInfo->hLine,
            lv_EventString );
    }
    else
    {
        lv_LogStr  = "(E) ";
        lv_LogStr += lv_EventString;
    }
    lv_ThisPtr->m_LogList.AddString(lv_LogStr);
    lv_ThisPtr->m_LogList.SetTopIndex(lv_ThisPtr->m_LogList.GetCount() - 1);

    // Now handle this event...
    try
    {
        // Lookup the right instance to pass this event to...
        switch ( fi_Category )
        {
        case EVENT_CATEGORY_CALLSTATE:
            {
                // CALLSTATE events signify a change in state of a call.
                // States range from the notification of a new call to ringing to connection
                // established to changes in audio state (starting sending, stop sending) to 
                // termination of a call.
                SIPX_CALLSTATE_INFO* lv_pCallStateInfo = (SIPX_CALLSTATE_INFO*)fi_pInfo;

                lv_Line = lv_pCallStateInfo->hLine;

                // Use the hLine data to find the corresponding index in our administration
                for (int i = 0 ; i < lv_ThisPtr->m_NrOfAccounts ; i++)
                {
                    if ( lv_ThisPtr->m_Lines[i].m_SipLine == lv_Line )
                    {
                        lv_ThisPtr->SipXHandleCallState( i, lv_pCallStateInfo );
                    }
                }
            }
            break ;    

        case EVENT_CATEGORY_LINESTATE:  
            {
                // LINESTATE events indicate changes in the status of a line appearance. 
                // Lines identify inbound and outbound identities and can be either provisioned 
                // (hardcoded) or configured to automatically register with a registrar. Lines 
                // also encapsulate the authentication criteria needed for dynamic registrations.
                SIPX_LINESTATE_INFO* lv_pLineStateInfo = (SIPX_LINESTATE_INFO*) fi_pInfo;

                lv_Line = lv_pLineStateInfo->hLine;

                for (int i = 0 ; i < 4 ; i++)
                {
                    if ( lv_ThisPtr->m_Lines[i].m_SipLine == lv_Line )
                    {
                        lv_ThisPtr->SipXHandleLineState( i, lv_pLineStateInfo );
                    }
                }

            }
            break ;    

        case EVENT_CATEGORY_INFO_STATUS:
            // INFO_STATUS events are sent when the application requests sipXtapi to send an 
            // INFO message to another user agent. The status event includes the response for 
            // the INFO method. Application developers should look at this event to determine 
            // the outcome of the INFO message. 
            break;

        case EVENT_CATEGORY_INFO:
            // INFO events are sent to the application whenever an INFO message is received 
            // by the sipXtapi user agent. INFO messages are sent to a specific call. sipXtapi 
            // will automatically acknowledges the INFO message at the protocol layer. 
            break;

        case EVENT_CATEGORY_SUB_STATUS:
            // SUB_STATUS events are sent to the application layer for information on the 
            // subscription state (e.g. OK, Expired). 
            break;

        case EVENT_CATEGORY_NOTIFY:
            // NOTIFY evens are send to the application layer after a remote publisher has 
            // sent data to the application. 
            // The application layer can retrieve the data from this event. 
            break;

        case EVENT_CATEGORY_CONFIG:
            // CONFIG events signify changes in configuration. 
            // For example, when requesting STUN support, a notification is sent with the 
            // STUN outcome (either SUCCESS or FAILURE) 
            break;

        case EVENT_CATEGORY_SECURITY:
            // SECURITY events signify occurences in call security processing. 
            // These events are only sent when using S/MIME or TLS. 
            break;

        case EVENT_CATEGORY_MEDIA:
            // MEDIA events signify changes in the audio state for sipXtapi or a particular call.
            break;

        case EVENT_CATEGORY_KEEPALIVE:
            // KEEPALIVE events signal when a keepalive is started/stopped/fails. 
            // A feedback event will report back your NAT-mapped IP address in some cases. 
            break;

        default:
            {
            }    
            break;
        }
    }
    catch (...)
    {
    }

    return true;
}


// ------------------------------------------------------------------
// Function: SipXHandleCallState
//
// Abstract: Eventhandler for call state changes
//           
// Pre     : Called via GenericSipXEventCallbackHdlr
//
// Remarks : Mind that sipXtapi does not implement/uses all supported
//           states as enumerated below.
//
void CSimpleSipxTermDlg::SipXHandleCallState
(
 int                  fi_LineAdmIndex,
 SIPX_CALLSTATE_INFO* fi_pCallInfo
 )
{
    CString     lv_String;
    char        lv_URI[128];
    char        lv_DNR[24];
    unsigned    lv_Written;
    SIPX_CALL   lv_ThisCall = fi_pCallInfo->hCall;

    // Collect more info on which SIP-client this event occurred...
    if ( SIPX_RESULT_SUCCESS == sipxLineGetURI( fi_pCallInfo->hLine, lv_URI, sizeof( lv_URI ), &lv_Written) )
    {
        // BTW: lv_DNR is actually the so-called DisplayName.
        if ( SIPX_RESULT_SUCCESS != sipxUtilUrlGetDisplayName( lv_URI, lv_DNR, sizeof( lv_DNR ) ) )
        {
            lv_URI[0]  = 0x00;
            lv_DNR[0]  = 0x00;
            lv_Written = 0;
        }
    }
    else
    {
        lv_URI[0]  = 0x00;
        lv_Written = 0;
    }
    if ( 0x00 == lv_URI[0] )
    {
        strcpy( lv_URI, "-" );
    }

    switch ( fi_pCallInfo->event )
    {
    case CALLSTATE_NEWCALL:
        // The NEWCALL event indicates that a new call has been created 
        // automatically by the sipXtapi. This event is most frequently 
        // generated in response to an inbound call request.
        break;

    case CALLSTATE_OFFERING:
        // An OFFERING state indicates that a new call invitation has been extended this 
        // user agent. Application developers should invoke sipxCallAccept(), sipxCallReject() 
        // or sipxCallRedirect() in response. 
        // Not responding will result in an implicit call sipXcallReject().
        // ---

        // In case of new incoming call, immediately accept this.
        // When doing so, the state enters ALERTING. Else the call is automatically
        // cleared after few seconds.

        if ( SIPX_CALL_NULL == m_Lines[fi_LineAdmIndex].m_ActiveCall )
        {
            m_LogList.AddString("--> Accepting this call");
            sipxCallAccept( lv_ThisCall );
        }
        else
        {
            m_LogList.AddString("--> REJECTING this call (already in call");
            sipxCallReject( lv_ThisCall );
        }

        break;

    case CALLSTATE_ALERTING:
        // An ALERTING state indicates that an inbound call has been accepted and the 
        // application layer should alert the end user. The alerting state is limited 
        // to 3 minutes in most configurations; afterwards the call will be canceled. 
        // Applications will generally play some sort of ringing tone in response to this event.

        // This is active call. Take it over...
        m_Lines[fi_LineAdmIndex].m_ActiveCall = lv_ThisCall;

        lv_String.Format("%s <= %s", m_Lines[fi_LineAdmIndex].m_Dnr, lv_URI);

        m_Lines[fi_LineAdmIndex].m_PhoneDlg->ChangeUpperDisplay( lv_String );
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("RINGING"));
        m_Lines[fi_LineAdmIndex].m_State = TS_ALERTING;
        break;

    case CALLSTATE_DIALTONE:
        // The DIALTONE event indicates that a new call has been created for the 
        // purpose of placing an outbound call. The application layer should determine 
        // if it needs to simulate dial tone for the end user

        if ( TS_ENQ_DIALING == m_Lines[fi_LineAdmIndex].m_State )
        {
            m_Lines[fi_LineAdmIndex].m_State = TS_ENQ_DIALING;
        }
        else
        {
            m_Lines[fi_LineAdmIndex].m_State = TS_DIALING;
        }
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("DIALTONE - Waiting for input"));

        break;

    case CALLSTATE_CONNECTED:
        // The CONNECTED state indicates that call has been setup between the local and 
        // remote party. Network audio should be flowing provided and the microphone and 
        // speakers should be engaged
        m_Lines[fi_LineAdmIndex].m_ActiveCall = lv_ThisCall;

        m_Lines[fi_LineAdmIndex].m_PhoneDlg->m_UpperDisplay.GetWindowText( lv_String );
        lv_String.Replace("=>", "==");
        lv_String.Replace("<=", "==");
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->ChangeUpperDisplay( lv_String );
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("CONNECTED"));
        m_Lines[fi_LineAdmIndex].m_State = TS_CONNECTED;
        break;

    case CALLSTATE_DISCONNECTED:
        // The DISCONNECTED state indicates that a call was disconnected or failed 
        // to connect. A call may move into the DISCONNECTED states from almost 
        // every other state. 
        // Please review the DISCONNECTED minor events to understand the cause.
        sipxCallDestroy( &fi_pCallInfo->hCall );

        // Check which call this is...

        if ( m_Lines[fi_LineAdmIndex].m_ActiveCall  == lv_ThisCall )
        {
            m_Lines[fi_LineAdmIndex].m_ActiveCall  = SIPX_CALL_NULL;

            m_Lines[fi_LineAdmIndex].m_PhoneDlg->ChangeUpperDisplay( "" );

            m_Lines[fi_LineAdmIndex].m_State = TS_DISCONNECTED;
            m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("IDLE"));
        }
        else
        {
            if ( m_Lines[fi_LineAdmIndex].m_HeldCall  == lv_ThisCall )
            {
                m_Lines[fi_LineAdmIndex].m_PhoneDlg->ChangeLowerDisplay( lv_String );
                m_Lines[fi_LineAdmIndex].m_HeldCall = SIPX_CALL_NULL;
            }
            else
            {
                // This disconnecting call is not part of our
                // current active/held parties...
            }
        }

        break;

    case CALLSTATE_REMOTE_OFFERING:
        // The REMOTE_OFFERING event indicates that a call setup invitation has been 
        // sent to the remote party. The invitation may or may not every receive a response. 
        // If a response is not received in a timely manor, sipXtapi will move the call into 
        // a disconnected state. 
        // If calling another sipXtapi user agent, the reciprocal state is OFFER
        m_Lines[fi_LineAdmIndex].m_State = TS_WAIT_FOR_ANSWER;

        m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("DELIVERED-offering"));
        break;

    case CALLSTATE_REMOTE_ALERTING:
        // The REMOTE_ALERTING event indicates that a call setup invitation has been 
        // accepted and the end user is in the alerting state (ringing). Depending on 
        // the SIP configuration, end points, and proxy servers involved, this event 
        // should only last for 3 minutes. Afterwards,the state will automatically move 
        // to DISCONNECTED. 
        // If calling another sipXtapi user agent, the reciprocate state is ALERTING. 
        // 
        // Pay attention to the cause code for this event. If the cause code is 
        // "CALLSTATE_CAUSE_EARLY_MEDIA", the remote the party is sending early media 
        // (e.g. gateway is producing ringback or audio feedback). In this case, the 
        // user agent should not produce local ringback
        m_Lines[fi_LineAdmIndex].m_State = TS_WAIT_FOR_ANSWER;
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("DELIVERED-accepted"));
        break;

    case CALLSTATE_BRIDGED:
        // The BRIDGED state indicates that a call is active, however, the local 
        // microphone/speaker are not engaged. If this call is part of a conference, the 
        // party will be able to talk with other BRIDGED conference parties. Application 
        // developers can still play and record media

        m_Lines[fi_LineAdmIndex].m_State = TS_CONNECTED;
        m_Lines[fi_LineAdmIndex].m_ActiveCall = lv_ThisCall;
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->m_UpperDisplay.GetWindowText( lv_String );
        lv_String.Replace("=>", "==");
        lv_String.Replace("<=", "==");
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->ChangeUpperDisplay( lv_String );
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("CONNECTED"));
        break;

    case CALLSTATE_HELD:
        // The HELD state indicates that a call is both locally and remotely held. No 
        // network audio is flowing and the local microphone and speaker are not engaged.

        if ( TS_ENQ_DIALING != m_Lines[fi_LineAdmIndex].m_State )
        {
            m_Lines[fi_LineAdmIndex].m_State = TS_HOLD;
            m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("IDLE (with call on hold)"));
        }

        // But probably we always want this be TS_ENQ_DIALING or do
        // nothing on this event...
        break;

    case CALLSTATE_REMOTE_HELD:
        // The REMOTE_HELD state indicates that the remote party is on hold. Locally, the 
        // microphone and speaker are still engaged, however, no network audio is flowing.

        m_Lines[fi_LineAdmIndex].m_State = TS_CONNECTED;
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("CONNECTED (remote on hold)"));
        break;

    case CALLSTATE_DESTROYED:
        // The DESTORYED event indicates the underlying resources have been removed
        // for a call. This is the last event that the application will receive for 
        // any call. The call handle is invalid after this event is received.

        // When both calls are destroyed, then this terminal is ON-HOOK state

        if ( ( SIPX_CALL_NULL == m_Lines[fi_LineAdmIndex].m_ActiveCall ) &&
             ( SIPX_CALL_NULL == m_Lines[fi_LineAdmIndex].m_HeldCall )
            )
        {
            m_Lines[fi_LineAdmIndex].m_PhoneDlg->ChangeUpperDisplay( "" );
            m_Lines[fi_LineAdmIndex].m_PhoneDlg->ChangeLowerDisplay( "" );
            m_Lines[fi_LineAdmIndex].m_State = TS_ON_HOOK;
            m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("IDLE"));
        }

        break;

    case CALLSTATE_TRANSFER_EVENT:
        // The transfer state indicates a state change in a transfer attempt. Please 
        // see the CALLSTATE_TRANSFER_EVENT cause codes for details on each state transition.
        switch( fi_pCallInfo->cause )
        {
        case CALLSTATE_CAUSE_TRANSFER_INITIATED:
            // A transfer attempt has been initiated. 
            // This event is sent when a user agent attempts either a blind or 
            // consultative transfer.
            m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("TRANSFERING"));
            break;

        case CALLSTATE_CAUSE_TRANSFER_ACCEPTED:
            // A transfer attempt has been accepted by the remote transferee. 
            // This event indicates that the transferee supports transfers (REFER method). 
            // The event is fired upon a 2xx class response to the SIP REFER request. 
            m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("TRANSFERED"));
            break;

        case CALLSTATE_CAUSE_TRANSFER_TRYING:
            // The transfer target is attempting the transfer. 
            // This event is sent when transfer target (or proxy / B2BUA) receives the call 
            // invitation, but before the the tranfer target accepts is. 
            break;

        case CALLSTATE_CAUSE_TRANSFER_RINGING:
            // The transfer target is ringing. 
            // This event is generally only sent during blind transfer. Consultative 
            // transfer should proceed directly to TRANSFER_SUCCESS or TRANSFER_FAILURE.
            break;

        case CALLSTATE_CAUSE_TRANSFER_SUCCESS:
            // The transfer was completed successfully. 
            // The original call to transfer target will automatically disconnect.
            break;

        case CALLSTATE_CAUSE_TRANSFER_FAILURE:
            // The transfer failed. 
            // After a transfer fails, the application layer is responsible for recovering 
            // original call to the transferee. That call is left on hold. 
            break;

        default:
            m_Lines[fi_LineAdmIndex].m_State = TS_CONNECTED;
            m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("CONNECTED"));
            break;
        }
        break;

    case CALLSTATE_UNKNOWN:
        // An UNKNOWN event is generated when the state for a call is no longer known. 
        // This is generally an error condition; see the minor event for specific causes.
        break;

    default:
        break;
    }
}

// ------------------------------------------------------------------
// Function: SipXHandleLineState
//
// Abstract: Eventhandler for Line State changes
//           
// Pre     : Called via GenericSipXEventCallbackHdlr
//
// Remarks : 
//
void CSimpleSipxTermDlg::SipXHandleLineState
(
 int                  fi_LineAdmIndex,
 SIPX_LINESTATE_INFO* fi_pLineStateInfo
 )
{
    CString lv_String;
    CString lv_LogStr;

    switch ( fi_pLineStateInfo->event )
    {
    case LINESTATE_REGISTERING:
        // The REGISTERING event is fired when sipXtapi has successfully sent a REGISTER 
        // message, but has not yet received a success response from the registrar server.
        break;

    case LINESTATE_REGISTERED:
        // The REGISTERED event is fired after sipXtapi has received a response from the 
        // registrar server, indicating a successful registration.

        m_Lines[fi_LineAdmIndex].m_Activated = true;

        lv_String.Format("%s (Registered)", m_Lines[fi_LineAdmIndex].m_Dnr);
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->SetWindowText(lv_String);
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->UpdateConnState(_T("IDLE"));
        break;

    case LINESTATE_UNREGISTERING:
        // The UNREGISTERING event is fired when sipXtapi has successfully sent a REGISTER 
        // message with an expires=0 parameter, but has not yet received a success response 
        // from the registrar server.
        m_Lines[fi_LineAdmIndex].m_Activated = false;
        break;

    case LINESTATE_UNREGISTERED:
        // The UNREGISTERED event is fired after sipXtapi has received a response from the 
        // registrar server, indicating a successful un-registration.
        m_Lines[fi_LineAdmIndex].m_Activated = false;
        break;

    case LINESTATE_REGISTER_FAILED:
        // The REGISTER_FAILED event is fired to indicate a failure of REGISTRATION.
        //
        // It is fired in the following cases: The client could not connect to the registrar 
        // server. The registrar server challenged the client for authentication credentials, 
        // and the client failed to supply valid credentials. The registrar server did not 
        // generate a success response (status code == 200) within a timeout period. 
        m_Lines[fi_LineAdmIndex].m_Activated = false;
        lv_String.Format("%s (Registration ** FAILED **)", m_Lines[fi_LineAdmIndex].m_Dnr);
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->SetWindowText(lv_String);
        break;

    case LINESTATE_UNREGISTER_FAILED:
        // The UNREGISTER_FAILED event is fired to indicate a failure of un-REGISTRATION. 
        //
        // It is fired in the following cases: The client could not connect to the registrar 
        // server. The registrar server challenged the client for authentication credentials, 
        // and the client failed to supply valid credentials. The registrar server did not 
        // generate a success response (status code == 200) within a timeout period. 

        break;

    case LINESTATE_PROVISIONED :
        // The PROVISIONED event is fired when a sipXtapi Line is added, and Registration is 
        // not requested (i.e. sipxLineAdd is called with a bRegister parameter of false. 
        lv_String.Format("%s (Trying to register...)", m_Lines[fi_LineAdmIndex].m_Dnr);
        m_Lines[fi_LineAdmIndex].m_PhoneDlg->SetWindowText(lv_String);
        break;

    case LINESTATE_UNKNOWN:
        // Initial line state
        break;

    default:
        break;
    }
}

void CSimpleSipxTermDlg::OnBnClickedOk()
{
    // TODO: Add your control notification handler code here
    OnOK();
}
