// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "TTS.h"

#include "MainFrm.h"
#include ".\mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_UPDATE_COMMAND_UI(ID_VIEWPHONEMS, OnUpdateViewphonems)
	ON_COMMAND(ID_VIEWPHONEMS, OnViewphonems)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}


int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
//		if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD |
//		WS_VISIBLE | CBRS_TOP
//		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
//		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
//	{
//		TRACE0("Failed to create toolbar\n");
//		return -1;      // fail to create
//	}
//
//m_wndToolBar.ShowWindow(0);
//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
//	EnableDocking(CBRS_ALIGN_ANY);
//	DockControlBar(&m_wndToolBar);


// Create the Phonems toolbar **********************************************************
 
	
//	if (!m_wndPhonemsBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | TBSTYLE_WRAPABLE |
//        WS_VISIBLE | CBRS_LEFT| CBRS_GRIPPER | CBRS_TOOLTIPS |CBRS_FLYBY | CBRS_SIZE_FIXED   ) || 
//        !m_wndPhonemsBar.LoadToolBar(IDR_TOOLBAR1))
//   
//	
//	{
//        TRACE0("Failed to create toolbar\n");
//        return FALSE;    // fail to create
//    }
//
//// TODO: Delete these three lines if you don't want the toolbar to be dockable
//m_wndPhonemsBar.EnableDocking(0);// if we specify 0 the toolbar won't dock.
//m_wndPhonemsBar.SetWindowTextA(CString("Phonems"));
//
//	//EnableDocking(CBRS_ALIGN_ANY);
//	//DockControlBar(&m_wndPhonemsBar);
//UINT nStyle = m_wndPhonemsBar.GetButtonStyle( 5 );
//// Augment the state for wrapping
//nStyle |= TBBS_WRAPPED;
//m_wndPhonemsBar.SetButtonStyle( 4, nStyle );
//
//nStyle = m_wndPhonemsBar.GetButtonStyle( 9 );
//// Augment the state for wrapping
//nStyle |= TBBS_WRAPPED;
//m_wndPhonemsBar.SetButtonStyle(9, nStyle );
//
//nStyle = m_wndPhonemsBar.GetButtonStyle( 14);
//// Augment the state for wrapping
//nStyle |= TBBS_WRAPPED;
//m_wndPhonemsBar.SetButtonStyle( 14, nStyle );
//
//nStyle = m_wndPhonemsBar.GetButtonStyle( 19);
//// Augment the state for wrapping
//nStyle |= TBBS_WRAPPED;
//m_wndPhonemsBar.SetButtonStyle( 19, nStyle );
//
//nStyle = m_wndPhonemsBar.GetButtonStyle( 24);
//// Augment the state for wrapping
//nStyle |= TBBS_WRAPPED;
//m_wndPhonemsBar.SetButtonStyle( 24, nStyle );
//
//nStyle = m_wndPhonemsBar.GetButtonStyle( 29);
//// Augment the state for wrapping
//nStyle |= TBBS_WRAPPED;
//m_wndPhonemsBar.SetButtonStyle( 29, nStyle );
//
//nStyle = m_wndPhonemsBar.GetButtonStyle( 34);
//// Augment the state for wrapping
//nStyle |= TBBS_WRAPPED;
//m_wndPhonemsBar.SetButtonStyle( 34, nStyle );
//
//nStyle = m_wndPhonemsBar.GetButtonStyle( 39);
//// Augment the state for wrapping
//nStyle |= TBBS_WRAPPED;
//m_wndPhonemsBar.SetButtonStyle( 39, nStyle );
//
//nStyle = m_wndPhonemsBar.GetButtonStyle( 44);
//// Augment the state for wrapping
//nStyle |= TBBS_WRAPPED;
//m_wndPhonemsBar.SetButtonStyle( 44, nStyle );
//
//nStyle = m_wndPhonemsBar.GetButtonStyle( 49);
//// Augment the state for wrapping
//nStyle |= TBBS_WRAPPED;
//m_wndPhonemsBar.SetButtonStyle( 49, nStyle );



// we move the toolbar
//CFrameWnd::FloatControlBar(&m_wndPhonemsBar,CPoint(400,400),CBRS_ALIGN_RIGHT);
// NOW WE CREATE THE STATUS BAR
if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}


	return true;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
cs.cx = 1024;
cs.cy= 768;
cs.style=  WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU| WS_MINIMIZEBOX  ;
cs.x=0;
cs.y=0;

cs.lpszName="Text to Speech Syntesys by DMM";

	return TRUE;
}


// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG


// CMainFrame message handlers


BOOL CMainFrame::OnEraseBkgnd(CDC* pDC)
{
	// TODO: aggiungere qui il codice per la gestione dei messaggi e/o chiamare il codice predefinito.


	return CFrameWnd::OnEraseBkgnd(pDC);
}

void CMainFrame::OnUpdateViewphonems(CCmdUI *pCmdUI)
{


	// TODO: aggiungere qui il codice per la gestione dell'interfaccia di aggiornamento dei comandi.
}

void CMainFrame::OnViewphonems()
{
	// TODO: aggiungere qui il codice per la gestione dei comandi.
}
