// SimpleSipxTerm.h : main header file for the SIMPLESIPXTERM application
//

#if !defined(AFX_SIMPLESIPXTERM_H__D52B92B9_806B_4B8B_8F5B_D81E49EF5151__INCLUDED_)
#define AFX_SIMPLESIPXTERM_H__D52B92B9_806B_4B8B_8F5B_D81E49EF5151__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "StartupSettingsDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CSimpleSipxTermApp:
// See SimpleSipxTerm.cpp for the implementation of this class
//

class CSimpleSipxTermApp : public CWinApp
{
public:
	CSimpleSipxTermApp();

  void UpdateWithRegistry( bool fi_Startup, CStartupSettingsDlg* fi_SettingsDlg );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimpleSipxTermApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSimpleSipxTermApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMPLESIPXTERM_H__D52B92B9_806B_4B8B_8F5B_D81E49EF5151__INCLUDED_)
