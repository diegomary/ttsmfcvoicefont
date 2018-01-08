#pragma once


// finestra di dialogo PasswordCheck

class PasswordCheck : public CDialog
{
	DECLARE_DYNAMIC(PasswordCheck)

public:
	PasswordCheck(CWnd* pParent = NULL);   // costruttore standard
	virtual ~PasswordCheck();

// Dati della finestra di dialogo
	enum { IDD = IDD_PASSWORD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV

	DECLARE_MESSAGE_MAP()
public:
	CString m_password;
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
private:
	int m_timeleft;
public:
	virtual BOOL OnInitDialog();
};
