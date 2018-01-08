// this object was created to handle the split function
// the code was found in a French archive.

#pragma once


class CStringPLUS :	public CString
{
private:
CString alfa;
public:

CStringPLUS(void);
CStringPLUS(CString n){alfa=n;}
~CStringPLUS(void);
//	CStringPLUS &operator=(const CStringPLUS& string){alfa=string.alfa;return *this; }

CString *split(int &outNumber, char character) 
{
int lg=alfa.GetLength();
char * p = alfa.GetBuffer(lg);

    outNumber=0;
    int cptChar=0;
    int start=0;
    int *tabStart=new int[lg]; 
    int *tabLength=new int[lg]; 
    for(int i=0;i<=lg;++i) 
    {
        if(p[i]==character ||p[i]==0 ||p[i]== 45 )//||p[i]== 13  ) // we have removed 
        {
            tabStart[outNumber]=start;
            tabLength[outNumber]=cptChar;
           ++outNumber;
            cptChar=0;
            start=i+1;
        }    
        else
        {
            ++cptChar;
        }
    }
    CString *result=new CString[outNumber];
    for(int i=0; i<outNumber; ++i)
    {
		result[i]=alfa.Mid(tabStart[i], tabLength[i]);
    }
    delete[]tabStart;
    delete[]tabLength;
	alfa.ReleaseBuffer();
    return result;
}

};
