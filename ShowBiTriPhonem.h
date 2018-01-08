#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "atltypes.h"


// ShowPhonem dialog

class ShowBiTriPhonem : public CDialog
{
	DECLARE_DYNAMIC(ShowBiTriPhonem)

public:
	ShowBiTriPhonem(CWnd* pParent = NULL);   // standard constructor
	virtual ~ShowBiTriPhonem();
CSize m_szSize;
// Dialog Data
	enum { IDD = IDD_SHOWBITRIPHONEM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	byte **bufsound;
	byte **bufsoundS;
protected:
	virtual void PostNcDestroy();
public:
	afx_msg void OnPaint();
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
public:
	int *lenbuf;
public:
	int showphonem;

public:
	CString coord;
	
public:
	virtual BOOL OnInitDialog();
public:
	CEdit showphon;
private:
CFont m_fSampFont,m_fSampFont1;	


int scrollpos;
CBrush * m_brushback;// for the background color




private:
	
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonminus();
	afx_msg void OnBnClickedButtonplus();
	/*afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButtonhann();*/
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
public:
	afx_msg void OnBnClickedButtonglobalgain();
public:
	afx_msg void OnBnClickedButtongainminus();
public:
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
private:
	int newsearch;

bool flagscroll;
	bool isdown;
	CPoint ptprev,ptpost;CPoint *total;
	CRect rectM,rectM1;
	bool flagzoom;
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
public:
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
public:
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
public:
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
public:
	
public:

public:
	CListBox cb;
public:
	afx_msg void OnBnClickedCancel();
public:
	afx_msg void OnLbnSelchangeListp();
public:
	afx_msg void OnBnClickedButton0left();
public:
	afx_msg void OnBnClickedButtonzoomsel();
public:
	afx_msg void OnBnClickedButtonzoomext();
public:
	afx_msg void OnWindowingBlackman();
public:
	afx_msg void OnWindowingHamming();
public:
	afx_msg void OnWindowingHanning();
	CScrollBar scroller;
	SCROLLINFO infoscroll;
public:
	afx_msg void OnNMThemeChangedScrollphonem(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:
	afx_msg void OnBnClickedButtonconnect();

public:
	afx_msg void OnBnClickedButtonfade();
public:
	afx_msg void OnBnClickedButtonfadeb();
public:
	int SelPhon;
	int Duration;
	int phonemnumber;
	int LenPhon;
	int selstart;
public:
	
	int selend;
public:
	afx_msg void OnBnClickedButtonshorten();
public:
	afx_msg void OnFileSavepersonalizedvoiceas();
public:
	afx_msg void OnFileCreateblankvoiceformodelling();
public:
	afx_msg void OnFileLoadphoneme();
public:
	afx_msg void OnFileSavephoneme();
public:
	afx_msg void OnBnClickedPlaybitri();
	CString findit;
public:
	afx_msg void OnBnClickedFindtribi();
public:
	afx_msg void OnBnClickedResetsearch();
	int numfound;
};
