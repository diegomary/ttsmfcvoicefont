// TTS.h : main header file for the TTS application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
//#include "resource1.h"		// audio Fonts

// CTTSApp:
// See TTS.cpp for the implementation of this class
//

class CTTSApp : public CWinApp
{
public:
	CTTSApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	COleTemplateServer m_server;
		// Server object for document creation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	};

extern CTTSApp theApp;