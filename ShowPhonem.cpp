// ShowPhonem.cpp : implementation file
//

#include "stdafx.h"
#include "TTS.h"
#include "ShowPhonem.h"
#include <math.h>
#include "Mmsystem.h"
#include "Mmreg.h"//per operazioni wave

// ShowPhonem dialog

IMPLEMENT_DYNAMIC(ShowPhonem, CDialog)

ShowPhonem::ShowPhonem(CWnd* pParent /*=NULL*/)
	: CDialog(ShowPhonem::IDD, pParent)
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
{
m_brushback = new CBrush(RGB( 4, 80, 221 ));
}

ShowPhonem::~ShowPhonem()
{
}

void ShowPhonem::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT234, coord);
	DDX_Control(pDX, IDC_EDIT234, showphon);

	DDX_Control(pDX, IDC_LISTP, cb);
	DDX_Control(pDX, IDC_SCROLLPHONEM, scroller);

	DDX_Text(pDX, IDC_SELPHON, SelPhon);
	DDX_Text(pDX, IDC_SELSTART, selstart);
	DDX_Text(pDX, IDC_SELEND, selend);
}


BEGIN_MESSAGE_MAP(ShowPhonem, CDialog)
	ON_BN_CLICKED(IDOK, &ShowPhonem::OnBnClickedOk)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTONBACK, &ShowPhonem::OnBnClickedButtonback)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTONMINUS, &ShowPhonem::OnBnClickedButtonminus)
	ON_BN_CLICKED(IDC_BUTTONPLUS, &ShowPhonem::OnBnClickedButtonplus)
	ON_COMMAND(ID_FILE_SAVEPERSONALIZEDVOICEAS, &ShowPhonem::OnFileSavepersonalizedvoiceas)
	ON_BN_CLICKED(IDC_BUTTONGLOBALGAIN, &ShowPhonem::OnBnClickedButtonglobalgain)
	ON_BN_CLICKED(IDC_BUTTONGAINMINUS, &ShowPhonem::OnBnClickedButtongainminus)
	ON_WM_MBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	
	ON_LBN_SELCHANGE(IDC_LISTP, &ShowPhonem::OnLbnSelchangeListp)
	ON_BN_CLICKED(IDC_BUTTON0LEFT, &ShowPhonem::OnBnClickedButton0left)
	ON_BN_CLICKED(IDC_BUTTONZOOMSEL, &ShowPhonem::OnBnClickedButtonzoomsel)
	ON_BN_CLICKED(IDC_BUTTONZOOMEXT, &ShowPhonem::OnBnClickedButtonzoomext)
	ON_COMMAND(ID_WINDOWING_BLACKMAN, &ShowPhonem::OnWindowingBlackman)
	ON_COMMAND(ID_WINDOWING_HAMMING, &ShowPhonem::OnWindowingHamming)
	ON_COMMAND(ID_WINDOWING_HANNING, &ShowPhonem::OnWindowingHanning)
	
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTONCONNECT, &ShowPhonem::OnBnClickedButtonconnect)
	ON_BN_CLICKED(IDC_BUTTONFADE, &ShowPhonem::OnBnClickedButtonfade)
	ON_BN_CLICKED(IDC_BUTTONFADEB, &ShowPhonem::OnBnClickedButtonfadeb)
	ON_BN_CLICKED(IDC_BUTTONSHORTEN, &ShowPhonem::OnBnClickedButtonshorten)
	ON_COMMAND(ID_FILE_CREATEBLANKVOICEFORMODELLING, &ShowPhonem::OnFileCreateblankvoiceformodelling)
	ON_COMMAND(ID_FILE_LOADPHONEME, &ShowPhonem::OnFileLoadphoneme)
	ON_COMMAND(ID_FILE_SAVEPHONEME, &ShowPhonem::OnFileSavephoneme)
	ON_BN_CLICKED(IDC_PLAYPHONEM, &ShowPhonem::OnBnClickedPlayphonem)
END_MESSAGE_MAP()


// ShowPhonem message handlers

void ShowPhonem::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
delete this;
delete m_brushback;


	CDialog::PostNcDestroy();
}

void ShowPhonem::OnBnClickedOk()
{
double gg; double hh=88200,ff=1000;


CString tempus; 
showphonem+=1;
if(showphonem > 53) showphonem=0;
cb.SetCurSel(showphonem);


delete [] total;
total= new CPoint[lenbuf[showphonem]/2];
rectM=CRect(0,0,0,0);

//The info for the scrollbar
infoscroll.nMax=lenbuf[showphonem]/2;
infoscroll.nPage=10;
infoscroll.nPos=0;
infoscroll.nTrackPos=0;
scroller.SetScrollInfo(&infoscroll,1);
// end info.



if(showphonem==0 && bufsound[0] != NULL){
coord="(AA)  f A ther  ";
gg = (lenbuf[0] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[0];

tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==1 && bufsound[1] != NULL){
coord="(AE)  c A t  ";
gg = (lenbuf[1] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[1];

tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==2 && bufsound[2] != NULL){
coord="(AH)  b U t  ";
gg = (lenbuf[2] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[2];

tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==3 && bufsound[3] != NULL){
coord="(AO)  d O g  ";
gg = (lenbuf[3] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[3];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==4 && bufsound[4] != NULL){
coord="(OO)  s O urce  ";
gg = (lenbuf[4] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[4];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==5 && bufsound[5] != NULL){
coord="(AW)  f OU l  ";
gg = (lenbuf[5] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[5];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==6 && bufsound[6] != NULL){
coord="(AX)  A go  ";
gg = (lenbuf[6] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[6];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==7 && bufsound[7] != NULL){
coord="(AY)  b I te  ";
gg = (lenbuf[7] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[7];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==8 && bufsound[8] != NULL){
coord="(B)  B ig  ";
gg = (lenbuf[8] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[8];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==9 && bufsound[9] != NULL){
coord="(CH)  CH in  ";
gg = (lenbuf[9] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[9];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==10 && bufsound[10] != NULL){
coord="(D)  D addy  ";
gg = (lenbuf[10] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[10];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==11 && bufsound[11] != NULL){
coord="(DH)  TH en  ";
gg = (lenbuf[11] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[11];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==12 && bufsound[12] != NULL){
coord="(EH)  p E t  ";
gg = (lenbuf[12] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[12];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==13 && bufsound[13] != NULL){
coord="(ER)  f UR  ";
gg = (lenbuf[13] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[13];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==14 && bufsound[14] != NULL){
coord="(EY)  A te  ";
gg = (lenbuf[14] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[14];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==15 && bufsound[15] != NULL){
coord="(F)  F inally  ";
gg = (lenbuf[15] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[15];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==16 && bufsound[16] != NULL){
coord="(G)  G ut  ";
gg = (lenbuf[16] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[16];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==17 && bufsound[17] != NULL){
coord="(H)  H ouse  ";
gg = (lenbuf[17] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[17];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==18 && bufsound[18] != NULL){
coord="(IH)  f I ll  ";
gg = (lenbuf[18] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[18];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==19 && bufsound[19] != NULL){
coord="(IY)  f EE ling  ";
gg = (lenbuf[19] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[19];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==20 && bufsound[20] != NULL){
coord="(JH)  J oy  ";
gg = (lenbuf[20] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[20];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==21 && bufsound[21] != NULL){
coord="(K)  Kitten  ";
gg = (lenbuf[21] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[21];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}

if(showphonem==22 && bufsound[22] != NULL){
coord="(L)  L ibertine  ";
gg = (lenbuf[22] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[22];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==23 && bufsound[23] != NULL){
coord="(M)  M arking ";
gg = (lenbuf[23] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[23];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==24 && bufsound[24] != NULL){
coord="(N)  N arrow  ";
gg = (lenbuf[24] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[24];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==25 && bufsound[25] != NULL){
coord="(NG)  barki NG  ";
gg = (lenbuf[25] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[25];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==26 && bufsound[26] != NULL){
coord="(OW)  c OA t  ";
gg = (lenbuf[26] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[26];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==27 && bufsound[27] != NULL){
coord="(OY)  enj OY ";
gg = (lenbuf[27] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[27];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==28 && bufsound[28] != NULL){
coord="(P)  P eace  ";
gg = (lenbuf[28] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[28];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==29 && bufsound[29] != NULL){
coord="(R)  f R ee  ";
gg = (lenbuf[29] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[29];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==30 && bufsound[30] != NULL){
coord="(S)  out S ide   ";
gg = (lenbuf[30] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[30];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==31 && bufsound[31] != NULL){
coord="(SH)  SH ame  ";
gg = (lenbuf[31] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[31];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==32 && bufsound[32] != NULL){
coord="(T)  T amed  ";
gg = (lenbuf[32] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[32];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==33 && bufsound[33] != NULL){
coord="(TH)  TH in  ";
gg = (lenbuf[33] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[33];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==34 && bufsound[34] != NULL){
coord="(UH)  l OO k ";
gg = (lenbuf[34] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[34];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==35 && bufsound[35] != NULL){
coord="(UW)  z OO  ";
gg = (lenbuf[35] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[35];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==36 && bufsound[36] != NULL){
coord="(V)  V ariety  ";
gg = (lenbuf[36] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[36];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==37 && bufsound[37] != NULL){
coord="(W)  W akeful  ";
gg = (lenbuf[37] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[37];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==38 && bufsound[38] != NULL){
coord="(Y)  Y elling  ";
gg = (lenbuf[38] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[38];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==39 && bufsound[39] != NULL){
coord="(Z)  Z ebras  ";
gg = (lenbuf[39] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[39];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==40 && bufsound[40] != NULL){
coord="(ZH)  plea S uring  ";
gg = (lenbuf[40] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[40];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==41 && bufsound[41] != NULL){
coord="UNKNOWN  ";
gg = (lenbuf[41] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[41];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==42 && bufsound[42] != NULL){
coord="COMMA  ";
gg = (lenbuf[42] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[42];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==43 && bufsound[43] != NULL){
coord="PERIOD  ";
gg = (lenbuf[43] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[43];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==44 && bufsound[44] != NULL){
coord="TWOPOINTS  ";
gg = (lenbuf[44] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[44];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==45 && bufsound[45] != NULL){
coord="SEMICOLON  ";
gg = (lenbuf[45] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[45];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==46 && bufsound[46] != NULL){
coord="PRIMARYSTRESS  ";
gg = (lenbuf[46] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[46];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==47 && bufsound[47] != NULL){
coord="SILENCE  ";
gg = (lenbuf[47] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[47];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==48 && bufsound[48] != NULL){
coord="(PP)  peo P le  ";
gg = (lenbuf[48] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[48];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==49 && bufsound[49] != NULL){
coord="(TT)  vas T  ";
gg = (lenbuf[49] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[49];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==50 && bufsound[50] != NULL){
coord="(LL)  a LL  ";
gg = (lenbuf[50] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[50];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==51 && bufsound[51] != NULL){
coord="(DD)  a DD  ";
gg = (lenbuf[51] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[51];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==52 && bufsound[52] != NULL){
coord="(GG)  ea G le  ";
gg = (lenbuf[52] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[52];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==53 && bufsound[53] != NULL){
coord="(VV)  o F  ";
gg = (lenbuf[53] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[53];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
flagzoom=false;
flagscroll=false;
Invalidate(true);

selstart=0;
gg = (LenPhon / hh) * ff;
selend=(int)gg;
gg = (LenPhon / hh) * ff;
SelPhon=(int)gg;

UpdateData(FALSE);
selstart=0;
selend=LenPhon;
SelPhon=LenPhon;




	// TODO: Add your control notification handler code here
	//OnOK();
}

void ShowPhonem::OnPaint()
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
CBrush brush( RGB( 0, 0, 0 ) );
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
pen.CreatePen(PS_SOLID, 0, RGB(0,255,0)); // the green for the wave
dc.SelectObject(&pen);
short *buffer = reinterpret_cast< short* >(bufsound[showphonem]); // from byte to short 16 bit with sign
for( int p = 0; p < lenbuf[showphonem]/2; ++p )
{
total[p].x=p;
total[p].y=buffer[p];
}
dc.Polyline(total,lenbuf[showphonem] / 2);
pen.CreatePen(PS_SOLID, 1, RGB(255,255,60) );
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

void ShowPhonem::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);


if( cx == 0 || cy == 0 ) return;
  m_szSize = CSize( cx, cy );
  Invalidate(1);



	// TODO: Add your message handler code here
}

BOOL ShowPhonem::OnInitDialog()
{
CDialog::OnInitDialog();
int timetoplay;


rectM=CRect(0,0,0,0);
CRect rect;double gg; double hh=88200,ff=1000;CString tempus;
showphonem=0 ;
total= new CPoint[lenbuf[showphonem]/2];
coord="(AA)  fAther  ";
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

        // Set the font for the sample display area


    showphon.SetFont(&m_fSampFont);
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
cb.ResetContent();
if(bufsound[0] != NULL){
coord="(AA)  f A ther  ";
gg = (lenbuf[0] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[1] != NULL){
coord="(AE)  c A t  ";
gg = (lenbuf[1] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[2] != NULL){
coord="(AH)  b U t  ";
gg = (lenbuf[2] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[3] != NULL){
coord="(AO)  d O g  ";
gg = (lenbuf[3] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[4] != NULL){
coord="(OO)  s O urce  ";
gg = (lenbuf[4] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[5] != NULL){
coord="(AW)  f OU l  ";
gg = (lenbuf[5] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[6] != NULL){
coord="(AX)  A go  ";
gg = (lenbuf[6] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[7] != NULL){
coord="(AY)  b I te  ";
gg = (lenbuf[7] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[8] != NULL){
coord="(B)  B ig  ";
gg = (lenbuf[8] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[9] != NULL){
coord="(CH)  CH in  ";
gg = (lenbuf[9] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[10] != NULL){
coord="(D)  D addy  ";
gg = (lenbuf[10] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[11] != NULL){
coord="(DH)  TH en  ";
gg = (lenbuf[11] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[12] != NULL){
coord="(EH)  p E t  ";
gg = (lenbuf[12] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[13] != NULL){
coord="(ER)  f UR  ";
gg = (lenbuf[13] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[14] != NULL){
coord="(EY)  A te  ";
gg = (lenbuf[14] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[15] != NULL){
coord="(F)  F inally  ";
gg = (lenbuf[15] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[16] != NULL){
coord="(G)  G ut  ";
gg = (lenbuf[16] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[17] != NULL){
coord="(H)  H ouse  ";
gg = (lenbuf[17] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[18] != NULL){
coord="(IH)  f I ll  ";
gg = (lenbuf[18] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[19] != NULL){
coord="(IY)  f EE ling  ";
gg = (lenbuf[19] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[20] != NULL){
coord="(JH)  J oy  ";
gg = (lenbuf[20] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[21] != NULL){
coord="(K)  Kitten  ";
gg = (lenbuf[21] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}

if(bufsound[22] != NULL){
coord="(L)  L ibertine  ";
gg = (lenbuf[22] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[23] != NULL){
coord="(M)  M arking ";
gg = (lenbuf[23] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[24] != NULL){
coord="(N)  N arrow  ";
gg = (lenbuf[24] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[25] != NULL){
coord="(NG)  barki NG  ";
gg = (lenbuf[25] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[26] != NULL){
coord="(OW)  c OA t  ";
gg = (lenbuf[26] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[27] != NULL){
coord="(OY)  enj OY ";
gg = (lenbuf[27] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[28] != NULL){
coord="(P)  P eace  ";
gg = (lenbuf[28] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[29] != NULL){
coord="(R)  f R ee  ";
gg = (lenbuf[29] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[30] != NULL){
coord="(S)  out S ide   ";
gg = (lenbuf[30] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[31] != NULL){
coord="(SH)  SH ame  ";
gg = (lenbuf[31] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[32] != NULL){
coord="(T)  T amed  ";
gg = (lenbuf[32] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[33] != NULL){
coord="(TH)  TH in  ";
gg = (lenbuf[33] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[34] != NULL){
coord="(UH)  l OO k ";
gg = (lenbuf[34] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[35] != NULL){
coord="(UW)  z OO  ";
gg = (lenbuf[35] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[36] != NULL){
coord="(V)  V ariety  ";
gg = (lenbuf[36] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[37] != NULL){
coord="(W)  W akeful  ";
gg = (lenbuf[37] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[38] != NULL){
coord="(Y)  Y elling  ";
gg = (lenbuf[38] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[39] != NULL){
coord="(Z)  Z ebras  ";
gg = (lenbuf[39] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[40] != NULL){
coord="(ZH)  plea S uring  ";
gg = (lenbuf[40] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[41] != NULL){
coord="UNKNOWN  ";
gg = (lenbuf[41] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[42] != NULL){
coord="COMMA  ";
gg = (lenbuf[42] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[43] != NULL){
coord="PERIOD  ";
gg = (lenbuf[43] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[44] != NULL){
coord="TWOPOINTS  ";
gg = (lenbuf[44] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[45] != NULL){
coord="SEMICOLON  ";
gg = (lenbuf[45] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[46] != NULL){
coord="PRIMARYSTRESS  ";
gg = (lenbuf[46] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[47] != NULL){
coord="SILENCE  ";
gg = (lenbuf[47] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[48] != NULL){
coord="(PP)  peo P le  ";
gg = (lenbuf[48] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[49] != NULL){
coord="(TT)  vas T  ";
gg = (lenbuf[49] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[50] != NULL){
coord="(LL)  a LL  ";
gg = (lenbuf[50] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[51] != NULL){
coord="(DD)  a DD  ";
gg = (lenbuf[51] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[52] != NULL){
coord="(GG)  ea G le  ";
gg = (lenbuf[52] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}
if(bufsound[53] != NULL){
coord="(VV)  o F  ";
gg = (lenbuf[53] / hh) * ff;  
timetoplay=(int)gg;
tempus.Format("%d",timetoplay);tempus.Append(" ms.");coord.Append(tempus);
cb.AddString(CString(coord));
}

//************************************************************************************

cb.SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void ShowPhonem::OnBnClickedButtonback()
{
double gg; double hh=88200,ff=1000;
	CString tempus; 
showphonem-=1;
if(showphonem < 1) showphonem=0;

cb.SetCurSel(showphonem);
delete [] total;
total= new CPoint[lenbuf[showphonem]/2];
rectM=CRect(0,0,0,0);


//The info for the scrollbar
infoscroll.nMax=lenbuf[showphonem]/2;
infoscroll.nPage=10;
infoscroll.nPos=0;
infoscroll.nTrackPos=0;
scroller.SetScrollInfo(&infoscroll,1);
// end info.

if(showphonem==0 && bufsound[0] != NULL){
coord="(AA)  f A ther  ";
gg = (lenbuf[0] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[0];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==1 && bufsound[1] != NULL){
coord="(AE)  c A t  ";
gg = (lenbuf[1] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[1];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==2 && bufsound[2] != NULL){
coord="(AH)  b U t  ";
gg = (lenbuf[2] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[2];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==3 && bufsound[3] != NULL){
coord="(AO)  d O g  ";
gg = (lenbuf[3] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[3];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==4 && bufsound[4] != NULL){
coord="(OO)  s O urce  ";
gg = (lenbuf[4] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[4];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==5 && bufsound[5] != NULL){
coord="(AW)  f OU l  ";
gg = (lenbuf[5] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[5];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==6 && bufsound[6] != NULL){
coord="(AX)  A go  ";
gg = (lenbuf[6] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[6];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==7 && bufsound[7] != NULL){
coord="(AY)  b I te  ";
gg = (lenbuf[7] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[7];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==8 && bufsound[8] != NULL){
coord="(B)  B ig  ";
gg = (lenbuf[8] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[8];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==9 && bufsound[9] != NULL){
coord="(CH)  CH in  ";
gg = (lenbuf[9] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[9];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==10 && bufsound[10] != NULL){
coord="(D)  D addy  ";
gg = (lenbuf[10] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[10];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==11 && bufsound[11] != NULL){
coord="(DH)  TH en  ";
gg = (lenbuf[11] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[11];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==12 && bufsound[12] != NULL){
coord="(EH)  p E t  ";
gg = (lenbuf[12] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[12];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==13 && bufsound[13] != NULL){
coord="(ER)  f UR  ";
gg = (lenbuf[13] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[13];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==14 && bufsound[14] != NULL){
coord="(EY)  A te  ";
gg = (lenbuf[14] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[14];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==15 && bufsound[15] != NULL){
coord="(F)  F inally  ";
gg = (lenbuf[15] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[15];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==16 && bufsound[16] != NULL){
coord="(G)  G ut  ";
gg = (lenbuf[16] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[16];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==17 && bufsound[17] != NULL){
coord="(H)  H ouse  ";
gg = (lenbuf[17] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[17];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==18 && bufsound[18] != NULL){
coord="(IH)  f I ll  ";
gg = (lenbuf[18] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[18];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==19 && bufsound[19] != NULL){
coord="(IY)  f EE ling  ";
gg = (lenbuf[19] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[19];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==20 && bufsound[20] != NULL){
coord="(JH)  J oy  ";
gg = (lenbuf[20] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[20];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==21 && bufsound[21] != NULL){
coord="(K)  Kitten  ";
gg = (lenbuf[21] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[21];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}

if(showphonem==22 && bufsound[22] != NULL){
coord="(L)  L ibertine  ";
gg = (lenbuf[22] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[22];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==23 && bufsound[23] != NULL){
coord="(M)  M arking ";
gg = (lenbuf[23] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[23];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==24 && bufsound[24] != NULL){
coord="(N)  N arrow  ";
gg = (lenbuf[24] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[24];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==25 && bufsound[25] != NULL){
coord="(NG)  barki NG  ";
gg = (lenbuf[25] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[25];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==26 && bufsound[26] != NULL){
coord="(OW)  c OA t  ";
gg = (lenbuf[26] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[26];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==27 && bufsound[27] != NULL){
coord="(OY)  enj OY ";
gg = (lenbuf[27] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[27];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==28 && bufsound[28] != NULL){
coord="(P)  P eace  ";
gg = (lenbuf[28] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[28];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==29 && bufsound[29] != NULL){
coord="(R)  f R ee  ";
gg = (lenbuf[29] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[29];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==30 && bufsound[30] != NULL){
coord="(S)  out S ide   ";
gg = (lenbuf[30] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[30];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==31 && bufsound[31] != NULL){
coord="(SH)  SH ame  ";
gg = (lenbuf[31] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[31];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==32 && bufsound[32] != NULL){
coord="(T)  T amed  ";
gg = (lenbuf[32] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[32];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==33 && bufsound[33] != NULL){
coord="(TH)  TH in  ";
gg = (lenbuf[33] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[33];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==34 && bufsound[34] != NULL){
coord="(UH)  l OO k ";
gg = (lenbuf[34] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[34];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==35 && bufsound[35] != NULL){
coord="(UW)  z OO  ";
gg = (lenbuf[35] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[35];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==36 && bufsound[36] != NULL){
coord="(V)  V ariety  ";
gg = (lenbuf[36] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[36];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==37 && bufsound[37] != NULL){
coord="(W)  W akeful  ";
gg = (lenbuf[37] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[37];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==38 && bufsound[38] != NULL){
coord="(Y)  Y elling  ";
gg = (lenbuf[38] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[38];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==39 && bufsound[39] != NULL){
coord="(Z)  Z ebras  ";
gg = (lenbuf[39] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[39];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==40 && bufsound[40] != NULL){
coord="(ZH)  plea S uring  ";
gg = (lenbuf[40] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[40];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==41 && bufsound[41] != NULL){
coord="UNKNOWN  ";
gg = (lenbuf[41] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[41];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==42 && bufsound[42] != NULL){
coord="COMMA  ";
gg = (lenbuf[42] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[42];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==43 && bufsound[43] != NULL){
coord="PERIOD  ";
gg = (lenbuf[43] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[43];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==44 && bufsound[44] != NULL){
coord="TWOPOINTS  ";
gg = (lenbuf[44] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[44];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==45 && bufsound[45] != NULL){
coord="SEMICOLON  ";
gg = (lenbuf[45] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[45];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==46 && bufsound[46] != NULL){
coord="PRIMARYSTRESS  ";
gg = (lenbuf[46] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[46];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==47 && bufsound[47] != NULL){
coord="SILENCE  ";
gg = (lenbuf[47] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[47];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==48 && bufsound[48] != NULL){
coord="(PP)  peo P le  ";
gg = (lenbuf[48] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[48];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==49 && bufsound[49] != NULL){
coord="(TT)  vas T  ";
gg = (lenbuf[49] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[49];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==50 && bufsound[50] != NULL){
coord="(LL)  a LL  ";
gg = (lenbuf[50] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[50];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==51 && bufsound[51] != NULL){
coord="(DD)  a DD  ";
gg = (lenbuf[51] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[51];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==52 && bufsound[52] != NULL){
coord="(GG)  ea G le  ";
gg = (lenbuf[52] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[52];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
if(showphonem==53 && bufsound[53] != NULL){
coord="(VV)  o F  ";
gg = (lenbuf[53] / hh) * ff;  
Duration=(int)gg;
LenPhon=lenbuf[53];
tempus.Format("%d",Duration);tempus.Append(" ms.");coord.Append(tempus);
UpdateData(FALSE);
}
flagzoom=false;
flagscroll=false;
Invalidate(true);

selstart=0;
gg = (LenPhon / hh) * ff;
selend=(int)gg;
gg = (LenPhon / hh) * ff;
SelPhon=(int)gg;

UpdateData(FALSE);
selstart=0;
selend=LenPhon;
SelPhon=LenPhon;


	// TODO: Add your control notification handler code here
	//OnOK();
}




void ShowPhonem::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
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

void ShowPhonem::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

void ShowPhonem::OnBnClickedButtonminus()
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

void ShowPhonem::OnBnClickedButtonplus()
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

void ShowPhonem::OnFileSavepersonalizedvoiceas()
{
	 static char BASED_CODE szFilter[] = "Modified Voice Files (*.vxa)|*.vxa|Only DMM Modified voice Files(*vxa*)|*vxa*||";
    CFileDialog fileDlg (FALSE, "vxa", "*.vxa",
    OFN_HIDEREADONLY, szFilter, this);
	if( fileDlg.DoModal ()==IDOK ){int len = 54;
	CFile Fi(fileDlg.GetPathName(),CFile::modeCreate|CFile::modeWrite);
	Fi.Write(&len, sizeof(int));
	Fi.Write(lenbuf,sizeof(int)*54);
		for(int i = 0 ; i < 54; ++i)
		{
		Fi.Write(bufsound[i],lenbuf[i]);
		}
	Fi.Close();
	
	}
}

BOOL ShowPhonem::PreCreateWindow(CREATESTRUCT& cs)
{

// TODO: Add your specialized code here and/or call the base class

	return CDialog::PreCreateWindow(cs);
}

void ShowPhonem::OnBnClickedButtonglobalgain()
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

void ShowPhonem::OnBnClickedButtongainminus()
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

void ShowPhonem::OnMButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnMButtonDown(nFlags, point);
}

void ShowPhonem::OnMouseMove(UINT nFlags, CPoint point)
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

void ShowPhonem::OnMButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnMButtonUp(nFlags, point);
}

void ShowPhonem::OnRButtonDown(UINT nFlags, CPoint point)
{
	CDialog::OnRButtonDown(nFlags, point);
}

void ShowPhonem::OnLButtonDown(UINT nFlags, CPoint point)
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

void ShowPhonem::OnLButtonUp(UINT nFlags, CPoint point)
{	// TODO: Add your message handler code here and/or call default
	isdown=false;
	CDialog::OnLButtonUp(nFlags, point);
}




void ShowPhonem::OnLbnSelchangeListp()
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

void ShowPhonem::OnBnClickedButton0left()
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



void ShowPhonem::OnBnClickedButtonzoomsel()
{
	if(rectM == CRect(0,0,0,0)) return;

infoscroll.nMax=lenbuf[showphonem]/2-abs(rectM.right-rectM.left);
scroller.SetScrollInfo(&infoscroll,1);
scroller.ShowWindow(true);
scroller.SetScrollPos(rectM.left);
	flagzoom=true;
	Invalidate();
}

void ShowPhonem::OnBnClickedButtonzoomext()
{scroller.ShowWindow(false);
flagzoom=false;
flagscroll=false;
rectM1=CRect(0,0,0,0);
	Invalidate();

	// TODO: Add your control notification handler code here
}

void ShowPhonem::OnWindowingBlackman()
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

void ShowPhonem::OnWindowingHamming()
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

void ShowPhonem::OnWindowingHanning()
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

void ShowPhonem::OnNMThemeChangedScrollphonem(NMHDR *pNMHDR, LRESULT *pResult)
{
	// This feature requires Windows XP or greater.
	// The symbol _WIN32_WINNT must be >= 0x0501.
	// TODO: Add your control notification handler code here
	*pResult = 0;
}

HBRUSH ShowPhonem::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

void ShowPhonem::OnBnClickedButtonconnect()
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

void ShowPhonem::OnBnClickedButtonfade()
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

void ShowPhonem::OnBnClickedButtonfadeb()
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

void ShowPhonem::OnBnClickedButtonshorten()
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

void ShowPhonem::OnFileCreateblankvoiceformodelling()
{
	// we create an emptu voice to model later
	for(int i = 0 ; i < 54; ++i)
		{

HeapFree(GetProcessHeap(), 0,bufsound[i]);
bufsound[i]= (byte*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,1000 );

lenbuf[i]=1000;

HeapFree(GetProcessHeap(), 0,bufsoundS[i]);
bufsoundS[i]= (byte*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,1000 );


		}
Invalidate();
	rectM=CRect(0,0,0,0);
int posphon=cb.GetCurSel();
OnInitDialog();
cb.SetCurSel(posphon);
OnLbnSelchangeListp();

}

void ShowPhonem::OnFileLoadphoneme()
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
HeapFree(GetProcessHeap(), 0,bufsoundS[showphonem]);
bufsoundS[showphonem]= (byte*)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,lenfile );
memcpy(bufsoundS[showphonem],bufsound[showphonem],lenfile);
// we have to amplyfy the stressed phoneme
short * buftemp;
buftemp= reinterpret_cast< short* >(bufsoundS[showphonem]); // from byte to short 16 bit with sign
for(int h=0; h< lenfile/2;++h)
{
buftemp[h]=(short)((float)buftemp[h]* 1.20);
}

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

void ShowPhonem::OnFileSavephoneme()
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

void ShowPhonem::OnBnClickedPlayphonem()
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
