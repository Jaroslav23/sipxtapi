// SimpleSipxTermDlg.h : header file
//

#if !defined(AFX_SIMPLESIPXTERMDLG_H__ABC316A8_6A13_46A1_912C_0373317B092A__INCLUDED_)
#define AFX_SIMPLESIPXTERMDLG_H__ABC316A8_6A13_46A1_912C_0373317B092A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Include the sipXtapi header files...
#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"


/////////////////////////////////////////////////////////////////////////////
// CSimpleSipxTermDlg dialog
class CSipPhoneDlg;
class CStartupSettingsDlg;

class CSimpleSipxTermDlg : public CDialog
{
    // Construction
public:
    CSimpleSipxTermDlg(CWnd* pParent = NULL);	// standard constructor

    // Dialog Data
    //{{AFX_DATA(CSimpleSipxTermDlg)
    enum { IDD = IDD_SIMPLESIPXTERM_DIALOG };
    CListBox	m_LogList;
    //}}AFX_DATA

    typedef DWORD TerminalState;
    enum
    {
        // Make it suitable for bit mapping
        TS_ON_HOOK         = 0x0001,
        TS_OFF_HOOK        = 0x0002,
        TS_DIALING         = 0x0004,  // See also enquiry dialing
        TS_WAIT_FOR_ANSWER = 0x0008,
        TS_ALERTING        = 0x0010,
        TS_CONNECTED       = 0x0020,
        TS_DISCONNECTED    = 0x0040,
        TS_HOLD            = 0x0080,
        TS_CALL_WAITING    = 0x0100,
        // -------------------------
        TS_ENQ_DIALING     = 0x0200,
        TS_SPARE_2         = 0x0400,
        TS_SPARE_3         = 0x0800,
        TS_SPARE_4         = 0x1000,
        TS_SPARE_5         = 0x2000,
        // -------------------------
        TS_INIT            = 0x4000,
        TS_NULL            = 0x8000
    };

    typedef DWORD PhoneButtonBitmaps;
    enum 
    {
        PB_NONE         = 0L,
        PB_MAKECALL     = 0x00000001,
        PB_ANSWERCALL   = 0x00000010,
        PB_CLEARCALL    = 0x00000100,
        PB_HOLDCALL     = 0x00001000,
        PB_TRANSFERCALL = 0x00010000,
        PB_ALL          = 0x11111111
    };

    class LineInfos {
    public:
        LineInfos();
        virtual ~LineInfos();

        CSipPhoneDlg* m_PhoneDlg;
        CString       m_Dnr;
        SIPX_LINE     m_SipLine;
        SIPX_CALL     m_ActiveCall;
        SIPX_CALL     m_HeldCall;
        TerminalState m_State;

        bool          m_Activated;
    };

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSimpleSipxTermDlg)
protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
    //}}AFX_VIRTUAL

public:
    void OnSipPhoneEvent( void );
    void CopySettings   ( CStartupSettingsDlg* fi_SettingsDlg );

    void DoAcceptRejectCall( SIPX_LINE fi_Line, SIPX_CALL );

    static bool GenericSipXCallbackHdlr( SIPX_EVENT_CATEGORY fi_Category,
                                         void*               fi_pInfo,
                                         void*               fi_pUserData ); 

    void SipXHandleLineState ( int fi_LineAdmIndex, SIPX_LINESTATE_INFO* fi_pLineStateInfo );
    void SipXHandleCallState ( int fi_LineAdmIndex, SIPX_CALLSTATE_INFO* fi_pCallInfo );

    // Call Manipulation
    void MakeCall           ( int             fi_PhoneIndex, 
                              const CString&  fi_DestDnr );
    void ClearCall          ( int             fi_PhoneIndex );
    void AnswerCall         ( int             fi_PhoneIndex );
    void HoldUnholdCall     ( int             fi_PhoneIndex );
    void TransferCall       ( int             fi_PhoneIndex );

    // Implementation
protected:
    HICON m_hIcon;

    CString         m_Interface;
    int             m_SipPort;
    int             m_RtpPort;
    int             m_NrOfAccounts;
    CString         m_SipServerIP;
    int             m_StartDnr;
    int             m_SipServerPort;

    LineInfos       m_Lines[4];
    int             m_NbrOfLines;


    // sipX 
    SIPX_INST   m_sipXInstance;


    // Generated message map functions
    //{{AFX_MSG(CSimpleSipxTermDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnExit();
    afx_msg void OnDestroy();
    afx_msg void OnShowTerminals();
    afx_msg void OnBnClickedOk();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMPLESIPXTERMDLG_H__ABC316A8_6A13_46A1_912C_0373317B092A__INCLUDED_)
