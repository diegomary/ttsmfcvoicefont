// DataAn.cpp : implementation file
//

#include "stdafx.h"
#include "TTS.h"
#include "DataAn.h"
#include "dictionary.h"
#include "stringplus.h"
// DataAn dialog
IMPLEMENT_DYNAMIC(DataAn, CDialog)
DataAn::DataAn(CWnd* pParent /*=NULL*/)
	: CDialog(DataAn::IDD, pParent)
	, wordDB(NULL)
	, DBinfo(_T(""))
	, MissWords(_T(""))
	, TotalWords(_T(""))
	, WordSingle(_T(""))
	, richinspect(NULL)
	, triphone(_T(""))
	, numtriph(_T(""))
	, resultf(_T(""))
{
m_allStaticBkBrush = new CBrush(RGB(242,242,178));
}

DataAn::~DataAn()
{
}





void DataAn::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATINFO, DBinfo);
	DDX_Text(pDX, IDC_STATICMISS, MissWords);
	DDX_Text(pDX, IDC_STATICTOTW, TotalWords);
	DDX_Text(pDX, IDC_EDITSINGLE, WordSingle);
	DDX_Control(pDX, IDC_MISSWORDS, listmiss);
	DDX_Control(pDX, IDC_EDITSINGLE, singlemiss);
	DDX_Text(pDX, IDC_EDIT2, triphone);
	DDX_Text(pDX, IDC_STATICNUMBERTRIP, numtriph);
	DDX_Control(pDX, IDC_EDIT2, testsearch);
	DDX_Control(pDX, IDC_LISTTRIPHONE, listtripho);
	DDX_Control(pDX, IDC_LISTFINDBITRI, cbfound);
	DDX_Text(pDX, IDC_QUERYFOUND, resultf);
}


BEGIN_MESSAGE_MAP(DataAn, CDialog)
	ON_BN_CLICKED(IDOK, &DataAn::OnBnClickedOk)
	ON_BN_CLICKED(IDC_INSPECT, &DataAn::OnBnClickedInspect)
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTONMISS, &DataAn::OnBnClickedButtonmiss)
	ON_LBN_SELCHANGE(IDC_MISSWORDS, &DataAn::OnLbnSelchangeMisswords)
	ON_STN_CLICKED(IDC_STATICMISS, &DataAn::OnStnClickedStaticmiss)
	ON_BN_CLICKED(IDC_FINDTRIPHO, &DataAn::OnBnClickedFindtripho)
	ON_BN_CLICKED(IDC_FINDALL, &DataAn::OnBnClickedFindall)
END_MESSAGE_MAP()


// DataAn message handlers

void DataAn::OnBnClickedOk()
{

	
	
	// TODO: Add your control notification handler code here
	//OnOK();
}

void DataAn::OnBnClickedInspect()
{
// first we prepare the list box
CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTINSP);
CListBox * pList1 =(CListBox*)GetDlgItem(IDC_NWORDS);
pList->ResetContent();
pList->AddString(CString("0") + CString("   numbers")); // we mark the beginning as numbers
CString temp;
int *  n; n= new int[28];
int location=0;bool flag=false;

// here we search for the range of numbers and we give it to the first item of the array.
//97 is the start point for the letter a. when we find it it means that the range from zero
// to location belongs to pure numbers.
for(int k= 0 ; k < 26; k++)
{
while(!flag && location < wordDB->GetCount())
{
if((*(dictionary*)(wordDB->GetAt(location))).GetWritten().Left(1).CompareNoCase(CString((char)(97+k)))==0)
flag=true;
else location++;
}

n[k]=location-1;flag=false;
temp.Format("%d",n[k]); temp = temp + "  " + char(97+k);
pList->AddString(temp);
}
// having cicled for all letter we have filled the array with the range values of any letter.
// the array contains n[0] - > n[1]  = range of a.   n[1] - > n[2] range of b and so on.
flag=false;
// now we have to consider the words that are marked with the ~ carachter at the beginning and 
// that are acronyms, abbreviations initials and other stuff.
while(!flag && location < wordDB->GetCount())
{
if((*(dictionary*)(wordDB->GetAt(location))).GetWritten().Left(1).CompareNoCase(CString('~'))==0)
flag=true;
else location++;
}
n[26]=location-1;flag=false;
temp.Format("%d",n[26]); temp = temp + "  " + CString('~'); //from n[26]to n[27] we have the range of
//acronyms, abbreviations initials and other stuff.

pList->AddString(temp);
n[27]=(int)(wordDB->GetCount()-1);
temp.Format("%d",n[27]);temp = temp + "   " + "END"; // here we have the end of database
pList->AddString(temp);

// now we calculate the word number for any initials
pList1->ResetContent();
temp.Format("%d",n[0]);
temp=temp + "  numbers";
pList1->AddString(temp);

// we must subtract the next from the previous to obtain the word number
// here we cycle trough carachters starting the word.
for(int j= 1; j< 27;++j)
{
temp.Format("%d",n[j]- n[j-1]);
pList1->AddString(temp + "   " + (char)(96 +j));
}
// and here we calculate the number of initials, and acronims
temp.Format("%d",n[27]- n[26]);
pList1->AddString(temp + "   " + CString('~'));
}

BOOL DataAn::OnInitDialog()
{
	CDialog::OnInitDialog();
CRect rect;
// Get the dimensions of the font sample display area
singlemiss.GetWindowRect(&rect);
        // Release the current font
        m_fSampFont.Detach();
        // Create the font to be used
        m_fSampFont.CreateFont((rect.Height()- 5), 0, 0, 0, FW_NORMAL,
                0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS,
                CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH |
                FF_DONTCARE, CString("Verdana"));

        // Set the font for the sample display area
      listmiss.SetFont(&m_fSampFont);
		singlemiss.SetFont(&m_fSampFont);
		// now we create another font a little smaller.
listtripho.SetFont(&m_fSampFont);
testsearch.SetFont(&m_fSampFont);
cbfound.SetFont(&m_fSampFont);


// first we prepare the list box
CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTINSP);
CListBox * pList1 =(CListBox*)GetDlgItem(IDC_NWORDS);
pList->ResetContent();
pList->AddString(CString("0") + CString("   numbers")); // we mark the beginning as numbers
CString temp;
int *  n; n= new int[28];
int location=0;bool flag=false;

// here we search for the range of numbers and we give it to the first item of the array.
//97 is the start point for the letter a. when we find it it means that the range from zero
// to location belongs to pure numbers.
for(int k= 0 ; k < 26; k++)
{
while(!flag && location < wordDB->GetCount())
{
if((*(dictionary*)(wordDB->GetAt(location))).GetWritten().Left(1).CompareNoCase(CString((char)(97+k)))==0)
flag=true;
else location++;
}

n[k]=location-1;flag=false;
temp.Format("%d",n[k]); temp = temp + "  " + char(97+k);
pList->AddString(temp);
}
// having cicled for all letter we have filled the array with the range values of any letter.
// the array contains n[0] - > n[1]  = range of a.   n[1] - > n[2] range of b and so on.
flag=false;
// now we have to consider the words that are marked with the ~ carachter at the beginning and 
// that are acronyms, abbreviations initials and other stuff.
while(!flag && location < wordDB->GetCount())
{
if((*(dictionary*)(wordDB->GetAt(location))).GetWritten().Left(1).CompareNoCase(CString('~'))==0)
flag=true;
else location++;
}
n[26]=location-1;flag=false;
temp.Format("%d",n[26]); temp = temp + "  " + CString('~'); //from n[26]to n[27] we have the range of
//acronyms, abbreviations initials and other stuff.

pList->AddString(temp);
n[27]=(int)(wordDB->GetCount()-1);
temp.Format("%d",n[27]);temp = temp + "   " + "END"; // here we have the end of database
pList->AddString(temp);

// now we calculate the word number for any initials
pList1->ResetContent();
temp.Format("%d",n[0]);
temp=temp + "  numbers";
pList1->AddString(temp);

// we must subtract the next from the previous to obtain the word number
// here we cycle trough carachters starting the word.
for(int j= 1; j< 27;++j)
{
temp.Format("%d",n[j]- n[j-1]);
pList1->AddString(temp + "   " + (char)(96 +j));
}
// and here we calculate the number of initials, and acronims
temp.Format("%d",n[27]- n[26]);
pList1->AddString(temp + "   " + CString('~'));


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void DataAn::OnPaint()
{
	CPaintDC dc(this); // device context for painting
CRect rect;
GetClientRect(&rect);
CBrush black(RGB(212,242,178));
CBrush old;
dc.SelectObject(black);
dc.Rectangle(&rect);
}

HBRUSH DataAn::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
 switch (nCtlColor) {
    

 case CTLCOLOR_STATIC : // remember that we have also created the CBrush object in the cdialog 
	 //constructor

			pDC->SetTextColor(RGB(67,69, 224));
			pDC->SetBkColor(RGB(242,242,178));
            return (HBRUSH)(m_allStaticBkBrush->GetSafeHandle());
		

          default:
              return hbr;
          }

}

void DataAn::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class
delete this;
	CDialog::PostNcDestroy();
}

void DataAn::OnBnClickedButtonmiss()
{
	int nWordsXRow; CString ret; int pos=0; CString confront;
	
	//extern CRichEditCtrl rich1;
CStringPLUS *test;
CString * vec;
CListBox * pList =(CListBox*)GetDlgItem(IDC_MISSWORDS); pList->ResetContent();
CString Document;


if (richinspect->GetSelectionType() & (SEL_TEXT | SEL_MULTICHAR)) 
Document = richinspect->GetSelText(); 
else richinspect->GetTextRange(0,richinspect->GetTextLength(),Document);

Document.Replace('\r',' ');
Document.Replace('(',' ');
Document.Replace(')',' ');
Document.Replace((char)34,' ');


// NORMALIZATION
Document.Trim();
int Lalfa; Lalfa= Document.GetLength();
for(int q=0; q< Lalfa;++q)
{
	if (Document.GetAt(q) == ' ')
	{
		for (int c=q+1; c< Lalfa; c++)
		{
if (Document.GetAt(c)== ' ')
{
	Document.Delete(c,1);
	Lalfa=Document.GetLength(); c--;
} else break;
		}
	}
}
// END NORMALIZATION
test=new CStringPLUS(Document);
vec = test->split(nWordsXRow,' ');
for(int y = 0; y< nWordsXRow; y++)
{
vec[y].Trim();
vec[y].TrimRight((_T(",;.:!?-'")));
vec[y].TrimRight((_T((char)34)));


ret=SearchWords(vec[y],wordDB,pos);

//if(pos == -1){confront= vec[y]; confront.TrimRight("s");ret=SearchWords(confront,wordDB,pos);}
//if(pos == -1){confront= vec[y];confront.TrimRight("S");ret=SearchWords(confront,wordDB,pos);}
//if(pos == -1){confront= vec[y];confront.TrimRight("er");ret=SearchWords(confront,wordDB,pos);}
//if(pos == -1){confront= vec[y];confront.TrimRight("ER");ret=SearchWords(confront,wordDB,pos);}
//if(pos == -1){confront= vec[y];confront.TrimRight("st");ret=SearchWords(confront,wordDB,pos);}
//if(pos == -1){confront= vec[y];confront.TrimRight("ST");ret=SearchWords(confront,wordDB,pos);}
//if(pos == -1){confront= vec[y];confront.TrimRight("ed");ret=SearchWords(confront,wordDB,pos);}
//if(pos == -1){confront= vec[y];confront.TrimRight("ED");ret=SearchWords(confront,wordDB,pos);}
//if(pos == -1){confront= vec[y];confront.TrimRight("ly");ret=SearchWords(confront,wordDB,pos);}
//if(pos == -1){confront= vec[y];confront.TrimRight("LY");ret=SearchWords(confront,wordDB,pos);}
//if(pos == -1){confront= vec[y];confront.TrimRight("ing");ret=SearchWords(confront,wordDB,pos);}
//if(pos == -1){confront= vec[y];confront.TrimRight("ING");ret=SearchWords(confront,wordDB,pos);}
//
//if(pos == -1){confront= vec[y]; 
//if(confront.Right(3).CompareNoCase("ing")==0)
//{
//confront=confront.Left(vec[y].GetLength()-3);
//confront.Append("e");
//ret=SearchWords(confront,wordDB,pos);
//}
//
//}
if(pos == -1){
vec[y].Trim();
if(vec[y].Compare(CString(""))!=0)
{
if(pList->FindString(-1,vec[y])==CB_ERR  ) pList->AddString(vec[y].MakeLower());
}


}


pos=0;
}
TotalWords.Format("%d",nWordsXRow);
TotalWords.Append("   Total number of words in the document");    
MissWords.Format("%d",pList->GetCount());
MissWords.Append("   Words are missing in the database");
UpdateData(FALSE);

}

void DataAn::OnLbnSelchangeMisswords()
{
CListBox * pList =(CListBox*)GetDlgItem(IDC_MISSWORDS);
static char pointerstr[_MAX_PATH];
//int n = pList->GetTextLen(pList->GetCurSel());
pList->GetText( pList->GetCurSel(), pointerstr );
WordSingle= pointerstr;
UpdateData(FALSE);
}

void DataAn::OnStnClickedStaticmiss()
{
	// TODO: Add your control notification handler code here
}

void DataAn::OnBnClickedFindtripho()
{
	UpdateData(TRUE);
CListBox * pList =(CListBox*)GetDlgItem(IDC_LISTTRIPHONE);
pList->ResetContent();
CString l;


int up = (int)(wordDB->GetSize()-1);
for(int n= 0;n <= up ;n++)
{
l=(*((dictionary*)wordDB->GetAt(n))).GetPronounced();
if(l.Find(triphone.MakeLower(),0)>-1)
pList->AddString((*((dictionary*)wordDB->GetAt(n))).GetWritten() + " : " + l );
}

numtriph.Format("%d %s   %s    %s" ,pList->GetCount(), "Triphones", triphone,"In Database");
UpdateData(FALSE);
// TODO: Add your control notification handler code here
}

void DataAn::OnBnClickedFindall()
{
	cbfound.ResetContent();
CString l;
CListBox Templist;
Templist.Create(NULL,CRect(0,0,0,0),this,0);
CString ltemp;
int temp;

int up = (int)(wordDB->GetSize()-1);
for(int n= 0;n <= up ;n++)
{
Templist.ResetContent();
ltemp=(*((dictionary*)wordDB->GetAt(n))).GetPronounced();
if(ltemp.Replace(" "," ")==2 || ltemp.Replace(" "," ")==1) 
{

for(int nn= 0;nn <= up ;nn++)
{
l=(*((dictionary*)wordDB->GetAt(nn))).GetPronounced();
if(l.Find(ltemp,0)>-1)
Templist.AddString("1");
}
l.Format("%s: %d"," <-Oc-> ",Templist.GetCount());
cbfound.AddString((*((dictionary*)wordDB->GetAt(n))).GetPronounced().MakeUpper() + "   :" +(*((dictionary*)wordDB->GetAt(n))).GetWritten() + l);
}


}







resultf.Format("%s %d","Found:",cbfound.GetCount());
UpdateData(FALSE);

// TODO: Add your control notification handler code here
}
