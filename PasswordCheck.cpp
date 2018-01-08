// PasswordCheck.cpp : file di implementazione
//

#include "stdafx.h"
#include "TTS.h"
#include "PasswordCheck.h"


// finestra di dialogo PasswordCheck

IMPLEMENT_DYNAMIC(PasswordCheck, CDialog)

PasswordCheck::PasswordCheck(CWnd* pParent /*=NULL*/)
	: CDialog(PasswordCheck::IDD, pParent)
	, m_password(_T(""))
	, m_timeleft(0)
{

}

PasswordCheck::~PasswordCheck()
{
}

void PasswordCheck::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PASSWORDCHECK, m_password);
	DDV_MaxChars(pDX, m_password, 20);
	DDX_Text(pDX, IDC_TIMELEFT, m_timeleft);
}


BEGIN_MESSAGE_MAP(PasswordCheck, CDialog)
	ON_BN_CLICKED(IDOK, &PasswordCheck::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &PasswordCheck::OnBnClickedCancel)
END_MESSAGE_MAP()


// gestori di messaggi PasswordCheck

void PasswordCheck::OnBnClickedOk()
{
	UpdateData(TRUE);
	//if(m_password =="diegus" && m_timeleft > 0) {OnOK();} else {exit(0);}
	OnOK();
}

void PasswordCheck::OnBnClickedCancel()
{
	exit(0);
	// TODO: aggiungere qui il codice per la gestione della notifica del controllo.
	//OnCancel();
}

BOOL PasswordCheck::OnInitDialog()
{
	CDialog::OnInitDialog();

COleDateTime alfa=COleDateTime::GetCurrentTime();
COleDateTime future;
future.SetDateTime(2007,12,30,5,10,12);
COleDateTimeSpan elapsed = future-alfa;
m_timeleft = (int)(elapsed.GetDays());
UpdateData(FALSE);




	// TODO:  Add extra initialization here

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
