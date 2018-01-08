#include "StdAfx.h"
#include ".\dictionary.h"



IMPLEMENT_SERIAL(dictionary,CObject,1)

dictionary::dictionary(void) // this is the right place to zero classe members
:written(_T(""))
,pronounced (_T(""))


{
}

dictionary::~dictionary(void)
{
}



void dictionary::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{	// storing code
ar << written << pronounced  ;

}
	
	else
	{	// loading code
ar >> written  >> pronounced ;
	}
}








