#pragma once


// ShowBiTriPhonem dialog

class ShowBiTriPhonem : public CDialog
{
	DECLARE_DYNAMIC(ShowBiTriPhonem)

public:
	ShowBiTriPhonem(CWnd* pParent = NULL);   // standard constructor
	virtual ~ShowBiTriPhonem();

// Dialog Data
	enum { IDD = IDD_SHOWBITRIPHONEM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
