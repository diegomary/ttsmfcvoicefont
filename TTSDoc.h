// TTSDoc.h : interface of the CTTSDoc class
//


#pragma once
#include "dictionary.h"

class CTTSDoc : public CDocument
{
protected: // create from serialization only
	CTTSDoc();
	DECLARE_DYNCREATE(CTTSDoc)

// Attributes
public:
	

// Operations
public:
CObArray Total;
int curposition;
private:


// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	dictionary* AddNewRecord(void);
// Implementation
public:
	virtual ~CTTSDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()

	// Generated OLE dispatch map functions

	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
private:
	
public:

	int GetTotalRecords(void)
	{
		return (int)(Total.GetSize());
	}

	int GetCurRecordNbr(void)
	{
		return curposition +1 ;
	}
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	dictionary* GetCurRecord(void);
	dictionary* GetFirstRecord(void);
	dictionary* GetNextRecord(void);
	dictionary* GetPrevRecord(void);
	dictionary* GetLastRecord(void);
	virtual void DeleteContents();
	void DeleteCurrentRecord(void);
	dictionary * GetRecNumber(int numrec);
};


