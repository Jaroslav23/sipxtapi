// SimpleSipxTerm.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "SimpleSipxTerm.h"
#include "SimpleSipxTermDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimpleSipxTermApp

BEGIN_MESSAGE_MAP(CSimpleSipxTermApp, CWinApp)
    //{{AFX_MSG_MAP(CSimpleSipxTermApp)
    // NOTE - the ClassWizard will add and remove mapping macros here.
    //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSimpleSipxTermApp construction

CSimpleSipxTermApp::CSimpleSipxTermApp()
{
    // TODO: add construction code here,
    // Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CSimpleSipxTermApp object

CSimpleSipxTermApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CSimpleSipxTermApp initialization

BOOL CSimpleSipxTermApp::InitInstance()
{
    if (!AfxSocketInit())
    {
        AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
        return FALSE;
    }

    AfxEnableControlContainer();

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    //  of your final executable, you should remove from the following
    //  the specific initialization routines you do not need.

#ifdef _AFXDLL
    Enable3dControls();       // Call this when using MFC in a shared DLL
                              // Not needed anymore with VS2005 but kept here for VS6 environments
#else
    Enable3dControlsStatic(); // Call this when linking to MFC statically
#endif
    AfxInitRichEdit2();

    CStartupSettingsDlg* lv_StartupDlg = NULL;

    lv_StartupDlg = new CStartupSettingsDlg;

    UpdateWithRegistry( true, lv_StartupDlg );

    int nResponse = lv_StartupDlg->DoModal();

    if (nResponse == IDOK)
    {
        UpdateWithRegistry( false, lv_StartupDlg );

        CSimpleSipxTermDlg dlg;
        m_pMainWnd = &dlg;

        dlg.CopySettings( lv_StartupDlg );

        nResponse = dlg.DoModal();
        if (nResponse == IDOK)
        {
            // TODO: Place code here to handle when the dialog is
            //  dismissed with OK
        }
        else if (nResponse == IDCANCEL)
        {
            // TODO: Place code here to handle when the dialog is
            //  dismissed with Cancel
        }
    }
    else
    {
    }

    delete lv_StartupDlg;

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}

static const CString REGISTRY_MAIN_ENTRY    = "SOFTWARE\\NECP\\";
static const CString PROFILE_ENTRY          = "SimpleSipXterm";
static const CString ENTRY_INTERFACE        = "Interface";
static const CString ENTRY_SIP_LISTEN_PORT  = "LocalSipListenPort";
static const CString ENTRY_RTP_PORT         = "LocalRtpPort";
static const CString ENTRY_NR_CLIENTS       = "NbrOfClients";
static const CString ENTRY_START_DNR        = "StartDnr";
static const CString ENTRY_SIP_SERVER_IP    = "SipServerIP";
static const CString ENTRY_SIP_SERVER_PORT  = "SipServerPort";

// DEFAULTS
static const int  DEFAULT_NR_CLIENTS        = 4;
static const int  DEFAULT_SIP_LISTEN_PORT   = 5066;
static const int  DEFAULT_RTP_PORT          = 49000;
static const int  DEFAULT_START_DNR         = 2100;
static const char DEFAULT_INTERFACE[]       = "0.0.0.0";
static const char DEFAULT_SIP_SERVER_IP[]   = "127.0.0.1";
static const int  DEFAULT_SIP_SERVER_PORT   = 5060;

void CSimpleSipxTermApp::UpdateWithRegistry( bool fi_Startup, CStartupSettingsDlg* fi_SettingsDlg )
{
    HKEY  lv_Key = NULL;
    DWORD lv_Size;
    DWORD lv_Type;
    DWORD lv_Data;
    DWORD lv_IntResult;
    bool  lv_BuildRegistry = false;
    char  szMainEntry[256];
    strcpy(szMainEntry, REGISTRY_MAIN_ENTRY );
    strcat(szMainEntry, PROFILE_ENTRY);

    // Mind that some very basic registry coding is done here. 
    // No doublechecking in case new registry entries were added with a
    // new version of this piece of software...
    DWORD lv_Result = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
        szMainEntry,
        0,
        0,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WRITE,
        0,
        &lv_Key,
        &lv_IntResult);

    if ( lv_Result == ERROR_SUCCESS )
    {
        // Now... did we need to create the entry?
        if ( lv_IntResult == REG_CREATED_NEW_KEY )
        {
            lv_BuildRegistry = true;
        }
        else
        {
            // Key exist. But do the entries also exist?
            // Try to read one of them.

            // Buffersize
            lv_Result = RegQueryValueEx( lv_Key,
                ENTRY_NR_CLIENTS,
                0,
                &lv_Type,
                (PBYTE)&lv_Data,
                &lv_Size );

            if (lv_Result != ERROR_SUCCESS )
            {
                lv_BuildRegistry = true;
            }
        }
    }
    else
    {
        // Key exist. But do the entries also exist?
        // Try to read one of them.

        // Buffersize
        lv_Result = RegQueryValueEx( lv_Key,
            ENTRY_NR_CLIENTS,
            0,
            &lv_Type,
            (PBYTE)&lv_Data,
            &lv_Size );

        if (lv_Result != ERROR_SUCCESS )
        {
            lv_BuildRegistry = true;
        }
    }

    if ( lv_BuildRegistry == true )
    {
        // Yes.. so create all defaults

        // -------------------------------------
        RegSetValueEx( lv_Key,
            ENTRY_INTERFACE,
            0,
            REG_SZ,
            (PBYTE)DEFAULT_INTERFACE,
            sizeof(DEFAULT_INTERFACE)
            );
        fi_SettingsDlg->m_Interface = DEFAULT_INTERFACE;

        // -------------------------------------
        lv_Data = DEFAULT_SIP_LISTEN_PORT;
        RegSetValueEx( lv_Key,
            ENTRY_SIP_LISTEN_PORT,
            0,
            REG_DWORD,
            (PBYTE)&lv_Data,
            sizeof(DWORD)
            );
        fi_SettingsDlg->m_SipPort = DEFAULT_SIP_LISTEN_PORT;

        // -------------------------------------
        lv_Data = DEFAULT_RTP_PORT;
        RegSetValueEx( lv_Key,
            ENTRY_RTP_PORT,
            0,
            REG_DWORD,
            (PBYTE)&lv_Data,
            sizeof(DWORD)
            );
        fi_SettingsDlg->m_RtpPort = DEFAULT_RTP_PORT;

        // -------------------------------------
        lv_Data = DEFAULT_NR_CLIENTS;
        lv_Result = RegSetValueEx( lv_Key,
            ENTRY_NR_CLIENTS,
            0,
            REG_DWORD,
            (PBYTE)&lv_Data,
            sizeof(DWORD) );
        fi_SettingsDlg->m_NrOfAccounts = DEFAULT_NR_CLIENTS;

        // -------------------------------------
        lv_Data = DEFAULT_START_DNR;
        RegSetValueEx( lv_Key,
            ENTRY_START_DNR,
            0,
            REG_DWORD,
            (PBYTE)&lv_Data,
            sizeof(DWORD)
            );
        fi_SettingsDlg->m_StartDnr = DEFAULT_START_DNR;

        // -------------------------------------
        RegSetValueEx( lv_Key,
            ENTRY_SIP_SERVER_IP,
            0,
            REG_SZ,
            (PBYTE)DEFAULT_SIP_SERVER_IP,
            sizeof(DEFAULT_SIP_SERVER_IP)
            );
        fi_SettingsDlg->m_SipServerIP = DEFAULT_SIP_SERVER_IP;

        // -------------------------------------
        lv_Data = DEFAULT_SIP_SERVER_PORT;
        RegSetValueEx( lv_Key,
            ENTRY_SIP_SERVER_PORT,
            0,
            REG_DWORD,
            (PBYTE)&lv_Data,
            sizeof(DWORD)
            );
        fi_SettingsDlg->m_SipServerPort = DEFAULT_SIP_SERVER_PORT;
    }
    else
    {
        DWORD lv_Size;
        DWORD lv_Type;
        char  lv_TmpString[256];

        if ( fi_Startup )
        {
            // Starting up, so reading registry

            // -------------------------------------
            {
                char* lv_Buffer;
                lv_Result = RegQueryValueEx( lv_Key,
                    ENTRY_INTERFACE,
                    0,
                    &lv_Type,
                    NULL,
                    &lv_Size );

                lv_Buffer = (char*)malloc( lv_Size + 16 );

                lv_Result = RegQueryValueEx( lv_Key,
                    ENTRY_INTERFACE,
                    0,
                    &lv_Type,
                    (unsigned char*)lv_Buffer,
                    &lv_Size );

                fi_SettingsDlg->m_Interface = lv_Buffer;

                delete( lv_Buffer );
                lv_Buffer = NULL;
            }

            // -------------------------------------
            lv_Result = RegQueryValueEx( lv_Key,
                ENTRY_SIP_LISTEN_PORT,
                0,
                &lv_Type,
                (PBYTE)&lv_Data,
                &lv_Size );
            fi_SettingsDlg->m_SipPort = lv_Data;

            // -------------------------------------
            lv_Result = RegQueryValueEx( lv_Key,
                ENTRY_RTP_PORT,
                0,
                &lv_Type,
                (PBYTE)&lv_Data,
                &lv_Size );
            fi_SettingsDlg->m_RtpPort = lv_Data;

            // -------------------------------------
            lv_Result = RegQueryValueEx( lv_Key,
                ENTRY_NR_CLIENTS,
                0,
                &lv_Type,
                (PBYTE)&lv_Data,
                &lv_Size );
            fi_SettingsDlg->m_NrOfAccounts = lv_Data;
            // -------------------------------------
            lv_Result = RegQueryValueEx( lv_Key,
                ENTRY_START_DNR,
                0,
                &lv_Type,
                (PBYTE)&lv_Data,
                &lv_Size );
            fi_SettingsDlg->m_StartDnr = lv_Data;

            // -------------------------------------
            {
                char* lv_Buffer;
                lv_Result = RegQueryValueEx( lv_Key,
                    ENTRY_SIP_SERVER_IP,
                    0,
                    &lv_Type,
                    NULL,
                    &lv_Size );

                lv_Buffer = (char*)malloc( lv_Size + 16 );

                lv_Result = RegQueryValueEx( lv_Key,
                    ENTRY_SIP_SERVER_IP,
                    0,
                    &lv_Type,
                    (unsigned char*)lv_Buffer,
                    &lv_Size );

                fi_SettingsDlg->m_SipServerIP = lv_Buffer;

                delete( lv_Buffer );
                lv_Buffer = NULL;
            }


            // -------------------------------------
            lv_Result = RegQueryValueEx( lv_Key,
                ENTRY_SIP_SERVER_PORT,
                0,
                &lv_Type,
                (PBYTE)&lv_Data,
                &lv_Size );
            fi_SettingsDlg->m_SipServerPort = lv_Data;
        }
        else
        {
            // Closing down, so updating towards registry
            // --------------------------------------------

            strcpy( lv_TmpString, fi_SettingsDlg->m_Interface );
            RegSetValueEx( lv_Key,
                ENTRY_INTERFACE,
                0,
                REG_SZ,
                (PBYTE)lv_TmpString,
                sizeof(lv_TmpString)
                );

            // -------------------------------------
            lv_Data = fi_SettingsDlg->m_SipPort;
            RegSetValueEx( lv_Key,
                ENTRY_SIP_LISTEN_PORT,
                0,
                REG_DWORD,
                (PBYTE)&lv_Data,
                sizeof(DWORD)
                );
            // -------------------------------------
            lv_Data = fi_SettingsDlg->m_RtpPort;
            RegSetValueEx( lv_Key,
                ENTRY_RTP_PORT,
                0,
                REG_DWORD,
                (PBYTE)&lv_Data,
                sizeof(DWORD)
                );
            // -------------------------------------
            lv_Data = fi_SettingsDlg->m_NrOfAccounts;
            lv_Result = RegSetValueEx( lv_Key,
                ENTRY_NR_CLIENTS,
                0,
                REG_DWORD,
                (PBYTE)&lv_Data,
                sizeof(DWORD) );
            // -------------------------------------
            lv_Data = fi_SettingsDlg->m_StartDnr;
            RegSetValueEx( lv_Key,
                ENTRY_START_DNR,
                0,
                REG_DWORD,
                (PBYTE)&lv_Data,
                sizeof(DWORD)
                );
            // -------------------------------------

            strcpy( lv_TmpString, fi_SettingsDlg->m_SipServerIP );
            RegSetValueEx( lv_Key,
                ENTRY_SIP_SERVER_IP,
                0,
                REG_SZ,
                (PBYTE)lv_TmpString,
                sizeof(lv_TmpString)
                );

            // -------------------------------------
            lv_Data = fi_SettingsDlg->m_SipServerPort;
            RegSetValueEx( lv_Key,
                ENTRY_SIP_SERVER_PORT,
                0,
                REG_DWORD,
                (PBYTE)&lv_Data,
                sizeof(DWORD)
                );
        }
    }

    // And close registry key
    RegCloseKey( lv_Key );

}