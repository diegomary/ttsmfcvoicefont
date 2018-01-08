// TTSDoc.cpp : implementation of the CTTSDoc class
//

#include "stdafx.h"
#include "TTS.h"

#include "TTSDoc.h"

#include "TTSView.h"
#include ".\ttsdoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



// CTTSDoc

IMPLEMENT_DYNCREATE(CTTSDoc, CDocument)

BEGIN_MESSAGE_MAP(CTTSDoc, CDocument)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CTTSDoc, CDocument)
END_DISPATCH_MAP()

// Note: we add support for IID_ITTS to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .IDL file.

// {12A81D1F-2DD0-4B39-9ED6-04435A687ADB}
static const IID IID_ITTS =
{ 0x12A81D1F, 0x2DD0, 0x4B39, { 0x9E, 0xD6, 0x4, 0x43, 0x5A, 0x68, 0x7A, 0xDB } };

BEGIN_INTERFACE_MAP(CTTSDoc, CDocument)
	INTERFACE_PART(CTTSDoc, IID_ITTS, Dispatch)
END_INTERFACE_MAP()


// CTTSDoc construction/destruction

CTTSDoc::CTTSDoc()
{
Total.SetSize(30000); // we set the prevision to 30000 words
	EnableAutomation();
	AfxOleLockApp();
}

CTTSDoc::~CTTSDoc()
{
	AfxOleUnlockApp();
}

BOOL CTTSDoc::OnNewDocument()
{if (!CDocument::OnNewDocument())
return FALSE;
if(!AddNewRecord())
return FALSE;
POSITION pos = GetFirstViewPosition();
CTTSView* pView = (CTTSView*)GetNextView(pos);
if (pView) pView->NewDataset();


return TRUE;
}




// CTTSDoc serialization

void CTTSDoc::Serialize(CArchive& ar)
{
// this fragment sorts the CObarray;
int k,i,j,number;
dictionary  *h;
h= new dictionary;
number=(int)Total.GetCount();
  k=(int)(Total.GetCount()/2);
  while (k>0)
  {for (i=0; i<number-k; ++i)
	 { j=i;
	 while (j>=0 &&  ((dictionary*)Total.GetAt(j))->GetWritten()>((dictionary*)Total.GetAt(j+k))->GetWritten())
	 {  
	 *h =*((dictionary*)Total.GetAt(j));
	*((dictionary*)(Total.GetAt(j)))=*((dictionary*)(Total.GetAt(j+k)));
	*((dictionary*)Total.GetAt(j+k))=*h;
	j=j-k;    
	 }
	 }
	 k=k/2;
  }
 // this is the end of the sorting process.
  // now we can serialize the archive and we know for sure that every member
  // is sorted


	Total.Serialize(ar);

}


// CTTSDoc diagnostics

#ifdef _DEBUG
void CTTSDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTTSDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CTTSDoc commands

dictionary* CTTSDoc::AddNewRecord(void)
{
dictionary *newdic = NULL;
newdic = new dictionary();
Total.Add(newdic);
SetModifiedFlag();
curposition= (int)((Total.GetSize()-1));
return newdic;
}

BOOL CTTSDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;


POSITION pos = GetFirstViewPosition();
CTTSView* pView = (CTTSView*)GetNextView(pos);
if (pView) pView->NewDataset();


	// TODO:  Aggiungere qui il codice di creazione specifico.

	return TRUE;
}

dictionary* CTTSDoc::GetCurRecord(void)
{if(curposition >=0)
return(dictionary*)Total.GetAt(curposition);
else
	return NULL;
}

dictionary* CTTSDoc::GetFirstRecord(void)
{
	if(Total.GetSize()>0)
	{curposition=0;
	return (dictionary*)Total.GetAt(0);
	}
	else
	return NULL;
}

dictionary* CTTSDoc::GetNextRecord(void)
{
	if(++curposition < Total.GetSize())
		return(dictionary*)Total.GetAt(curposition);
	else
		return AddNewRecord();

}

dictionary* CTTSDoc::GetPrevRecord(void)
{
if(Total.GetSize() > 0)
{
	if(--curposition < 0) curposition=0;

		return (dictionary*)Total.GetAt(curposition);
}
else
	return NULL;
}

dictionary* CTTSDoc::GetLastRecord(void)
{
if(Total.GetSize() > 0)
{curposition=(int)Total.GetUpperBound();
return(dictionary*)Total.GetAt(curposition);
}
else
	return NULL;
}

void CTTSDoc::DeleteContents()
{
	// TODO: aggiungere qui il codice specifico e/o chiamare la classe base
int iCount = (int)Total.GetSize();
int iPos;
for(iPos = 0; iPos < iCount; ++iPos)
delete(dictionary*) Total.GetAt(iPos);
Total.RemoveAll();


	CDocument::DeleteContents();
}

void CTTSDoc::DeleteCurrentRecord()
{
	dictionary* toDelete;
	int n = GetCurRecordNbr()-1;
	if( n > -1) {
	toDelete = (dictionary*)Total.GetAt(n);
	Total.RemoveAt(n);
	delete toDelete;}
// after deletion we must resort the array
// this fragment sorts the CObarray;
int k,i,j,number;
dictionary  *h;
h= new dictionary;
number=(int)Total.GetCount();
  k=(int)(Total.GetCount()/2);
  while (k>0)
  {for (i=0; i<number-k; ++i)
	 { j=i;
	 while (j>=0 &&  ((dictionary*)Total.GetAt(j))->GetWritten()>((dictionary*)Total.GetAt(j+k))->GetWritten())
	 {  
	 *h =*((dictionary*)Total.GetAt(j));
	*((dictionary*)(Total.GetAt(j)))=*((dictionary*)(Total.GetAt(j+k)));
	*((dictionary*)Total.GetAt(j+k))=*h;
	j=j-k;    
	 }
	 }
	 k=k/2;
  }




}

dictionary * CTTSDoc::GetRecNumber(int numrec)
{

if(Total.GetSize()>0)
	{curposition=numrec;
	return (dictionary*)Total.GetAt(numrec);
	}
	else
	return NULL;


	
}
