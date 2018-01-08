// TTSView.h : interface of the CTTSView class
#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "DataAn.h"
#include "showphonem.h"
#include "ShowBiTriPhonem.h"
#include <afxdao.h> // Needed for MFC DAO classes.


class CTTSView : public CFormView
{
protected: // create from serialization only
	CTTSView();
	DECLARE_DYNCREATE(CTTSView)

public:
	enum{ IDD = IDD_TTS_FORM };

// Attributes
public:
	CTTSDoc* GetDocument() const;

// Operations
public:

// Overrides
	public:
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void OnInitialUpdate(); // called first time after construct

// Implementation
public:
	virtual ~CTTSView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedAddword();
	afx_msg void OnBnClickedRetrieve();
	CString m_wr;
	CString m_wp;
	CRect rect;
	CFont m_fSampFont;	
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
	void NewDataset(void);
	void PopulateView(void);
	CString m_position;
private:
	dictionary* m_curdictionary;
public:
	afx_msg void OnBnClickedFirst();
	afx_msg void OnBnClickedPrevious();
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedLast();
	afx_msg void OnEnChangeWritten();
	afx_msg void OnEnChangePron();
	CString findword;
	afx_msg void OnBnClickedFind();
	afx_msg void OnEnKillfocusWritten();
	afx_msg void OnLbnSelchangeListword();
	void ChangeListAspect(void);
	afx_msg void OnBnClickedPlaywave();
	afx_msg void OnBnClickedBtnspeak();	
	afx_msg void OnFontsTextstyle();
	afx_msg void OnRecordFirst32774();
	afx_msg void OnRecordLast32775();
	afx_msg void OnRecordNext32783();
	afx_msg void OnRecordPrevious();
	CSliderCtrl* PauseBetweenWords;

private:
CHARFORMAT2 cf;	// to prepare the richedit object in oninitialupdate
CBrush * m_pListBkBrush; // to work with control colors.,(the listword control)
CBrush * m_pEditWBkBrush;// the written word edit box
CBrush * m_pEditPBkBrush;// the Pronounced word edit box
CBrush * m_pEditFBkBrush;// the Find word edit box
CBrush * m_allStaticBkBrush;// all buttons back color
CBrush * m_sliderBkBrush;
CBrush * m_speedslBkBrush;
CMenu  m_NewMenu;
CMenu m_FileMenu;
CPoint mousepoint;
bool modevoice;
int lentripho;

bool flag; //this flag is used to launch only once the richedit initialization in
// the cview::oninitialupdate function.
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedCopyclip();
	afx_msg void OnBnClickedPasteclip();
	void SetAA(void);
	void SetAE(void);
	void SetAH(void);
	void SetAO(void);
	void SetOO(void);
	void SetAW(void);
	void SetAX(void);
	void SetB(void);
	void SetCH(void);
	void SetD(void);
	void SetDH(void);
	void SetEH(void);
	void SetER(void);
	void SetEY(void);
	void SetF(void);
	void SetG(void);
	void SetH(void);
	void SetIH(void);
	void SetIY(void);
	void SetJH(void);
	void SetK(void);
	void SetL(void);
	void SetM(void);
	void SetN(void);
	void SetNG(void);
	void SetOW(void);
	void SetOY(void);
	void SetP(void);
	void SetR(void);
	void SetS(void);
	void SetSH(void);
	void SetT(void);
	void SetTH(void);
	void SetUH(void);
	void SetUW(void);
	void SetV(void);
	void SetW(void);
	void SetY(void);
	void SetZ(void);
	void SetZH(void);
	void SetAY(void);
public:
	// CopyClip
	void CopyClip(void);
public:
	void PasteClip(void);

	CSliderCtrl *SliderVoicePitch;
//CSliderCtrl *PauseBetweenWords;


	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnFileSavetexttowavefile();
	void SetSTRESS(void);
	afx_msg void OnBnClickedStopspeak();
	void CreateFileForWordResource(void);
	afx_msg void OnBnClickedCleartext();
public:
	afx_msg void OnBnClickedButton4();
protected:
	virtual void PostNcDestroy();
public:
	void SetPP(void);
	afx_msg void OnBnClickedSpeaksel();
	afx_msg void OnBnClickedPause();
	afx_msg void OnBnClickedResume();
	afx_msg void OnBnClickedOpentextfile();
	afx_msg void OnPaint();
	CEdit Cwritten;
	CEdit Cpronounced;
	CEdit Cfind;
	CStatic Cstat1;
	CStatic Cstat2;
	CStatic CStat3;
	CListBox Clistwords;
	CStatic Stat4;
	CStatic Stat5;
	afx_msg void OnRecordDeletecurrent();
	afx_msg void OnBnClickedLoadvoice();
	CStatic StatVoice;
	CString VoiceType;
	afx_msg void OnFileSaveseltexttowavefile();
	afx_msg void OnBnClickedSavetextfile();
	
public:
	afx_msg void OnDataDataindexing();
public:
	DataAn *Inspect;
	ShowPhonem * viewer;
ShowBiTriPhonem * bitriviewer;
public:
	void SetTT(void);
public:
	void SetRF(void);
public:
	void SetLL(void);
public:
	void SetDD(void);
public:
	void SetGG(void);
public:
	CStatic CStatP;
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
public:
	afx_msg void OnBnClickedButtonresetspeed();
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	void SetVV(void);
public:
	CRichEditCtrl richtemp;
public:
	afx_msg void OnEnMsgfilterRichedittext(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
public:
	afx_msg void OnEditUndo();
public:
	afx_msg void OnDataPhonemsvisualizer();
public:
	afx_msg void OnNMCustomdrawSliderpitch(NMHDR *pNMHDR, LRESULT *pResult);
public:
	afx_msg void OnVoiceLoadoriginalvoicemodule();
public:
	afx_msg void OnVoiceLoadusermodifiedvoicemodule();
public:
	int pause;
public:
	CProgressCtrl progresssave;
public:
	CSliderCtrl sliderpause;
public:
	afx_msg void OnFileSaveAsText();
public:
	afx_msg void OnFileExportdatabasetoanaccesfile();
public:
	afx_msg void OnFileExportwordstoadbefile();
public:
	afx_msg void OnFileImportwordsfromadbefile();
public:
	afx_msg void OnFileImportwordsfromadbefileforfullarchive();
public:
	afx_msg void OnEnChangeEditfind();
public:
	afx_msg void OnVoiceLoadtriphonesknowledge();
public:
	afx_msg void OnDataBitriphonesvisualizer();
public:
	afx_msg void OnFileImportwordsfromanaccessfile();
public:
	afx_msg void OnFileImportfromaccessfile();
	afx_msg void OnNMCustomdrawSliderpause(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnStnClickedSposition();
};

#ifndef _DEBUG  // debug version in TTSView.cpp
inline CTTSDoc* CTTSView::GetDocument() const
   { return reinterpret_cast<CTTSDoc*>(m_pDocument); }
#endif

