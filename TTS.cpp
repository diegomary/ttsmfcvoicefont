// TTS.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "TTS.h"
#include "MainFrm.h"

#include "TTSDoc.h"
#include "TTSView.h"
#include ".\tts.h"
#include "passwordcheck.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CTTSApp

BEGIN_MESSAGE_MAP(CTTSApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	
END_MESSAGE_MAP()


// CTTSApp construction

CTTSApp::CTTSApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CTTSApp object

CTTSApp theApp;
// This identifier was generated to be statistically unique for your app
// You may change it if you prefer to choose a specific identifier

// {45747DEE-03D5-42F3-A860-AA19A0887724}
static const CLSID clsid =
{ 0x45747DEE, 0x3D5, 0x42F3, { 0xA8, 0x60, 0xAA, 0x19, 0xA0, 0x88, 0x77, 0x24 } };
const GUID CDECL BASED_CODE _tlid =
		{ 0x22EC0402, 0x1E83, 0x43DB, { 0xB7, 0x81, 0xB4, 0xB6, 0xED, 0xA8, 0xC1, 0xB0 } };
const WORD _wVerMajor = 1;
const WORD _wVerMinor = 0;


// CTTSApp initialization

BOOL CTTSApp::InitInstance()
{
	PasswordCheck alfa;
	alfa.DoModal();
	
	
	
	// i added the following line on 23 /2/2006 to allow richedit 2 in the form view.
	AfxInitRichEdit2( );

// the following enables the low fragmented heap.
ULONG  HeapFragValue = 2;
    if(HeapSetInformation(GetProcessHeap(),
                       HeapCompatibilityInformation,
                       &HeapFragValue,
                       sizeof(HeapFragValue))
    )
    {
        AfxMessageBox("low-fragmentation heap feature\n is sustained and activated !!\nThis is only available in\nWindows XP\nAll Windows Vista\nVersion.",MB_OK |MB_ICONINFORMATION ,0);
    }
   else AfxMessageBox("NO low-fragmentation heap feature!!\n",MB_OK |MB_ICONINFORMATION ,0);

//************************** end heap managing
	// InitCommonControls() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CTTSDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CTTSView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);
	// Connect the COleTemplateServer to the document template
	//  The COleTemplateServer creates new documents on behalf
	//  of requesting OLE containers by using information
	//  specified in the document template
	m_server.ConnectTemplate(clsid, pDocTemplate, TRUE);
		// Note: SDI applications register server objects only if /Embedding
		//   or /Automation is present on the command line
	// Enable DDE Execute open
	EnableShellOpen();
	RegisterShellFileTypes(TRUE);
	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// App was launched with /Embedding or /Automation switch.
	// Run app as automation server.
	if (cmdInfo.m_bRunEmbedded || cmdInfo.m_bRunAutomated)
	{
		// Register all OLE server factories as running.  This enables the
		//  OLE libraries to create objects from other applications
		COleTemplateServer::RegisterAll();

		// Don't show the main window
		return TRUE;
	}
	// App was launched with /Unregserver or /Unregister switch.  Unregister
	// typelibrary.  Other unregistration occurs in ProcessShellCommand().
	else if (cmdInfo.m_nShellCommand == CCommandLineInfo::AppUnregister)
	{
		UnregisterShellFileTypes();
		m_server.UpdateRegistry(OAT_DISPATCH_OBJECT, NULL, NULL, FALSE);
		AfxOleUnregisterTypeLib(_tlid, _wVerMajor, _wVerMinor);
	}
	// App was launched standalone or with other switches (e.g. /Register
	// or /Regserver).  Update registry entries, including typelibrary.
	else
	{
		m_server.UpdateRegistry(OAT_DISPATCH_OBJECT);
		COleObjectFactory::UpdateRegistryAll();
		AfxOleRegisterTypeLib(AfxGetInstanceHandle(), _tlid);
	}
	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	
public:
	afx_msg void OnBnClickedButton1();
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	
	ON_BN_CLICKED(IDC_BUTTON1, &CAboutDlg::OnBnClickedButton1)
END_MESSAGE_MAP()

// App command to run the dialog
void CTTSApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CTTSApp message handlers




void CAboutDlg::OnBnClickedButton1()
{

	// TODO: Add your control notification handler code here
}
