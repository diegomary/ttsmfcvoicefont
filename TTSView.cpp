// TTSView.cpp : implementation of the CTTSView class
// the database_support Header file has been put in the TTSView.h header   <afxdao.h>
// ON MAY 17 2006 WE HAVE INTRODUCED A SUBSTITUTION 
//WE HAVE USED AN MFC OBJECT CCriticalSection instead of CRITICAL_SECTION

#include "stdafx.h"

// random numbers generator
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
// ****************** end random number generator
#include "TTS.h"
#include "TTSDoc.h"
#include "TTSView.h"
#include ".\ttsview.h"
#include"StringPLUS.h"

#include "Mmreg.h"//per operazioni wave
// for _makepath funtion and _getdcwd
#include <conio.h>
#include <direct.h>
#include "Mmsystem.h"
#include <Windows.h>
#include "dataan.h"
#include "showphonem.h"
#include "ShowBiTriPhonem.h"
#include <math.h>
#include <afxmt.h> // for the CCriticalSection object
#include <vector>
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define ID_RICH_COPY                    12562 //richedit menu copy paste
#define ID_RICH_PASTE                   12563 // same
// this is the declaration of the thread.
UINT SpeakThread( LPVOID pParam ); //Thread for save to wav file
UINT SpeakThreadDirect( LPVOID pParam ); // thread for speaking
UINT AddBuf( LPVOID pParam ); // thread that play the temporary wave file of verbosity
static DWORD CALLBACK MyStreamInCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
static DWORD CALLBACK MyStreamOutCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb);
static void CALLBACK waveOutProc(HWAVEOUT hWaveOut,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2);
inline CString BinarySearch(CString tofind,CObArray * words, int &ps);
inline CString BinarySearchI(CString tofind,CObArray * words, int &ps, int * index);//standard Search
inline CString BinarySearchAC(CString tofind,CObArray * words, int &ps, int * index);//acronim search
inline CString CheckDate(CString toverify);
inline int *  IndexData(CObArray * words);
CObArray * TotalGlobal; // this is a pointer to the document archive used to globalized its content in another process
static byte ** soundsbuffer=NULL;
static byte ** soundstriphon=NULL;
static byte ** soundsbufferS=NULL;
static int * countlen=NULL;
static int * lentriphon=NULL;
FINDTEXTEX findt;
static CFile *AudioFile; // use to assemble the phonemes in a wav file
static CFile *AudioFile1; // used in wave operation
CMemFile * AudioFileMem;
static int  controlThread;
static int counter =0;
//CRITICAL_SECTION sect;
CWinThread* speakdirect;
CCriticalSection zeta;
HMODULE vox;// the HMODULE that represent the voice resource to load
static int GlobalPauSeWords;
// structeres used for resource operation ( save the word dictionary in a resource in the exe file
 typedef struct  UnitWord {
 int ncharW;
 int ncharP;
	LPCSTR Writ;
	LPCSTR Pron;
} Words ;
// the same
typedef struct  UnitWordR {
 int ncharW;
 int ncharP;
	LPTSTR Writ;
	LPTSTR Pron;
} WordsR ;
// structure used for speech operation, they contain data to be sent to the thread for speaking
typedef struct  StPause {
 CString  T;
 int pauselen;
 CString path;
 CProgressCtrl * progcompute;
	} stp ;

typedef struct  ForDirectS {
 CRichEditCtrl * rich;
 CSliderCtrl * sli;
 	} fdt ;
// the following variables are used in the waveform handling
// playing is complete.
int bufSize=11000;// we can change it 
static bool endOfrec=false;
static int byterec=0;
static HWAVEOUT hw;
static double hh=88200,ff=1000; // parameter necessary to compute the time to sleep.
double gg; // parameter necessary to compute the time to sleep.
double xx; // parameter necessary to compute the time to sleep.
DWORD TimeToPlay;// parameter necessary to compute the time to sleep.
MMTIME compute;// parameter necessary to compute the time to sleep.
static WAVEFORMATEX wf;
//static LPWAVEHDR headwav ;

// This structure is used internally by the XSleep function 
struct XSleep_Structure
{
	int duration;
	HANDLE eventHandle;
};


//////////////////////////////////////////////////////////////////////
// Function  : XSleepThread()
// Purpose   : The thread which will sleep for the given duration
// Returns   : DWORD WINAPI
// Parameters:       
//  1. pWaitTime -
//////////////////////////////////////////////////////////////////////
DWORD WINAPI XSleepThread(LPVOID pWaitTime)
{
	XSleep_Structure *sleep = (XSleep_Structure *)pWaitTime;

	Sleep(sleep->duration);
	SetEvent(sleep->eventHandle);

	return 0;
}

//////////////////////////////////////////////////////////////////////
// Function  : XSleep()
// Purpose   : To make the application sleep for the specified time
//			   duration.
//			   Duration the entire time duration XSleep sleeps, it
//			   keeps processing the message pump, to ensure that all
//			   messages are posted and that the calling thread does
//			   not appear to block all threads!
// Returns   : none
// Parameters:       
//  1. nWaitInMSecs - Duration to sleep specified in miliseconds.
//////////////////////////////////////////////////////////////////////
void XSleep(int nWaitInMSecs)
{
	XSleep_Structure sleep;
	
	sleep.duration		= nWaitInMSecs;
	sleep.eventHandle	= CreateEvent(NULL, TRUE, FALSE, NULL);

	DWORD threadId;
	CreateThread(NULL, 0, &XSleepThread, &sleep, 0, &threadId);

	MSG msg;
	
	while(::WaitForSingleObject(sleep.eventHandle, 0) == WAIT_TIMEOUT)
	{
		//get and dispatch messages
		if(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	CloseHandle(sleep.eventHandle);
}




































using namespace std;
// CTTSView ************************************************************************
IMPLEMENT_DYNCREATE(CTTSView, CFormView)

BEGIN_MESSAGE_MAP(CTTSView, CFormView)
	ON_BN_CLICKED(IDC_FIRST, OnBnClickedFirst)
	ON_BN_CLICKED(IDC_PREVIOUS, OnBnClickedPrevious)
	ON_BN_CLICKED(IDC_NEXT, OnBnClickedNext)
	ON_BN_CLICKED(IDC_LAST, OnBnClickedLast)
	ON_EN_CHANGE(IDC_WRITTEN, OnEnChangeWritten)
	ON_EN_CHANGE(IDC_PRON, OnEnChangePron)
	ON_BN_CLICKED(IDC_FIND, OnBnClickedFind)
	ON_EN_KILLFOCUS(IDC_WRITTEN, OnEnKillfocusWritten)
	ON_LBN_SELCHANGE(IDC_LISTWORD, OnLbnSelchangeListword)
//	ON_WM_PAINT()
ON_BN_CLICKED(IDC_PLAYWAVE, OnBnClickedPlaywave)
ON_BN_CLICKED(IDC_BTNSPEAK, OnBnClickedBtnspeak)
ON_COMMAND(ID_FONTS_TEXTSTYLE, OnFontsTextstyle)
ON_COMMAND(ID_RECORD_FIRST32774, OnRecordFirst32774)
ON_COMMAND(ID_RECORD_LAST32775, OnRecordLast32775)
ON_COMMAND(ID_RECORD_NEXT32783, OnRecordNext32783)
ON_COMMAND(ID_RECORD_PREVIOUS, OnRecordPrevious)
ON_COMMAND(ID_BUTTONAA, SetAA)
ON_COMMAND(ID_BUTTONAE, SetAE)
ON_COMMAND(ID_BUTTONAH, SetAH)
ON_COMMAND(ID_BUTTONAO, SetAO)
ON_COMMAND(ID_BUTTONOO, SetOO)
ON_COMMAND(ID_BUTTONAW, SetAW)
ON_COMMAND(ID_BUTTONAX, SetAX)
ON_COMMAND(ID_BUTTONAY, SetAY)
ON_COMMAND(ID_BUTTONB, SetB)
ON_COMMAND(ID_BUTTONCH, SetCH)
ON_COMMAND(ID_BUTTOND, SetD)
ON_COMMAND(ID_BUTTONDH, SetDH)
ON_COMMAND(ID_BUTTONEH, SetEH)
ON_COMMAND(ID_BUTTONER, SetER)
ON_COMMAND(ID_BUTTONEY, SetEY)
ON_COMMAND(ID_BUTTONF, SetF)
ON_COMMAND(ID_BUTTONG, SetG)
ON_COMMAND(ID_BUTTONH, SetH)
ON_COMMAND(ID_BUTTONIH, SetIH)
ON_COMMAND(ID_BUTTONIY, SetIY)
ON_COMMAND(ID_BUTTONIH, SetIH)
ON_COMMAND(ID_BUTTONJH, SetJH)
ON_COMMAND(ID_BUTTONIH, SetIH)
ON_COMMAND(ID_BUTTONK, SetK)
ON_COMMAND(ID_BUTTONL, SetL)
ON_COMMAND(ID_BUTTONM, SetM)
ON_COMMAND(ID_BUTTONN, SetN)
ON_COMMAND(ID_BUTTONNG, SetNG)
ON_COMMAND(ID_BUTTONOW, SetOW)
ON_COMMAND(ID_BUTTONOY, SetOY)
ON_COMMAND(ID_BUTTONP, SetP)
ON_COMMAND(ID_BUTTONR, SetR)
ON_COMMAND(ID_BUTTONS, SetS)
ON_COMMAND(ID_BUTTONSH, SetSH)
ON_COMMAND(ID_BUTTONT, SetT)
ON_COMMAND(ID_BUTTONTH, SetTH)
ON_COMMAND(ID_BUTTONUH, SetUH)
ON_COMMAND(ID_BUTTONUW, SetUW)
ON_COMMAND(ID_BUTTONV, SetV)
ON_COMMAND(ID_BUTTONW, SetW)
ON_COMMAND(ID_BUTTONY, SetY)
ON_COMMAND(ID_BUTTONZ, SetZ)
ON_COMMAND(ID_BUTTONZH, SetZH)
ON_COMMAND(ID_BUTTONSTRESS, SetSTRESS)
ON_COMMAND(ID_BUTTONPP, SetPP)
ON_COMMAND(ID_BUTTONTT, SetTT)
ON_COMMAND(ID_BUTTONRF, SetRF)
ON_COMMAND(ID_BUTTONLL, SetLL)
ON_COMMAND(ID_BUTTONDD, SetDD)
ON_COMMAND(ID_BUTTONGG, SetGG)
ON_COMMAND(ID_BUTTONVV, SetVV)

ON_COMMAND(ID_RICH_COPY, CopyClip)
ON_COMMAND(ID_RICH_PASTE, PasteClip)


ON_WM_CTLCOLOR()
ON_BN_CLICKED(IDC_COPYCLIP, OnBnClickedCopyclip)
ON_BN_CLICKED(IDC_PASTECLIP, OnBnClickedPasteclip)
ON_WM_ERASEBKGND()
ON_WM_VSCROLL()
ON_COMMAND(ID_FILE_SAVETEXTTOWAVEFILE, OnFileSavetexttowavefile)
ON_BN_CLICKED(IDC_STOPSPEAK, OnBnClickedStopspeak)


ON_BN_CLICKED(IDC_CLEARTEXT, OnBnClickedCleartext)
ON_BN_CLICKED(IDC_SPEAKSEL, &CTTSView::OnBnClickedSpeaksel)
ON_BN_CLICKED(IDC_PAUSE, &CTTSView::OnBnClickedPause)
ON_BN_CLICKED(IDC_RESUME, &CTTSView::OnBnClickedResume)
ON_BN_CLICKED(IDC_OPENTEXTFILE, &CTTSView::OnBnClickedOpentextfile)
ON_WM_PAINT()
ON_COMMAND(ID_RECORD_DELETECURRENT, &CTTSView::OnRecordDeletecurrent)
ON_BN_CLICKED(IDC_LOADVOICE, &CTTSView::OnBnClickedLoadvoice)
ON_COMMAND(ID_FILE_SAVESELTEXTTOWAVEFILE, &CTTSView::OnFileSaveseltexttowavefile)
ON_BN_CLICKED(IDC_SAVETEXTFILE, &CTTSView::OnBnClickedSavetextfile)
ON_WM_CONTEXTMENU()
ON_COMMAND(ID_DATA_DATAINDEXING, &CTTSView::OnDataDataindexing)
ON_WM_HSCROLL()
ON_BN_CLICKED(IDC_BUTTONRESETSPEED, &CTTSView::OnBnClickedButtonresetspeed)
ON_NOTIFY(EN_MSGFILTER, IDC_RICHEDITTEXT, &CTTSView::OnEnMsgfilterRichedittext)
ON_WM_MOUSEMOVE()
ON_COMMAND(ID_EDIT_UNDO, &CTTSView::OnEditUndo)
ON_COMMAND(ID_DATA_PHONEMSVISUALIZER, &CTTSView::OnDataPhonemsvisualizer)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERPITCH, &CTTSView::OnNMCustomdrawSliderpitch)
ON_COMMAND(ID_VOICE_LOADORIGINALVOICEMODULE, &CTTSView::OnVoiceLoadoriginalvoicemodule)
ON_COMMAND(ID_VOICE_LOADUSERMODIFIEDVOICEMODULE, &CTTSView::OnVoiceLoadusermodifiedvoicemodule)
ON_COMMAND(ID_FILE_SAVE_AS_TEXT, &CTTSView::OnFileSaveAsText)
ON_COMMAND(ID_FILE_EXPORTDATABASETOANACCESFILE, &CTTSView::OnFileExportdatabasetoanaccesfile)
ON_COMMAND(ID_FILE_EXPORTWORDSTOADBEFILE, &CTTSView::OnFileExportwordstoadbefile)
ON_COMMAND(ID_FILE_IMPORTWORDSFROMADBEFILE, &CTTSView::OnFileImportwordsfromadbefile)
ON_COMMAND(ID_FILE_IMPORTWORDSFROMADBEFILEFORFULLARCHIVE, &CTTSView::OnFileImportwordsfromadbefileforfullarchive)
ON_EN_CHANGE(IDC_EDITFIND, &CTTSView::OnEnChangeEditfind)
ON_COMMAND(ID_VOICE_LOADTRIPHONESKNOWLEDGE, &CTTSView::OnVoiceLoadtriphonesknowledge)
ON_COMMAND(ID_DATA_BITRIPHONESVISUALIZER, &CTTSView::OnDataBitriphonesvisualizer)

//ON_COMMAND(ID_FILE_IMPORTFROMACCESSFILE, &CTTSView::OnFileImportfromaccessfile)
ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERPAUSE, &CTTSView::OnNMCustomdrawSliderpause)
ON_STN_CLICKED(IDC_SPOSITION, &CTTSView::OnStnClickedSposition)
END_MESSAGE_MAP()
// CTTSView construction/destruction
CTTSView::CTTSView()
	: CFormView(CTTSView::IDD)
	, m_wr(_T(""))
	, m_wp(_T(""))
	, m_position(_T(""))
	, m_curdictionary(NULL)
	, findword(_T(""))
	 , VoiceType(_T("...... Currently no Voice pattern is loaded into the application ......"))
	 , Inspect(NULL)
	 , pause(0)
{
flag=false;
	// TODO: add construction code here
m_pListBkBrush = new CBrush(RGB(0, 0, 255));
m_pEditWBkBrush=new CBrush(RGB(255, 0, 0));
m_pEditPBkBrush=new CBrush(RGB(239, 248, 137));
m_pEditFBkBrush=new CBrush(RGB(0, 255, 0));
m_allStaticBkBrush = new CBrush(RGB(0, 0, 0));
m_sliderBkBrush= new CBrush(RGB(204,17,97));
m_speedslBkBrush= new CBrush(RGB(184,241,247));

}

CTTSView::~CTTSView()
{
}

void CTTSView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_WRITTEN, m_wr);
	DDX_Text(pDX, IDC_PRON, m_wp);
	DDX_Text(pDX, IDC_SPOSITION, m_position);
	DDX_Text(pDX, IDC_EDITFIND, findword);
	DDX_Control(pDX, IDC_WRITTEN, Cwritten);
	DDX_Control(pDX, IDC_PRON, Cpronounced);
	DDX_Control(pDX, IDC_EDITFIND, Cfind);
	DDX_Control(pDX, IDC_STATIC1, Cstat1);
	DDX_Control(pDX, IDC_STATIC2, Cstat2);
	DDX_Control(pDX, IDC_SPOSITION, CStat3);
	DDV_MaxChars(pDX, m_wr, 22);
	DDV_MaxChars(pDX, findword, 22);
	DDX_Control(pDX, IDC_LISTWORD, Clistwords);
	DDX_Control(pDX, IDC_STATIC3, Stat4);
	DDX_Control(pDX, IDC_STATIC4, Stat5);
	DDX_Control(pDX, IDC_STATVOICETYPE, StatVoice);
	DDX_Text(pDX, IDC_STATVOICETYPE, VoiceType);
	DDX_Control(pDX, IDC_STATICPITCH, CStatP);
	DDX_Control(pDX, IDC_RICHEDITTEXT, richtemp);
	DDX_Text(pDX, IDC_PAUSEW, pause);
	DDX_Control(pDX, IDC_PROGSAVE, progresssave);
	DDX_Control(pDX, IDC_SLIDERPAUSE, sliderpause);
}

BOOL CTTSView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CFormView::PreCreateWindow(cs);
}

void CTTSView::OnInitialUpdate()
{
		if (flag==false)// we initialize the richedit and set his format
		// only once 
		{flag=true;
		// the following line inform the view to process mouseevents coming
		//from the richedit  see the function  OnEnMsgfilterRichedittext(NMHDR *pNMHDR, LRESULT *pResult)
		richtemp.SetEventMask(richtemp.GetEventMask() |  ENM_MOUSEEVENTS);
		// The critical section
	
		//InitializeCriticalSection(&sect);
// we create a menu for the richedit that supports clipboard operations
m_NewMenu.CreateMenu();
m_FileMenu.CreatePopupMenu();
m_NewMenu.AppendMenu(MF_POPUP, (UINT) m_FileMenu.m_hMenu, "&Clipboard");
m_FileMenu.AppendMenu(MF_STRING, ID_RICH_COPY, (LPCTSTR)"Copy");
m_FileMenu.AppendMenu(MF_STRING, ID_RICH_PASTE, (LPCTSTR)"Paste");
richtemp.SetMenu(&m_NewMenu);
// end menu creation
		
richtemp.SetBackgroundColor(FALSE, RGB(216,217, 200));
//// ****************** This fragment determines the default text format 
//	// in the richedit
//CHARFORMAT2 cf;
cf.cbSize=sizeof(CHARFORMAT2);
cf.dwMask = CFM_COLOR|CFM_SIZE|CFM_BOLD|CFM_FACE|CFM_CHARSET  ;//|CFM_BACKCOLOR;
cf.dwEffects=CFE_BOLD;
cf.crTextColor=RGB(0,0,255);
cf.bCharSet=ANSI_CHARSET;

//cf.crBackColor=RGB(0,255,255);
// we use this function to copy the default caracter in the buffer
// of the CHARFORMAT2 structure
lstrcpyn(cf.szFaceName, "Arial", LF_FACESIZE);
cf.yHeight=250;
richtemp.SetDefaultCharFormat(cf);
//*************************** end text formatting
	richtemp.LimitText(900000); // the content of the richtext box is limited to 
// 64 KB  infact 0 is the default value and means 64 kb.
// NOte if we set a higher value of 64000 we note a degradation of performance

// ******************** voice initialization now I create the paths for the phonemes
//CString Phon[49];
//Phon[0]="AA";Phon[1]="AE";Phon[2]="AH";Phon[3]="AO";Phon[4]="OO";Phon[5]="AW"; 
//Phon[6]="AX";Phon[7]="AY";Phon[8]="B";Phon[9]="CH";Phon[10]="D";Phon[11]="DH"; 
//Phon[12]="EH";Phon[13]="ER";Phon[14]="EY";Phon[15]="F";Phon[16]="G";Phon[17]="H"; 
//Phon[18]="IH";Phon[19]="IY";Phon[20]="JH";Phon[21]="K";Phon[22]="L";Phon[23]="M";
//Phon[24]="N";Phon[25]="NG";Phon[26]="OW";Phon[27]="OY";Phon[28]="P";Phon[29]="R";
//Phon[30]="S";Phon[31]="SH";Phon[32]="T";Phon[33]="TH";Phon[34]="UH";Phon[35]="UW"; 
//Phon[36]="V";Phon[37]="W";Phon[38]="Y";Phon[39]="Z";Phon[40]="ZH";Phon[41]="BIP";
//Phon[42]="COMMA";Phon[43]="PERIOD";Phon[44]="TWOPOINTS";Phon[45]="SEMICOLON"; 
//Phon[46]="PRIMSTRESS";Phon[47]="SILENCE";Phon[48]="PP";  
//
//CString Path[49];
//CString perc="res\\" ;
//int  curdrive;
//static char path[_MAX_PATH];
//curdrive = _getdrive();
//_getdcwd( curdrive, path, _MAX_PATH );
//for (int i=0; i<49 ; ++i)
//{
//char path_buffer[_MAX_PATH];
//CString temp;
////perc.Append(Phon[i]);
//temp=perc+Phon[i];
//_makepath_s(path_buffer,_MAX_PATH,"",path,temp,"pcm");
////_makepath(path_buffer, "", path, temp, "pcm" );
//Path[i]=path_buffer;
////pList->AddString(Path[i]);
//}
//
//
//// now the array Path[] contains the path of the 45 phonems
//CFile * file; 
////static char ** buffer;
//soundsbuffer = new char*[49];// we create an array of 45 pointer to char
//countlen= new int[49];
//for(int i=0; i<49; ++i)
//{
//file = new CFile(Path[i], CFile::modeRead | CFile::shareExclusive);
//ULONGLONG len = file->GetLength();
//soundsbuffer[i]= new char[len];
//file->Read(soundsbuffer[i],(UINT)len);
//countlen[i]=len;
//file->Close();
//delete file;
//}


// Pitch voice initialization
SliderVoicePitch=(CSliderCtrl*)GetDlgItem(IDC_SLIDERPITCH);
SliderVoicePitch->SetRange(16000,72200);
SliderVoicePitch->SetTicFreq(2200);
SliderVoicePitch->SetSelection(35000,53200);
SliderVoicePitch->SetPos(44100);
// Pause Between Words Initialization
PauseBetweenWords=(CSliderCtrl*)GetDlgItem(IDC_SLIDERPAUSE);
PauseBetweenWords->SetRange(100,4000);
GlobalPauSeWords=100;
pause=(int)((GlobalPauSeWords / hh ) * ff);
UpdateData(FALSE);
// some dealing of the progress bar for saving wav file
progresssave.ShowWindow(SW_HIDE);
progresssave.SetRange(0,10);
progresssave.SetPos(0);


 
//**************************** WAVEFORMATEX initialization
wf.nSamplesPerSec=44100;
wf.wFormatTag=WAVE_FORMAT_PCM;
wf.wBitsPerSample=16;
wf.nChannels=1;
wf.nBlockAlign=(wf.nChannels * wf.wBitsPerSample)/8;
wf.nAvgBytesPerSec=(wf.nSamplesPerSec * wf.nChannels * (wf.wBitsPerSample/8));
if(bufSize % wf.nBlockAlign != 0 ){ bufSize += wf.nBlockAlign - (bufSize % wf.nBlockAlign);}

// now we configure the font size and style of the controls


// Get the dimensions of the font sample display area
   Cwritten.GetWindowRect(&rect);
        // Release the current font
        m_fSampFont.Detach();
        // Create the font to be used
        m_fSampFont.CreateFont((rect.Height()- 5), 0, 0, 0, FW_NORMAL,
                0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS,
                CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH |
                FF_DONTCARE, CString("Verdana"));

        // Set the font for the sample display area
        Cwritten.SetFont(&m_fSampFont);
		Cpronounced.SetFont(&m_fSampFont);
		Cfind.SetFont(&m_fSampFont);
		Clistwords.SetFont(&m_fSampFont);
		// now we create another font a little smaller.
		
		Cstat1.GetWindowRect(&rect);
		m_fSampFont.Detach();
        // Create the font to be used
        m_fSampFont.CreateFont((rect.Height()- 5), 0, 0, 0, FW_BOLD ,
                0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS,
                CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH |
                FF_DONTCARE, CString("Arial"));

		// the following are the Cstatic controls
		Cstat1.SetFont(&m_fSampFont);
		Cstat2.SetFont(&m_fSampFont);
		CStat3.SetFont(&m_fSampFont);
		Stat4.SetFont(&m_fSampFont);
		Stat5.SetFont(&m_fSampFont);
		CStatP.SetFont(&m_fSampFont);
		StatVoice.SetFont(&m_fSampFont);

		} // this is the end of the FIRST TIME CONFIGURATION




	CFormView::OnInitialUpdate();
	GetParentFrame()->RecalcLayout();
	ResizeParentToFit();
}

// CTTSView diagnostics

#ifdef _DEBUG
void CTTSView::AssertValid() const
{
	CFormView::AssertValid();
}

void CTTSView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}

CTTSDoc* CTTSView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTTSDoc)));
	return (CTTSDoc*)m_pDocument;
}
#endif //_DEBUG


// CTTSView message handlers

void CTTSView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{

}

void CTTSView::NewDataset(void)
{
	OnBnClickedFirst();
}

void CTTSView::PopulateView(void)
{
CTTSDoc* pDoc = GetDocument();
if (pDoc)
{
m_position.Format("Record %d of %d", pDoc->GetCurRecordNbr(),pDoc->GetTotalRecords());
}
if (m_curdictionary)
{
m_wr=m_curdictionary->GetWritten();
m_wp=m_curdictionary->GetPronounced();
}
UpdateData(FALSE);

}

void CTTSView::OnBnClickedFirst()
{
		CTTSDoc* pDoc = GetDocument();
		if (pDoc)
		{m_curdictionary=pDoc->GetFirstRecord();
		if (m_curdictionary) 
		{
			PopulateView();
		}
		}


CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTWORD);
pList->SetCurSel(pDoc->GetCurRecordNbr()-1);



}

void CTTSView::OnBnClickedPrevious()
{
	CTTSDoc* pDoc = GetDocument();
if (pDoc)
{m_curdictionary=pDoc->GetPrevRecord();
if (m_curdictionary)
{ 
	PopulateView();
}
}

CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTWORD);
pList->SetCurSel(pDoc->GetCurRecordNbr()-1);


	// TODO: aggiungere qui il codice per la gestione della notifica del controllo.
}

void CTTSView::OnBnClickedNext()
{
	CTTSDoc* pDoc = GetDocument();

if (pDoc)
{m_curdictionary=pDoc->GetNextRecord();

if (m_curdictionary)
{ 
	PopulateView();
}

}


CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTWORD);
pList->SetCurSel(pDoc->GetCurRecordNbr()-1);














	// TODO: aggiungere qui il codice per la gestione della notifica del controllo.
}

void CTTSView::OnBnClickedLast()
{
	CTTSDoc* pDoc = GetDocument();
if (pDoc)
{m_curdictionary=pDoc->GetLastRecord();
if (m_curdictionary)
{ 
	PopulateView();
}
}

CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTWORD);
pList->SetCurSel(pDoc->GetCurRecordNbr()-1);



	// TODO: aggiungere qui il codice per la gestione della notifica del controllo.
}

void CTTSView::OnEnChangeWritten()
{
	

}

void CTTSView::OnEnChangePron()
{
UpdateData(TRUE);
if (m_curdictionary)
m_curdictionary->SetPronounced(m_wp);
}

void CTTSView::OnBnClickedFind()
{


}

void CTTSView::OnEnKillfocusWritten()
{

}

void CTTSView::OnLbnSelchangeListword()
{

}


void CTTSView::ChangeListAspect(void)
{
// we don't have the bitmap resource.
//CButton * pB =(CButton*)GetDlgItem(IDC_BTNSPEAK);
//pB->SetWindowText("SPEAK");
//CDC*memoria;
//CBitmap* bitmap;
//CSize sorgente,destinazione;
//memoria=new CDC;
//bitmap=new CBitmap;
//BITMAP bm;
//bitmap->LoadBitmap(IDB_SPEAKBACK);
//memoria->CreateCompatibleDC((pB->GetDC()));
//memoria->SelectObject(bitmap);
//bitmap->GetObject(sizeof(bm),&bm);
//sorgente.cx=bm.bmWidth;
//sorgente.cy=bm.bmHeight;
//pB->SetWindowText("SPEAK");
//pB->GetDC()->BitBlt(0,0,sorgente.cx,sorgente.cy,memoria,0,0,SRCAND);
//ReleaseDC (memoria);
}

void CTTSView::OnBnClickedPlaywave()
{
if(soundsbuffer==NULL) return;// if no voice is loaded we go home
CString temp;
int  curdrive;
static char path[_MAX_PATH];
static char path_buffer[_MAX_PATH];
curdrive = _getdrive();
_getdcwd( curdrive, path, _MAX_PATH );
//_makepath( path_buffer, "", path, "data", "wav" );
_makepath_s(path_buffer,_MAX_PATH,"",path,"dataS","tmp");
AudioFile = new CFile(path_buffer, CFile::modeCreate|CFile::modeWrite | CFile::shareDenyNone);
LPSTR header = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
AudioFile->Write(header,44);

//**********************************************************************+
UpdateData(TRUE);

CString data=m_wp.Trim(); // now we eliminate the rfinal from the pronounciation pattern 
// because we play one word at a time.
if(data.Right(2).CompareNoCase(CString("*R"))==0) data=data.Left(data.GetLength()-2).Trim();
//********************************************************
CString * Ph;int  phonems;
int q = data.GetLength();
int h=0;
for(int i= 0; i < q; ++i)
{	if(data.GetAt(i)== CString(" ")){ h=h+1; }   }
// we must dimension with h+1 because ahead we use the last phonem
//--------->   Ph[h]=data.Right(2).Trim();
Ph = new CString[h+1];

h=0;
for(int i= 0; i < q; ++i)
{
	if(data.GetAt(i)== CString(" "))
	{
	Ph[h]=(data.Mid(i-2,2)).Trim();
	h=h+1;
}
}
Ph[h]=data.Right(2).Trim();
//**********************************************************
// now CString Ph is ready and rightly dimensioned

for (int i=0 ; i< h+1; ++i)
{
phonems=-1;// We start assuming that no phonems is matching
if(Ph[i].CompareNoCase(CString("AA")) == 0)  phonems=0;
if(Ph[i].CompareNoCase(CString("AE")) == 0)  phonems=1;
if(Ph[i].CompareNoCase(CString("AH")) == 0)  phonems=2;
if(Ph[i].CompareNoCase(CString("AO")) == 0)  phonems=3;
if(Ph[i].CompareNoCase(CString("OO")) == 0)  phonems=4;
if(Ph[i].CompareNoCase(CString("AW")) == 0)   phonems=5;
if(Ph[i].CompareNoCase(CString("AX")) == 0)  phonems=6;
if(Ph[i].CompareNoCase(CString("AY")) == 0)  phonems=7;
if(Ph[i].CompareNoCase(CString("B")) == 0)   phonems=8;
if(Ph[i].CompareNoCase(CString("CH")) == 0)  phonems=9;
if(Ph[i].CompareNoCase(CString("D")) == 0)   phonems=10;
if(Ph[i].CompareNoCase(CString("DH")) == 0)  phonems=11;
if(Ph[i].CompareNoCase(CString("EH")) == 0)  phonems=12;
if(Ph[i].CompareNoCase(CString("ER")) == 0)  phonems=13;
if(Ph[i].CompareNoCase(CString("EY")) == 0)  phonems=14;
if(Ph[i].CompareNoCase(CString("F")) == 0)   phonems=15;
if(Ph[i].CompareNoCase(CString("G")) == 0)   phonems=16;
if(Ph[i].CompareNoCase(CString("H")) == 0)   phonems=17;
if(Ph[i].CompareNoCase(CString("IH")) == 0)   phonems=18;
if(Ph[i].CompareNoCase(CString("IY")) == 0)   phonems=19;
if(Ph[i].CompareNoCase(CString("JH")) == 0)   phonems=20;
if(Ph[i].CompareNoCase(CString("K")) == 0)   phonems=21;
if(Ph[i].CompareNoCase(CString("L")) == 0)   phonems=22;
if(Ph[i].CompareNoCase(CString("M")) == 0)   phonems=23;
if(Ph[i].CompareNoCase(CString("N")) == 0)   phonems=24;
if(Ph[i].CompareNoCase(CString("NG")) == 0)   phonems=25;
if(Ph[i].CompareNoCase(CString("OW")) == 0)   phonems=26;
if(Ph[i].CompareNoCase(CString("OY")) == 0)   phonems=27;
if(Ph[i].CompareNoCase(CString("P")) == 0)   phonems=28;
if(Ph[i].CompareNoCase(CString("R")) == 0)   phonems=29;
if(Ph[i].CompareNoCase(CString("S")) == 0)   phonems=30;
if(Ph[i].CompareNoCase(CString("SH")) == 0)   phonems=31;
if(Ph[i].CompareNoCase(CString("T")) == 0)   phonems=32;
if(Ph[i].CompareNoCase(CString("TH")) == 0)  phonems=33;
if(Ph[i].CompareNoCase(CString("UH")) == 0)   phonems=34;
if(Ph[i].CompareNoCase(CString("UW")) == 0)  phonems=35;
if(Ph[i].CompareNoCase(CString("V")) == 0)   phonems=36;
if(Ph[i].CompareNoCase(CString("W")) == 0)   phonems=37;
if(Ph[i].CompareNoCase(CString("Y")) == 0)   phonems=38;
if(Ph[i].CompareNoCase(CString("Z")) == 0)   phonems=39;
if(Ph[i].CompareNoCase(CString("ZH")) == 0)   phonems=40;
// if the word is not yet in the database
if(Ph[i].CompareNoCase(CString("BP")) == 0)   phonems=41;

if(Ph[i].CompareNoCase(CString("1")) == 0)    phonems=46; // primary stress;
if(Ph[i].CompareNoCase(CString("PP")) == 0)   phonems=48; // LOng p as in PEOPLE
if(Ph[i].CompareNoCase(CString("TT")) == 0)   phonems=49; // LONG and final t as in at most ect.
if(Ph[i].CompareNoCase(CString("LL")) == 0)   phonems=50; // long L as IN ALL
if(Ph[i].CompareNoCase(CString("DD")) == 0)   phonems=51; // long d as IN ADD
if(Ph[i].CompareNoCase(CString("GG")) == 0)   phonems=52; // hard G as in enGage
if(Ph[i].CompareNoCase(CString("VV")) == 0)   phonems=53; // final v as in solve

// concatenation process
if(phonems > -1) AudioFile->Write(soundsbuffer[phonems],countlen[phonems]);
// if an unknown phonems is encountered it is merely spoken as a bip 
else AudioFile->Write(soundsbuffer[41],countlen[41]); //BIP
					}
delete [] Ph;


// we write the wave header and we use the windows function PlaySound for one word.
WAVEFORMATEX  formatoUse ; 
ULONGLONG byterec= AudioFile->GetLength()-44;
ULONGLONG chunksize=byterec+36;
AudioFile->SeekToBegin();
LPSTR teta = "RIFF";
AudioFile->Write(teta,4);
AudioFile->Write(&chunksize,sizeof(int));
teta = "WAVEfmt ";
AudioFile->Write(teta,8);
int qq=16;
AudioFile->Write(&qq,sizeof(int));
//formatoUse.nSamplesPerSec=44100;

formatoUse.nSamplesPerSec=wf.nSamplesPerSec;

formatoUse.wFormatTag=WAVE_FORMAT_PCM;
formatoUse.wBitsPerSample=16;
formatoUse.nChannels=1;
formatoUse.nBlockAlign=(formatoUse.nChannels * formatoUse.wBitsPerSample)/8;
formatoUse.nAvgBytesPerSec=(formatoUse.nSamplesPerSec * formatoUse.nChannels * (formatoUse.wBitsPerSample/8));
AudioFile->Write(&formatoUse,16); // la struttura WAVEFORMATEX ha size pari a 16 no cbSize;
teta = "data";
AudioFile->Write(teta,4);
AudioFile->Write(&byterec,sizeof(int));
AudioFile->Close();
// Now it's time to listen to the result.
PlaySound(path_buffer,NULL,SND_ASYNC);
//AudioFile->Remove(path_buffer); // if we remove we have an error because playsound returns immediately
delete AudioFile;
}

void CTTSView::OnBnClickedBtnspeak()
{
waveOutOpen(&hw,WAVE_MAPPER,&wf,(DWORD)waveOutProc,0,CALLBACK_FUNCTION); 
PauseBetweenWords=(CSliderCtrl*)GetDlgItem(IDC_SLIDERPAUSE);
PauseBetweenWords->EnableWindow(FALSE);


if(soundsbuffer==NULL) return;// if no voice is loaded we go home
if (speakdirect != NULL) return; // if we are alredy speakingn we return.
UpdateData(TRUE);
//GlobalPauSeWords=(int)((pause/ff)*hh);

CTTSDoc* pDoc = GetDocument();
if(pDoc->Total.GetCount() < 2) return; // at the beginning in an empty document there is one element
// the document cobarray representing the word archive  in the Speaking thread contest
controlThread=11; counter =0;
TotalGlobal = new CObArray;
TotalGlobal = &(pDoc->Total);
//***********************

ForDirectS * go;
go= new ForDirectS;
go->rich= new CRichEditCtrl;
go->rich= &richtemp;
go->sli= new CSliderCtrl;
go->sli=&sliderpause;


//speakdirect=AfxBeginThread(SpeakThreadDirect,(LPVOID)temp,THREAD_PRIORITY_NORMAL,0,0, NULL);
//speakdirect=AfxBeginThread(SpeakThreadDirect,(LPVOID)&richtemp,THREAD_PRIORITY_NORMAL,0,0, NULL);
speakdirect=AfxBeginThread(SpeakThreadDirect,(LPVOID)go,THREAD_PRIORITY_NORMAL,0,0, NULL);


//AfxBeginThread(SpeakThreadDirect,(LPVOID)&richtemp,THREAD_PRIORITY_NORMAL,0,0, NULL);
}

void CTTSView::OnFontsTextstyle()
{
			CHARFORMAT2 cf;
			CString newfont;
			CFontDialog dlg;
			if (dlg.DoModal() == IDOK) dlg.GetCharFormat(cf);
richtemp.SetDefaultCharFormat(cf);

}

void CTTSView::OnRecordFirst32774()
{CTTSDoc* pDoc = GetDocument();
		if (pDoc)
		{m_curdictionary=pDoc->GetFirstRecord();
		if (m_curdictionary) 
		{
			PopulateView();
		}
		}

CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTWORD);
pList->SetCurSel(pDoc->GetCurRecordNbr()-1);


	
}

void CTTSView::OnRecordLast32775()
{	CTTSDoc* pDoc = GetDocument();
if (pDoc)
{m_curdictionary=pDoc->GetLastRecord();
if (m_curdictionary)
{ 
	PopulateView();
}
}
CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTWORD);
pList->SetCurSel(pDoc->GetCurRecordNbr()-1);


	
}

void CTTSView::OnRecordNext32783()
{
	CTTSDoc* pDoc = GetDocument();

if (pDoc)
{m_curdictionary=pDoc->GetNextRecord();

if (m_curdictionary)
{ 
	PopulateView();
}

}

CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTWORD);
pList->SetCurSel(pDoc->GetCurRecordNbr()-1);


	
}

void CTTSView::OnRecordPrevious()
{	CTTSDoc* pDoc = GetDocument();
if (pDoc)
{m_curdictionary=pDoc->GetPrevRecord();
if (m_curdictionary)
{ 
	PopulateView();
}
}

CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTWORD);
pList->SetCurSel(pDoc->GetCurRecordNbr()-1);




}

HBRUSH CTTSView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);
 switch (nCtlColor) {

          case CTLCOLOR_LISTBOX:
         // case CTLCOLOR_MSGBOX:
             if(pWnd->GetDlgCtrlID() == IDC_LISTWORD )
			 {
              pDC->SetTextColor(RGB(241, 251, 64));
			  pDC->SetBkColor(RGB(0, 0, 255));
              return (HBRUSH)(m_pListBkBrush->GetSafeHandle());
			 }
		  case CTLCOLOR_EDIT:
				if(pWnd->GetDlgCtrlID() == IDC_WRITTEN)
			 {

			pDC->SetTextColor(RGB(255, 255, 255));
			  pDC->SetBkColor(RGB(255, 0, 0));
              return (HBRUSH)(m_pEditWBkBrush->GetSafeHandle());
             
			 }
	if(pWnd->GetDlgCtrlID() == IDC_PRON)
			 {
			  pDC->SetTextColor(RGB(0, 0, 255));
			  pDC->SetBkColor(RGB(239, 248, 137));
              return (HBRUSH)(m_pEditPBkBrush->GetSafeHandle());
              }

if(pWnd->GetDlgCtrlID() == IDC_EDITFIND)
			 {
			  pDC->SetTextColor(RGB(0, 0, 0));
			  pDC->SetBkColor(RGB(0, 255, 0));
              return (HBRUSH)(m_pEditFBkBrush->GetSafeHandle());
              }

 case CTLCOLOR_STATIC :
if(pWnd->GetDlgCtrlID() != IDC_SLIDER1  &&  pWnd->GetDlgCtrlID() != IDC_SLIDERPITCH   )
				{
				pDC->SetTextColor(RGB(255, 255, 255));


			  pDC->SetBkColor(RGB(0, 0, 0));
              return (HBRUSH)(m_allStaticBkBrush->GetSafeHandle());
				}


else if(pWnd->GetDlgCtrlID() == IDC_SLIDER1 )
				{
				pDC->SetTextColor(RGB(255, 255, 255));
			  pDC->SetBkColor(RGB(204,17,97));
              return (HBRUSH)(m_sliderBkBrush->GetSafeHandle());
				}

else if(pWnd->GetDlgCtrlID() == IDC_SLIDERPITCH )
				{
				pDC->SetTextColor(RGB(255, 255, 255));
			  pDC->SetBkColor(RGB(184,241,247));
              return (HBRUSH)(m_speedslBkBrush->GetSafeHandle());
				}
          default:
              return hbr;
          }

}

void CTTSView::OnBnClickedCopyclip()
{
 richtemp.Copy();
}

void CTTSView::OnBnClickedPasteclip()
{
richtemp.Paste();
//richtemp.LineScroll(-richtemp.GetFirstVisibleLine());
}

void CTTSView::SetAA(void)
{
	if( m_wp.GetLength()==0) m_wp=m_wp+"aa"; else m_wp=m_wp+" aa";
UpdateData(FALSE);
OnEnChangePron();

}

void CTTSView::SetAE(void)
{if( m_wp.GetLength()==0) m_wp=m_wp+"ae"; else m_wp=m_wp+" ae";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetAH(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"ah"; else m_wp=m_wp+" ah";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetAO(void)
{if( m_wp.GetLength()==0) m_wp=m_wp+"ao"; else m_wp=m_wp+" ao";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetOO(void)
{if( m_wp.GetLength()==0) m_wp=m_wp+"oo"; else m_wp=m_wp+" oo";
UpdateData(FALSE);
OnEnChangePron();

}

void CTTSView::SetAW(void)
{if( m_wp.GetLength()==0) m_wp=m_wp+"aw"; else m_wp=m_wp+" aw";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetAX(void)
{if( m_wp.GetLength()==0) m_wp=m_wp+"ax"; else m_wp=m_wp+" ax";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetB(void)
{if( m_wp.GetLength()==0) m_wp=m_wp+"b"; else m_wp=m_wp+" b";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetCH(void)
{if( m_wp.GetLength()==0) m_wp=m_wp+"ch"; else m_wp=m_wp+" ch";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetD(void)
{if( m_wp.GetLength()==0) m_wp=m_wp+"d"; else m_wp=m_wp+" d";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetDH(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"dh"; else m_wp=m_wp+" dh";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetEH(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"eh"; else m_wp=m_wp+" eh";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetER(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"er"; else m_wp=m_wp+" er";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetEY(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"ey"; else m_wp=m_wp+" ey";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetF(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"f"; else m_wp=m_wp+" f";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetG(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"g"; else m_wp=m_wp+" g";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetH(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"h"; else m_wp=m_wp+" h";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetIH(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"ih"; else m_wp=m_wp+" ih";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetIY(void)
{if( m_wp.GetLength()==0) m_wp=m_wp+"iy"; else m_wp=m_wp+" iy";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetJH(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"jh"; else m_wp=m_wp+" jh";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetK(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"k"; else m_wp=m_wp+" k";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetL(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"l"; else m_wp=m_wp+" l";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetM(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"m"; else m_wp=m_wp+" m";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetN(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"n"; else m_wp=m_wp+" n";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetNG(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"ng"; else m_wp=m_wp+" ng";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetOW(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"ow"; else m_wp=m_wp+" ow";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetOY(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"oy"; else m_wp=m_wp+" oy";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetP(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"p"; else m_wp=m_wp+" p";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetR(void)
{if( m_wp.GetLength()==0) m_wp=m_wp+"r"; else m_wp=m_wp+" r";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetS(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"s"; else m_wp=m_wp+" s";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetSH(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"sh"; else m_wp=m_wp+" sh";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetT(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"t"; else m_wp=m_wp+" t";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetTH(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"th"; else m_wp=m_wp+" th";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetUH(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"uh"; else m_wp=m_wp+" uh";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetUW(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"uw"; else m_wp=m_wp+" uw";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetV(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"v"; else m_wp=m_wp+" v";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetW(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"w"; else m_wp=m_wp+" w";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetY(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"y"; else m_wp=m_wp+" y";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetZ(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"z"; else m_wp=m_wp+" z";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetZH(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"zh"; else m_wp=m_wp+" zh";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetAY(void)
{if( m_wp.GetLength()==0) m_wp=m_wp+"ay"; else m_wp=m_wp+" ay";
UpdateData(FALSE);
OnEnChangePron();
}


void CTTSView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{

PauseBetweenWords=( CSliderCtrl *) pScrollBar;
GlobalPauSeWords=(int)(PauseBetweenWords->GetPos());
pause=(int)((GlobalPauSeWords / hh ) * ff);
//CString hh;hh.Format("%d",GlobalPauSeWords);

UpdateData(FALSE);
//Cfind.SetWindowTextA(hh);


CFormView::OnVScroll(nSBCode, nPos, pScrollBar);
}


UINT SpeakThreadDirect( LPVOID pParam )
{
// The following rows are for determinining the handle of the main window
//CWnd *der;
//der = CWnd::FindWindow(NULL,_T("Text to Speech Syntesys by DMM"));// the search is performed providing the window title
//der->SetWindowText("CIAO"); // we can change the window text
//der->ShowWindow(SW_HIDE); // we can also hide or show the window
// how to find a child control inside the main window?


//// this fragment is used to initialize the FINDTEXTEXA structure for searching the word
//// in the richedit while the speaking process is on.
bool marktext ; CString to_speak;

if (((ForDirectS*)pParam)->rich->GetSelectionType() & (SEL_TEXT | SEL_MULTICHAR)) 
{ marktext=false ; to_speak = ((ForDirectS*)pParam)->rich->GetSelText();          }
else
{// The case we speak all text we consider the caret position.
long alfa,beta;
((ForDirectS*)pParam)->rich->GetSel(alfa,beta);
findt.chrg.cpMin = alfa;
findt.chrg.cpMax = -1;
marktext=true;
((ForDirectS*)pParam)->rich->GetTextRange(alfa,((ForDirectS*)pParam)->rich->GetTextLength(),to_speak);
}

int  * sectors  = IndexData(TotalGlobal); // I retrieve the boundaries of each starting letter
// in the database // we have to delete this array at the end of the thread.
CStringPLUS *test;
int nWordsXRow;CString purenumbtime,lef,rig;
bool stressed = false;	
int dostressed=0;
char punctuation='0';
		char genitivity='0';
		char degrees ='0';
		char rfinal='0'; // for the Final R
		char regularnoun='0';// for the simple plural
		char comparative='0';
		char regverb='0';
		char regverb1='0';
		char reggerund='0';
		char superlative1='0';
		char adverbLY='0';
		char adverbNESS='0';
		char adverbLESSNESS='0';
		char adverbLESS='0';
		char pausebefore='0';
		CString regular;
		CString confront;
		CString resData; // the return value of the checkdate function.
		bool foundword=false;	
		CString ret; // return value for the binary search function.
		char * more;// This pointer is declared in order to deal with pauses duration between words		
		// here we have to ensure a pair value of the pause because a sample is two bytes
		double PP = (double)GlobalPauSeWords;
		double PP1; int GlobalPauSeWordsA;

		if((PP/2)-(int)(PP/2) > 0) PP--;
		GlobalPauSeWords=(int)PP;
		// now we can allocate the memory
		more=(char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, GlobalPauSeWords );
		// here we set the pause between phonemes
		PP1 = (double)(GlobalPauSeWords/2); //We consider the pause between two phonemes in a word
	
		
// to be half the value of the pause between words
		if((PP1/2)-(int)(PP1/2) > 0) PP1--;
		GlobalPauSeWordsA=(int)PP1;
		AudioFileMem= new CMemFile(); // we create the audio streamer

CString * vec; // the array of written words in a row
CString * vecP;// the array of pronounced words in a row
CString * Ph;int  phonems;int  ntriphones;
int  curdrive;
static char path[_MAX_PATH];
static char path_buffer[_MAX_PATH];
curdrive = _getdrive();
_getdcwd( curdrive, path, _MAX_PATH );
//_makepath( path_buffer, "", path, "data", "wav" );
_makepath_s(path_buffer,_MAX_PATH,"",path,"data","wav");
// the working dir is the one in which is stored the executable of the application.
 //****************NORMALIZATION****************************
to_speak.Replace('\r',' ');
to_speak.Replace('\n',' ');
to_speak.Replace('\t',' ');

// the following is because if the word ends with a stop and is an acronym we must detach the punctuation.
to_speak.Replace(CString(".."),CString(". ."));
to_speak.Replace(CString(".,"),CString(". ,"));
to_speak.Replace(CString(".;"),CString(". ;"));
to_speak.Replace(CString(".:"),CString(". :"));
to_speak.Replace(CString(".!"),CString(". !"));
to_speak.Replace(CString(".?"),CString(". ?"));
//*********************************************************
to_speak.Replace(CString("?") , CString("? ")  );
to_speak.Replace(CString("("),CString(" "));
to_speak.Replace(CString(")"),CString(" "));
to_speak.Replace(CString("-"),CString(" "));
to_speak.Trim();
int Lalfa; Lalfa= to_speak.GetLength();
for(int q=0; q< Lalfa;++q)
{
	if (to_speak.GetAt(q) == ' ')
	{
		for (int c=q+1; c< Lalfa; c++)
		{
if (to_speak.GetAt(c)== ' ')
{
	to_speak.Delete(c,1);
	Lalfa=to_speak.GetLength(); c--;
} else break;
		}
	}
}
//*****************END NORMALIZATION*******************
// now we need to split the string of the line in words, to do this
// we use the split function
// as every word inserted in the database is all not capitalized we provide
//to_speak.Append(".");
test=new CStringPLUS(to_speak);
// **********************************************************************
vec = test->split(nWordsXRow,' ');// we find the words in the string
//vecP= new CString[nWordsXRow]; // the correspondent pronounced array of words
vecP= new CString[1]; // the correspondent pronounced array of words
//*****************************************************************************+
CString l;// for bynary search
//int left ; int right;//int found;
//int location=0;
int pos=0; //for bynary search
//int q11; // used to divide the pronounced word
int numbOfBlanks=0;// used to divide the pronounced word
int numbOfblanksX=0;
// now we have to discriminate periods and commas and other punctuation
static bool nop;
for(int h = 0; h<nWordsXRow ;++h)
{
zeta.Lock();// WE GAIN  OWNERSHIP OF THE THREAD by loking the ccriticalsection mfc
nop=false;lef="";rig="";// the splitted strings for the time formate type 12:20




if(vec[h].GetLength() == 1)// if we have a single punctuation
{
	if(vec[h].Compare(",")==0) {punctuation=','; nop=true;}
	if(vec[h].Compare(".")==0){punctuation='.'; nop=true;}
	if(vec[h].Compare(":")==0) {punctuation=':'; nop=true;}
	if(vec[h].Compare(";")==0) {punctuation=';'; nop=true;}
	if(vec[h].Compare("?")==0) {punctuation='?'; nop=true;}
	if(vec[h].Compare("!")==0) {punctuation='!'; nop=true;}
	if(vec[h].Compare(CString((char)34))==0) {punctuation='"'; nop=true;} 

}
else // if the word has an attached sign of punctuation and is greater than char
{
if(vec[h].Right(1).Compare(",")==0){ punctuation=','; vec[h]=vec[h].Left(vec[h].GetLength()-1);} 
if(vec[h].Right(1).Compare(".")==0){ punctuation='.'; vec[h]=vec[h].Left(vec[h].GetLength()-1);}
if(vec[h].Right(1).Compare(":")==0){ punctuation=':'; vec[h]=vec[h].Left(vec[h].GetLength()-1);}
if(vec[h].Right(1).Compare(";")==0){ punctuation=';'; vec[h]=vec[h].Left(vec[h].GetLength()-1);}
if(vec[h].Right(1).Compare("?")==0){ punctuation='?'; vec[h]=vec[h].Left(vec[h].GetLength()-1);}
if(vec[h].Right(1).Compare("!")==0){ punctuation='!'; vec[h]=vec[h].Left(vec[h].GetLength()-1);}
if(vec[h].Right(1).Compare(CString((char)34))==0){ punctuation='"'; vec[h]=vec[h].Left(vec[h].GetLength()-1);}
// the following deals with a pause caused by the attached "to the word
if(vec[h].Left(1).Compare(CString((char)34))==0){ pausebefore='y'; vec[h].TrimLeft(CString((char)34));}

// this routine is for resolving time isuue like this: 12:34   .... 00:12 etc.
purenumbtime = vec[h].SpanIncluding("0123:4567890");	
if(purenumbtime.GetLength()==vec[h].GetLength())
{
int g=purenumbtime.Find(':');
lef=purenumbtime.Left(g);
rig=purenumbtime.Right(purenumbtime.GetLength()-g-1);
}
// end time issue
// we have to check the presence of a genitive pattern of kind  s' and a genitive without 's
if(vec[h].Right(2).CompareNoCase(CString("'S")) == 0 ) {genitivity='G'; vec[h]=vec[h].Left(vec[h].GetLength() -2);}
else{if(vec[h].Right(1).CompareNoCase(CString("'")) == 0 ){vec[h]=vec[h].Left(vec[h].GetLength() -1);       }                    }
}

//******************************* // the degrees °
if(vec[h].Right(1).CompareNoCase(CString("°")) == 0 ) {degrees='D'; vec[h]=vec[h].Left(vec[h].GetLength() -1);}

//*****************************


// we must check the following word if it has a vowel sound.
if (vec[h].Right(2).CompareNoCase(CString("*R"))== 0 && h<nWordsXRow-1)//only to this
			// because at the end of the speech we don't add a r
			{	//possible placement of a final r.
				if ((vec[h+1].Left(1)).CompareNoCase(CString("A"))==0) rfinal ='R';
				if (vec[h+1].Left(1).CompareNoCase(CString("H"))==0) rfinal ='R';
				if (vec[h+1].Left(1).CompareNoCase(CString("E"))==0) rfinal ='R';
				if (vec[h+1].Left(1).CompareNoCase(CString("I"))==0) rfinal ='R';
				if (vec[h+1].Left(1).CompareNoCase(CString("O"))==0) rfinal ='R';
				if (vec[h+1].Left(1).CompareNoCase(CString("U"))==0) rfinal ='R';
				if (vec[h+1].Left(1).CompareNoCase(CString("W"))==0) rfinal ='R';
			}
//***************** end of check for r final
// the following is the pronounciation of the definite article (to perfect)
if (vec[h].CompareNoCase(CString("THE"))== 0 && h<nWordsXRow-1)
{
if ((vec[h+1].Left(1)).CompareNoCase(CString("A"))==0) vec[h] = "TI";
if ((vec[h+1].Left(1)).CompareNoCase(CString("E"))==0) vec[h] = "TI";
if ((vec[h+1].Left(1)).CompareNoCase(CString("I"))==0) vec[h] = "TI";
if ((vec[h+1].Left(1)).CompareNoCase(CString("O"))==0) vec[h] = "TI";
if ((vec[h+1].Left(1)).CompareNoCase(CString("U"))==0) vec[h] = "TI";
}

if(!nop) //////////////////////we do not have punctuation so we have to perform search and 
			
			// concatenation
			{
			foundword=false;
// First of all we take a look in the ~ container which is placed at the end of database. there we can
// find a lot of useful stuff which could be duplicated (for example roman numbers like v)
			confront=CString("");
			confront.Append(CString((char)126)); // the acronims or initials or other stuff is marked
			// with the ~ carachter at the end of the database.
			confront.Append(vec[h]);
			ret= BinarySearchAC(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			foundword=true;}
// if we don't find anithing in the general container we proceed further.

			else{
			ret= BinarySearchI(vec[h],TotalGlobal,pos,sectors);			
			if(pos > -1){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();foundword=true;}			
			else {
			//******************************************************PLURAL
			regular = vec[h].Right(1) ;
					if(regular.CompareNoCase("S")==0)
					{		
			confront=vec[h].Left(vec[h].GetLength() -1);

			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			regularnoun='y';foundword=true;}
			else{
			// here we search for an adverb ending NESS 
			regular = vec[h].Right(4) ;
			if(regular.CompareNoCase("NESS")==0)
			{		
			confront=vec[h].Left(vec[h].GetLength() -4);
			ret= BinarySearchI(confront,TotalGlobal,pos, sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			adverbNESS='y';foundword=true;}
			else{
				if(vec[h].GetLength() > 9){
			regular = vec[h].Right(8) ;
			if(regular.CompareNoCase("LESSNESS")==0)
			{		
			confront=vec[h].Left(vec[h].GetLength() -8);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			adverbLESSNESS='y';foundword=true;}
			}}}}
			else{			
			regular = vec[h].Right(4) ;
			if(regular.CompareNoCase("LESS")==0)
			{		
			confront=vec[h].Left(vec[h].GetLength() -4);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			adverbLESS='y';foundword=true;}
			}}}}					
			//******************************END S final
			//******************************************************COMPARATIVE
					else{
					regular = vec[h].Right(2) ;
					if(regular.CompareNoCase("ER")==0)
					{		
			confront=vec[h].Left(vec[h].GetLength() -2);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			comparative='y';foundword=true;}
					}
			//******************************END COMPARATIVE
					//REGULAR AND IRREGULAR VERB MANAGING
					else{
					regular = vec[h].Right(2) ;
					if(regular.CompareNoCase("ED")==0)
					{		
			confront=vec[h].Left(vec[h].GetLength() -2);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			regverb='y';foundword=true;}
					else{
					regular = vec[h].Right(1) ;
					if(regular.CompareNoCase("D")==0)
					{		
			confront=vec[h].Left(vec[h].GetLength() -1);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			regverb1='y';foundword=true;}
				}}
			//REGULAR AND IRREGULAR VERB MANAGING end routine
					}					
					// the following is for the regular and irregular  gerund	
					else{
					regular = vec[h].Right(3) ;

				if(regular.CompareNoCase("ING")==0)
					{		
			confront=vec[h].Left(vec[h].GetLength() -3);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);			
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			reggerund='y';foundword=true;}

			else{confront.Append("e");
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			reggerund='y';foundword=true;}
			}
				}

				else{// here we search for a superlative of ST kind				
					regular = vec[h].Right(2) ;
				if(regular.CompareNoCase("ST")==0)
					{		
			confront=vec[h].Left(vec[h].GetLength() -2);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			superlative1='y';foundword=true;}
				}
				else{
				// here we search for an adverb ending LY 
				regular = vec[h].Right(2) ;
				if(regular.CompareNoCase("LY")==0)
				{		
			confront=vec[h].Left(vec[h].GetLength() -2);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			adverbLY='y';foundword=true;}
				}
				else{resData=CheckDate(vec[h]);
				if(resData.CompareNoCase(CString("n_d")) != 0)
				{		
				vecP[0]=resData;foundword=true;	
				}
	
				
			else{}		
				
				
			}
			}	
			}
			}
			}
			}
			}
			}

			if(!foundword)
			{confront=CString("");
			confront.Append(CString((char)126)); // the acronims or initials or other stuff is marked
			// with the ~ carachter at the end of the database.
			confront.Append(vec[h]);confront.Append(CString("."));
			ret= BinarySearchAC(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			if(punctuation=='.') punctuation='0';
			foundword=true;}
			}

// checking time issue.
if(rig != CString("") && lef != CString("") && !foundword)
{
bool allfound=false;
ret= BinarySearchI(lef,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			allfound=true;pos=-1;
			}

ret= BinarySearchI(rig,TotalGlobal,pos,sectors);
			if(pos > -1 && allfound==true){ vecP[0].Append(" "); vecP[0].Append((*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced());
			foundword=true;}
}
// end checking time isuue 12:34


//****************************************************
// this is to spell every single cipher of a string that contains only numbers.
if(!foundword)
				{

if (vec[h].Find("0") != -1  || vec[h].Find("1")!= -1 || vec[h].Find("2") != -1 || vec[h].Find("3") != -1 || vec[h].Find("4") != -1 || vec[h].Find("5") != -1 || vec[h].Find("6") != -1 || vec[h].Find("7") != -1 || vec[h].Find("8") != -1 || vec[h].Find("9") != -1  )

{CString tp;vecP[0]=CString("");
for(int g=0;g<vec[h].GetLength();g++)
{
tp=vec[h].GetAt(g);
ret= BinarySearchI(tp,TotalGlobal,pos,sectors);
if(pos > -1 ){ vecP[0].Append((*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced());
vecP[0].Append(" 1 ");pos=-1;}
}
vecP[0].Trim();
foundword=true;
}

				}
//**********************************************************


			if(!foundword)vecP[0]="BP";			
			// if the word is HOWEVER not yet in the database AFTER all searches.
			// END OF BINARY SEARCH ***********************************
			// here we create an array of pronounced phonems for the word
			numbOfBlanks=0;

			for(int i= 0; i < vecP[0].GetLength(); ++i)
					{
						if(vecP[0].GetAt(i)== CString(" ")) numbOfBlanks=numbOfBlanks+1;
					}   
			// we must dimension with h+1 because ahead we use the last phonem
			//--------->   Ph[h]=data.Right(2).Trim();
			
			
			
			
			
			Ph = new CString[numbOfBlanks+1];
			numbOfblanksX=numbOfBlanks+1;// this is only intended to have the IT biphone alone or in company
			numbOfBlanks=0;
			for(int i= 0; i < vecP[0].GetLength(); ++i)
			{
				if(vecP[0].GetAt(i)== CString(" "))
				{
				Ph[numbOfBlanks]=(vecP[0].Mid(i-2,2)).Trim();
				numbOfBlanks++;
				}

				Ph[numbOfBlanks]=vecP[0].Right(2).Trim();


				//**********************************************************
			// now CString Ph is ready and rightly dimensioned
					

			} // here we have the array of phonemes that makes up the word. Ph
			
			//AudioFileMem= new CMemFile();
			for (int i=0 ; i<= numbOfBlanks; ++i) // HERE IS THE POSITION WHERE THE continue STATEMANT RESTART
			{

	
			phonems=-12345;// We start assuming that no phonems is matching

//********************************************************************************************
//********************************************************************************************
if(soundstriphon!=NULL) // we have triphones/BIphonems kpowledge on hand so we use it
{

ntriphones=-12345;// We start assuming that no triphonems is matching
//**************************************************************************CONCATENATION SPECIALIZED

// QUADRIPHONES CONCATENATION
if(i<=numbOfBlanks-3)
{
CString tmp=CString(Ph[i]+Ph[i+1]+Ph[i+2]+Ph[i+3]); // We do the sum operation only once

if(tmp.CompareNoCase(CString("PIYPL"))==0 ) { ntriphones=50;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
i+=3;
		continue;}

if(tmp.CompareNoCase(CString("MOWST"))==0 ) { ntriphones=53;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=3;
		continue;}
}


// TRIPHONES CONCATENATION 
if(i<=numbOfBlanks-2)
{
CString tmp=CString(Ph[i]+Ph[i+1]+Ph[i+2]); // We do the sum operation only once

if(tmp.CompareNoCase(CString("WAOT"))==0 ) { ntriphones=52;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(tmp.CompareNoCase(CString("KAXN"))==0 )  { ntriphones=54;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("H")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0  &&  Ph[i+2].CompareNoCase(CString("V")) == 0  ) { ntriphones=55;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}


if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=0;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=1;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("B")) == 0  &&  Ph[i+2].CompareNoCase(CString("L")) == 0  ) { ntriphones=2;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=3;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=4;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=5;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=6;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=7;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=8;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=9;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=10;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("SH")) == 0  ) { ntriphones=11;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0  &&  Ph[i+2].CompareNoCase(CString("AE")) == 0  ) { ntriphones=12;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("NG")) == 0  &&  Ph[i+2].CompareNoCase(CString("K")) == 0  ) { ntriphones=13;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}


if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("D")) == 0  ) { ntriphones=17;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("UH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0  &&  Ph[i+2].CompareNoCase(CString("L")) == 0  ) { ntriphones=21;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("SH")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=22;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0  &&  Ph[i+2].CompareNoCase(CString("AO")) == 0  ) { ntriphones=23;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("D")) == 0  ) { ntriphones=30;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0  &&  Ph[i+2].CompareNoCase(CString("S")) == 0  ) { ntriphones=35;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0  &&  Ph[i+2].CompareNoCase(CString("AX")) == 0  ) { ntriphones=38;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("EH")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0  &&  Ph[i+2].CompareNoCase(CString("IH")) == 0  ) { ntriphones=39;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("P")) == 0  &&  Ph[i+2].CompareNoCase(CString("L")) == 0  ) { ntriphones=40;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("AY")) == 0  &&  Ph[i+2].CompareNoCase(CString("D")) == 0  ) { ntriphones=42;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0  &&  Ph[i+2].CompareNoCase(CString("UH")) == 0  ) { ntriphones=49;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("Y")) == 0  &&  Ph[i+2].CompareNoCase(CString("UW")) == 0  ) { ntriphones=56;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("V")) == 0  ) { ntriphones=58;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}


if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("Y")) == 0  &&  Ph[i+2].CompareNoCase(CString("UH")) == 0  ) { ntriphones=60;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("D")) == 0  ) { ntriphones=67;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=69;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0  &&  Ph[i+2].CompareNoCase(CString("IH")) == 0  ) { ntriphones=71;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("AH")) == 0  &&  Ph[i+2].CompareNoCase(CString("P")) == 0  ) { ntriphones=72;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("AE")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0  &&  Ph[i+2].CompareNoCase(CString("D")) == 0  ) { ntriphones=74;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("D")) == 0  ) { ntriphones=80;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}


if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("S")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=91;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("AY")) == 0  &&  Ph[i+2].CompareNoCase(CString("V")) == 0  ) { ntriphones=96;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}


if(Ph[i].CompareNoCase(CString("EH")) == 0  &&  Ph[i+1].CompareNoCase(CString("V")) == 0  &&  Ph[i+2].CompareNoCase(CString("AX")) == 0  ) { ntriphones=110;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0  &&  Ph[i+2].CompareNoCase(CString("S")) == 0  ) { ntriphones=112;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0  &&  Ph[i+2].CompareNoCase(CString("Y")) == 0  ) { ntriphones=117;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("AW")) == 0  &&  Ph[i+2].CompareNoCase(CString("AX")) == 0  ) { ntriphones=120;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("Y")) == 0  &&  Ph[i+2].CompareNoCase(CString("UH")) == 0  ) { ntriphones=124;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}


if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0  &&  Ph[i+2].CompareNoCase(CString("L")) == 0  ) { ntriphones=131;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("W")) == 0  &&  Ph[i+1].CompareNoCase(CString("AH")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=133;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("Y")) == 0  &&  Ph[i+2].CompareNoCase(CString("UW")) == 0  ) { ntriphones=141;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("EY")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=142;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("W")) == 0  &&  Ph[i+2].CompareNoCase(CString("EH")) == 0  ) { ntriphones=143;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("EY")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=144;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}


if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=145;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("K")) == 0  ) { ntriphones=146;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
 continue;}


if(Ph[i].CompareNoCase(CString("EY")) == 0  &&  Ph[i+1].CompareNoCase(CString("SH")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=147;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}


if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("AH")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=148;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("DH")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("S")) == 0  ) { ntriphones=149;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}


if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("AH")) == 0  &&  Ph[i+2].CompareNoCase(CString("Z")) == 0  ) { ntriphones=150;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("AO")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=151;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}


if(Ph[i].CompareNoCase(CString("W")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0  &&  Ph[i+2].CompareNoCase(CString("L")) == 0  ) { ntriphones=152;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}


if(Ph[i].CompareNoCase(CString("H")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0  &&  Ph[i+2].CompareNoCase(CString("S")) == 0  ) { ntriphones=153;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
continue;}

if(Ph[i].CompareNoCase(CString("DH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=169;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
 continue;}

if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0  &&  Ph[i+2].CompareNoCase(CString("IH")) == 0  ) { ntriphones=170;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0  &&  Ph[i+2].CompareNoCase(CString("L")) == 0  ) { ntriphones=171;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("SH")) == 0  &&  Ph[i+1].CompareNoCase(CString("IY")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=172;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0  &&  Ph[i+2].CompareNoCase(CString("S")) == 0  ) { ntriphones=173;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0  &&  Ph[i+2].CompareNoCase(CString("R")) == 0  ) { ntriphones=174;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	 continue;}

if(Ph[i].CompareNoCase(CString("AE")) == 0  &&  Ph[i+1].CompareNoCase(CString("K")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=175;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0  &&  Ph[i+2].CompareNoCase(CString("IH")) == 0  ) { ntriphones=176;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	continue;}

}


// BIPHONES COCATENATION
if(i<=numbOfBlanks-1)
{
// TO DO
CString tmp=CString(Ph[i]+Ph[i+1]); // We do the sum operation only once


if(tmp.CompareNoCase(CString("YUW"))==0 ){ ntriphones=14;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}


if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0 && numbOfblanksX==2 ) { ntriphones=15;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("Z")) == 0) { ntriphones=16;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("UW")) == 0) { ntriphones=18;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=19;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("AY")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=20;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("Y")) == 0) { ntriphones=24;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=25;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("AO")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=26;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("W")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=27;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
   continue;}

if(Ph[i].CompareNoCase(CString("W")) == 0  &&  Ph[i+1].CompareNoCase(CString("ER")) == 0) { ntriphones=28;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("EY")) == 0) { ntriphones=29;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("SH")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=31;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=32;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=33;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("EY")) == 0) { ntriphones=34;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("JH")) == 0) { ntriphones=36;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("AY")) == 0  &&  Ph[i+1].CompareNoCase(CString("Z")) == 0) { ntriphones=37;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("AY")) == 0) { ntriphones=41;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("AH")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=43;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("AE")) == 0  &&  Ph[i+1].CompareNoCase(CString("M")) == 0) { ntriphones=44;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=45;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=46;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=47;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("OO")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=48;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("OW")) == 0) { ntriphones=51;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("Y")) == 0  &&  Ph[i+1].CompareNoCase(CString("UW")) == 0) { ntriphones=57;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("NG")) == 0) { ntriphones=59;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}


if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=61;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=62;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0) { ntriphones=63;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=64;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("W")) == 0) { ntriphones=65;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=66;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("EY")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=68;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("AO")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=70;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("SH")) == 0) { ntriphones=73;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=75;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("D")) == 0) { ntriphones=76;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("D")) == 0) { ntriphones=77;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=78;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("K")) == 0) { ntriphones=79;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("EY")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=81;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=82;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
 continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("D")) == 0) { ntriphones=83;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("p")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=84;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=85;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=86;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("AH")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=87;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=88;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("V")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=89;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=90;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("M")) == 0) { ntriphones=92;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
continue;}


if(Ph[i].CompareNoCase(CString("AA")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=93;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=94;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
 continue;}

if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("AY")) == 0) { ntriphones=95;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=97;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("V")) == 0) { ntriphones=98;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("AO")) == 0  &&  Ph[i+1].CompareNoCase(CString("V")) == 0) { ntriphones=99;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=100;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=101;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("JH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=102;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=103;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("Z")) == 0) { ntriphones=104;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}


if(Ph[i].CompareNoCase(CString("G")) == 0  &&  Ph[i+1].CompareNoCase(CString("W")) == 0) { ntriphones=105;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
 continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=106;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("W")) == 0) { ntriphones=107;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("IY")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=108;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("S")) == 0) { ntriphones=109;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("V")) == 0  &&  Ph[i+1].CompareNoCase(CString("IY")) == 0) { ntriphones=111;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("F")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=113;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=114;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("V")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=115;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=116;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("AE")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=118;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("AW")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=119;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=121;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
 continue;}


if(Ph[i].CompareNoCase(CString("Y")) == 0  &&  Ph[i+1].CompareNoCase(CString("UH")) == 0) { ntriphones=122;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("Y")) == 0) { ntriphones=123;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}


if(Ph[i].CompareNoCase(CString("F")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0) { ntriphones=125;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=126;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("M")) == 0) { ntriphones=127;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=128;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("AO")) == 0) { ntriphones=129;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("D")) == 0) { ntriphones=130;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("CH")) == 0) { ntriphones=132;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("H")) == 0  &&  Ph[i+1].CompareNoCase(CString("OW")) == 0) { ntriphones=134;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0) { ntriphones=135;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("G")) == 0) { ntriphones=136;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("Z")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0) { ntriphones=137;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("JH")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=138;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=139;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=140;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}



if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("W")) == 0) { ntriphones=154;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=155;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("AE")) == 0  &&  Ph[i+1].CompareNoCase(CString("K")) == 0) { ntriphones=156;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("AE")) == 0  &&  Ph[i+1].CompareNoCase(CString("P")) == 0) { ntriphones=157;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("F")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=158;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}


if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=159;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("OW")) == 0) { ntriphones=160;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("ZH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=161;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0) { ntriphones=162;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=163;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=164;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}


if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("M")) == 0) { ntriphones=165;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}


if(Ph[i].CompareNoCase(CString("F")) == 0  &&  Ph[i+1].CompareNoCase(CString("ER")) == 0) { ntriphones=166;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("ER")) == 0) { ntriphones=167;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("UH")) == 0) { ntriphones=168;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("K")) == 0) { ntriphones=177;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=178;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}


if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=179;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("F")) == 0) { ntriphones=180;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=181;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("H")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=182;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	continue;}

if(Ph[i].CompareNoCase(CString("Z")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=183;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
 continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("EY")) == 0) { ntriphones=184;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("IY")) == 0  &&  Ph[i+1].CompareNoCase(CString("Z")) == 0) { ntriphones=185;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("AA")) == 0  &&  Ph[i+1].CompareNoCase(CString("S")) == 0) { ntriphones=186;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=187;
AudioFileMem->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	 continue;}

}
}
//********************************************************************************************
//********************************************************************************************

			if(Ph[i].CompareNoCase(CString("AA")) == 0)  phonems=0;
			if(Ph[i].CompareNoCase(CString("AE")) == 0)  phonems=1;
			if(Ph[i].CompareNoCase(CString("AH")) == 0)  phonems=2;
			if(Ph[i].CompareNoCase(CString("AO")) == 0)  phonems=3;
			if(Ph[i].CompareNoCase(CString("OO")) == 0)  phonems=4;
			if(Ph[i].CompareNoCase(CString("AW")) == 0)   phonems=5;
			if(Ph[i].CompareNoCase(CString("AX")) == 0)  phonems=6;
			if(Ph[i].CompareNoCase(CString("AY")) == 0)  phonems=7;
			if(Ph[i].CompareNoCase(CString("B")) == 0)   phonems=8;
			if(Ph[i].CompareNoCase(CString("CH")) == 0)  phonems=9;
			if(Ph[i].CompareNoCase(CString("D")) == 0)   phonems=10;
			if(Ph[i].CompareNoCase(CString("DH")) == 0)  phonems=11;
			if(Ph[i].CompareNoCase(CString("EH")) == 0)  phonems=12;
			if(Ph[i].CompareNoCase(CString("ER")) == 0)  phonems=13;
			if(Ph[i].CompareNoCase(CString("EY")) == 0)  phonems=14;
			if(Ph[i].CompareNoCase(CString("F")) == 0)   phonems=15;
			if(Ph[i].CompareNoCase(CString("G")) == 0)   phonems=16;
			if(Ph[i].CompareNoCase(CString("H")) == 0)   phonems=17;
			if(Ph[i].CompareNoCase(CString("IH")) == 0)   phonems=18;
			if(Ph[i].CompareNoCase(CString("IY")) == 0)   phonems=19;
			if(Ph[i].CompareNoCase(CString("JH")) == 0)   phonems=20;
			if(Ph[i].CompareNoCase(CString("K")) == 0)   phonems=21;
			if(Ph[i].CompareNoCase(CString("L")) == 0)   phonems=22;
			if(Ph[i].CompareNoCase(CString("M")) == 0)   phonems=23;
			if(Ph[i].CompareNoCase(CString("N")) == 0)   phonems=24;
			if(Ph[i].CompareNoCase(CString("NG")) == 0)   phonems=25;
			if(Ph[i].CompareNoCase(CString("OW")) == 0)   phonems=26;
			if(Ph[i].CompareNoCase(CString("OY")) == 0)   phonems=27;
			if(Ph[i].CompareNoCase(CString("P")) == 0)   phonems=28;
			if(Ph[i].CompareNoCase(CString("R")) == 0)   phonems=29;
			if(Ph[i].CompareNoCase(CString("S")) == 0)   phonems=30;
			if(Ph[i].CompareNoCase(CString("SH")) == 0)   phonems=31;
			if(Ph[i].CompareNoCase(CString("T")) == 0)   phonems=32;
			if(Ph[i].CompareNoCase(CString("TH")) == 0)  phonems=33;
			if(Ph[i].CompareNoCase(CString("UH")) == 0)   phonems=34;
			if(Ph[i].CompareNoCase(CString("UW")) == 0)  phonems=35;
			if(Ph[i].CompareNoCase(CString("V")) == 0)   phonems=36;
			if(Ph[i].CompareNoCase(CString("W")) == 0)   phonems=37;
			if(Ph[i].CompareNoCase(CString("Y")) == 0)   phonems=38;
			if(Ph[i].CompareNoCase(CString("Z")) == 0)   phonems=39;
			if(Ph[i].CompareNoCase(CString("ZH")) == 0)   phonems=40;
			// if the word is not yet in the database
			if(Ph[i].CompareNoCase(CString("BP")) == 0)   phonems=41;
			if(Ph[i].CompareNoCase(CString("1")) == 0)    {phonems=46;stressed=true;} // primary stress;
			if(Ph[i].CompareNoCase(CString("PP")) == 0)   phonems=48; // long p as in the second p of people or up;
			if(Ph[i].CompareNoCase(CString("TT")) == 0)   phonems=49; // long t as at
			if(Ph[i].CompareNoCase(CString("LL")) == 0)   phonems=50; // long L as IN ALL
			if(Ph[i].CompareNoCase(CString("DD")) == 0)   phonems=51; // long d as IN ADD
			if(Ph[i].CompareNoCase(CString("GG")) == 0)   phonems=52; // hard G as in enGage
			if(Ph[i].CompareNoCase(CString("VV")) == 0)   phonems=53; // final v as in solve
			// if we press the stop button we close the thread
		
			
			if(controlThread==10) {
					delete  AudioFileMem;
					delete test; 
					delete [] vec;
					delete [] vecP;
// if we stop we have to reenable the pause slider and delete the memory involved
((ForDirectS*)pParam)->sli->EnableWindow(TRUE);
//delete ((ForDirectS*)pParam)->rich;
//delete ((ForDirectS*)pParam)->sli;
delete pParam;// that's it we have eliminated all the memory
					
					delete [] sectors;	//The index of starting letters of words in the database	
					// at the end of the thread the slider must reestabilish the activated state.
				zeta.Unlock();
					//LeaveCriticalSection(&sect);
					speakdirect=NULL;	
					
					AfxEndThread(0,true);
				}// Stop Button pressed


// before the word we must check for an attached "
			if (pausebefore=='y'){			
			AudioFileMem->Write(soundsbuffer[43],countlen[43]);
			pausebefore='0';
			}//THE pause at start word caused by the attached "

//***********************************************************************************
// start basic concatenation concatenation of phonems
			if(phonems != -12345){
//Stressing phonemes
if(stressed==true){dostressed=i+1;stressed=false;}
if(i==dostressed){AudioFileMem->Write(soundsbufferS[phonems],countlen[phonems]);AudioFileMem->Write(more,GlobalPauSeWordsA);}
// We add a pause tu simulate that the stressed phoneme should be longer one day we'll have a longer phoneme
else{if(countlen[phonems] != 46) AudioFileMem->Write(soundsbuffer[phonems],countlen[phonems]);}

//******************************  SILENCE AMONG PHONEMES
memset(more,0,GlobalPauSeWordsA);
AudioFileMem->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES PHONEMES*/

			}

}
//*****************CHECK GRAMMAR RULES*******************************************************

			if(regularnoun=='y'){
				AudioFileMem->Write(soundsbuffer[39],countlen[39]); //ENDING Z
			regularnoun='0';//THE FINAL S OF REGULAR NOUNS
			}

			if(comparative=='y'){
				AudioFileMem->Write(soundsbuffer[6],countlen[6]); 
				// we must check here for the final r of the comparative.
if(h<nWordsXRow-1)
{
if (vec[h+1].Left(1).CompareNoCase(CString("A"))==0) AudioFileMem->Write(soundsbuffer[29],countlen[29]);
if (vec[h+1].Left(1).CompareNoCase(CString("H"))==0) AudioFileMem->Write(soundsbuffer[29],countlen[29]);
if (vec[h+1].Left(1).CompareNoCase(CString("E"))==0) AudioFileMem->Write(soundsbuffer[29],countlen[29]);
if (vec[h+1].Left(1).CompareNoCase(CString("I"))==0) AudioFileMem->Write(soundsbuffer[29],countlen[29]);
if (vec[h+1].Left(1).CompareNoCase(CString("O"))==0) AudioFileMem->Write(soundsbuffer[29],countlen[29]);
if (vec[h+1].Left(1).CompareNoCase(CString("U"))==0) AudioFileMem->Write(soundsbuffer[29],countlen[29]);
if (vec[h+1].Left(1).CompareNoCase(CString("W"))==0) AudioFileMem->Write(soundsbuffer[29],countlen[29]);		
}			
			comparative='0';//THE COMPARATIVE
			}
			// regular verb ending   ED.
			if (regverb=='y'){
			AudioFileMem->Write(soundsbuffer[32],countlen[32]);
			regverb='0'; 
			} // the sound of a regular verb.
			// regular verb ending   D.
			if (regverb1=='y'){
			AudioFileMem->Write(soundsbuffer[10],countlen[10]);
			regverb1='0'; 
			} // the sound of a regular verb. ending D
			if (genitivity=='G'){AudioFileMem->Write(soundsbuffer[30],countlen[30]);
			genitivity='0'; //GENITIVE
			}
// the degrees  ° case
if (degrees=='D'){
AudioFileMem->Write(soundsbuffer[10],countlen[10]);
AudioFileMem->Write(soundsbuffer[18],countlen[18]);
AudioFileMem->Write(soundsbuffer[46],countlen[46]);
AudioFileMem->Write(soundsbuffer[16],countlen[16]);
AudioFileMem->Write(soundsbuffer[29],countlen[29]);
AudioFileMem->Write(soundsbuffer[19],countlen[19]);
AudioFileMem->Write(soundsbuffer[39],countlen[39]);
			degrees='0'; //GENITIVE
			}


			if (rfinal=='R'){AudioFileMem->Write(soundsbuffer[29],countlen[29]);
			rfinal='0';
			}//THE FINAL R // we must add vowel discrimination here


			if (reggerund=='y'){AudioFileMem->Write(soundsbuffer[18],countlen[18]);
			AudioFileMem->Write(soundsbuffer[25],countlen[25]);
			reggerund='0';
			}//THE REGULAR GERUND.

			if (superlative1=='y'){AudioFileMem->Write(soundsbuffer[18],countlen[18]); // I
			AudioFileMem->Write(soundsbuffer[30],countlen[30]);// S
			AudioFileMem->Write(soundsbuffer[32],countlen[32]); //T
			superlative1='0';
			}//THE SUPERLATIVE OF KIND ST

			if (adverbLY=='y'){AudioFileMem->Write(soundsbuffer[22],countlen[22]);
			AudioFileMem->Write(soundsbuffer[18],countlen[18]);
			adverbLY='0';
			}//THE ADVERB ENDING LY
			if (adverbNESS=='y'){AudioFileMem->Write(soundsbuffer[24],countlen[24]);
			AudioFileMem->Write(soundsbuffer[6],countlen[6]);
			AudioFileMem->Write(soundsbuffer[30],countlen[30]);
			adverbNESS='0';
			}//THE ADVERB ENDING NESS
			if (adverbLESSNESS=='y'){			
			AudioFileMem->Write(soundsbuffer[22],countlen[22]); // l
			AudioFileMem->Write(soundsbuffer[6],countlen[6]); // e reverse
			AudioFileMem->Write(soundsbuffer[30],countlen[30]); // s
			AudioFileMem->Write(soundsbuffer[24],countlen[24]);
			AudioFileMem->Write(soundsbuffer[6],countlen[6]);
			AudioFileMem->Write(soundsbuffer[30],countlen[30]);
			adverbLESSNESS='0';
			}//THE ADVERB ENDING LESSNESS
			if (adverbLESS=='y'){			
			AudioFileMem->Write(soundsbuffer[22],countlen[22]);
			AudioFileMem->Write(soundsbuffer[6],countlen[6]);
			AudioFileMem->Write(soundsbuffer[30],countlen[30]);
				adverbLESS='0';
			}
		
			//AudioFileMem->Write(soundsbuffer[47],countlen[47]); /*SILENCE THAT SEPARATES WORDS*/
			AudioFileMem->Write(more,GlobalPauSeWords); /*SILENCE THAT SEPARATES WORDS*/		
		if (punctuation==',')AudioFileMem->Write(soundsbuffer[42],countlen[42]); //COMMA
		if (punctuation=='.')AudioFileMem->Write(soundsbuffer[43],countlen[43]); //PERIOD
		if (punctuation==':')AudioFileMem->Write(soundsbuffer[44],countlen[44]); //TWO POINTS
		if (punctuation==';')AudioFileMem->Write(soundsbuffer[45],countlen[45]); //SEMICOLON
		if (punctuation=='?')AudioFileMem->Write(soundsbuffer[43],countlen[43]); //Question mark
		if (punctuation=='!')AudioFileMem->Write(soundsbuffer[43],countlen[43]); //Exclamatiom
		if (punctuation=='"')AudioFileMem->Write(soundsbuffer[43],countlen[43]); // " we use now the exlamation silence
		punctuation='0';
}
else{// if there are punctuation like that this   ,   is   ,   a   test
//AudioFileMem= new CMemFile();
AudioFileMem->SetLength(0);
if (punctuation==',')AudioFileMem->Write(soundsbuffer[42],countlen[42]); //COMMA
if (punctuation=='.')AudioFileMem->Write(soundsbuffer[43],countlen[43]); //PERIOD
if (punctuation==':')AudioFileMem->Write(soundsbuffer[44],countlen[44]); //TWO POINTS
if (punctuation==';')AudioFileMem->Write(soundsbuffer[45],countlen[45]); //SEMICOLON
if (punctuation=='?')AudioFileMem->Write(soundsbuffer[43],countlen[43]);
if (punctuation=='!')AudioFileMem->Write(soundsbuffer[43],countlen[43]); //Exclamatiom
if (punctuation=='"')AudioFileMem->Write(soundsbuffer[43],countlen[43]); // " we use now the exlamation silence

punctuation='0';

}
//************************************END CHECK GRAMMAR RULES*******************************
if(nop ==false)
{
delete [] Ph;
}  
nop=false; // in case it was true

AudioFileMem->SeekToBegin(); // if we want to play it we must put its pointer to the beginning 
zeta.Unlock();
//LeaveCriticalSection(&sect);
AfxBeginThread(AddBuf,NULL,0,THREAD_PRIORITY_NORMAL,0);
XSleep(2);// time for the wave thread to gain ownership
// here we march the richedit when we speak a word with a mini selection in order
// to allow the reader to keep track of the fluency.
if(marktext==true) // only if we speak the whole text 
{
if(vec[h]==CString("TI"))findt.lpstrText = CString("THE");
else findt.lpstrText =  vec[h];
long n = (*((ForDirectS*)pParam)->rich).FindTextA( FR_DOWN, &findt);
if (n != -1)
   (*((ForDirectS*)pParam)->rich).SetSel(findt.chrgText);
findt.chrg.cpMin = findt.chrgText.cpMax;
if(h < nWordsXRow -1 ) findt.chrg.cpMax=findt.chrgText.cpMax + vec[h+1].GetLength()+3;
// we end and we send for speaking
}

			}
delete [] sectors; // the index array
delete test; // the CStringPlus object;	
delete [] vec;// the array of written words in a row
delete [] vecP;// the array of pronounced words in a row
//delete [] headwav;// when the playing is ended we delete the buffers
((ForDirectS*)pParam)->sli->EnableWindow(TRUE);
//delete ((ForDirectS*)pParam)->rich;
//delete ((ForDirectS*)pParam)->sli;
delete pParam;// because we passed an array of CString to the thread so we must clean it too
waveOutClose(hw);
speakdirect=NULL;
HeapFree(GetProcessHeap(), 0, more);
AfxEndThread(0,true);
return 0;
}


UINT SpeakThread( LPVOID pParam )
{
	CStdioFile TextStream(CString( "test.txt"), CFile::modeCreate | CFile::modeWrite | CFile::typeText );

static CFile *AudioFileZ; // use to assemble the phonemes in a wav file
AudioFileZ = new CFile(((StPause*)pParam)->path, CFile::modeCreate|CFile::modeReadWrite | CFile::shareDenyNone); 	


CString to_speak;
to_speak=(CString)((StPause*)pParam)->T;
int  * sectors  = IndexData(TotalGlobal); // I retrieve the boundaries of each starting letter
// in the database // we have to delete this array at the end of the thread.
LPSTR header = "THIS IS A TEMPORARY WRITING THEN WE'LL GET**";// the temporary header placeholder
AudioFileZ->Write(header,44);
CStringPLUS *test;
int nWordsXRow;CString purenumbtime,lef,rig;
bool stressed = false;	
int dostressed=0;
		char punctuation='0';
		char genitivity='0';
		char degrees ='0';
		char rfinal='0'; // for the Final R
		char regularnoun='0';// for the simple plural
		char comparative='0';
		char regverb='0';
		char regverb1='0';
		char reggerund='0';
		char superlative1='0';
		char adverbLY='0';
		char adverbNESS	='0';
		char adverbLESSNESS	='0';
		char adverbLESS	='0';
		char pausebefore='0';
		CString regular;
		CString confront;
		CString resData; // the return value of the checkdate function.
		bool foundword=false;
		CString ret; // return value from BinarySearch function

int amount=(int)((StPause*)pParam)->pauselen;

char * more;// This pointer is declared in order to deal with pauses duration between words
		
		// here we have to ensure a pair value of the pause because a sample is two bytes
		double PP = (double)amount;
		double PP1; int GlobalPauSeWordsA;


		if((PP/2)-(int)(PP/2) > 0) PP--;
		amount=(int)PP;
		// now we can allocate the memory
		more=(char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, amount );

	PP1 = (double)(GlobalPauSeWords/2); //We consider the pause between two phonemes in a word
	// to be half the value of the pause between words
	if((PP1/2)-(int)(PP1/2) > 0) PP1--;
	GlobalPauSeWordsA=(int)PP1;

CString * vec; // the array of written words in a row
CString * vecP;// the array of pronounced words in a row
CString * Ph;int  phonems;int  ntriphones;
int  curdrive;
static char path[_MAX_PATH];
static char path_buffer[_MAX_PATH];
curdrive = _getdrive();
_getdcwd( curdrive, path, _MAX_PATH );
//_makepath( path_buffer, "", path, "data", "wav" );
_makepath_s(path_buffer,_MAX_PATH,"",path,"data","wav");
 //****************NORMALIZATION****************************
to_speak.Replace('\r',' ');
to_speak.Replace('\n',' ');
to_speak.Replace('\t',' ');
// the following is because if the word ends with a stop and is an acronym we must detach the punctuation.
to_speak.Replace(CString(".."),CString(". ."));
to_speak.Replace(CString(".,"),CString(". ,"));
to_speak.Replace(CString(".;"),CString(". ;"));
to_speak.Replace(CString(".:"),CString(". :"));
to_speak.Replace(CString(".!"),CString(". !"));
to_speak.Replace(CString(".?"),CString(". ?"));
//*********************************************************
to_speak.Replace(CString("?") , CString("? ")  );
to_speak.Replace(CString("("),CString(" "));
to_speak.Replace(CString(")"),CString(" "));
to_speak.Replace(CString("-"),CString(" "));

to_speak.Trim();
int Lalfa; Lalfa= to_speak.GetLength();
for(int q=0; q< Lalfa;++q)
{
	if (to_speak.GetAt(q) == ' ')
	{
		for (int c=q+1; c< Lalfa; c++)
		{
if (to_speak.GetAt(c)== ' ')
{
	to_speak.Delete(c,1);
	Lalfa=to_speak.GetLength(); c--;
} else break;
		}
	}
}
//*****************END NORMALIZATION*******************
// now we need to split the string of the line in words, to do this
// we use the split function
// as every word inserted in the database is all not capitalized we provide
//to_speak.Append(".");
test=new CStringPLUS(to_speak);
// **********************************************************************
vec = test->split(nWordsXRow,' ');// we find the words in the string
//vecP= new CString[nWordsXRow]; // the correspondent pronounced array of words
vecP= new CString[1]; // the correspondent pronounced array of words
//*****************************************************************************+
CString l;// for bynary search
int left ; int right;int found;int location;int pos; //for bynary search
//int q11; // used to divide the pronounced word
int numbOfBlanks=0;// used to divide the pronounced word
int numbOfblanksX=0;
static bool nop;
// the following lines are to deal with the progress control
((StPause*)pParam)->progcompute->ShowWindow(SW_SHOW);
((StPause*)pParam)->progcompute->SetStep(1);
((StPause*)pParam)->progcompute->SetBkColor(RGB(237,42,190));
((StPause*)pParam)->progcompute->SetRange32(0,nWordsXRow-1);

for(int h = 0; h<nWordsXRow ;++h)
{
// the progress control advances
((StPause*)pParam)->progcompute->SetPos(h);
//((StPause*)pParam)->progcompute->StepIt();


nop=false;lef="";rig="";// the splitted strings for the time formate type 12:20
// here we remove the call to the waveplay. no need.
if(vec[h].GetLength() == 1)// if we have a single punctuation
{
	if(vec[h].Compare(",")==0) {punctuation=','; nop=true;}
	if(vec[h].Compare(".")==0){punctuation='.'; nop=true;}
	if(vec[h].Compare(":")==0) {punctuation=':'; nop=true;}
	if(vec[h].Compare(";")==0) {punctuation=';'; nop=true;}
	if(vec[h].Compare("?")==0) {punctuation='?'; nop=true;}
	if(vec[h].Compare("!")==0) {punctuation='!'; nop=true;}
	if(vec[h].Compare(CString((char)34))==0) {punctuation='"'; nop=true;} 

}
else // if the word has an attached sign of punctuation and is greater than char
{
if(vec[h].Right(1).Compare(",")==0){ punctuation=','; vec[h]=vec[h].Left(vec[h].GetLength()-1);} 
if(vec[h].Right(1).Compare(".")==0){ punctuation='.'; vec[h]=vec[h].Left(vec[h].GetLength()-1);}
if(vec[h].Right(1).Compare(":")==0){ punctuation=':'; vec[h]=vec[h].Left(vec[h].GetLength()-1);}
if(vec[h].Right(1).Compare(";")==0){ punctuation=';'; vec[h]=vec[h].Left(vec[h].GetLength()-1);}
if(vec[h].Right(1).Compare("?")==0){ punctuation='?'; vec[h]=vec[h].Left(vec[h].GetLength()-1);}
if(vec[h].Right(1).Compare("!")==0){ punctuation='!'; vec[h]=vec[h].Left(vec[h].GetLength()-1);}
// the following deals with a pause caused by the attached "to the word
if(vec[h].Left(1).Compare(CString((char)34))==0){ pausebefore='y'; vec[h].TrimLeft(CString((char)34));}

// this routine is for resolving time isuue like this: 12:34   .... 00:12 etc.
purenumbtime = vec[h].SpanIncluding("0123:4567890");	
if(purenumbtime.GetLength()==vec[h].GetLength())
{
int g=purenumbtime.Find(':');
lef=purenumbtime.Left(g);
rig=purenumbtime.Right(purenumbtime.GetLength()-g-1);
}
// end time issue

// we have to check the presence of a genitive pattern of kind  s' and a genitive without 's
if(vec[h].Right(2).CompareNoCase(CString("'S")) == 0 ) {genitivity='G'; vec[h]=vec[h].Left(vec[h].GetLength() -2);}
else{if(vec[h].Right(1).CompareNoCase(CString("'")) == 0 ){vec[h]=vec[h].Left(vec[h].GetLength() -1);       }                    }
}

//******************************* // the degrees °
if(vec[h].Right(1).CompareNoCase(CString("°")) == 0 ) {degrees='D'; vec[h]=vec[h].Left(vec[h].GetLength() -1);}

//*****************************


// we must check the following word if it has a vowel sound.
if (vec[h].Right(2).CompareNoCase(CString("*R"))== 0 && h<nWordsXRow-1)//only to this
			// because at the end of the speech we don't add a r
			{	//possible placement of a final r.
				if ((vec[h+1].Left(1)).CompareNoCase(CString("A"))==0) rfinal ='R';
				if (vec[h+1].Left(1).CompareNoCase(CString("H"))==0) rfinal ='R';
				if (vec[h+1].Left(1).CompareNoCase(CString("E"))==0) rfinal ='R';
				if (vec[h+1].Left(1).CompareNoCase(CString("I"))==0) rfinal ='R';
				if (vec[h+1].Left(1).CompareNoCase(CString("O"))==0) rfinal ='R';
				if (vec[h+1].Left(1).CompareNoCase(CString("U"))==0) rfinal ='R';
				if (vec[h+1].Left(1).CompareNoCase(CString("W"))==0) rfinal ='R';
			}
//***************** end of check for r final
// the following is the pronounciation of the definite article (to perfect)
if (vec[h].CompareNoCase(CString("THE"))== 0 && h<nWordsXRow-1)
{
if ((vec[h+1].Left(1)).CompareNoCase(CString("A"))==0) vec[h] = "TI";
if ((vec[h+1].Left(1)).CompareNoCase(CString("E"))==0) vec[h] = "TI";
if ((vec[h+1].Left(1)).CompareNoCase(CString("I"))==0) vec[h] = "TI";
if ((vec[h+1].Left(1)).CompareNoCase(CString("O"))==0) vec[h] = "TI";
if ((vec[h+1].Left(1)).CompareNoCase(CString("U"))==0) vec[h] = "TI";
}
if(!nop) //////////////////////we do not have punctuation so we have to perform search and 
			
			// concatenation
			{
			foundword=false;
// First of all we take a look in the ~ container which is placed at the end of database. there we can
// find a lot of useful stuff which could be duplicated (for example roman numbers like v)
			confront=CString("");
			confront.Append(CString((char)126)); // the acronims or initials or other stuff is marked
			// with the ~ carachter at the end of the database.
			confront.Append(vec[h]);
			ret= BinarySearchAC(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			foundword=true;}
// if we don't find anithing in the general container we proceed further.

			else{
			ret= BinarySearchI(vec[h],TotalGlobal,pos,sectors);			
			if(pos > -1){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();foundword=true;}			
			else {
			//******************************************************PLURAL
			regular = vec[h].Right(1) ;
					if(regular.CompareNoCase("S")==0)
					{		
			confront=vec[h].Left(vec[h].GetLength() -1);

			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			regularnoun='y';foundword=true;}
			else{
			// here we search for an adverb ending NESS 
			regular = vec[h].Right(4) ;
			if(regular.CompareNoCase("NESS")==0)
			{		
			confront=vec[h].Left(vec[h].GetLength() -4);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			adverbNESS='y';foundword=true;}
			else{
				if(vec[h].GetLength() > 9){
			regular = vec[h].Right(8) ;
			if(regular.CompareNoCase("LESSNESS")==0)
			{		
			confront=vec[h].Left(vec[h].GetLength() -8);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			adverbLESSNESS='y';foundword=true;}
			}}}}
			else{			
			regular = vec[h].Right(4) ;
			if(regular.CompareNoCase("LESS")==0)
			{		
			confront=vec[h].Left(vec[h].GetLength() -4);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			adverbLESS='y';foundword=true;}
			}}}}					
			//******************************END s final
			//******************************************************COMPARATIVE
					else{
					regular = vec[h].Right(2) ;
					if(regular.CompareNoCase("ER")==0)
					{		
			confront=vec[h].Left(vec[h].GetLength() -2);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);	
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			comparative='y';foundword=true;}
					}
			//******************************END COMPARATIVE
					//REGULAR AND IRREGULAR VERB MANAGING
					else{
			regular = vec[h].Right(2) ;
					if(regular.CompareNoCase("ED")==0)
					{		
			confront=vec[h].Left(vec[h].GetLength() -2);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);	
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			regverb='y';foundword=true;}
			else{regular = vec[h].Right(1) ;
				if(regular.CompareNoCase("D")==0)
					{		
			confront=vec[h].Left(vec[h].GetLength() -1);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);	
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			regverb1='y';foundword=true;}
					}}
			//REGULAR AND IRREGULAR VERB MANAGING end routine
					}					
					// the following is for the regular and irregular  gerund	
					else{
					regular = vec[h].Right(3) ;
				if(regular.CompareNoCase("ING")==0)
					{		
			confront=vec[h].Left(vec[h].GetLength() -3);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);	
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			reggerund='y';foundword=true;}
			else{confront.Append("e");
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);	
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			reggerund='y';foundword=true;}
			}
					}
				else{// here we search for a superlative of ST kind				
					regular = vec[h].Right(2) ;
				if(regular.CompareNoCase("ST")==0)
					{		
			confront=vec[h].Left(vec[h].GetLength() -2);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);	
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced()	;
			superlative1='y';foundword=true;}
			}
				else{
		// here we search for an adverb ending LY 
				regular = vec[h].Right(2) ;
				if(regular.CompareNoCase("LY")==0)
				{		
			confront=vec[h].Left(vec[h].GetLength() -2);
			ret= BinarySearchI(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			adverbLY='y';foundword=true;}
				}
				else{resData=CheckDate(vec[h]);
				if(resData.CompareNoCase(CString("n_d")) != 0)
				{		
				vecP[0]=resData;foundword=true;	
				}
				
				else{} // another search pattern here
			}	
			}	
			}
			}
			}
			}
			}
			}

			if(!foundword)
			{confront=CString("");
			confront.Append(CString((char)126)); // the acronims or initials or other stuff is marked
			// with the ~ carachter at the end of the database.
			confront.Append(vec[h]);confront.Append(CString("."));
			ret= BinarySearchAC(confront,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			if(punctuation=='.') punctuation='0';
			foundword=true;}
	
			}

// checking time issue.
if(rig != CString("") && lef != CString("") && !foundword)
{
bool allfound=false;
ret= BinarySearchI(lef,TotalGlobal,pos,sectors);
			if(pos > -1 ){ vecP[0]=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();
			allfound=true;pos=-1;
			}

ret= BinarySearchI(rig,TotalGlobal,pos,sectors);
			if(pos > -1 && allfound==true){ vecP[0].Append(" "); vecP[0].Append((*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced());
			foundword=true;}
}
// end checking time isuue 12:34

//****************************************************
// this is to spell every single cipher of a string that contains only numbers.
if(!foundword)
				{

if (vec[h].Find("0") != -1  || vec[h].Find("1")!= -1 || vec[h].Find("2") != -1 || vec[h].Find("3") != -1 || vec[h].Find("4") != -1 || vec[h].Find("5") != -1 || vec[h].Find("6") != -1 || vec[h].Find("7") != -1 || vec[h].Find("8") != -1 || vec[h].Find("9") != -1  )

{CString tp;vecP[0]=CString("");
for(int g=0;g<vec[h].GetLength();g++)
{
tp=vec[h].GetAt(g);
ret= BinarySearchI(tp,TotalGlobal,pos,sectors);
if(pos > -1 ){ vecP[0].Append((*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced());
vecP[0].Append(" 1 ");pos=-1;}
}
vecP[0].Trim();
foundword=true;
}

				}
//**********************************************************

if(!foundword){vecP[0]="BP" ;
TextStream.WriteString(vec[h]+'\n'); // we add the Tab escape in order to have a line per entry
}			
			// if the word is HOWEVER not yet in the database AFTER all searches.
			// END OF BINARY SEARCH ***********************************
			// here we create an array of pronounced phonems for the word
			numbOfBlanks=0;
			for(int i= 0; i < vecP[0].GetLength(); ++i)
					{
						if(vecP[0].GetAt(i)== CString(" ")) numbOfBlanks=numbOfBlanks+1;
					}   
			// we must dimension with h+1 because ahead we use the last phonem
			//--------->   Ph[h]=data.Right(2).Trim();
			Ph = new CString[numbOfBlanks+1];
			numbOfblanksX=numbOfBlanks+1;// this is only intended to have the IT biphone alone or in company
			numbOfBlanks=0;
			for(int i= 0; i < vecP[0].GetLength(); ++i)
			{
				if(vecP[0].GetAt(i)== CString(" "))
				{
				Ph[numbOfBlanks]=(vecP[0].Mid(i-2,2)).Trim();
				numbOfBlanks++;
				}

				Ph[numbOfBlanks]=vecP[0].Right(2).Trim();
				//**********************************************************
			// now CString Ph is ready and rightly dimensioned
					
			} // here we have the array of phonemes that makes up the word. Ph

			//AudioFileZ = new CFile(path_buffer, CFile::modeCreate|CFile::modeWrite | CFile::shareDenyNone);
			for (int i=0 ; i<= numbOfBlanks; ++i)
			{
	
			phonems=-12345;// We start assuming that no phonems is matching

if(soundstriphon!=NULL) // we have triphones/BIphonems kpowledge on hand so we use it
{

ntriphones=-12345;// We start assuming that no triphonems is matching
//**************************************************************************CONCATENETION SPECIALIZED
if(i<=numbOfBlanks-2)
{

// QUADRIPHONES CONCATENATION

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("IY")) == 0  &&  Ph[i+2].CompareNoCase(CString("P")) == 0  &&  Ph[i+3].CompareNoCase(CString("L")) == 0  ) { ntriphones=50;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=3;
	if(numbOfBlanks==3) break;else continue;}

if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("OW")) == 0  &&  Ph[i+2].CompareNoCase(CString("S")) == 0  &&  Ph[i+3].CompareNoCase(CString("T")) == 0  ) { ntriphones=53;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=3;
	if(numbOfBlanks==3) break;else continue;}

// TRIPHONES CONCATENATION

if(Ph[i].CompareNoCase(CString("W")) == 0  &&  Ph[i+1].CompareNoCase(CString("AO")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=52;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=54;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("H")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0  &&  Ph[i+2].CompareNoCase(CString("V")) == 0  ) { ntriphones=55;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}


if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=0;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=1;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("B")) == 0  &&  Ph[i+2].CompareNoCase(CString("L")) == 0  ) { ntriphones=2;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=3;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=4;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=5;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=6;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=7;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}
if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=8;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=9;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("NG")) == 0  ) { ntriphones=10;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("SH")) == 0  ) { ntriphones=11;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0  &&  Ph[i+2].CompareNoCase(CString("AE")) == 0  ) { ntriphones=12;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("NG")) == 0  &&  Ph[i+2].CompareNoCase(CString("K")) == 0  ) { ntriphones=13;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}


if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("D")) == 0  ) { ntriphones=17;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("UH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0  &&  Ph[i+2].CompareNoCase(CString("L")) == 0  ) { ntriphones=21;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("SH")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=22;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0  &&  Ph[i+2].CompareNoCase(CString("AO")) == 0  ) { ntriphones=23;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("D")) == 0  ) { ntriphones=30;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0  &&  Ph[i+2].CompareNoCase(CString("S")) == 0  ) { ntriphones=35;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0  &&  Ph[i+2].CompareNoCase(CString("AX")) == 0  ) { ntriphones=38;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("EH")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0  &&  Ph[i+2].CompareNoCase(CString("IH")) == 0  ) { ntriphones=39;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("P")) == 0  &&  Ph[i+2].CompareNoCase(CString("L")) == 0  ) { ntriphones=40;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("AY")) == 0  &&  Ph[i+2].CompareNoCase(CString("D")) == 0  ) { ntriphones=42;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0  &&  Ph[i+2].CompareNoCase(CString("UH")) == 0  ) { ntriphones=49;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("Y")) == 0  &&  Ph[i+2].CompareNoCase(CString("UW")) == 0  ) { ntriphones=56;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("V")) == 0  ) { ntriphones=58;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}


if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("Y")) == 0  &&  Ph[i+2].CompareNoCase(CString("UH")) == 0  ) { ntriphones=60;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("D")) == 0  ) { ntriphones=67;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=69;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0  &&  Ph[i+2].CompareNoCase(CString("IH")) == 0  ) { ntriphones=71;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("AH")) == 0  &&  Ph[i+2].CompareNoCase(CString("P")) == 0  ) { ntriphones=72;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("AE")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0  &&  Ph[i+2].CompareNoCase(CString("D")) == 0  ) { ntriphones=74;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("D")) == 0  ) { ntriphones=80;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}


if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("S")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=91;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}


if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("AY")) == 0  &&  Ph[i+2].CompareNoCase(CString("V")) == 0  ) { ntriphones=96;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}


if(Ph[i].CompareNoCase(CString("EH")) == 0  &&  Ph[i+1].CompareNoCase(CString("V")) == 0  &&  Ph[i+2].CompareNoCase(CString("AX")) == 0  ) { ntriphones=110;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0  &&  Ph[i+2].CompareNoCase(CString("S")) == 0  ) { ntriphones=112;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0  &&  Ph[i+2].CompareNoCase(CString("Y")) == 0  ) { ntriphones=117;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("AW")) == 0  &&  Ph[i+2].CompareNoCase(CString("AX")) == 0  ) { ntriphones=120;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("Y")) == 0  &&  Ph[i+2].CompareNoCase(CString("UH")) == 0  ) { ntriphones=124;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0  &&  Ph[i+2].CompareNoCase(CString("L")) == 0  ) { ntriphones=131;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("W")) == 0  &&  Ph[i+1].CompareNoCase(CString("AH")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=133;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("Y")) == 0  &&  Ph[i+2].CompareNoCase(CString("UW")) == 0  ) { ntriphones=141;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("EY")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=142;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("W")) == 0  &&  Ph[i+2].CompareNoCase(CString("EH")) == 0  ) { ntriphones=143;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}


if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("EY")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=144;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=145;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}



if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("K")) == 0  ) { ntriphones=146;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}


if(Ph[i].CompareNoCase(CString("EY")) == 0  &&  Ph[i+1].CompareNoCase(CString("SH")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=147;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("AH")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=148;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}


if(Ph[i].CompareNoCase(CString("DH")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0  &&  Ph[i+2].CompareNoCase(CString("S")) == 0  ) { ntriphones=149;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}


if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("AH")) == 0  &&  Ph[i+2].CompareNoCase(CString("Z")) == 0  ) { ntriphones=150;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("AO")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=151;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}



if(Ph[i].CompareNoCase(CString("W")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0  &&  Ph[i+2].CompareNoCase(CString("L")) == 0  ) { ntriphones=152;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}



if(Ph[i].CompareNoCase(CString("H")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0  &&  Ph[i+2].CompareNoCase(CString("S")) == 0  ) { ntriphones=153;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("DH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=169;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0  &&  Ph[i+2].CompareNoCase(CString("IH")) == 0  ) { ntriphones=170;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0  &&  Ph[i+2].CompareNoCase(CString("L")) == 0  ) { ntriphones=171;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("SH")) == 0  &&  Ph[i+1].CompareNoCase(CString("IY")) == 0  &&  Ph[i+2].CompareNoCase(CString("N")) == 0  ) { ntriphones=172;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0  &&  Ph[i+2].CompareNoCase(CString("S")) == 0  ) { ntriphones=173;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0  &&  Ph[i+2].CompareNoCase(CString("R")) == 0  ) { ntriphones=174;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("AE")) == 0  &&  Ph[i+1].CompareNoCase(CString("K")) == 0  &&  Ph[i+2].CompareNoCase(CString("T")) == 0  ) { ntriphones=175;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0  &&  Ph[i+2].CompareNoCase(CString("IH")) == 0  ) { ntriphones=176;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=2;
	if(numbOfBlanks==2) break;else continue;}











}


// BIPHONES COCATENATION

if(i<=numbOfBlanks-1)
{


// TO DO

if(Ph[i].CompareNoCase(CString("Y")) == 0  &&  Ph[i+1].CompareNoCase(CString("UW")) == 0) { ntriphones=14;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}
//
//
if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0 && numbOfblanksX==2 ) { ntriphones=15;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("Z")) == 0) { ntriphones=16;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("UW")) == 0) { ntriphones=18;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=19;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AY")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=20;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("Y")) == 0) { ntriphones=24;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=25;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AO")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=26;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("W")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=27;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("W")) == 0  &&  Ph[i+1].CompareNoCase(CString("ER")) == 0) { ntriphones=28;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("EY")) == 0) { ntriphones=29;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("SH")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=31;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=32;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=33;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("EY")) == 0) { ntriphones=34;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("JH")) == 0) { ntriphones=36;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AY")) == 0  &&  Ph[i+1].CompareNoCase(CString("Z")) == 0) { ntriphones=37;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("AY")) == 0) { ntriphones=41;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AH")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=43;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AE")) == 0  &&  Ph[i+1].CompareNoCase(CString("M")) == 0) { ntriphones=44;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=45;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=46;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=47;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("OO")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=48;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("OW")) == 0) { ntriphones=51;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("Y")) == 0  &&  Ph[i+1].CompareNoCase(CString("UW")) == 0) { ntriphones=57;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("NG")) == 0) { ntriphones=59;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=61;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=62;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0) { ntriphones=63;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=64;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("W")) == 0) { ntriphones=65;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=66;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("EY")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=68;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("AO")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=70;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("SH")) == 0) { ntriphones=73;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=75;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("D")) == 0) { ntriphones=76;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("D")) == 0) { ntriphones=77;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=78;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("K")) == 0) { ntriphones=79;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("EY")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=81;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=82;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("D")) == 0) { ntriphones=83;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("p")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=84;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=85;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=86;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AH")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=87;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=88;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("V")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=89;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=90;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("M")) == 0) { ntriphones=92;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("AA")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=93;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("B")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=94;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("AY")) == 0) { ntriphones=95;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=97;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("V")) == 0) { ntriphones=98;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AO")) == 0  &&  Ph[i+1].CompareNoCase(CString("V")) == 0) { ntriphones=99;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=100;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=101;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("JH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=102;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=103;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("Z")) == 0) { ntriphones=104;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("G")) == 0  &&  Ph[i+1].CompareNoCase(CString("W")) == 0) { ntriphones=105;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=106;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("W")) == 0) { ntriphones=107;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("IY")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=108;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("S")) == 0) { ntriphones=109;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("V")) == 0  &&  Ph[i+1].CompareNoCase(CString("IY")) == 0) { ntriphones=111;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("F")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=113;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("V")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=115;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("L")) == 0) { ntriphones=116;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AE")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=118;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AW")) == 0  &&  Ph[i+1].CompareNoCase(CString("N")) == 0) { ntriphones=119;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=121;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("Y")) == 0  &&  Ph[i+1].CompareNoCase(CString("UH")) == 0) { ntriphones=122;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("Y")) == 0) { ntriphones=123;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("F")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0) { ntriphones=125;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=126;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("M")) == 0) { ntriphones=127;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=128;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("AO")) == 0) { ntriphones=129;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("L")) == 0  &&  Ph[i+1].CompareNoCase(CString("D")) == 0) { ntriphones=130;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("CH")) == 0) { ntriphones=132;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("H")) == 0  &&  Ph[i+1].CompareNoCase(CString("OW")) == 0) { ntriphones=134;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0) { ntriphones=135;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("G")) == 0) { ntriphones=136;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("Z")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0) { ntriphones=137;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("JH")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=138;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=140;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("M")) == 0  &&  Ph[i+1].CompareNoCase(CString("W")) == 0) { ntriphones=154;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("D")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=155;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}



if(Ph[i].CompareNoCase(CString("AE")) == 0  &&  Ph[i+1].CompareNoCase(CString("K")) == 0) { ntriphones=156;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AE")) == 0  &&  Ph[i+1].CompareNoCase(CString("P")) == 0) { ntriphones=157;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("F")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=158;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=159;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("OW")) == 0) { ntriphones=160;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("ZH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=161;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("ZH")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=161;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("K")) == 0  &&  Ph[i+1].CompareNoCase(CString("AE")) == 0) { ntriphones=162;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("R")) == 0  &&  Ph[i+1].CompareNoCase(CString("AX")) == 0) { ntriphones=163;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("R")) == 0) { ntriphones=164;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("M")) == 0) { ntriphones=165;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("F")) == 0  &&  Ph[i+1].CompareNoCase(CString("ER")) == 0) { ntriphones=166;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("ER")) == 0) { ntriphones=167;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("UH")) == 0) { ntriphones=168;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("AX")) == 0  &&  Ph[i+1].CompareNoCase(CString("K")) == 0) { ntriphones=177;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=178;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("P")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=179;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


if(Ph[i].CompareNoCase(CString("S")) == 0  &&  Ph[i+1].CompareNoCase(CString("F")) == 0) { ntriphones=180;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("N")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=181;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("H")) == 0  &&  Ph[i+1].CompareNoCase(CString("EH")) == 0) { ntriphones=182;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("Z")) == 0  &&  Ph[i+1].CompareNoCase(CString("IH")) == 0) { ntriphones=183;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("T")) == 0  &&  Ph[i+1].CompareNoCase(CString("EY")) == 0) { ntriphones=184;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("IY")) == 0  &&  Ph[i+1].CompareNoCase(CString("Z")) == 0) { ntriphones=185;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}

if(Ph[i].CompareNoCase(CString("IH")) == 0  &&  Ph[i+1].CompareNoCase(CString("T")) == 0) { ntriphones=187;
AudioFileZ->Write(soundstriphon[ntriphones],lentriphon[ntriphones]);AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES TRIPHONES*/
	i+=1;
	if(numbOfBlanks==1) break;else continue;}


}
}
//********************************************************************************************
//********************************************************************************************
			if(Ph[i].CompareNoCase(CString("AA")) == 0)  phonems=0;
			if(Ph[i].CompareNoCase(CString("AE")) == 0)  phonems=1;
			if(Ph[i].CompareNoCase(CString("AH")) == 0)  phonems=2;
			if(Ph[i].CompareNoCase(CString("AO")) == 0)  phonems=3;
			if(Ph[i].CompareNoCase(CString("OO")) == 0)  phonems=4;
			if(Ph[i].CompareNoCase(CString("AW")) == 0)   phonems=5;
			if(Ph[i].CompareNoCase(CString("AX")) == 0)  phonems=6;
			if(Ph[i].CompareNoCase(CString("AY")) == 0)  phonems=7;
			if(Ph[i].CompareNoCase(CString("B")) == 0)   phonems=8;
			if(Ph[i].CompareNoCase(CString("CH")) == 0)  phonems=9;
			if(Ph[i].CompareNoCase(CString("D")) == 0)   phonems=10;
			if(Ph[i].CompareNoCase(CString("DH")) == 0)  phonems=11;
			if(Ph[i].CompareNoCase(CString("EH")) == 0)  phonems=12;
			if(Ph[i].CompareNoCase(CString("ER")) == 0)  phonems=13;
			if(Ph[i].CompareNoCase(CString("EY")) == 0)  phonems=14;
			if(Ph[i].CompareNoCase(CString("F")) == 0)   phonems=15;
			if(Ph[i].CompareNoCase(CString("G")) == 0)   phonems=16;
			if(Ph[i].CompareNoCase(CString("H")) == 0)   phonems=17;
			if(Ph[i].CompareNoCase(CString("IH")) == 0)   phonems=18;
			if(Ph[i].CompareNoCase(CString("IY")) == 0)   phonems=19;
			if(Ph[i].CompareNoCase(CString("JH")) == 0)   phonems=20;
			if(Ph[i].CompareNoCase(CString("K")) == 0)   phonems=21;
			if(Ph[i].CompareNoCase(CString("L")) == 0)   phonems=22;
			if(Ph[i].CompareNoCase(CString("M")) == 0)   phonems=23;
			if(Ph[i].CompareNoCase(CString("N")) == 0)   phonems=24;
			if(Ph[i].CompareNoCase(CString("NG")) == 0)   phonems=25;
			if(Ph[i].CompareNoCase(CString("OW")) == 0)   phonems=26;
			if(Ph[i].CompareNoCase(CString("OY")) == 0)   phonems=27;
			if(Ph[i].CompareNoCase(CString("P")) == 0)   phonems=28;
			if(Ph[i].CompareNoCase(CString("R")) == 0)   phonems=29;
			if(Ph[i].CompareNoCase(CString("S")) == 0)   phonems=30;
			if(Ph[i].CompareNoCase(CString("SH")) == 0)   phonems=31;
			if(Ph[i].CompareNoCase(CString("T")) == 0)   phonems=32;
			if(Ph[i].CompareNoCase(CString("TH")) == 0)  phonems=33;
			if(Ph[i].CompareNoCase(CString("UH")) == 0)   phonems=34;
			if(Ph[i].CompareNoCase(CString("UW")) == 0)  phonems=35;
			if(Ph[i].CompareNoCase(CString("V")) == 0)   phonems=36;
			if(Ph[i].CompareNoCase(CString("W")) == 0)   phonems=37;
			if(Ph[i].CompareNoCase(CString("Y")) == 0)   phonems=38;
			if(Ph[i].CompareNoCase(CString("Z")) == 0)   phonems=39;
			if(Ph[i].CompareNoCase(CString("ZH")) == 0)   phonems=40;
			// if the word is not yet in the database
			if(Ph[i].CompareNoCase(CString("BP")) == 0)   phonems=41;
			if(Ph[i].CompareNoCase(CString("1")) == 0)    {phonems=46;stressed=true;} // primary stress;
			if(Ph[i].CompareNoCase(CString("PP")) == 0)   phonems=48; // long p as in the second p of people or up;
			if(Ph[i].CompareNoCase(CString("TT")) == 0)   phonems=49; // log t as at
			if(Ph[i].CompareNoCase(CString("LL")) == 0)   phonems=50; // long L as IN ALL
			if(Ph[i].CompareNoCase(CString("DD")) == 0)   phonems=51; // long d as IN ADD
			if(Ph[i].CompareNoCase(CString("GG")) == 0)   phonems=52; // hard G as in enGage
			if(Ph[i].CompareNoCase(CString("VV")) == 0)   phonems=53; // final v as in solve
			
			
			// before the word we must check for an attached "
			if (pausebefore=='y'){			
			AudioFileZ->Write(soundsbuffer[43],countlen[43]);
			pausebefore='0';
			}//THE pause at start word caused by the attached "
	
			
//***********************************************************************************
// start basic concatenation concatenation
if(phonems != -12345){
//Stressing phonemes
if(stressed==true){dostressed=i+1;stressed=false;}
if(i==dostressed)AudioFileZ->Write(soundsbufferS[phonems],countlen[phonems]);
// We eliminate the pause of the stress phoneme
else{if(countlen[phonems] != 46) AudioFileZ->Write(soundsbuffer[phonems],countlen[phonems]);}
memset(more,0,GlobalPauSeWordsA);
AudioFileZ->Write(more,GlobalPauSeWordsA); /*SILENCE THAT SEPARATES PHONEMES*/

			}
//**************************************************************************************
		
			
			}
//*****************CHECK GRAMMAR RULES*******************************************************

			if(regularnoun=='y'){
				AudioFileZ->Write(soundsbuffer[39],countlen[39]); 
			regularnoun='0';//THE FINAL S OF REGULAR NOUNS
			}

			if(comparative=='y'){
				AudioFileZ->Write(soundsbuffer[6],countlen[6]); 
				// we must check here for the final r of the comparative.
if(h<nWordsXRow-1)
{
if (vec[h+1].Left(1).CompareNoCase(CString("A"))==0) AudioFileZ->Write(soundsbuffer[29],countlen[29]);
if (vec[h+1].Left(1).CompareNoCase(CString("H"))==0) AudioFileZ->Write(soundsbuffer[29],countlen[29]);
if (vec[h+1].Left(1).CompareNoCase(CString("E"))==0) AudioFileZ->Write(soundsbuffer[29],countlen[29]);
if (vec[h+1].Left(1).CompareNoCase(CString("I"))==0) AudioFileZ->Write(soundsbuffer[29],countlen[29]);
if (vec[h+1].Left(1).CompareNoCase(CString("O"))==0) AudioFileZ->Write(soundsbuffer[29],countlen[29]);
if (vec[h+1].Left(1).CompareNoCase(CString("U"))==0) AudioFileZ->Write(soundsbuffer[29],countlen[29]);
if (vec[h+1].Left(1).CompareNoCase(CString("W"))==0) AudioFileZ->Write(soundsbuffer[29],countlen[29]);		
}			
			comparative='0';//THE COMPARATIVE
			}
			// regular verb ending   ED.
			if (regverb=='y'){
			AudioFileZ->Write(soundsbuffer[32],countlen[32]);
			regverb='0'; 
			} // the sound of a regular verb.
			// regular verb ending   D.
			if (regverb1=='y'){
			AudioFileZ->Write(soundsbuffer[10],countlen[10]);
			regverb1='0'; 
			} // the sound of a regular verb. ending D
			if (genitivity=='G'){AudioFileZ->Write(soundsbuffer[30],countlen[30]);
			genitivity='0'; //GENITIVE
			}

// the degrees  ° case
if (degrees=='D'){
AudioFileZ->Write(soundsbuffer[10],countlen[10]);
AudioFileZ->Write(soundsbuffer[18],countlen[18]);
AudioFileZ->Write(soundsbuffer[46],countlen[46]);
AudioFileZ->Write(soundsbuffer[16],countlen[16]);
AudioFileZ->Write(soundsbuffer[29],countlen[29]);
AudioFileZ->Write(soundsbuffer[19],countlen[19]);
AudioFileZ->Write(soundsbuffer[39],countlen[39]);
			degrees='0'; //GENITIVE
			}




			if (rfinal=='R'){AudioFileZ->Write(soundsbuffer[29],countlen[29]);
			rfinal='0';
			}//THE FINAL R // we must add vowel discrimination here

			if (reggerund=='y'){AudioFileZ->Write(soundsbuffer[18],countlen[18]);
			AudioFileZ->Write(soundsbuffer[25],countlen[25]);
			reggerund='0';
			}//THE REGULAR GERUND.

			if (superlative1=='y'){AudioFileZ->Write(soundsbuffer[18],countlen[18]); // I
			AudioFileZ->Write(soundsbuffer[30],countlen[30]);// S
			AudioFileZ->Write(soundsbuffer[32],countlen[32]); //T
			superlative1='0';
			}//THE SUPERLATIVE OF KIND ST
			if (adverbLY=='y'){AudioFileZ->Write(soundsbuffer[22],countlen[22]);
			AudioFileZ->Write(soundsbuffer[18],countlen[18]);
			adverbLY='0';
			}//THE ADVERB ENDING LY
			if (adverbNESS=='y'){AudioFileZ->Write(soundsbuffer[24],countlen[24]);
			AudioFileZ->Write(soundsbuffer[6],countlen[6]);
			AudioFileZ->Write(soundsbuffer[30],countlen[30]);
			adverbNESS='0';
			}//THE ADVERB ENDING NESS
			if (adverbLESSNESS=='y'){			
			AudioFileZ->Write(soundsbuffer[22],countlen[22]); // l
			AudioFileZ->Write(soundsbuffer[6],countlen[6]); // e reverse
			AudioFileZ->Write(soundsbuffer[30],countlen[30]); // s
			AudioFileZ->Write(soundsbuffer[24],countlen[24]);
			AudioFileZ->Write(soundsbuffer[6],countlen[6]);
			AudioFileZ->Write(soundsbuffer[30],countlen[30]);
			adverbLESSNESS='0';
			}//THE ADVERB ENDING LESSNESS
			if (adverbLESS=='y'){			
			AudioFileZ->Write(soundsbuffer[22],countlen[22]);
			AudioFileZ->Write(soundsbuffer[6],countlen[6]);
			AudioFileZ->Write(soundsbuffer[30],countlen[30]);
				adverbLESS='0';
			}//THE ADVERB ENDING LESS

AudioFileZ->Write(more,amount); /*SILENCE THAT SEPARATES WORDS*/


		//AudioFileZ->Write(soundsbuffer[47],countlen[47]); /*SILENCE THAT SEPARATES WORDS*/
		if (punctuation==',')AudioFileZ->Write(soundsbuffer[42],countlen[42]); //COMMA
		if (punctuation=='.')AudioFileZ->Write(soundsbuffer[43],countlen[43]); //PERIOD
		if (punctuation==':')AudioFileZ->Write(soundsbuffer[44],countlen[44]); //TWO POINTS
		if (punctuation==';')AudioFileZ->Write(soundsbuffer[45],countlen[45]); //SEMICOLON
		if (punctuation=='?')AudioFileZ->Write(soundsbuffer[43],countlen[43]); //question mark
		if (punctuation=='!')AudioFileZ->Write(soundsbuffer[43],countlen[43]); //exlamation
		if (punctuation=='"')AudioFileZ->Write(soundsbuffer[43],countlen[43]); // " we use now the exlamation silence



		punctuation='0';

}

else{// if there are punctuation like that this   ,   is   ,   a   test
	
//AudioFileZ = new CFile(path_buffer, CFile::modeCreate|CFile::modeWrite | CFile::shareDenyNone);
//AudioFileZ->Write(soundsbuffer[47],countlen[47]); //SILENCE
if (punctuation==',')AudioFileZ->Write(soundsbuffer[42],countlen[42]); //COMMA
if (punctuation=='.')AudioFileZ->Write(soundsbuffer[43],countlen[43]); //PERIOD
if (punctuation==':')AudioFileZ->Write(soundsbuffer[44],countlen[44]); //TWO POINTS
if (punctuation==';')AudioFileZ->Write(soundsbuffer[45],countlen[45]); //SEMICOLON
if (punctuation=='?')AudioFileZ->Write(soundsbuffer[43],countlen[43]); //question mark
if (punctuation=='!')AudioFileZ->Write(soundsbuffer[43],countlen[43]); //exlamation
if (punctuation=='"')AudioFileZ->Write(soundsbuffer[43],countlen[43]); // " we use now the exlamation silence


punctuation='0';

}

//************************************END CHECK GRAMMAR RULES*******************************

if(nop ==false)
{

delete [] Ph;
}  
nop=false; // in case it was true
			}
delete test; // the CStringPlus object;	
delete [] vec;// the array of written words in a row
delete [] vecP;// the array of pronounced words in a row
delete [] sectors; // the index array
// now it is time to save in the Wave format for later listening
WAVEFORMATEX  formatoUse ; 
int byterec= AudioFileZ->GetLength()-44;
int chunksize=byterec+36;
AudioFileZ->SeekToBegin();
LPSTR teta = "RIFF";
AudioFileZ->Write(teta,4);
AudioFileZ->Write(&chunksize,sizeof(int));
teta = "WAVEfmt ";
AudioFileZ->Write(teta,8);
int q=16;
AudioFileZ->Write(&q,sizeof(int));
formatoUse.nSamplesPerSec=wf.nSamplesPerSec; // doing so we can save the wav
//file with the speed ruled by the slider control
formatoUse.wFormatTag=WAVE_FORMAT_PCM;
formatoUse.wBitsPerSample=16;
formatoUse.nChannels=1;
formatoUse.nBlockAlign=(formatoUse.nChannels * formatoUse.wBitsPerSample)/8;
formatoUse.nAvgBytesPerSec=(formatoUse.nSamplesPerSec * formatoUse.nChannels * (formatoUse.wBitsPerSample/8));
AudioFileZ->Write(&formatoUse,16); // la struttura WAVEFORMATEX ha size pari a 16 no cbSize;
teta = "data";
AudioFileZ->Write(teta,4);
AudioFileZ->Write(&byterec,sizeof(int));
AudioFileZ->Close();
// we hide the progresscontrol and we delete the newly created member of the structure
((StPause*)pParam)->progcompute->ShowWindow(SW_HIDE);
//delete ((StPause*)pParam)->progcompute; // this slider was allocated on the heap when
//((StPause*)pParam)->progcompute=NULL; // we null the pointer;
delete pParam;// we delete the data sent to the thread
// we invoked the saveto filewav function and now it is the right time to destroy it

delete AudioFileZ;
HeapFree(GetProcessHeap(), 0, more);// we free the memory used to pause between words and phonemes


AfxMessageBox("The process of export to a wav file is successfuly accomplished." ,MB_OK,0);  
// now we can finish
TextStream.Close();
speakdirect=NULL;
AfxEndThread(0,true);
return 0;
}





void CTTSView::OnFileSavetexttowavefile()
{
if(soundsbuffer==NULL) return;// if no voice is loaded we go home
   // "*.my" for "MyType Files" and "*.*' for "All Files."
   static char BASED_CODE szFilter[] = "Windows Waveform 44 KHz (*.wav)|*.wav|Only wav files (*wav*)|*wav*||";
   // Create an Open dialog; the default file name extension is ".my".
   CFileDialog fileDlg (FALSE, "wav", "*.wav",
      OFN_OVERWRITEPROMPT| OFN_HIDEREADONLY, szFilter, this);
        if( fileDlg.DoModal ()==IDOK )
   {

StPause *ok; // this is the correct way to prepare the parameter for the thread we 'll delete it at the end of the thread
ok = new StPause;
ok->path=fileDlg.GetPathName();
ok->pauselen=GlobalPauSeWords;
ok->progcompute=new CProgressCtrl;
ok->progcompute=&progresssave;

//CString * temp;
//temp = new CString[1];
try
{//t the pointer rich1 to richtemp for a fast memory identity and to use 
richtemp.GetTextRange(0,richtemp.GetTextLength(),ok->T);}
catch(int){ return;}
if(ok->T.GetLength() < 1){ delete ok; return;}
CTTSDoc* pDoc = GetDocument();
if(pDoc->Total.GetCount() < 2) return; // at the beginning in an empty document there is one element
// the document cobarray representing the word archive  in the Speaking thread contest
controlThread=11; counter =0;
TotalGlobal = new CObArray;
TotalGlobal = &(pDoc->Total);
//***********************
//findword.Format("%d",GlobalPauSeWords);
//UpdateData(FALSE);
speakdirect=AfxBeginThread(SpeakThread,(LPVOID)ok,THREAD_PRIORITY_NORMAL,0,0, NULL);



   }
}

void CTTSView::SetSTRESS(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"1"; else m_wp=m_wp+" 1";
UpdateData(FALSE);
OnEnChangePron();

}

void CTTSView::OnBnClickedStopspeak()
{
PauseBetweenWords=(CSliderCtrl*)GetDlgItem(IDC_SLIDERPAUSE);
PauseBetweenWords->EnableWindow(TRUE);
//delete der; // and then no more use of it
controlThread=10; //abort code
waveOutClose(hw);
// TODO: aggiungere qui il codice per la gestione della notifica del controllo.
}










void CTTSView::CreateFileForWordResource(void)
{
   // "*.my" for "MyType Files" and "*.*' for "All Files."
   static char BASED_CODE szFilter[] = "English TTS Database (*.dbe)|*.dbe|Only archive files(*dbe*)|*dbe*||";
   // Create an Open dialog; the default file name extension is ".my".
   CFileDialog fileDlg (FALSE, "DBE", "*.dbe",
      OFN_OVERWRITEPROMPT| OFN_HIDEREADONLY, szFilter, this);
      if( fileDlg.DoModal ()==IDOK )
   {
	  
CTTSDoc* pDoc = GetDocument();
TotalGlobal = &(pDoc->Total);
INT_PTR num = TotalGlobal->GetCount();
UnitWord * test;
test = new UnitWord[num];
CFile * content;
content = new CFile(fileDlg.GetPathName(), CFile::modeCreate|CFile::modeWrite | CFile::shareDenyNone);
content->Write(&num,sizeof(int)); // The total size
for(int r = 0; r< num; r++)
{
test[r].ncharW=((dictionary*)TotalGlobal->GetAt(r))->GetWritten().GetLength();
test[r].Writ= new char[test[r].ncharW];
test[r].Writ=((dictionary*)TotalGlobal->GetAt(r))->GetWritten();
test[r].ncharP=((dictionary*)TotalGlobal->GetAt(r))->GetPronounced().GetLength();
test[r].Pron= new char[test[r].ncharP];
test[r].Pron =((dictionary*)TotalGlobal->GetAt(r))->GetPronounced();
content->Write(&test[r].ncharW,sizeof(int));
content->Write(&test[r].ncharP,sizeof(int));
content->Write(test[r].Writ,test[r].ncharW);
content->Write(test[r].Pron,test[r].ncharP);
}
content->Close();
delete [] test;
   }

}

inline CString CheckDate(CString toverify)
{
int pos; bool flagdate=false;
CString purenumb = toverify.SpanIncluding("1234567890");	
CString toreturn;
if(toverify.GetLength()==purenumb.GetLength() && toverify.GetLength()== 4) 
{
CString lowyear = purenumb.Right(2);
CString hiyear = purenumb.Left(2);
CString ret= BinarySearch(hiyear,TotalGlobal,pos);
if(pos > -1 ){ toreturn=(*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced();flagdate=true;}
else flagdate=false;
ret= BinarySearch(lowyear,TotalGlobal,pos);
if(pos > -1 ){ toreturn.Append(" 1 ");// we use primary stress to separate 
toreturn.Append( (*((dictionary*)TotalGlobal->GetAt(pos))).GetPronounced() ); flagdate=true;}
else flagdate = false;
}
if(flagdate) return toreturn;
else return CString("n_d");
}

inline CString BinarySearch(CString tofind,CObArray * words, int &ps)
{
// this function return a string and a reference to the position where the string was found.
int left =0; int right= words->GetSize()-1;int location;int pos=-1;
dictionary *temp;
while(left <= right)
{location =(left+right)/2;
CString l;
l=(*((dictionary*)words->GetAt(location))).GetWritten();
if(l.Trim().CompareNoCase(tofind.Trim())== 0 ) pos=location;
if ((*(dictionary*)(words->GetAt(location))).GetWritten().CompareNoCase(tofind) < 0) left = location+1;
else right = location -1;
}

if(pos > -1)
{
temp = (dictionary*)words->GetAt(pos);
ps=pos;
return temp->GetWritten();
}
else{ps=-1; return "n_f";}
}

inline CString BinarySearchI(CString tofind,CObArray * words, int &ps, int * index)
{
int left =0; int right= 0;int location;int pos=-1;
// this function return a string and a reference to the position where the string was found.
CString purenumb = tofind.SpanIncluding("1234567890");	
if(tofind.GetLength()==purenumb.GetLength()) // this is a pure number so we look for it
{left = 0; right=(index[0])+1;}
else
{
//we find the search range and we drammaticaly reduce the search time 
for(int l = 0 ; l <26; l++)
{
if(tofind.Left(1).CompareNoCase(CString((char)(97+l)))==0)
				{
					left = (index[l])+1; right = (index[l+1])+1;break;
				} // no point to continue if we find the range
}
}

// ************************* READY TO SEARCH IN THE RANGE

// now left and right know where to 
dictionary *temp;
while(left <= right)
{location =(left+right)/2;
CString l;
l=(*((dictionary*)words->GetAt(location))).GetWritten();
if(l.Trim().CompareNoCase(tofind.Trim())== 0 ) pos=location;
if ((*(dictionary*)(words->GetAt(location))).GetWritten().CompareNoCase(tofind.Trim()) < 0) left = location+1;
else right = location -1;
}

if(pos > -1)
{
temp = (dictionary*)words->GetAt(pos);
ps=pos;
return temp->GetWritten();
}
else{ps=-1; return "n_f";}
}

inline CString BinarySearchAC(CString tofind,CObArray * words, int &ps, int * index)
{
// this function is a short form of BinarySearchI function and it servers to find
// the acronims or initials that are at the end of database marked with the  ~ carachter.
int left =0; int right= 0;int location;int pos=-1;
left = (index[26]+1); right = words->GetCount()-1;
// the array of integers index has been obtained by the function indexdata which 
// fills it with the propers value of starting letter of words through the database. 
// ************************* READY TO SEARCH IN THE RANGE
// now left and right know where to 
dictionary *temp;
while(left <= right)
{location =(left+right)/2;
CString l;
l=(*((dictionary*)words->GetAt(location))).GetWritten();
if(l.Trim().CompareNoCase(tofind.Trim())== 0 ) pos=location;
if ((*(dictionary*)(words->GetAt(location))).GetWritten().CompareNoCase(tofind.Trim()) < 0) left = location+1;
else right = location -1;
}
if(pos > -1)
{
temp = (dictionary*)words->GetAt(pos);
ps=pos;
return temp->GetWritten();
}
else{ps=-1; return "n_f";}
}


inline int *  IndexData(CObArray * words)
{
int *  n; n= new int[27];
int location=0;
for(int k= 0 ; k < 26; k++)
{
while(location < words->GetCount())
{
if((*(dictionary*)(words->GetAt(location))).GetWritten().Left(1).CompareNoCase(CString((char)(97+k)))==0)
break;
else location++;
}
n[k]=location-1;
}
while(location < words->GetCount())
{
if((*(dictionary*)(words->GetAt(location))).GetWritten().Left(1).CompareNoCase(CString('~'))==0)
break;
else location++;
}
n[26]=location-1;
return n;
}


UINT AddBuf (LPVOID pParam)
{
//ISSUE in late February 07 we discover that the waveout open function cannot stay in this neighbourhood because it 
// causes slowing down and scatting of the streaming fluency
// so we open the wave once for all and we close it when every single word has just been spoken
zeta.Lock();
long readBytes;
WAVEHDR headwav0; // this is for playing the speech
WAVEHDR headwav1;
WAVEHDR headwav2;
WAVEHDR headwav3;
endOfrec=false;
headwav0.dwFlags=WHDR_DONE;headwav0.dwUser=1;
headwav1.dwFlags=WHDR_DONE;headwav1.dwUser=1;
headwav2.dwFlags=WHDR_DONE;headwav2.dwUser=1;
headwav3.dwFlags=WHDR_DONE;headwav3.dwUser=1;
long lfile=AudioFileMem->GetLength();
gg = (lfile / hh ) * ff; // Time in milliseconds of the wave
if(lfile <= bufSize) // if the file is smaller than bufSize
{
TimeToPlay  = (long)gg; // now we have the sleep time
headwav0.dwBufferLength=(DWORD)lfile;
headwav0.lpData=(char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, lfile );
readBytes=AudioFileMem->Read(headwav0.lpData,lfile);
waveOutPrepareHeader(hw,&headwav0,sizeof(WAVEHDR));
waveOutWrite(hw,&headwav0,sizeof(WAVEHDR));
XSleep(TimeToPlay);// we use the sleep time now to play all.
//delete AudioFileMem;
AudioFileMem->SetLength(0);
waveOutUnprepareHeader(hw,&headwav0,sizeof(WAVEHDR));
HeapFree(GetProcessHeap(), 0, headwav0.lpData);
waveOutReset(hw);
zeta.Unlock();
AfxEndThread(0,true);
return 0;
}
//Instead if the file is bigger we use quintuple buffer 

while(!endOfrec)
{


if (headwav0.dwFlags==WHDR_DONE) 
{
		if(headwav0.dwUser==7)
		{
		HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, headwav0.lpData) ; headwav0.dwUser=1;
		waveOutUnprepareHeader(hw,&headwav0,sizeof(WAVEHDR));
		}

headwav0.lpData=(char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, bufSize );
readBytes=AudioFileMem->Read(headwav0.lpData,bufSize);
			
if(readBytes < bufSize)
			{
			headwav0.dwBufferLength=readBytes; // the effective number of bytes
			waveOutPrepareHeader(hw,&headwav0,sizeof(WAVEHDR));
			waveOutWrite(hw,&headwav0,sizeof(WAVEHDR));
			endOfrec=true;
			}
			else{
headwav0.dwBufferLength=bufSize;
waveOutPrepareHeader(hw,&headwav0,sizeof(WAVEHDR));
waveOutWrite(hw,&headwav0,sizeof(WAVEHDR));}

}



if (headwav1.dwFlags==WHDR_DONE) 
{
		if(headwav1.dwUser==7)
		{
		HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, headwav1.lpData) ; headwav1.dwUser=1;
		waveOutUnprepareHeader(hw,&headwav1,sizeof(WAVEHDR));
		}

headwav1.lpData=(char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, bufSize );
readBytes=AudioFileMem->Read(headwav1.lpData,bufSize);

			if(readBytes < bufSize)
			{
			headwav1.dwBufferLength=readBytes; // the effective number of bytes
			waveOutPrepareHeader(hw,&headwav1,sizeof(WAVEHDR));
			waveOutWrite(hw,&headwav1,sizeof(WAVEHDR));
			endOfrec=true;
			}
			else{
headwav1.dwBufferLength=bufSize;
waveOutPrepareHeader(hw,&headwav1,sizeof(WAVEHDR));
waveOutWrite(hw,&headwav1,sizeof(WAVEHDR));}
}





if (headwav2.dwFlags==WHDR_DONE) 
{
		if(headwav2.dwUser==7)
		{
		HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, headwav2.lpData) ; headwav2.dwUser=1;
		waveOutUnprepareHeader(hw,&headwav2,sizeof(WAVEHDR));
		}

headwav2.lpData=(char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, bufSize );
readBytes=AudioFileMem->Read(headwav2.lpData,bufSize);

			if(readBytes < bufSize)
			{
			headwav2.dwBufferLength=readBytes; // the effective number of bytes
			waveOutPrepareHeader(hw,&headwav2,sizeof(WAVEHDR));
			waveOutWrite(hw,&headwav2,sizeof(WAVEHDR));
			endOfrec=true;
			}
		else{
headwav2.dwBufferLength=bufSize;
waveOutPrepareHeader(hw,&headwav2,sizeof(WAVEHDR));
waveOutWrite(hw,&headwav2,sizeof(WAVEHDR));}
}


if (headwav3.dwFlags==WHDR_DONE) 
{
		if(headwav3.dwUser==7)
		{
		HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, headwav3.lpData) ; headwav3.dwUser=1;
		waveOutUnprepareHeader(hw,&headwav3,sizeof(WAVEHDR));
		}

headwav3.lpData=(char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, bufSize );
readBytes=AudioFileMem->Read(headwav3.lpData,bufSize);

			if(readBytes < bufSize)
			{
			headwav3.dwBufferLength=readBytes; // the effective number of bytes
			waveOutPrepareHeader(hw,&headwav3,sizeof(WAVEHDR));
			waveOutWrite(hw,&headwav3,sizeof(WAVEHDR));
			endOfrec=true;
			}
			else {
				headwav3.dwBufferLength=bufSize;
waveOutPrepareHeader(hw,&headwav3,sizeof(WAVEHDR));
waveOutWrite(hw,&headwav3,sizeof(WAVEHDR));}
}




}

// this is the position where endOfrec becomes true
compute.wType=TIME_BYTES;// We want to inquire the MMTIME structure retrieving bytes info
if(waveOutGetPosition(hw,&compute,sizeof(compute)) == MMSYSERR_NOERROR )
{
xx = (compute.u.ms / hh ) * ff;
TimeToPlay  = (long)gg - (long)xx;
XSleep(TimeToPlay); // we must take care of it 
AudioFileMem->SetLength(0);

//delete AudioFileMem;
}
// now it's time to close the wave

waveOutReset(hw);
if(headwav0.dwUser==7)HeapFree(GetProcessHeap(), 0, headwav0.lpData);;
if(headwav1.dwUser==7)HeapFree(GetProcessHeap(), 0, headwav1.lpData);;
if(headwav2.dwUser==7)HeapFree(GetProcessHeap(), 0, headwav2.lpData);;
if(headwav3.dwUser==7)HeapFree(GetProcessHeap(), 0, headwav3.lpData);;

zeta.Unlock();// free the critical section
AfxEndThread(0,true);
return 0;
}

// My callback procedure that writes the rich edit control contents
// to a file.
static DWORD CALLBACK MyStreamInCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{  CFile* pFile = (CFile*) dwCookie;
   *pcb = pFile->Read(pbBuff, cb);
   return 0;
}


static DWORD CALLBACK MyStreamOutCallback(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
   CFile* pFile = (CFile*) dwCookie;
   pFile->Write(pbBuff, cb);
   *pcb = cb;
   return 0;
}


// this is a callback procedure for waveform operation
static void CALLBACK waveOutProc(HWAVEOUT hWaveOut,UINT uMsg,DWORD dwInstance,DWORD dwParam1,DWORD dwParam2)
{ 
if(uMsg==WOM_DONE){ ((LPWAVEHDR)dwParam1)->dwFlags=WHDR_DONE;((LPWAVEHDR)dwParam1)->dwUser=7; }
}

void CTTSView::OnBnClickedCleartext()
{
richtemp.SetSel(0, -1);
richtemp.Clear();

}


void CTTSView::PostNcDestroy()
{
if(soundsbuffer!=NULL) // if the Voice type is an user modified voice
// we dealloc with heapfree
{
if(modevoice==true) {for(int j= 0 ; j<54; ++j) HeapFree(GetProcessHeap(), 0, soundsbuffer[j]) ; }
delete [] soundsbuffer;
 soundsbuffer=NULL;
delete [] soundsbufferS;
soundsbufferS=NULL;
delete [] countlen;
}

if(soundstriphon!=NULL) { for(int j= 0 ; j < lentripho; ++j) HeapFree(GetProcessHeap(), 0, soundstriphon[j]) ; 
delete[] soundstriphon; 
soundstriphon=NULL; 
delete[] lentriphon;
lentriphon=NULL;}

if(vox != NULL)FreeLibrary(vox);

// TODO: Add your specialized code here and/or call the base class
delete m_pListBkBrush; // the brush that controls listword background in the window
delete m_pEditWBkBrush;// the brush that controls written word editbox background in the window
delete m_pEditPBkBrush;// the Pronounced word edit box
delete m_pEditFBkBrush;// the Find word edit box
delete m_allStaticBkBrush;// the background colors of all buttons
delete m_sliderBkBrush;// the background of the slider.
delete m_speedslBkBrush; // the slider speed mod.
	CFormView::PostNcDestroy();
}

void CTTSView::SetPP(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"pp"; else m_wp=m_wp+" pp";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::OnBnClickedSpeaksel()
{
if(soundsbuffer==NULL) return;// if no voice is loaded we go home
if (speakdirect != NULL) return; // if we are alredy speakingn we return.

controlThread=11; counter =0;
CTTSDoc* pDoc = GetDocument();
TotalGlobal = new CObArray;
TotalGlobal = &(pDoc->Total);
if(pDoc->Total.GetCount() < 2) return; // at the beginning in an empty document there is one element
//***********************

//CString * temp;
//temp = new CString[1];
//temp[0] = richtemp.GetSelText();
//speakdirect=AfxBeginThread(SpeakThreadDirect,(LPVOID)temp,THREAD_PRIORITY_NORMAL,0,0, NULL);

speakdirect=AfxBeginThread(SpeakThreadDirect,(LPVOID)&richtemp,THREAD_PRIORITY_NORMAL,0,0, NULL);


}

void CTTSView::OnBnClickedPause()
{
	if(speakdirect != NULL)
		speakdirect->SuspendThread();

	// TODO: Add your control notification handler code here
}

void CTTSView::OnBnClickedResume()
{
	if(speakdirect != NULL)
		speakdirect->ResumeThread();
}


void CTTSView::OnBnClickedOpentextfile()
{
   // "*.my" for "MyType Files" and "*.*' for "All Files."
   static char BASED_CODE szFilter[] = "Text Files (*.rtf)|*.rtf|Text Files(*txt*)|*txt*||";
   // Create an Open dialog; the default file name extension is ".my".
   CFileDialog fileDlg (TRUE, "rtf", "*.rtf",
       OFN_HIDEREADONLY, szFilter, this);
      if( fileDlg.DoModal ()==IDOK )
   {richtemp.Clear();
	  
			richtemp.SetSel(0, -1);
			richtemp.Clear();
CFile cFile(TEXT(fileDlg.GetPathName()), CFile::modeRead);
EDITSTREAM es;
es.dwCookie = (DWORD) &cFile;
es.pfnCallback = (EDITSTREAMCALLBACK)MyStreamInCallback; 
if(fileDlg.GetFileExt()=="rtf")
{richtemp.SetTextMode(TM_RICHTEXT); richtemp.StreamIn(SF_RTF  , es);}
else
{
	
richtemp.SetBackgroundColor(FALSE, RGB(216,217, 200));
//// ****************** This fragment determines the default text format 
//	// in the richedit
//CHARFORMAT2 cf;
cf.cbSize=sizeof(CHARFORMAT2);
cf.dwMask = CFM_COLOR|CFM_SIZE|CFM_BOLD|CFM_FACE|CFM_CHARSET  ;//|CFM_BACKCOLOR;
cf.dwEffects=CFE_BOLD;
cf.crTextColor=RGB(0,0,255);
cf.bCharSet=ANSI_CHARSET;

//cf.crBackColor=RGB(0,255,255);
// we use this function to copy the default caracter in the buffer
// of the CHARFORMAT2 structure
lstrcpyn(cf.szFaceName, "Arial", LF_FACESIZE);
cf.yHeight=250;
richtemp.SetDefaultCharFormat(cf);
//*************************** end text formatting	
richtemp.SetTextMode(TM_PLAINTEXT); richtemp.StreamIn(SF_TEXT  , es);}

cFile.Close();

	  }
}

void CTTSView::OnPaint()
{
CPaintDC dc(this); // device context for painting
CRect rect;
GetClientRect(&rect);
CBrush black(RGB(0,0,0));
CBrush old;
dc.SelectObject(black);
dc.Rectangle(&rect);
}

void CTTSView::OnRecordDeletecurrent()
{
		CTTSDoc* pDoc = GetDocument();
		pDoc->DeleteCurrentRecord();
		m_curdictionary=pDoc->GetPrevRecord();
		if(!m_curdictionary) {pDoc->curposition=-1;m_wr="";m_wp="";UpdateData(FALSE); }
		PopulateView();
// now we have to update the list box and the following code is very useful for this scope
CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTWORD);
int q = (int)(pDoc->Total.GetSize());
		if(q>=0)
	{
		pList->ResetContent();
		dictionary * temp;
		for (int p = 0; p < q; ++p)
			{
				temp = (dictionary*)pDoc->Total.GetAt(p);
				CString alfa=temp->GetWritten();
				pList->AddString((LPCSTR)alfa);
			}
	}
	
}

void CTTSView::OnBnClickedLoadvoice()
{float hamming=0,hann=0,blackman=0,filter=0;short * buftemp=NULL;double pi=4*atan((double)1);//windowing 
	
   static char BASED_CODE szFilter[] = "DMM Voice Files (*.vxv)|*.vxv|Only DMM Voice Files(*vxv*)|*vxv*||";
    CFileDialog fileDlg (TRUE, "vxv", "*.vxv",
       OFN_HIDEREADONLY, szFilter, this);
      if( fileDlg.DoModal ()==IDOK )
   {




if(soundsbuffer!=NULL) // if the Voice type is an user modified voice
// we dealloc with heapfree
{
if(modevoice==true)
{for(int j= 0 ; j<54; ++j) HeapFree(GetProcessHeap(), 0, soundsbuffer[j]) ; }
//else {for(int j= 0 ; j<54; ++j) delete  [] soundsbuffer[j]; }
delete soundsbuffer;
delete soundsbufferS;
delete [] countlen;
soundsbuffer=NULL;
soundsbufferS=NULL;
}
 // if a voice is
 // already loaded we clean first. Note that we delete the array of pointers and the array containing lenghts of single phonemes
if(vox != NULL) FreeLibrary(vox); // we detach the original voice module if attached.	   
   
	   
HRSRC hRes;
HGLOBAL hResLoad; 
int lenres;

vox= LoadLibrary(fileDlg.GetPathName());
if(vox==NULL) ExitProcess(0);


soundsbuffer = new byte*[54];// we create an array of 49 pointer to char
soundsbufferS = new byte*[54];// we create an array of 49 pointer to char
countlen= new int[54];
for(int i=0; i<54; ++i)
{
hRes = FindResource(vox, MAKEINTRESOURCE(1100+i), "AUDIO"); 
hResLoad = LoadResource(vox, hRes); 
lenres = SizeofResource(vox,hRes);countlen[i]=lenres;

//soundsbuffer[i]= (byte*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, countlen[i] );

modevoice=false;
soundsbuffer[i]=(byte*)(LockResource(hResLoad)); 
soundsbufferS[i]=(byte*)(LockResource(hResLoad)); 
FreeResource(hResLoad);
hRes=NULL;
hResLoad=NULL;



//windowing ****************************************************

//buftemp = reinterpret_cast< short* >(soundsbuffer[i]); // from byte to short 16 bit with sign
//for(int h=0; h< countlen[i]/2;++h)
//{
//hamming=0.54 -(0.46*cos((2*pi*h)/((countlen[i]/2)-1)));
//hann=0.50*(1 - cos(2*pi*h/(countlen[i]/2)-1));
//blackman=0.42 - 0.50*cos(2*pi*h/(countlen[i]/2)-1) + 0.08*cos(4*pi*h/(countlen[i]/2)-1);
//buftemp[h]=(short)((float)buftemp[h]*blackman);
//buftemp[h]=(short)((float)buftemp[h]*hamming);
//buftemp[h]=(short)((float)buftemp[h]*hann);
//}
//end windowing*****************************************************

//filtering low pass WINDOWED SINC FILTER

//buftemp = reinterpret_cast< short* >(soundsbuffer[i]); // from byte to short 16 bit with sign
//short * filtered; filtered = new short[countlen[i]/2];
//float H[100];
//float fc=0.04;// low pass filter to 1764 HZ for a sample rate of 44100
//int M=100;
//for(int j=0; j<=100;++j)
//{
//if((j-M/2)==0) H[j]=2*pi*fc;
//if((j-M/2) != 0) H[j]= sin(2*pi*fc*(j-(M/2)))/(j-(M/2));
//H[j]= H[j]*(0.54-0.46*cos(2*pi*j/M));
//}
//for(int j = 0; j<countlen[i]/2; ++j)
//{
//filtered[j]=0;
//for(int l = 0; l<=100; l++) filtered[j]= filtered[j]+ buftemp[j-l]*H[l];
//}
//// finally we add a window to smooth ends further.
//for(int q=0; q<countlen[i]/2;++q)
//{
//hamming=0.54 -(0.46*cos((2*pi*q)/((countlen[i]/2)-1)));
//buftemp[q]=filtered[q]*hamming;
//}
//delete [] filtered;

//end filtering LOW PASS WINDOWED SINC FILTER


}
// Information of the voice.
LoadString(vox,155,VoiceType.GetBuffer(75),75);
VoiceType.ReleaseBuffer();
UpdateData(FALSE);
PauseBetweenWords=(CSliderCtrl*)GetDlgItem(IDC_SLIDERPAUSE);
PauseBetweenWords->SetPos(countlen[47]);
GlobalPauSeWords=countlen[47];
pause=(int)((GlobalPauSeWords / hh ) * ff);
UpdateData(FALSE);



  }
}

void CTTSView::OnFileSaveseltexttowavefile()
{

if(soundsbuffer==NULL) return;// if no voice is loaded we go home
   // "*.my" for "MyType Files" and "*.*' for "All Files."
   static char BASED_CODE szFilter[] = "Windows Waveform 44 KHz (*.wav)|*.wav|Only wav files (*wav*)|*wav*||";
   // Create an Open dialog; the default file name extension is ".my".
   CFileDialog fileDlg (FALSE, "wav", "*.wav",
      OFN_OVERWRITEPROMPT| OFN_HIDEREADONLY, szFilter, this);
        if( fileDlg.DoModal ()==IDOK )
   {

StPause *ok; // this is the correct way to prepare the parameter for the thread we 'll delete it at the end of the thread
ok = new StPause;
ok->path=fileDlg.GetPathName();
ok->pauselen=GlobalPauSeWords;
ok->progcompute=new CProgressCtrl;
ok->progcompute=&progresssave;


try
{
ok->T = richtemp.GetSelText();
}
catch(int){ return;}
if(ok->T.GetLength() < 1){ delete ok; return;}
CTTSDoc* pDoc = GetDocument();
if(pDoc->Total.GetCount() < 2) return; // at the beginning in an empty document there is one element
// the document cobarray representing the word archive  in the Speaking thread contest
controlThread=11; counter =0;
TotalGlobal = new CObArray;
TotalGlobal = &(pDoc->Total);
//***********************
//findword.Format("%d",GlobalPauSeWords);
//UpdateData(FALSE);
speakdirect=AfxBeginThread(SpeakThread,(LPVOID)ok,THREAD_PRIORITY_NORMAL,0,0, NULL);
   }

}

void CTTSView::OnBnClickedSavetextfile()
{
	   // "*.my" for "MyType Files" and "*.*' for "All Files."
   static char BASED_CODE szFilter[] = "Text Files (*.txt)|*.txt|Only Text Files(*txt*)|*txt*||";
   // Create an Open dialog; the default file name extension is ".my".
   CFileDialog fileDlg (FALSE, "txt", "*.txt",
       OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY, szFilter, this);
      if( fileDlg.DoModal ()==IDOK )
   {
			
 CFile cFile(TEXT(fileDlg.GetPathName()), CFile::modeCreate|CFile::modeWrite);
   EDITSTREAM es;
   es.dwCookie = (DWORD) &cFile;
   es.pfnCallback = (EDITSTREAMCALLBACK)MyStreamOutCallback; 
 richtemp.StreamOut(SF_TEXT, es);
   cFile.Close();
	  }
}

void CTTSView::OnDataDataindexing()
{
if(Inspect != NULL) // as it is a modeless dialog we must destroy it
{
Inspect->ShowWindow(SW_HIDE);
delete Inspect;
}

//rich1 = richtemp;// we set the pointer rich1 to richtemp for a fast memory identity and to use 

Inspect = new DataAn(this);
CTTSDoc* pDoc = GetDocument();
Inspect->wordDB=&(pDoc->Total);
Inspect->richinspect =&richtemp;
Inspect->Create(IDD_DATAAN,NULL);
Inspect->ShowWindow(SW_SHOW);
}

void CTTSView::SetTT(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"tt"; else m_wp=m_wp+" tt";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetRF(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"*r"; else m_wp=m_wp+" *r";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetLL(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"ll"; else m_wp=m_wp+" ll";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetDD(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"dd"; else m_wp=m_wp+" dd";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::SetGG(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"gg"; else m_wp=m_wp+" gg";
UpdateData(FALSE);
OnEnChangePron();
}

void CTTSView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
SliderVoicePitch=( CSliderCtrl *) pScrollBar;
hh=2*SliderVoicePitch->GetPos();
wf.nSamplesPerSec=SliderVoicePitch->GetPos();
wf.wFormatTag=WAVE_FORMAT_PCM;
wf.wBitsPerSample=16;
wf.nChannels=1;
wf.nBlockAlign=(wf.nChannels * wf.wBitsPerSample)/8;
wf.nAvgBytesPerSec=(wf.nSamplesPerSec * wf.nChannels * (wf.wBitsPerSample/8));
if(bufSize % wf.nBlockAlign != 0 ){ bufSize += wf.nBlockAlign - (bufSize % wf.nBlockAlign);}
CFormView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CTTSView::OnBnClickedButtonresetspeed()
{hh=88200;
SliderVoicePitch=(CSliderCtrl*)GetDlgItem(IDC_SLIDERPITCH);
SliderVoicePitch->SetPos(44100);
wf.nSamplesPerSec=SliderVoicePitch->GetPos();
wf.wFormatTag=WAVE_FORMAT_PCM;
wf.wBitsPerSample=16;
wf.nChannels=1;
wf.nBlockAlign=(wf.nChannels * wf.wBitsPerSample)/8;
wf.nAvgBytesPerSec=(wf.nSamplesPerSec * wf.nChannels * (wf.wBitsPerSample/8));
if(bufSize % wf.nBlockAlign != 0 ){ bufSize += wf.nBlockAlign - (bufSize % wf.nBlockAlign);}

}

BOOL CTTSView::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class

	return CFormView::PreTranslateMessage(pMsg);
}

void CTTSView::SetVV(void)
{
if( m_wp.GetLength()==0) m_wp=m_wp+"vv"; else m_wp=m_wp+" vv";
UpdateData(FALSE);
OnEnChangePron();

}

void CTTSView::OnEnMsgfilterRichedittext(NMHDR *pNMHDR, LRESULT *pResult)
{	
	MSGFILTER *pMsgFilter = reinterpret_cast<MSGFILTER *>(pNMHDR);

	CRect rect;
	this->GetWindowRect(&rect); // the coordinates of the window
	richtemp.GetCaretPos();

	if(pMsgFilter->wParam==MK_RBUTTON) // the riht button pressed
	{
	CPoint point=richtemp.GetCaretPos();
	CMenu * pMenu;
pMenu = m_NewMenu.GetSubMenu(0);
if(richtemp.GetSelectionType( )==SEL_EMPTY)pMenu->EnableMenuItem(ID_RICH_COPY,MF_GRAYED);
else pMenu->EnableMenuItem(ID_RICH_COPY,MF_ENABLED);
if (richtemp.CanPaste())pMenu->EnableMenuItem(ID_RICH_PASTE,MF_ENABLED);
else pMenu->EnableMenuItem(ID_RICH_PASTE,MF_GRAYED);
pMenu->TrackPopupMenu(TPM_CENTERALIGN |TPM_RIGHTBUTTON,point.x+rect.left,point.y+rect.top+190,this);
	
	}
	// TODO:  The control will not send this notification unless you override the
	// CFormView::OnInitDialog() function to send the EM_SETEVENTMASK message
	// to the control with either the ENM_KEYEVENTS or ENM_MOUSEEVENTS flag 
	// ORed into the lParam mask.
	// TODO:  Add your control notification handler code here
	
	*pResult = 0;
}

void CTTSView::CopyClip(void)
{	richtemp.Copy(); }

void CTTSView::PasteClip(void)
{ richtemp.Paste();}
void CTTSView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CFormView::OnMouseMove(nFlags, point);
}

void CTTSView::OnEditUndo()
{
	richtemp.Undo();
	// TODO: Add your command handler code here
}

void CTTSView::OnDataPhonemsvisualizer()
{
//	if(viewer != NULL) // as it is a modeless dialog we must destroy it
//{
//viewer->ShowWindow(SW_HIDE);
//delete viewer;
//}

	if(soundsbuffer==NULL){
	
AfxMessageBox("No voice patterns are loaded in Memory\nPlease Load A Voice Module First!!",MB_OK |MB_ICONINFORMATION ,0);
return;
	}

viewer = new ShowPhonem(this);
viewer->bufsound=soundsbuffer;
viewer->bufsoundS=soundsbufferS;
viewer->lenbuf=countlen;




viewer->Create(IDD_SHOWPHONEM);
viewer->ShowWindow(SW_SHOW);
viewer->SetWindowText(VoiceType);
}

void CTTSView::OnNMCustomdrawSliderpitch(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	//*pResult = 0;
}

void CTTSView::OnVoiceLoadoriginalvoicemodule()
{
	float hamming,hann,blackman,filter;short * buftemp;float pi;
	pi=4*atan((double)1);//windowing 
	
   static char BASED_CODE szFilter[] = "DMM Voice Files (*.vxv)|*.vxv|Only DMM Voice Files(*vxv*)|*vxv*||";
    CFileDialog fileDlg (TRUE, "vxv", "*.vxv",
       OFN_HIDEREADONLY, szFilter, this);
      if( fileDlg.DoModal ()==IDOK )
   {
	 
if(soundsbuffer!=NULL) // if the Voice type is an user modified voice
// we dealloc with heapfree
{
if(modevoice==true)
{for(int j= 0 ; j<54; ++j){ HeapFree(GetProcessHeap(), 0, soundsbuffer[j]) ;HeapFree(GetProcessHeap(), 0, soundsbufferS[j]) ;}



}
//else {for(int j= 0 ; j<54; ++j) delete  [] soundsbuffer[j]; }
delete soundsbuffer;
delete soundsbufferS;

delete [] countlen;
soundsbuffer=NULL;soundsbufferS=NULL;
}

if(vox != NULL) FreeLibrary(vox); // we detach the original voice module if attached.
 // if a voice is
 // already loaded we clean first. Note that we delete the array of pointers and the array containing lenghts of single phonemes
HRSRC hRes;
HGLOBAL hResLoad; 
int lenres;
vox= LoadLibrary(fileDlg.GetPathName());
if(vox==NULL) ExitProcess(0);
soundsbuffer = new byte*[54];// we create an array of 49 pointer to char
soundsbufferS = new byte*[54];// we create an array of 49 pointer to char
countlen= new int[54];
for(int i=0; i<54; ++i)
{
hRes = FindResource(vox, MAKEINTRESOURCE(1100+i), "AUDIO"); 
hResLoad = LoadResource(vox, hRes); 
lenres = SizeofResource(vox,hRes);countlen[i]=lenres;
modevoice=false;
//soundsbuffer[i]= (byte*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, countlen[i] );
soundsbuffer[i]=(byte*)(LockResource(hResLoad)); 
soundsbufferS[i]=(byte*)(LockResource(hResLoad)); 
FreeResource(hResLoad);
}
// now we amplify the Stressed voice
// Here we amplify the stressed voice by a factor of 1.5
short * buftemp;
for(int j=0;j<54;++j)
{
buftemp = reinterpret_cast< short* >(soundsbufferS[j]); // from byte to short 16 bit with sign
for(int h=0; h< countlen[j]/2;++h)
{
buftemp[h]=(short)((float)buftemp[h]* 1.20);
}
}
// end stressed voice amplification



//windowing ****************************************************

//buftemp = reinterpret_cast< short* >(soundsbuffer[i]); // from byte to short 16 bit with sign
//for(int h=0; h< countlen[i]/2;++h)
//{
//hamming=0.54 -(0.46*cos((2*pi*h)/((countlen[i]/2)-1)));
//hann=0.50*(1 - cos(2*pi*h/(countlen[i]/2)-1));
//blackman=0.42 - 0.50*cos(2*pi*h/(countlen[i]/2)-1) + 0.08*cos(4*pi*h/(countlen[i]/2)-1);
//buftemp[h]=(short)((float)buftemp[h]*blackman);
//buftemp[h]=(short)((float)buftemp[h]*hamming);
//buftemp[h]=(short)((float)buftemp[h]*hann);
//}
//end windowing*****************************************************

//filtering low pass WINDOWED SINC FILTER

//buftemp = reinterpret_cast< short* >(soundsbuffer[i]); // from byte to short 16 bit with sign
//short * filtered; filtered = new short[countlen[i]/2];
//float H[100];
//float fc=0.04;// low pass filter to 1764 HZ for a sample rate of 44100
//int M=100;
//for(int j=0; j<=100;++j)
//{
//if((j-M/2)==0) H[j]=2*pi*fc;
//if((j-M/2) != 0) H[j]= sin(2*pi*fc*(j-(M/2)))/(j-(M/2));
//H[j]= H[j]*(0.54-0.46*cos(2*pi*j/M));
//}
//for(int j = 0; j<countlen[i]/2; ++j)
//{
//filtered[j]=0;
//for(int l = 0; l<=100; l++) filtered[j]= filtered[j]+ buftemp[j-l]*H[l];
//}
//// finally we add a window to smooth ends further.
//for(int q=0; q<countlen[i]/2;++q)
//{
//hamming=0.54 -(0.46*cos((2*pi*q)/((countlen[i]/2)-1)));
//buftemp[q]=filtered[q]*hamming;
//}
//delete [] filtered;

//end filtering LOW PASS WINDOWED SINC FILTER



// Information of the voice.
LoadString(vox,155,VoiceType.GetBuffer(75),75);
VoiceType.ReleaseBuffer();
UpdateData(FALSE);
PauseBetweenWords=(CSliderCtrl*)GetDlgItem(IDC_SLIDERPAUSE);
PauseBetweenWords->SetPos(countlen[47]);
GlobalPauSeWords=countlen[47];
pause=(GlobalPauSeWords / hh ) * ff;
UpdateData(FALSE);



  }
}

void CTTSView::OnVoiceLoadusermodifiedvoicemodule()
{
	 static char BASED_CODE szFilter[] = "Modified Voice Files (*.vxa)|*.vxa|Only DMM Modified voice Files(*vxa*)|*vxa*||";
    CFileDialog fileDlg (TRUE, "vxa", "*.vxa",
       OFN_HIDEREADONLY, szFilter, this);
	if( fileDlg.DoModal ()==IDOK ){
		int len ;
	
if(soundsbuffer!=NULL) // if the Voice type is an user modified voice
// we dealloc with heapfree
{
if(modevoice==true)
{for(int j= 0 ; j<54; ++j) {HeapFree(GetProcessHeap(), 0, soundsbuffer[j]) ;HeapFree(GetProcessHeap(), 0, soundsbufferS[j]) ; }}
//else {for(int j= 0 ; j<54; ++j) delete  [] soundsbuffer[j]; }
delete soundsbuffer;
delete soundsbufferS;
delete [] countlen;
soundsbuffer=NULL;
}

 // if a voice is
 // already loaded we clean first. Note that we delete the array of pointers and the array containing lenghts of single phonemes

if(vox != NULL) FreeLibrary(vox); // we detach the original voice module if attached.
CFile Fi(fileDlg.GetPathName(),CFile::modeRead|CFile::shareDenyNone);
Fi.Read(&len, sizeof(int));
countlen = new int[len];
Fi.Read(countlen, sizeof(int)*len);
soundsbuffer = new  byte*[len];
soundsbufferS = new  byte*[len];
//for(int i = 0 ; i < len; ++i){ soundsbuffer[i]= new byte[countlen[i]];
modevoice=true;
for(int i = 0 ; i < len; ++i)
{ soundsbuffer[i]= (byte*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, countlen[i] );
soundsbufferS[i]= (byte*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, countlen[i] );
Fi.Read(soundsbuffer[i],countlen[i]);
CopyMemory(soundsbufferS[i],soundsbuffer[i],countlen[i]);
}

Fi.Close();VoiceType=CString("NOT DMM voice Module: USER MOFIFIED VOICE:");

// Here we amplify the stressed voice by a factor of 1.5 or 1.35
short * buftemp;
for(int j=0;j<54;++j)
{
buftemp = reinterpret_cast< short* >(soundsbufferS[j]); // from byte to short 16 bit with sign
for(int h=0; h< countlen[j]/2;++h)
{
buftemp[h]=(short)((float)buftemp[h]* 1.20);
}
}




UpdateData(FALSE);
PauseBetweenWords=(CSliderCtrl*)GetDlgItem(IDC_SLIDERPAUSE);
PauseBetweenWords->SetPos(countlen[47]);
GlobalPauSeWords=countlen[47];
pause=(int)((GlobalPauSeWords / hh ) * ff);
UpdateData(FALSE);

}
}

void CTTSView::OnFileSaveAsText()
{
 
	// in this piece of routine we use the CStdioFile Class in order to export
	// the content of the archive to a text file formatted row by row.
	// every row will contain the written and pronounced word.
CTTSDoc* pDoc = GetDocument();
if(pDoc->Total.GetCount() < 2) {AfxMessageBox("Before Exporting please load the word database");
	return;}

 // "*.my" for "MyType Files" and "*.*' for "All Files."
   static char BASED_CODE szFilter[] = "Exported Word Dictionary (*.txt)|*.txt|Only Text files(*Txt*)|*Txt*||";
     CFileDialog fileDlg (FALSE, "Txt", "*.Txt",
      OFN_OVERWRITEPROMPT| OFN_HIDEREADONLY, szFilter, this);
      if( fileDlg.DoModal ()==IDOK )
   {
CStdioFile TextStream( fileDlg.GetPathName(), CFile::modeCreate | CFile::modeWrite | CFile::typeText );
CString Written,Pronounced;CString SQL;
INT_PTR num = pDoc->Total.GetCount();

for(int r = 0; r< num; r++) // the cicle is for all words
{
Written =((dictionary*)((pDoc->Total)).GetAt(r))->GetWritten();
Pronounced =((dictionary*)((pDoc->Total)).GetAt(r))->GetPronounced();

TextStream.WriteString(Written+'\t'+Pronounced+'\n'); // we add the Tab escape in order to have a line per entry
}
TextStream.Close();
AfxMessageBox("The export process is successfuly terminated");
	  }

// Some example of escape sequence.
//\a
// Bell (alert)
// 
//\b
// Backspace
// 
//\f
// Formfeed
// 
//\n
// New line
// 
//\r
// Carriage return
// 
//\t
// Horizontal tab
// 
//\v
// Vertical tab
// 
//\'
// Single quotation mark
// 
//\"
// Double quotation mark
// 
//\\
// Backslash
// 
//\?
// Literal question mark
}

void CTTSView::OnFileExportdatabasetoanaccesfile()
{
	
CTTSDoc* pDoc = GetDocument();
if(pDoc->Total.GetCount() < 2) {AfxMessageBox("Before Exporting please load the word database");
	return;}

 // "*.my" for "MyType Files" and "*.*' for "All Files."
   static char BASED_CODE szFilter[] = "Access Word Dictionary (*.mdb)|*.mdb|Only Access files(*Mdb*)|*Mdb*||";
     CFileDialog fileDlg (FALSE, "Mdb", "*.Mdb",
      OFN_OVERWRITEPROMPT| OFN_HIDEREADONLY, szFilter, this);
      if( fileDlg.DoModal ()==IDOK )
   {
CString Written,Pronounced;CString SQL;
INT_PTR num = pDoc->Total.GetCount();

// database creation
CDaoDatabase db;
db.Create(fileDlg.GetPathName(),dbLangGeneral,0 );
db.Execute("CREATE TABLE Words ( Written Text(50) NOT NULL, Pronounced Text(50) NOT NULL)" ,dbFailOnError );
// end database creation

for(int r = 0; r< num; r++) // the cicle is for all words
{
Written =((dictionary*)((pDoc->Total)).GetAt(r))->GetWritten();
Pronounced =((dictionary*)((pDoc->Total)).GetAt(r))->GetPronounced();
//SQL = "Insert into dati Values('" + Written + "' , '" + Pronounced + "')";
Written= char(34)+Written+char(34); // I add the " char in order not to conflict with the '
Pronounced=char(34)+Pronounced+char(34); // the same
SQL = "Insert into Words Values(" + Written + " , " + Pronounced + ")";
db.Execute(SQL ,dbFailOnError );

}

AfxMessageBox("The export to Access process is successfuly terminated");
	  }

}

void CTTSView::OnFileExportwordstoadbefile()
{
	CreateFileForWordResource();
}

void CTTSView::OnFileImportwordsfromadbefile()
{
	
//CTTSDoc* pDoc = GetDocument();
//int *p = IndexData(&(pDoc->Total));
//	
//	findword.Format("%d",p[26]);
//	UpdateData(FALSE);
	
	// Note: this function works if we have previously created a file and imported it
	// as a resource in the executable . To do so we must see the function
	// that provide it that is  CreateFileForWordResource()
	// this function create a file that must be imported as raw data resource.
	// in a WORDDATA Section and with the name IDR_WORDDATAINTERNAL
		
//	TotalGlobal = new CObArray;
//	
//CMemFile intern(1024); // this cmemfile is used to get data from the resource of Words 
//						// that are stored in the executable.
//HRSRC hRes;
//HGLOBAL hData;DWORD size;	
//
//hRes = ::FindResource(AfxGetResourceHandle(),MAKEINTRESOURCE(IDR_WORDDATAINTERNAL), _T("WORDDATA")); 
//hData =::LoadResource(AfxGetResourceHandle(), hRes);
//size = ::SizeofResource(AfxGetResourceHandle(),hRes);
//// now we have hData and size that tell us about the data and its size
//intern.Write(hData,size); // we write in the CMemFile the entire content of the resource
//intern.Seek(0,CFile::begin);// we take back the pointer of the CMemFile to the beginning 
//							// for the following reading operation.
//int num;// we know that the first 4 bytes of the data chunk is an integer that informs
//		// us on the total number of words in the archive.
//intern.Read(&num,sizeof (int)); // this is the total number of words.
//UnitWordR * test1; // our data structure is test1
//test1 = new UnitWordR[num];// know we create the proper array
//CString sW; CString sP; // we use two strings for temporary operations
//dictionary *newdic = NULL;
//
//for(int r = 0; r< num; r++) // the cicle is for all words
//{
//intern.Read(&test1[r].ncharW,sizeof(int));  // we read the number of chars that make up wordwritten
//test1[r].Writ= new char[test1[r].ncharW];// we dimension the array properly
//intern.Read(&test1[r].ncharP,sizeof(int));// we read the number of chars that make up wordpronounced
//test1[r].Pron= new char[test1[r].ncharP];// we dimension it
//intern.Read(test1[r].Writ,test1[r].ncharW);// now we read the buffer of word written
//intern.Read(test1[r].Pron,test1[r].ncharP);// we read the buffer of word pronounced
//newdic = new dictionary; // we create the temporary class
//sW= CString(test1[r].Writ); // we convert to string the buffer and we obtain rubbish at the right of it
//newdic->SetWritten(sW.Left(test1[r].ncharW));// we clean extracting only the lenght of the string
//sP= CString(test1[r].Pron);// the same
//newdic->SetPronounced(sP.Left(test1[r].ncharP));// the same
//TotalGlobal->Add(newdic);// the CObArray is adjurned and filled properly
//}
//
//FreeResource(hData);

// the following is the CFile version.

   static char BASED_CODE szFilter[] = "English TTS Database (*.dbe)|*.dbe|Only archive files(*dbe*)|*dbe*||";
   CFileDialog fileDlg (TRUE, "DBE", "*.dbe",
      OFN_OVERWRITEPROMPT| OFN_HIDEREADONLY, szFilter, this);
      if( fileDlg.DoModal ()==IDOK )
   {
CTTSDoc* pDoc = GetDocument();
if(pDoc->GetPathName()==CString("")) {AfxMessageBox("Before appending this archive must be saved with a noun!!!!"); return;}
pDoc->OnSaveDocument(pDoc->GetPathName());

	
int num;
CFile * content;
content = new CFile(fileDlg.GetPathName(), CFile::modeRead | CFile::shareDenyNone);
content->Read(&num,sizeof (int)); // this is the total number of words.
UnitWordR * test1;
test1 = new UnitWordR[num];
CString sW; CString sP;

vector <dictionary*> newdic(num);

CString tempFind;int state,numinserted=0;
for(int r = 0; r< num; r++)
{											//this is the loop start
content->Read(&test1[r].ncharW,sizeof(int));  
test1[r].Writ= new char[test1[r].ncharW];
content->Read(&test1[r].ncharP,sizeof(int));
test1[r].Pron= new char[test1[r].ncharP];
content->Read(test1[r].Writ,test1[r].ncharW);
content->Read(test1[r].Pron,test1[r].ncharP);
newdic[r] = new dictionary;
sW= CString(test1[r].Writ);
newdic[r]->SetWritten(sW.Left(test1[r].ncharW));
sP= CString(test1[r].Pron);
newdic[r]->SetPronounced(sP.Left(test1[r].ncharP));
// here we must verify if the word already exist in the mail database
 tempFind=BinarySearch(newdic[r]->GetWritten(),&(pDoc->Total),state);
		if(state == -1 )

		{pDoc->Total.Add(newdic[r]);numinserted++;

// this fragment sorts the CObarray;
		// this must be done everytime we add a word in order for the 
		// binary search to work properly next time
int k,i,j,number;
dictionary  *hdic;
hdic= new dictionary;
number=(int)pDoc->Total.GetCount();
  k=(int)(pDoc->Total.GetCount()/2);
  while (k>0)
  {for (i=0; i<number-k; ++i)
	 { j=i;
	 while (j>=0 &&  ((dictionary*)pDoc->Total.GetAt(j))->GetWritten()>((dictionary*)pDoc->Total.GetAt(j+k))->GetWritten())
	 {  
	 *hdic =*((dictionary*)pDoc->Total.GetAt(j));
	*((dictionary*)(pDoc->Total.GetAt(j)))=*((dictionary*)(pDoc->Total.GetAt(j+k)));
	*((dictionary*)pDoc->Total.GetAt(j+k))=*hdic;
	j=j-k;    
	 }
	 }
	 k=k/2;
  }
delete hdic;


		}
													// this is the loop end
}

content->Close(); delete [] test1; // no more use in it


 // now we have to populate the list box
 
CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTWORD);
pList->ResetContent();
int q = (int)(pDoc->Total.GetSize());
		if(q>=0)
	{
		pList->ResetContent();
		dictionary * temp;
		for (int p = 0; p < q; ++p)
			{
				temp = (dictionary*)pDoc->Total.GetAt(p);
				CString alfa=temp->GetWritten();
				pList->AddString((LPCSTR)alfa);
			}
	}
PopulateView();
// End list box population
		CString report;
		report.Format("Number of inserted words = %d",numinserted);
		AfxMessageBox(report);


	  }


}

void CTTSView::OnFileImportwordsfromadbefileforfullarchive()
{
	  static char BASED_CODE szFilter[] = "English TTS Database (*.dbe)|*.dbe|Only archive files(*dbe*)|*dbe*||";
   CFileDialog fileDlg (TRUE, "DBE", "*.dbe",
      OFN_OVERWRITEPROMPT| OFN_HIDEREADONLY, szFilter, this);
      if( fileDlg.DoModal ()==IDOK )
   {
CTTSDoc* pDoc = GetDocument();


	
int num;
CFile * content;
content = new CFile(fileDlg.GetPathName(), CFile::modeRead | CFile::shareDenyNone);
content->Read(&num,sizeof (int)); // this is the total number of words.
pDoc->Total.SetSize(num+512,1024);
pDoc->Total.RemoveAll();


UnitWordR * test1;
test1 = new UnitWordR[num];
CString sW; CString sP;

vector <dictionary*> newdic(num);

CString tempFind;int state,numinserted=0;
for(int r = 0; r< num; r++)
{											//this is the loop start
content->Read(&test1[r].ncharW,sizeof(int));  
test1[r].Writ= new char[test1[r].ncharW];
content->Read(&test1[r].ncharP,sizeof(int));
test1[r].Pron= new char[test1[r].ncharP];
content->Read(test1[r].Writ,test1[r].ncharW);
content->Read(test1[r].Pron,test1[r].ncharP);
newdic[r] = new dictionary;
sW= CString(test1[r].Writ);
newdic[r]->SetWritten(sW.Left(test1[r].ncharW));
sP= CString(test1[r].Pron);
newdic[r]->SetPronounced(sP.Left(test1[r].ncharP));
pDoc->Total.Add(newdic[r]);numinserted++;
													// this is the loop end
}

content->Close(); delete [] test1; // no more use in it


 // now we have to populate the list box
 
CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTWORD);
pList->ResetContent();
int q = (int)(pDoc->Total.GetSize());
		if(q>=0)
	{
		pList->ResetContent();
		dictionary * temp;
		for (int p = 0; p < q; ++p)
			{
				temp = (dictionary*)pDoc->Total.GetAt(p);
				CString alfa=temp->GetWritten();
				pList->AddString((LPCSTR)alfa);
			}
	}

PopulateView();
// End list box population
		CString report;
		report.Format("Number of inserted words = %d",numinserted);
		AfxMessageBox(report);


	  }


}

void CTTSView::OnEnChangeEditfind()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CFormView::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}




void CTTSView::OnVoiceLoadtriphonesknowledge()
{
	// The first thing to do is to create an interface to get the file name to open
static char BASED_CODE szFilter[]= "Triphones Info Voice Files(*.tri)|*.tri|Only DMM Modified voice Files(*tri*)|*tri*||";
CFileDialog fileDlg (TRUE, "tri", "*.tri",OFN_HIDEREADONLY, szFilter, this);
// now we can process the file we have just chosen
if( fileDlg.DoModal ()==IDOK )
{
int len ;
// we open our file for reading
CFile Fi(fileDlg.GetPathName(),CFile::modeRead|CFile::shareDenyNone);
// we read the first bytes of the file where we have the number of audio chunks
Fi.Read(&len, sizeof(int));
lentriphon = new int[len]; // we alloc memory for every chunk size container
Fi.Read(lentriphon, sizeof(int) * len); // we read the sizes of every chunk
// now lentriphon contains a number equal to len of chunk sizes
soundstriphon = new byte*[len]; // here we declare an array len audio chunks
for(int i = 0 ; i < len; ++i) // we cycle for every chunk
{ 
// we allocate the memory and we read the content of every chunk providing its length
soundstriphon[i]= (byte*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, lentriphon[i] );
Fi.Read(soundstriphon[i],lentriphon[i]);
}
// when done we can close our file
Fi.Close();VoiceType=CString("NOT DMM voice Module: USER MOFIFIED VOICE:");
}


}

void CTTSView::OnDataBitriphonesvisualizer()
{
		if(soundstriphon==NULL){
	
AfxMessageBox("No voice patterns are loaded in Memory\nPlease Load A Voice Module First!!",MB_OK |MB_ICONINFORMATION ,0);
return;
	}

bitriviewer = new ShowBiTriPhonem(this);
bitriviewer->bufsound=soundstriphon;

bitriviewer->lenbuf=lentriphon;

bitriviewer->Create(IDD_SHOWBITRIPHONEM);
bitriviewer->ShowWindow(SW_SHOW);
bitriviewer->SetWindowText("Spectral view of BiTriphonems Knowledge 5000 maximum storage");
}


//void CTTSView::OnFileImportfromaccessfile()
//{
//	CTTSDoc* pDoc = GetDocument();
//
//
// // "*.my" for "MyType Files" and "*.*' for "All Files."
//   static char BASED_CODE szFilter[] = "Access Word Dictionary (*.mdb)|*.mdb|Only Access files(*Mdb*)|*Mdb*||";
//     CFileDialog fileDlg (TRUE, "Mdb", "*.Mdb",
//      OFN_OVERWRITEPROMPT| OFN_HIDEREADONLY, szFilter, this);
//      if( fileDlg.DoModal ()==IDOK )
//   {
//CString Written,Pronounced;
//INT_PTR num = pDoc->Total.GetCount();
//
//// database creation
//COleVariant vWritten,vPronounced;
//CDaoDatabase db;
//db.Open(fileDlg.GetPathName(),FALSE,FALSE,_T("") );
//CDaoRecordset recordset = CDaoRecordset(&db);
//recordset.Open(dbOpenTable,CString("Words"),dbReadOnly);
//
//recordset.MoveFirst();
//pDoc->Total.RemoveAll();
//vector <dictionary*> newdic(recordset.GetRecordCount());
//for (int n = 0 ; n< recordset.GetRecordCount();n++)
//{
//recordset.GetFieldValue(CString("written"), vWritten);
//recordset.GetFieldValue(CString("pronounced"), vPronounced);
//Written = V_BSTRT( &vWritten );
//Pronounced = V_BSTRT( &vPronounced );
//newdic[n] = new dictionary;
//newdic[n]->SetWritten(Written);
//newdic[n]->SetPronounced(Pronounced);
//pDoc->Total.Add(newdic[n]);
//recordset.MoveNext();
//}
//	  }
// // now we have to populate the list box
// 
//CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTWORD);
//pList->ResetContent();
//int q = (int)(pDoc->Total.GetSize());
//		if(q>=0)
//	{
//		pList->ResetContent();
//		dictionary * temp;
//		for (int p = 0; p < q; ++p)
//			{
//				temp = (dictionary*)pDoc->Total.GetAt(p);
//				CString alfa=temp->GetWritten();
//				pList->AddString((LPCSTR)alfa);
//			}
//	}
//
//PopulateView();
//// End list box population
//		CString report;
//		report.Format("Number of inserted words = %d",pList->GetCount());
//
//OnBnClickedFirst();
//
//		AfxMessageBox(report);
//
//}

void CTTSView::OnNMCustomdrawSliderpause(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void CTTSView::OnStnClickedSposition()
{
	// TODO: Add your control notification handler code here
}
