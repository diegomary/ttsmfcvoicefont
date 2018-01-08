// ShowPhonem.cpp : implementation file
//

#include "stdafx.h"
#include "TTS.h"
#include "ShowBiTriPhonem.h"
#include <math.h>
#include "Mmsystem.h"
#include "Mmreg.h"//per operazioni wave

// ShowPhonem dialog

IMPLEMENT_DYNAMIC(ShowBiTriPhonem, CDialog)

ShowBiTriPhonem::ShowBiTriPhonem(CWnd* pParent /*=NULL*/)
	: CDialog(ShowBiTriPhonem::IDD, pParent)
	, bufsound(NULL)
	, lenbuf(NULL)
	, showphonem(0)
	,rectM1(0,0,0,0)
	, coord(_T(""))
	,flagzoom(false)
	,flagscroll(false)
	, ptprev(0)
	
	

	
	, SelPhon(0)
	, Duration(0)
	, selstart(0)
	
	, selend(0)
	, findit(_T(""))
	, numfound(0)
{
m_brushback = new CBrush(RGB( 4, 80, 221 ));
newsearch=0;


}

ShowBiTriPhonem::~ShowBiTriPhonem()

{
}




void ShowBiTriPhonem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT234, coord);
	DDX_Control(pDX, IDC_EDIT234, showphon);

	DDX_Control(pDX, IDC_LISTP, cb);
	DDX_Control(pDX, IDC_SCROLLPHONEM, scroller);

	DDX_Text(pDX, IDC_SELPHON, SelPhon);
	DDX_Text(pDX, IDC_SELSTART, selstart);
	DDX_Text(pDX, IDC_SELEND, selend);
	DDX_Text(pDX, IDC_EDITFINDIT, findit);
	DDX_Text(pDX, IDC_NUMBER, numfound);
}


BEGIN_MESSAGE_MAP(ShowBiTriPhonem, CDialog)

	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTONMINUS, &ShowBiTriPhonem::OnBnClickedButtonminus)
	ON_BN_CLICKED(IDC_BUTTONPLUS, &ShowBiTriPhonem::OnBnClickedButtonplus)
	ON_BN_CLICKED(IDC_BUTTONGLOBALGAIN, &ShowBiTriPhonem::OnBnClickedButtonglobalgain)
	ON_BN_CLICKED(IDC_BUTTONGAINMINUS, &ShowBiTriPhonem::OnBnClickedButtongainminus)
	ON_WM_MBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	
	ON_LBN_SELCHANGE(IDC_LISTP, &ShowBiTriPhonem::OnLbnSelchangeListp)
	ON_BN_CLICKED(IDC_BUTTON0LEFT, &ShowBiTriPhonem::OnBnClickedButton0left)
	ON_BN_CLICKED(IDC_BUTTONZOOMSEL, &ShowBiTriPhonem::OnBnClickedButtonzoomsel)
	ON_BN_CLICKED(IDC_BUTTONZOOMEXT, &ShowBiTriPhonem::OnBnClickedButtonzoomext)
	ON_COMMAND(ID_WINDOWING_BLACKMAN, &ShowBiTriPhonem::OnWindowingBlackman)
	ON_COMMAND(ID_WINDOWING_HAMMING, &ShowBiTriPhonem::OnWindowingHamming)
	ON_COMMAND(ID_WINDOWING_HANNING, &ShowBiTriPhonem::OnWindowingHanning)
	
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTONCONNECT, &ShowBiTriPhonem::OnBnClickedButtonconnect)
	ON_BN_CLICKED(IDC_BUTTONFADE, &ShowBiTriPhonem::OnBnClickedButtonfade)
	ON_BN_CLICKED(IDC_BUTTONFADEB, &ShowBiTriPhonem::OnBnClickedButtonfadeb)
	ON_BN_CLICKED(IDC_BUTTONSHORTEN, &ShowBiTriPhonem::OnBnClickedButtonshorten)
	
	
	ON_COMMAND(ID_FILE_SAVEPERSONALIZEDVOICEAS, &ShowBiTriPhonem::OnFileSavepersonalizedvoiceas)
	ON_COMMAND(ID_FILE_CREATEBLANKVOICEFORMODELLING, &ShowBiTriPhonem::OnFileCreateblankvoiceformodelling)
	ON_COMMAND(ID_FILE_LOADPHONEME, &ShowBiTriPhonem::OnFileLoadphoneme)
	ON_COMMAND(ID_FILE_SAVEPHONEME, &ShowBiTriPhonem::OnFileSavephoneme)
	ON_BN_CLICKED(IDC_PLAYBITRI, &ShowBiTriPhonem::OnBnClickedPlaybitri)
	ON_BN_CLICKED(IDC_FINDTRIBI, &ShowBiTriPhonem::OnBnClickedFindtribi)
	ON_BN_CLICKED(IDC_RESETSEARCH, &ShowBiTriPhonem::OnBnClickedResetsearch)
END_MESSAGE_MAP()


// ShowPhonem message handlers

void ShowBiTriPhonem::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
delete this;
delete m_brushback;


	CDialog::PostNcDestroy();
}



void ShowBiTriPhonem::OnPaint()
{
CPaintDC dc(this); // device context for painting
if(flagscroll)

{
CRect rectMtemp=rectM;
CRect rect;
CBrush brush( RGB( 4, 80, 221 ) );
GetClientRect( &rect );
dc.SelectObject(&brush);
dc.Rectangle(&rect);
//FillRect( dc, &rect, brush );
dc.SetMapMode(MM_ANISOTROPIC);
dc.SetWindowExt(rectMtemp.right-rectMtemp.left+1,  12000);
dc.SetViewportOrg( m_szSize.cx - rect.right, m_szSize.cy/2);
dc.SetViewportExt( m_szSize.cx , -m_szSize.cy );// i added the minus sign 
CBrush brush1( RGB( 120, 120, 120 ) );
CPen pen;
pen.CreatePen(PS_SOLID, 0, RGB(0,255,0)); // the green for the wave
dc.SelectObject(&pen);
dc.MoveTo(0,0);
dc.LineTo(rectM.right-rectM.left,0);
short *buffer = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign
for( int p = 0; p <= (rectM.right-rectM.left); ++p )
{
total[p].x=p;
total[p].y=buffer[rectM.left+p];
}
dc.Polyline(total,rectM.right-rectM.left+1);
return;
}



if(!flagzoom)
{
CRect rect;
CBrush brush( RGB( 255, 255, 130 ) );
GetClientRect( &rect );
FillRect( dc, &rect, brush );
dc.SetMapMode(MM_ANISOTROPIC);
dc.SetWindowExt(lenbuf[showphonem] / 2,  24000);
dc.SetViewportOrg( m_szSize.cx - rect.right, m_szSize.cy/2);
dc.SetViewportExt( m_szSize.cx , -m_szSize.cy );// i added the minus sign 
CBrush brush1( RGB( 120, 120, 120 ) );
if(rectM != CRect(0,0,0,0)) dc.FillRect(&rectM,&brush1);
CPen pen;
pen.CreatePen(PS_SOLID, 1, RGB(255,0,0) );
dc.SelectObject(&pen);
dc.MoveTo(0,600);dc.LineTo(lenbuf[showphonem]/2,600);
dc.MoveTo(0,-600);dc.LineTo(lenbuf[showphonem]/2,-600);
dc.MoveTo(0,400);dc.LineTo(lenbuf[showphonem]/2,400);
dc.MoveTo(0,-400);dc.LineTo(lenbuf[showphonem]/2,-400);
dc.MoveTo(0,800);dc.LineTo(lenbuf[showphonem]/2,800);
dc.MoveTo(0,-800);dc.LineTo(lenbuf[showphonem]/2,-800);
dc.MoveTo(0,1000);dc.LineTo(lenbuf[showphonem]/2,1000);
dc.MoveTo(0,-1000);dc.LineTo(lenbuf[showphonem]/2,-1000);
dc.MoveTo(0,1200);dc.LineTo(lenbuf[showphonem]/2,1200);
dc.MoveTo(0,-1200);dc.LineTo(lenbuf[showphonem]/2,-1200);
dc.MoveTo(0,1400);dc.LineTo(lenbuf[showphonem]/2,1400);
dc.MoveTo(0,-1400);dc.LineTo(lenbuf[showphonem]/2,-1400);
dc.MoveTo(0,1600);dc.LineTo(lenbuf[showphonem]/2,1600);
dc.MoveTo(0,-1600);dc.LineTo(lenbuf[showphonem]/2,-1600);
dc.MoveTo(0,1800);dc.LineTo(lenbuf[showphonem]/2,1800);
dc.MoveTo(0,-1800);dc.LineTo(lenbuf[showphonem]/2,-1800);
dc.MoveTo(0,2000);dc.LineTo(lenbuf[showphonem]/2,2000);
dc.MoveTo(0,-2000);dc.LineTo(lenbuf[showphonem]/2,-2000);

dc.MoveTo(0,4000);dc.LineTo(lenbuf[showphonem]/2,4000);
dc.MoveTo(0,-4000);dc.LineTo(lenbuf[showphonem]/2,-4000);


pen.CreatePen(PS_SOLID, 0, RGB(0,0,0)); // the green for the wave
dc.SelectObject(&pen);
short *buffer = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign
for( int p = 0; p < lenbuf[showphonem]/2; ++p )
{
total[p].x=p;
total[p].y=buffer[p];
}
dc.Polyline(total,lenbuf[showphonem] / 2);
pen.CreatePen(PS_SOLID, 1, RGB(0,0,255) );
dc.SelectObject(&pen);
dc.MoveTo(0,0);
dc.LineTo(lenbuf[showphonem]/2,0);
}

else // this is the zoom selection case
{CRect rectMtemp=rectM;
CRect rect;
CBrush brush( RGB( 4, 80, 221 ) );
GetClientRect( &rect );
FillRect( dc, &rect, brush );
dc.SetMapMode(MM_ANISOTROPIC);
dc.SetWindowExt(rectMtemp.right-rectMtemp.left+1,  12000);
dc.SetViewportOrg( m_szSize.cx - rect.right, m_szSize.cy/2);
dc.SetViewportExt( m_szSize.cx , -m_szSize.cy );// i added the minus sign 
CBrush brush1( RGB( 120, 120, 120 ) );
if(rectM1 != CRect(0,0,0,0))
{dc.SelectObject(&brush1);dc.SetROP2(R2_MERGEPEN); dc.Rectangle(&rectM1);dc.SetROP2(R2_COPYPEN);}
CPen pen;
pen.CreatePen(PS_SOLID, 0, RGB(0,255,0)); // the green for the wave
dc.SelectObject(&pen);
dc.MoveTo(0,0);
dc.LineTo(rectM.right-rectM.left,0);
short *buffer = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign
for( int p = 0; p <= (rectM.right-rectM.left); ++p )
{
total[p].x=p;
total[p].y=buffer[rectM.left+p];
}
dc.Polyline(total,rectM.right-rectM.left+1);
}



}

void ShowBiTriPhonem::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);


if( cx == 0 || cy == 0 ) return;
  m_szSize = CSize( cx, cy );
  Invalidate(1);



	// TODO: Add your message handler code here
}

BOOL ShowBiTriPhonem::OnInitDialog()
{
CDialog::OnInitDialog();
int timetoplay;


rectM=CRect(0,0,0,0);
CRect rect;double gg; double hh=88200,ff=1000;CString tempus;
showphonem=0 ;
total= new CPoint[lenbuf[showphonem]/2];
coord="M AE N  :MAN ";
gg = (lenbuf[0] / hh) * ff;  
timetoplay=(int)gg;
Duration=timetoplay;
LenPhon=lenbuf[0];
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);

selstart=0;
gg = (rectM.right*2 / hh) * ff;
selend=(int)gg;
gg = (LenPhon / hh) * ff;
SelPhon=(int)gg;
UpdateData(FALSE);
selstart=0;
selend=LenPhon;
SelPhon=LenPhon;

showphon.GetWindowRect(&rect);
// Release the current font
        m_fSampFont.Detach();
        // Create the font to be used
        m_fSampFont.CreateFont((rect.Height()- 5), 0, 0, 0, FW_NORMAL,
                0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS,
                CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH |
                FF_DONTCARE, CString("Verdana"));

 m_fSampFont1.CreateFont(16, 0, 0, 0, FW_NORMAL,
                0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS,
                CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH |
                FF_DONTCARE, CString("Verdana"));
        // Set the font for the sample display area
    showphon.SetFont(&m_fSampFont);
	cb.SetFont(&m_fSampFont1);
infoscroll.cbSize=sizeof(SCROLLINFO);
infoscroll.fMask=SIF_ALL;
infoscroll.nMin=0;
infoscroll.nMax=lenbuf[showphonem]/2;
infoscroll.nPage=10;
infoscroll.nPos=0;
infoscroll.nTrackPos=0;
scroller.SetScrollInfo(&infoscroll,1);
//scroller.SetScrollRange(0,lenbuf[showphonem]/2);

//*******************************************************************
cb.ResetContent();int nums=-1;CString tempstring;
if(bufsound[0] != NULL){
	coord="M AE N :rifleMAN ";
gg = (lenbuf[0] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[1] != NULL){
	coord="AX N T :astonishmeENT ";
gg = (lenbuf[1] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[2] != NULL){
	coord="AX B L :feasIBLE "; 
gg = (lenbuf[2] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[3] != NULL){
	coord="T IH NG :acceleraTING ";
gg = (lenbuf[3] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[4] != NULL){
	coord="S IH NG :announCING ";
gg = (lenbuf[4] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[5] != NULL){
	coord="P IH NG :hoPING ";
gg = (lenbuf[5] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[6] != NULL){
	coord="R IH NG :dRINking ";
gg = (lenbuf[6] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[7] != NULL){
	coord="L IH NG :feeLING ";
gg = (lenbuf[7] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[8] != NULL){
	coord="M IH NG :alarMing ";
gg = (lenbuf[8] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[9] != NULL){
	coord="N IH NG :listeNING ";
gg = (lenbuf[9] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[10] != NULL){
	coord="K IH NG :clocKING";
gg = (lenbuf[10] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[11] != NULL){
	coord="L IH SH :accompLISH ";
gg = (lenbuf[11] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[12] != NULL){
	coord="T R AE :TRAnsformed ";
gg = (lenbuf[12] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[13] != NULL){
	coord="IH NG K :drINk";
gg = (lenbuf[13] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[14] != NULL){
	coord="Y UW :YOU";
gg = (lenbuf[14] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[15] != NULL){
	coord="IH T :IT ";
gg = (lenbuf[15] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[16] != NULL){
	coord="IH Z :is ";
gg = (lenbuf[16] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[17] != NULL){
	coord="T IH D :expecTED ";
gg = (lenbuf[17] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[18] != NULL){
	coord="R UW :ROOm ";
gg = (lenbuf[18] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[19] != NULL){
	coord="S T :announcED ";
gg = (lenbuf[19] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[20] != NULL){
	coord="AY T :brIGHT ";
gg = (lenbuf[20] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[21] != NULL){
	coord="UH AX L :intellectUAL ";
gg = (lenbuf[21] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[22] != NULL){
	coord="IH SH N :ambITION ";
gg = (lenbuf[22] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[23] != NULL){
	coord="P R AO :PROblem ";
gg = (lenbuf[23] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[24] != NULL){
	coord="P IY :PEople ";
gg = (lenbuf[24] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[25] != NULL){
	coord="P L :peoPLE ";
gg = (lenbuf[25] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[26] != NULL){
	coord="AO N :ON ";
gg = (lenbuf[26] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[27] != NULL){
	coord="W EH :WHEn ";
gg = (lenbuf[27] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[28] != NULL){
	coord="W ER :WORds ";
gg = (lenbuf[28] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[29] != NULL){
	coord="N EY :NAture ";
gg = (lenbuf[29] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[30] != NULL){
	coord="R IH D :caRRIED ";
gg = (lenbuf[30] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[31] != NULL){
	coord="SH N :noTION  ";
gg = (lenbuf[31] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[32] != NULL){
	coord="IH N :INdependent ";
gg = (lenbuf[32] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[33] != NULL){
	coord="R IH :REmain ";
gg = (lenbuf[33] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[34] != NULL){
	coord="R EY :acceleRAte";
gg = (lenbuf[34] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[35] != NULL){
	coord="AX N S :intelligENCE ";
gg = (lenbuf[35] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[36] != NULL){
	coord="IH JH :manAGE ";
gg = (lenbuf[36] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[37] != NULL){
	coord="AY Z :characterIZE  ";
gg = (lenbuf[37] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[38] != NULL){
	coord="IH L AX :abILIty ";
gg = (lenbuf[38] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[39] != NULL){
	coord="EH R IH :vERIfy ";
gg = (lenbuf[39] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[40] != NULL){
	coord="M P L :siMPLe ";
gg = (lenbuf[40] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[41] != NULL){
	coord="B AY :BY ";
gg = (lenbuf[41] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[42] != NULL){
	coord="B AY D :aBIDE ";
gg = (lenbuf[42] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[43] != NULL){
	coord="AH N :someONE ";
gg = (lenbuf[43] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[44] != NULL){
	coord="AE M :AMbassador ";
gg = (lenbuf[44] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[45] != NULL){
	coord="S AX :ambaSSAdor  ";
gg = (lenbuf[45] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[46] != NULL){
	coord="D AX :accorDAnce ";
gg = (lenbuf[46] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[47] != NULL){
	coord="B IH :BEYond ";
gg = (lenbuf[47] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[48] != NULL){
	coord="OO L :ALL ";
gg = (lenbuf[48] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[49] != NULL){
	coord="N AX UH :NO ";
gg = (lenbuf[49] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[50] != NULL){
	coord="P IY P L :PEOPLE ";
gg = (lenbuf[50] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[51] != NULL){
	coord="N OW :KNOW ";
gg = (lenbuf[51] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[52] != NULL){
	coord="W AO T :WHAT ";
gg = (lenbuf[52] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[53] != NULL){
	coord="M OW S T :MOST ";
gg = (lenbuf[53] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[54] != NULL){
	coord="K AX N :CONtinue ";
gg = (lenbuf[54] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[55] != NULL){
	coord="H AE V :HAVE ";
gg = (lenbuf[55] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[56] != NULL){
	coord="T Y UW :inTUItive ";
gg = (lenbuf[56] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[57] != NULL){
	coord="Y UW :intUItive ";
gg = (lenbuf[57] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}
if(bufsound[58] != NULL){
	coord="T IH V :intuiTIVE ";
gg = (lenbuf[58] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[59] != NULL){
	coord="IH NG :beeING ";
gg = (lenbuf[59] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[60] != NULL){
	coord="K Y UH :doCUment ";
gg = (lenbuf[60] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[61] != NULL){
	coord="L AX :abiLIty ";
gg = (lenbuf[61] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[62] != NULL){
	coord="IH AX :bacterIA ";
gg = (lenbuf[62] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[63] != NULL){
	coord="B AE :BActeria ";
gg = (lenbuf[63] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[64] != NULL){
	coord="M EH :fundaMEntal ";
gg = (lenbuf[64] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[65] != NULL){
	coord="K W :aQUire ";
gg = (lenbuf[65] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[66] != NULL){
	coord="AX R :expERience ";
gg = (lenbuf[66] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[67] != NULL){
	coord="D IH D :recorDID ";
gg = (lenbuf[67] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[68] != NULL){
	coord="EY T :candidATE ";
gg = (lenbuf[68] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[69] != NULL){
	coord="K AE N :CAN ";
gg = (lenbuf[69] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[70] != NULL){
	coord="AO T :nOT ";
gg = (lenbuf[70] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[71] != NULL){
	coord="T L IH :abrupTLY ";
gg = (lenbuf[71] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[72] != NULL){
	coord="R AH P :abRUPtly ";
gg = (lenbuf[72] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[73] != NULL){
	coord="IH SH :fISHing ";
gg = (lenbuf[73] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[74] != NULL){
	coord="AE N D :AND ";
gg = (lenbuf[74] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}


if(bufsound[75] != NULL){
	coord="K T :marKED ";
gg = (lenbuf[75] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[76] != NULL){
	coord="IH D :accepTED ";
gg = (lenbuf[76] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[77] != NULL){
	coord="N D :arouND ";
gg = (lenbuf[77] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[78] != NULL){
	coord="N T :fragmeNT ";
gg = (lenbuf[78] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[79] != NULL){
	coord="S K :diSKo ";
gg = (lenbuf[79] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[80] != NULL){
	coord="P IH D :stuPID ";
gg = (lenbuf[80] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[81] != NULL){
	coord="EY N :ANgel ";
gg = (lenbuf[81] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[82] != NULL){
	coord="T IH :conTInue ";
gg = (lenbuf[82] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[83] != NULL){
	coord="AX D :hundrED ";
gg = (lenbuf[83] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[84] != NULL){
	coord="P L :simPLE ";
gg = (lenbuf[84] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[85] != NULL){
	coord="S IH :SImple ";
gg = (lenbuf[85] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[86] != NULL){
	coord="B R :BRight ";
gg = (lenbuf[86] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[87] != NULL){
	coord="AH L :dULL ";
gg = (lenbuf[87] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[88] != NULL){
	coord="S EH :SEveral ";
gg = (lenbuf[88] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[89] != NULL){
	coord="V R :seVERal ";
gg = (lenbuf[89] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[90] != NULL){
	coord="AX L :severAL ";
gg = (lenbuf[90] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[91] != NULL){
	coord="IH S T :greatEST ";
gg = (lenbuf[91] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[92] != NULL){
	coord="S M :SMart ";
gg = (lenbuf[92] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[93] != NULL){
	coord="AA T :smART ";
gg = (lenbuf[93] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[94] != NULL){
	coord="B L :feeBLest ";
gg = (lenbuf[94] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[95] != NULL){
	coord="L AY :aLIve ";
gg = (lenbuf[95] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[96] != NULL){
	coord="L AY V :aLIVE ";
gg = (lenbuf[96] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[97] != NULL){
	coord="AX N :AN ";
gg = (lenbuf[97] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[98] != NULL){
	coord="IH V :intuitIVE ";
gg = (lenbuf[98] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[99] != NULL){
	coord="AO V :OF ";
gg = (lenbuf[99] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[100] != NULL){
	coord="T EH :inTElligence ";
gg = (lenbuf[100] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[101] != NULL){
	coord="L IH :inteLLIgence ";
gg = (lenbuf[101] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[102] != NULL){
	coord="JH AX :aGEncy ";
gg = (lenbuf[102] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[103] != NULL){
	coord="N IH :maNY ";
gg = (lenbuf[103] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[104] != NULL){
	coord="D Z :carDS ";
gg = (lenbuf[104] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[105] != NULL){
	coord="G W :distinGUish ";
gg = (lenbuf[105] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[106] != NULL){
	coord="D IH :DIstinguish ";
gg = (lenbuf[106] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[107] != NULL){
	coord="T W :beTWeen ";
gg = (lenbuf[107] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}


if(bufsound[108] != NULL){
	coord="IY N :betwEEN ";
gg = (lenbuf[108] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[109] != NULL){
	coord="N S :intelligeNCE ";
gg = (lenbuf[109] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[110] != NULL){
	coord="EH V AX :whatEVER ";
gg = (lenbuf[110] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[111] != NULL){
	coord="V IY :VEnus ";
gg = (lenbuf[111] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[112] != NULL){
	coord="N AX S :veNUS ";
gg = (lenbuf[112] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[113] != NULL){
	coord="F R :diFFERent ";
gg = (lenbuf[113] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[114] != NULL){
	coord="L EH :LEvel ";
gg = (lenbuf[114] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[115] != NULL){
	coord="V L :leVEL ";
gg = (lenbuf[115] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[116] != NULL){
	coord="IH L :skILL ";
gg = (lenbuf[116] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[117] != NULL){
	coord="K T Y :intelleCTUal ";
gg = (lenbuf[117] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[118] != NULL){
	coord="AE N :lANguage ";
gg = (lenbuf[118] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[119] != NULL){
	coord="AW N :accoUNt ";
gg = (lenbuf[119] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[120] != NULL){
	coord="P AW AX :POWEr ";
gg = (lenbuf[120] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[121] != NULL){
	coord="D R :chilDRen ";
gg = (lenbuf[121] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[122] != NULL){
	coord="Y UH :dUring ";
gg = (lenbuf[122] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[123] != NULL){
	coord="D Y :camboDIa ";
gg = (lenbuf[123] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[124] != NULL){
	coord="D Y UH :DUring ";
gg = (lenbuf[124] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[125] != NULL){
	coord="F AE :FAmily ";
gg = (lenbuf[125] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[126] != NULL){
	coord="M AX :faMIly ";
gg = (lenbuf[126] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[127] != NULL){
	coord="IH M :IMportant ";
gg = (lenbuf[127] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[128] != NULL){
	coord="R T :impoRTant ";
gg = (lenbuf[128] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[129] != NULL){
	coord="P AO :imPOrtantt ";
gg = (lenbuf[129] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[130] != NULL){
	coord="L D :worLD ";
gg = (lenbuf[130] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[131] != NULL){
	coord="N T L :appareNTLy ";
gg = (lenbuf[131] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[132] != NULL){
	coord="S CH :queSTion ";
gg = (lenbuf[132] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}


if(bufsound[133] != NULL){
	coord="W AH N :ONE ";
gg = (lenbuf[133] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[134] != NULL){
	coord="H OW :HOping ";
gg = (lenbuf[134] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[135] != NULL){
	coord="P AE :aPPArent ";
gg = (lenbuf[135] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[136] != NULL){
	coord="IH G :EXist ";
gg = (lenbuf[136] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[137] != NULL){
	coord="Z AE :eXActly ";
gg = (lenbuf[137] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[138] != NULL){
	coord="JH EH :GEneral ";
gg = (lenbuf[138] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[139] != NULL){
	coord="N R :dictioNARy ";
gg = (lenbuf[139] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[140] != NULL){
	coord="P EH :susPEct ";
gg = (lenbuf[140] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[141] != NULL){
	coord="N Y UW :contINUE ";
gg = (lenbuf[141] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[142] != NULL){
	coord="B EY T :deBATE ";
gg = (lenbuf[142] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[143] != NULL){
	coord="K W EH :QUEstion ";
gg = (lenbuf[143] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[144] != NULL){
	coord="M EY N :reMAIN ";
gg = (lenbuf[144] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[145] != NULL){
	coord="P EH N :indePENdent ";
gg = (lenbuf[145] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}


if(bufsound[146] != NULL){
	coord="M IH K :acadeMIC ";
gg = (lenbuf[146] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[147] != NULL){
	coord="EY SH N :radiATION ";
gg = (lenbuf[147] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[148] != NULL){
	coord="B AH T :BUT ";
gg = (lenbuf[148] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[149] != NULL){
	coord="DH IH S :THIS ";
gg = (lenbuf[149] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[150] != NULL){
	coord="D AH Z :DOES ";
gg = (lenbuf[150] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[151] != NULL){
	coord="N AO T :NOT ";
gg = (lenbuf[151] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[152] != NULL){
	coord="W EH L :WELL ";
gg = (lenbuf[152] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}


if(bufsound[153] != NULL){
	coord="H AE S :HAS ";
gg = (lenbuf[153] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[154] != NULL){
	coord="M W :hoMEWard ";
gg = (lenbuf[154] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}


if(bufsound[155] != NULL){
	coord="D EH :acaDEmic ";
gg = (lenbuf[155] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}


if(bufsound[156] != NULL){
	coord="AE K :AKademic ";
gg = (lenbuf[156] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[157] != NULL){
	coord="AE P :adAPtability ";
gg = (lenbuf[157] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}


if(bufsound[158] != NULL){
	coord="F IH :deFInition ";
gg = (lenbuf[158] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[159] != NULL){
	coord="P R :PRopose ";
gg = (lenbuf[159] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}


if(bufsound[160] != NULL){
	coord="P OW :comPOse ";
gg = (lenbuf[160] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[161] != NULL){
	coord="ZH AX :meaSUre ";
gg = (lenbuf[161] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}


if(bufsound[162] != NULL){
	coord="K AE :CHAracterize ";
gg = (lenbuf[162] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[163] != NULL){
	coord="R AX :chaRActerize ";
gg = (lenbuf[163] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[164] != NULL){
	coord="AX R :charactERize ";
gg = (lenbuf[164] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[165] != NULL){
	coord="AX M :problEM ";
gg = (lenbuf[165] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[166] != NULL){
	coord="F ER :FIRst ";
gg = (lenbuf[166] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[167] != NULL){
	coord="S ER :CIRcular ";
gg = (lenbuf[167] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[168] != NULL){
	coord="T UH :undersTOOd ";
gg = (lenbuf[168] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[169] != NULL){
	coord="DH AE T :THAT ";
gg = (lenbuf[169] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[170] != NULL){
	coord="M L IH :calMLY ";
gg = (lenbuf[170] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[171] != NULL){
	coord="IH AX L :arborEAL ";
gg = (lenbuf[171] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[172] != NULL){
	coord="SH IY N :maCHINE ";
gg = (lenbuf[172] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}


if(bufsound[173] != NULL){
	coord="S N S :eSSENCE ";
gg = (lenbuf[173] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[174] != NULL){
	coord="S T R :abSTRact ";
gg = (lenbuf[174] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[175] != NULL){
	coord="AE K T :ACTed ";
gg = (lenbuf[175] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[176] != NULL){
	coord="IH L IH :easILY ";
gg = (lenbuf[176] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}


if(bufsound[177] != NULL){
	coord="AX K :ACcepptable ";
gg = (lenbuf[177] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[178] != NULL){
	coord="S EH :acCEptable ";
gg = (lenbuf[178] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[179] != NULL){
	coord="P T :accePTable ";
gg = (lenbuf[179] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[180] != NULL){
	coord="S F :forCEFul ";
gg = (lenbuf[180] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[181] != NULL){
	coord="N EH :NEst ";
gg = (lenbuf[181] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[182] != NULL){
	coord="H EH :aHEAd ";
gg = (lenbuf[182] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[183] != NULL){
	coord="Z IH :enviSAge";
gg = (lenbuf[183] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[184] != NULL){
	coord="T EY :mediTAtion";
gg = (lenbuf[184] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[185] != NULL){
	coord="IY Z :eaSIly";
gg = (lenbuf[185] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[186] != NULL){
	coord="AA S :ASked ";
gg = (lenbuf[186] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

if(bufsound[187] != NULL){
	coord="IH T :crITicized (it not alone)";
gg = (lenbuf[187] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
tempstring=coord;
tempstring.Format("%d%s %s",++nums,")",coord);
cb.AddString(tempstring);
}

//************************************************************************************

cb.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}



void ShowBiTriPhonem::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
// Get the minimum and maximum scroll-bar positions.
   int minpos;
   int maxpos;
   scroller.GetScrollRange(&minpos, &maxpos); 
   maxpos = scroller.GetScrollLimit();

   // Get the current position of scroll box.
   int curpos = scroller.GetScrollPos();

 switch (nSBCode)
   {
   case SB_LEFT:      // Scroll to far left.
      curpos = minpos;
      break;

   case SB_RIGHT:      // Scroll to far right.
      curpos = maxpos;
      break;

   case SB_ENDSCROLL:   // End scroll.
      break;

   case SB_LINELEFT:      // Scroll left.
      if (curpos > minpos)
         --curpos;
      break;

   case SB_LINERIGHT:   // Scroll right.
      if (curpos < maxpos)
         ++curpos;
      break;

   case SB_PAGELEFT:    // Scroll one page left.
   {
      // Get the page size. 
      SCROLLINFO   info;
      scroller.GetScrollInfo(&info, SIF_ALL);
   
      if (curpos > minpos)
      curpos = max(minpos, curpos - (int) info.nPage);
   }
      break;

   case SB_PAGERIGHT:      // Scroll one page right.
   {
      // Get the page size. 
      SCROLLINFO   info;
    scroller.GetScrollInfo(&info, SIF_ALL);

      if (curpos < maxpos)
         curpos = min(maxpos, curpos + (int) info.nPage);
   }
      break;

   case SB_THUMBPOSITION: // Scroll to absolute position. nPos is the position
      curpos = nPos;      // of the scroll box at the end of the drag operation.
      break;

   case SB_THUMBTRACK:   // Drag scroll box to specified position. nPos is the
      curpos = nPos;     // position that the scroll box has been dragged to.
      break;
   }

scroller.SetScrollPos(curpos);
int delta=rectM.left-curpos;
rectM.left=rectM.left-delta;
rectM.right=rectM.right-delta;
flagscroll=true;
Invalidate();




CDialog::OnHScroll(nSBCode, nPos, pScrollBar);

}

void ShowBiTriPhonem::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void ShowBiTriPhonem::OnBnClickedButtonminus()
{
 short * buftemp;
buftemp = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign
if(rectM != CRect(0,0,0,0))
{
	if(rectM1 != CRect(0,0,0,0))
	
	{
	
	if(rectM1.left < rectM1.right)
{
for(int h=rectM.left+rectM1.left; h< rectM.left+rectM1.left+rectM1.right-rectM1.left ;++h)
{
buftemp[h]=(short)((float)buftemp[h]/ 1.1);
}
}
else
{
for(int h=rectM.left+rectM1.right; h< rectM.left+rectM1.right+rectM1.left-rectM1.right ;++h)
{
buftemp[h]=(short)((float)buftemp[h]/ 1.1);
}
}	
	}

	else
	{
if(rectM.left < rectM.right)
{
for(int h=rectM.left; h< rectM.right ;++h)
{
buftemp[h]=(short)((float)buftemp[h]/ 1.1);
}
}
else
{
for(int h=rectM.right; h< rectM.left ;++h)
{
buftemp[h]=(short)((float)buftemp[h]/ 1.1);
}
}
}
}

else
{
for(int h=0; h< lenbuf[showphonem]/2;++h)
{
buftemp[h]=(short)((float)buftemp[h]/ 1.1);
}
}
flagscroll=false;
Invalidate(true);
}

void ShowBiTriPhonem::OnBnClickedButtonplus()
{
 short * buftemp;
buftemp = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign
if(rectM != CRect(0,0,0,0))
{
	if(rectM1 != CRect(0,0,0,0))
	
	{
	
	if(rectM1.left < rectM1.right)
{
for(int h=rectM.left+rectM1.left; h< rectM.left+rectM1.left+rectM1.right-rectM1.left ;++h)
{
buftemp[h]=(short)((float)buftemp[h]* 1.1);
}
}
else
{
for(int h=rectM.left+rectM1.right; h< rectM.left+rectM1.right+rectM1.left-rectM1.right ;++h)
{
buftemp[h]=(short)((float)buftemp[h]* 1.1);
}
}	
	}

	else
	{
if(rectM.left < rectM.right)
{
for(int h=rectM.left; h< rectM.right ;++h)
{
buftemp[h]=(short)((float)buftemp[h]* 1.1);
}
}
else
{
for(int h=rectM.right; h< rectM.left ;++h)
{
buftemp[h]=(short)((float)buftemp[h]* 1.1);
}
}
}
}

else
{
for(int h=0; h< lenbuf[showphonem]/2;++h)
{
buftemp[h]=(short)((float)buftemp[h]* 1.1);
}
}
flagscroll=false;
Invalidate(true);
}


BOOL ShowBiTriPhonem::PreCreateWindow(CREATESTRUCT& cs)
{

// TODO: Add your specialized code here and/or call the base class

	return CDialog::PreCreateWindow(cs);
}

void ShowBiTriPhonem::OnBnClickedButtonglobalgain()
{
 short * buftemp;

for(int j=0;j<54;++j)
{
buftemp = reinterpret_cast< short* >(bufsound[j]); // from byte to short 16 bit with sign

for(int h=0; h< lenbuf[j]/2;++h)
{
buftemp[h]=(short)((float)buftemp[h]* 1.1);
}

}
//delete buftemp;
Invalidate(true);
}

void ShowBiTriPhonem::OnBnClickedButtongainminus()
{
short * buftemp;
for(int j=0;j<54;++j)
{
buftemp = reinterpret_cast< short* >(bufsound[j]); // from byte to short 16 bit with sign
for(int h=0; h< lenbuf[j]/2;++h)
{
buftemp[h]=(short)((float)buftemp[h]/1.1);
}
}
//delete buftemp;
Invalidate(true);
}

void ShowBiTriPhonem::OnMButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnMButtonDown(nFlags, point);
}

void ShowBiTriPhonem::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
if(nFlags & MK_LBUTTON)
{
CClientDC dc(this);
if(!flagzoom)// zoom extent
{
CRect rect;ptpost=point;
GetClientRect( &rect );
dc.SetMapMode(MM_ANISOTROPIC);
dc.SetWindowExt(lenbuf[showphonem] / 2 ,  12000);
dc.SetViewportOrg( m_szSize.cx - rect.right, m_szSize.cy/2);
dc.SetViewportExt( m_szSize.cx , -m_szSize.cy );// i added the minus sign 
dc.DPtoLP(&point);
CBrush brush( RGB( 120, 120, 120 ) );
int mode = dc.SetROP2(R2_MERGEPEN);

if(ptprev.x < point.x) // we deag the mouse from left to right
rectM=CRect(ptprev.x,-5000,point.x,4500);
else // we drag the mouse from right to left
rectM=CRect(point.x,-5000,ptprev.x,4500);

double gg; double hh=88200,ff=1000;

gg = (rectM.left*2 / hh) * ff;
selstart=(int)gg;

gg = (rectM.right*2 / hh) * ff;
selend=(int)gg;

//gg = (LenPhon / hh) * ff;
SelPhon=selend-selstart;



UpdateData(FALSE);
selstart=rectM.left*2;
selend=rectM.right*2;
SelPhon=LenPhon;


dc.SelectObject(&brush);
dc.Rectangle(&rectM);
}
else // zoom to selection
{
CRect rect;ptpost=point;
GetClientRect( &rect );
dc.SetMapMode(MM_ANISOTROPIC);
dc.SetWindowExt(rectM.right-rectM.left+1,  6000);
dc.SetViewportOrg( m_szSize.cx - rect.right, m_szSize.cy/2);
dc.SetViewportExt( m_szSize.cx , -m_szSize.cy );// i added the minus sign 
dc.DPtoLP(&point);
CBrush brush( RGB( 120, 120, 120 ) );
int mode = dc.SetROP2(R2_MERGEPEN);
rectM1=CRect(ptprev.x,-2500,point.x,2500);
dc.SelectObject(&brush);
dc.Rectangle(&rectM1);
}




}
	CDialog::OnMouseMove(nFlags, point);
}

void ShowBiTriPhonem::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnMButtonUp(nFlags, point);
}

void ShowBiTriPhonem::OnRButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnRButtonDown(nFlags, point);
}

void ShowBiTriPhonem::OnLButtonDown(UINT nFlags, CPoint point)
{
// TODO: Add your message handler code here and/or call default
if(!flagzoom)
{
CClientDC dc(this);
CRect rect;
GetClientRect( &rect );
dc.SetMapMode(MM_ANISOTROPIC);
dc.SetWindowExt(lenbuf[showphonem] / 2,  12000);
dc.SetViewportOrg( m_szSize.cx - rect.right, m_szSize.cy/2);
dc.SetViewportExt( m_szSize.cx , -m_szSize.cy );// i added the minus sign 	
dc.DPtoLP(&point); 	
ptprev=point;
dc.LPtoDP(rectM);// i want to erase the selection
InvalidateRect(rectM);
rectM=CRect(0,0,0,0);
}
else
{

CClientDC dc(this);
CRect rect;
GetClientRect( &rect );
dc.SetMapMode(MM_ANISOTROPIC);
dc.SetWindowExt(rectM.right-rectM.left+1,  6000);
dc.SetViewportOrg( m_szSize.cx - rect.right, m_szSize.cy/2);
dc.SetViewportExt( m_szSize.cx , -m_szSize.cy );// i added the minus sign 
dc.DPtoLP(&point); 	
ptprev=point;
dc.LPtoDP(rectM1);// i want to erase the selection
InvalidateRect(rectM1);
rectM1=CRect(0,0,0,0);


}


CDialog::OnLButtonDown(nFlags, point);
}

void ShowBiTriPhonem::OnLButtonUp(UINT nFlags, CPoint point)
{	// TODO: Add your message handler code here and/or call default
	isdown=false;
	CDialog::OnLButtonUp(nFlags, point);
}




void ShowBiTriPhonem::OnLbnSelchangeListp()
{
double gg; double hh=88200,ff=1000;
showphonem=cb.GetCurSel();
delete [] total;
total= new CPoint[lenbuf[showphonem]/2];
LenPhon=lenbuf[showphonem];
rectM=CRect(0,0,0,0);

	

static char pointerstr[_MAX_PATH];
int n =cb.GetTextLen(cb.GetCurSel());
cb.GetText(cb.GetCurSel(), pointerstr );
coord = CString(pointerstr);



selstart=0;
gg = (LenPhon / hh) * ff;
selend=(int)gg;
gg = (LenPhon / hh) * ff;
SelPhon=(int)gg;
phonemnumber=showphonem;// we must identify the phoneme number
UpdateData(FALSE);
selstart=0;
selend=LenPhon;
SelPhon=LenPhon;

Invalidate(true);
	

}

void ShowBiTriPhonem::OnBnClickedButton0left()
{int numfpoint=5;
 short * buftemp;
buftemp = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign
// we zero ends
buftemp[0]=0;
buftemp[lenbuf[showphonem]/2-1]=0;
// ****************
short * output;
output=new short[lenbuf[showphonem]/2+(2*numfpoint)];
SecureZeroMemory(output,lenbuf[showphonem]/2+(2*numfpoint));
CopyMemory(&output[numfpoint],buftemp,lenbuf[showphonem]);// this is 5 left and 5 right pad of shorts.

short temp;
for(int q=numfpoint; q <(lenbuf[showphonem]/2 +numfpoint);++q)
{
temp=output[q];
output[q]=0;
for(int v=-(floor((double)(numfpoint)/2)); v<=(floor((double)(numfpoint)/2));v++)
{
output[q]=temp+output[q+v];
}
output[q]=output[q]/numfpoint;
}
SecureZeroMemory(buftemp,lenbuf[showphonem]);
CopyMemory(buftemp,&output[numfpoint],lenbuf[showphonem]);
delete [] output;
buftemp[lenbuf[showphonem]/2-1]=0;
Invalidate(true);
}



void ShowBiTriPhonem::OnBnClickedButtonzoomsel()
{
	if(rectM == CRect(0,0,0,0)) return;

infoscroll.nMax=lenbuf[showphonem]/2-abs(rectM.right-rectM.left);
scroller.SetScrollInfo(&infoscroll,1);
scroller.ShowWindow(true);
scroller.SetScrollPos(rectM.left);
	flagzoom=true;
	Invalidate();
}

void ShowBiTriPhonem::OnBnClickedButtonzoomext()
{scroller.ShowWindow(false);
flagzoom=false;
flagscroll=false;
rectM1=CRect(0,0,0,0);
	Invalidate();

	// TODO: Add your control notification handler code here
}

void ShowBiTriPhonem::OnWindowingBlackman()
{
	//windowing ****************************************************
float blackman;short * buftemp;float pi=4*atan((double)1);//windowing 
buftemp = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign
for(int h=0; h< lenbuf[showphonem]/2;++h)
{
blackman=0.42 - 0.50*cos(2*pi*h/(lenbuf[showphonem]/2)-1) + 0.08*cos(4*pi*h/(lenbuf[showphonem]/2)-1);
buftemp[h]=(short)((float)buftemp[h]*blackman);
}
//end windowing*****************************************************
delete buftemp;
Invalidate(true);
}

void ShowBiTriPhonem::OnWindowingHamming()
{float hamming,hann,blackman,filter;short * buftemp;float pi=4*atan((double)1);//windowing 

buftemp = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign
if(rectM != CRect(0,0,0,0))
{
	if(rectM1 != CRect(0,0,0,0))
	
	{
	
	if(rectM1.left < rectM1.right)
{
int diff=(rectM.left+rectM1.left+rectM1.right-rectM1.left)-(rectM.left+rectM1.left);
for(int h=rectM.left+rectM1.left; h< rectM.left+rectM1.left+rectM1.right-rectM1.left ;++h)
{
hamming=0.54 -(0.46*cos((2*pi*h)/(diff)));
buftemp[h]=(short)((float)buftemp[h]*hamming);
}
}
else
{
int diff=(rectM.left+rectM1.right+rectM1.left-rectM1.right)-(rectM.left+rectM1.right);
for(int h=rectM.left+rectM1.right; h< rectM.left+rectM1.right+rectM1.left-rectM1.right ;++h)
{
hamming=0.54 -(0.46*cos((2*pi*h)/(diff)));
buftemp[h]=(short)((float)buftemp[h]*hamming);
}
}	
	}

	else
	{
if(rectM.left < rectM.right)
{
int diff = rectM.right-rectM.left;
for(int h=rectM.left; h< rectM.right ;++h)
{
hamming=0.54 -(0.46*cos((2*pi*h)/(diff)));
buftemp[h]=(short)((float)buftemp[h]*hamming);
}
}
else
{
int diff=rectM.left-rectM.right;
for(int h=rectM.right; h< rectM.left ;++h)
{
hamming=0.54 -(0.46*cos((2*pi*h)/(diff)));
buftemp[h]=(short)((float)buftemp[h]*hamming);
}
}
}
}

else
{
for(int h=0; h< lenbuf[showphonem]/2;++h)
{
hamming=(float)(0.54 -(0.46*cos((2*pi*h)/((lenbuf[showphonem]/2)-1))));
buftemp[h]=(short)((float)buftemp[h]*hamming);
}
}
flagscroll=false;
Invalidate(true);
}

void ShowBiTriPhonem::OnWindowingHanning()
{
	//hann=0.50*(1 - cos(2*pi*h/(countlen[i]/2)-1));
//windowing ****************************************************
float hamming,hann,blackman,filter;short * buftemp;float pi=4*atan((double)1);//windowing 
buftemp = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign
for(int h=0; h< lenbuf[showphonem]/2;++h)
{
hann=0.50*(1 - cos(2*pi*h/(lenbuf[showphonem]/2)-1));
buftemp[h]=(short)((float)buftemp[h]*hann);
}
//end windowing*****************************************************
delete buftemp;
Invalidate(true);
}

void ShowBiTriPhonem::OnNMThemeChangedScrollphonem(NMHDR *pNMHDR, LRESULT *pResult)
{
	// This feature requires Windows XP or greater.
	// The symbol _WIN32_WINNT must be >= 0x0501.
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

HBRUSH ShowBiTriPhonem::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

switch (nCtlColor) {
 
 case CTLCOLOR_DLG  : // remember that we have also created the CBrush object in the cdialog 
	 //constructor

			pDC->SetTextColor(RGB(67,69, 224));
			pDC->SetBkColor(RGB( 4, 80, 221 ));
            return (HBRUSH)(m_brushback->GetSafeHandle());

          default:
              return hbr;
          }

}

void ShowBiTriPhonem::OnBnClickedButtonconnect()
{
 short * buftemp;
buftemp = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign

	if(rectM1 != CRect(0,0,0,0))
	{



	if(rectM1.left < rectM1.right)
{
float X1,Y1,X2,Y2;
float Q;
X1=(float)(rectM.left+rectM1.left);
Y1=buftemp[(int)X1];
X2=(float)(rectM.left+rectM1.left+rectM1.right-rectM1.left);
Y2=buftemp[(int)X2];
Q=((X2*Y1)-(X1*Y2))/(X2-X1);
UpdateData(FALSE);
for(int h=rectM.left+rectM1.left; h<= rectM.left+rectM1.left+rectM1.right-rectM1.left ;++h)
{
buftemp[h]=(short)((float) h*((Y2-Y1)/(X2-X1)))+Q;
}

	}

	else {int tempus;tempus=rectM1.left;rectM1.left=rectM1.right;rectM1.right=tempus;
	float X1,Y1,X2,Y2;
float Q;
X1=(float)rectM.left+rectM1.left;
Y1=buftemp[(int)X1];
X2=(float)(rectM.left+rectM1.left+rectM1.right-rectM1.left);
Y2=buftemp[(int)X2];
Q=((X2*Y1)-(X1*Y2))/(X2-X1);
UpdateData(FALSE);
for(int h=rectM.left+rectM1.left; h<= rectM.left+rectM1.left+rectM1.right-rectM1.left ;++h)
{
buftemp[h]=(short)(((float) h*((Y2-Y1)/(X2-X1)))+Q);
}	
	
	}





	}
Invalidate();

}

void ShowBiTriPhonem::OnBnClickedButtonfade()
{ 
short * buftemp;
 buftemp = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign
// we zero ends
buftemp[0]=0;
buftemp[lenbuf[showphonem]/2-1]=0;

if(rectM.left > rectM.right)
{
double alfa = abs(rectM.right-rectM.left);
double p=1;
for (int q=rectM.right; q<=rectM.left;++q)
{
buftemp[q]=(short)((double)buftemp[q]*(1-p/alfa));
p+=1;
}
}
else
{
double alfa = rectM.right-rectM.left;
double p=1;
for (int q=rectM.left; q<=rectM.right;++q)
{
buftemp[q]=(short)((double)buftemp[q]*(1-p/alfa));
p+=1;
}
}
Invalidate();
}

void ShowBiTriPhonem::OnBnClickedButtonfadeb()
{
short * buftemp;
 buftemp = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign
// we zero ends
buftemp[0]=0;
buftemp[lenbuf[showphonem]/2-1]=0;

if(rectM.left > rectM.right)
{
double alfa = abs(rectM.right-rectM.left);
double p=1;
for (int q=rectM.right; q<=rectM.left;++q)
{
buftemp[q]=(short)((double)buftemp[q]*(0+p/alfa));
p+=1;
}
}
else
{
double alfa = rectM.right-rectM.left;
double p=1;
for (int q=rectM.left; q<=rectM.right;++q)
{
buftemp[q]=(short)((double)buftemp[q]*(0+p/alfa));
p+=1;
}
}
Invalidate();
}

void ShowBiTriPhonem::OnBnClickedButtonshorten()
{

if(rectM==CRect(0,0,0,0)) return;
//selstart=rectM.left*2;
//selend=rectM.right*2;
//SelPhon=LenPhon;

byte * temp;
temp = new byte[selend-selstart];
CopyMemory(temp,&bufsound[showphonem][selstart],(selend-selstart));

delete[] bufsound[showphonem];
bufsound[showphonem] = new byte[selend-selstart];
CopyMemory(bufsound[showphonem],temp,(selend-selstart));
delete[] temp;
lenbuf[showphonem]=selend-selstart;
Invalidate();
rectM=CRect(0,0,0,0);
int posphon=cb.GetCurSel();
OnInitDialog();
cb.SetCurSel(posphon);
OnLbnSelchangeListp();
// TODO: Add your control notification handler code here
}







void ShowBiTriPhonem::OnFileSavepersonalizedvoiceas()
{

	 static char BASED_CODE szFilter[] = "BTriphonems Knowledge (*.tri)|*.tri|Only DMM BTriphonems Knowledge(*tri*)|*tri*||";
    CFileDialog fileDlg (FALSE, "tri", "*.tri",
    OFN_HIDEREADONLY, szFilter, this);
	if( fileDlg.DoModal ()==IDOK ){int len = 5000;
	CFile Fi(fileDlg.GetPathName(),CFile::modeCreate|CFile::modeWrite);
	Fi.Write(&len, sizeof(int));
	Fi.Write(lenbuf,sizeof(int)*5000);
		for(int i = 0 ; i < 5000; ++i)
		{
		Fi.Write(bufsound[i],lenbuf[i]);
		}
	Fi.Close();
	
	}	
}

void ShowBiTriPhonem::OnFileCreateblankvoiceformodelling()
{
	// we create an emptu voice to model later
	for(int i = 0 ; i < 5000; ++i)
		{
HeapFree(GetProcessHeap(), 0,bufsound[i]);
bufsound[i]= (byte*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,1000 );
lenbuf[i]=1000;
		}
Invalidate();
	rectM=CRect(0,0,0,0);
int posphon=cb.GetCurSel();
OnInitDialog();
cb.SetCurSel(posphon);
OnLbnSelchangeListp();
}

void ShowBiTriPhonem::OnFileLoadphoneme()
{
	static char BASED_CODE szFilter[] = "PCM 44 Khz Phoneme file (*.pcm)|*.pcm|Only Raw data PCM files(*pcm*)|*pcm*||";
    CFileDialog fileDlg (TRUE, "pcm", "*.pcm",
    OFN_HIDEREADONLY, szFilter, this);
	if( fileDlg.DoModal ()==IDOK )
	
	{
		CFile Fi(fileDlg.GetPathName(),CFile::modeRead);
int lenfile=Fi.GetLength();
HeapFree(GetProcessHeap(), 0,bufsound[showphonem]);
bufsound[showphonem]= (byte*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,lenfile );
	Fi.Read(bufsound[showphonem],Fi.GetLength());
	lenbuf[showphonem]=lenfile;
	Fi.Close();	
	}

Invalidate();
rectM=CRect(0,0,0,0);
int posphon=cb.GetCurSel();
OnInitDialog();
cb.SetCurSel(posphon);
OnLbnSelchangeListp();
	// TODO: Add your command handler code here
}

void ShowBiTriPhonem::OnFileSavephoneme()
{
	static char BASED_CODE szFilter[] = "PCM 44 Khz Phoneme file (*.pcm)|*.pcm|Only Raw data PCM files(*pcm*)|*pcm*||";
    CFileDialog fileDlg (FALSE, "pcm", "*.pcm",
    OFN_HIDEREADONLY, szFilter, this);
	if( fileDlg.DoModal ()==IDOK )
	
	{
CFile Fi(fileDlg.GetPathName(),CFile::modeCreate | CFile::modeWrite);
Fi.Write(bufsound[showphonem],lenbuf[showphonem]);
Fi.Close();	

	}

}

void ShowBiTriPhonem::OnBnClickedPlaybitri()
{
static HWAVEOUT hw;
static double hh=88200,ff=1000; // parameter necessary to compute the time to sleep.
double gg; // parameter necessary to compute the time to sleep.
DWORD TimeToPlay;// parameter necessary to compute the time to sleep.
int bufSize=lenbuf[showphonem];
static WAVEFORMATEX wf ;
wf.nSamplesPerSec=44100;
wf.wFormatTag=WAVE_FORMAT_PCM;
wf.wBitsPerSample=16;
wf.nChannels=1;
wf.nBlockAlign=(wf.nChannels * wf.wBitsPerSample)/8;
wf.nAvgBytesPerSec=(wf.nSamplesPerSec * wf.nChannels * (wf.wBitsPerSample/8));
if(bufSize % wf.nBlockAlign != 0 ){ bufSize += wf.nBlockAlign - (bufSize % wf.nBlockAlign);}
WAVEHDR headwav0; // this is for playing the speech
headwav0.dwFlags=WHDR_DONE;headwav0.dwUser=1;
gg = (lenbuf[showphonem] / hh ) * ff; // Time in milliseconds of the wave
waveOutOpen(&hw,WAVE_MAPPER,&wf,NULL,0,NULL); 
TimeToPlay  = (long)gg; // now we have the sleep time
headwav0.dwBufferLength=(DWORD)bufSize;
headwav0.lpData=(char*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY, bufSize );
memcpy(headwav0.lpData,bufsound[showphonem],lenbuf[showphonem]);
waveOutPrepareHeader(hw,&headwav0,sizeof(WAVEHDR));
waveOutWrite(hw,&headwav0,sizeof(WAVEHDR));
Sleep(TimeToPlay+25);// we use the sleep time now to play all.
waveOutUnprepareHeader(hw,&headwav0,sizeof(WAVEHDR));
HeapFree(GetProcessHeap(), 0, headwav0.lpData);
waveOutReset(hw);
waveOutClose(hw);
}

void ShowBiTriPhonem::OnBnClickedFindtribi()
{
UpdateData(TRUE);int temp;
CString tempS;


int nn=0;

for (nn = newsearch; nn < cb.GetCount(); nn++)
{
cb.GetText(nn, tempS );
if(tempS.MakeUpper().Find(findit.MakeUpper(),0)>-1) {cb.SetCurSel(nn);newsearch = nn+1;numfound=nn;UpdateData(FALSE); break;}
}
if (nn == (cb.GetCount())) {newsearch=0;cb.SetCurSel(0);numfound=0;UpdateData(FALSE);}
//cb.SetCurSel( temp=cb.FindString(-1,findit) >= 0 ? temp : 0 );
//OnLbnSelchangeListp();
//temp=cb.FindString(-1,findit);
//if (temp >= 0) {
//cb.SetCurSel(temp);
OnLbnSelchangeListp();

// TODO: Add your control notification handler code here
}

void ShowBiTriPhonem::OnBnClickedResetsearch()
{
findit="";
newsearch=0;
cb.SetCurSel(0);
numfound=0;
UpdateData(FALSE);
OnLbnSelchangeListp();

	// TODO: Add your control notification handler code here
}
