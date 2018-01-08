#pragma once
// DataAn dialog
#include "dictionary.h"
#include "afxwin.h"
class DataAn : public CDialog
{
DECLARE_DYNAMIC(DataAn)
public:
	DataAn(CWnd* pParent = NULL);   // standard constructor
	virtual ~DataAn();
// Dialog Data
	enum { IDD = IDD_DATAAN };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
public:
	CObArray *wordDB;
public:
	afx_msg void OnBnClickedOk();
public:
	afx_msg void OnBnClickedInspect();
public:
	virtual BOOL OnInitDialog();
public:
	CString DBinfo;
public:
	afx_msg void OnPaint();
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
private:
CBrush * m_allStaticBkBrush;// all static back color
CFont m_fSampFont;	
protected:
	virtual void PostNcDestroy();
public:
	CString MissWords;
public:
	CString TotalWords;
public:
	CString WordSingle;
public:
	afx_msg void OnBnClickedButtonmiss();
public:

	CString SearchWords(CString tofind, CObArray * words , int & ps)
	{
			// this function return a string and a reference to the position where the string was found.
int left =0; int right= (int)(words->GetSize()-1);int location;int pos=-1;
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

	









public:
	afx_msg void OnLbnSelchangeMisswords();
public:
	afx_msg void OnStnClickedStaticmiss();
public:
	CRichEditCtrl *richinspect;
	CListBox listmiss;
	CEdit singlemiss;
public:
	CString triphone;
public:
	afx_msg void OnBnClickedFindtripho();
	CString numtriph;
public:
	CEdit testsearch;
public:
	CListBox listtripho;
public:
	CListBox cbfound;
public:
	afx_msg void OnBnClickedFindall();
	CString resultf;
};
