#pragma once
#include "afx.h"
#include <afxtempl.h>

class dictionary :
	public CObject
{
DECLARE_SERIAL(dictionary);
public:

	dictionary(void);
	~dictionary(void);
	virtual void Serialize(CArchive& ar);

	void operator =(const dictionary& r)
	{
	written=r.written;
	pronounced=r.pronounced;	
	}
	

private:
CString written;
CString pronounced;

public:


	void SetWritten(CString wwritten)
	{written=wwritten;
	}

	

	void SetPronounced(CString wpronounced)
	{
		pronounced=wpronounced;
	}

	CString GetWritten(void)
	{
		return written;
	}

	CString GetPronounced(void)
	{
		return pronounced;
	}
};

