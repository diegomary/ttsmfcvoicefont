

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0622 */
/* at Tue Jan 19 04:14:07 2038
 */
/* Compiler settings for TTS.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.01.0622 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __TTS_h_h__
#define __TTS_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ITTS_FWD_DEFINED__
#define __ITTS_FWD_DEFINED__
typedef interface ITTS ITTS;

#endif 	/* __ITTS_FWD_DEFINED__ */


#ifndef __CTTSDoc_FWD_DEFINED__
#define __CTTSDoc_FWD_DEFINED__

#ifdef __cplusplus
typedef class CTTSDoc CTTSDoc;
#else
typedef struct CTTSDoc CTTSDoc;
#endif /* __cplusplus */

#endif 	/* __CTTSDoc_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __TTS_LIBRARY_DEFINED__
#define __TTS_LIBRARY_DEFINED__

/* library TTS */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_TTS;

#ifndef __ITTS_DISPINTERFACE_DEFINED__
#define __ITTS_DISPINTERFACE_DEFINED__

/* dispinterface ITTS */
/* [uuid] */ 


EXTERN_C const IID DIID_ITTS;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("12A81D1F-2DD0-4B39-9ED6-04435A687ADB")
    ITTS : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct ITTSVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITTS * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITTS * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITTS * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITTS * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITTS * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITTS * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITTS * This,
            /* [annotation][in] */ 
            _In_  DISPID dispIdMember,
            /* [annotation][in] */ 
            _In_  REFIID riid,
            /* [annotation][in] */ 
            _In_  LCID lcid,
            /* [annotation][in] */ 
            _In_  WORD wFlags,
            /* [annotation][out][in] */ 
            _In_  DISPPARAMS *pDispParams,
            /* [annotation][out] */ 
            _Out_opt_  VARIANT *pVarResult,
            /* [annotation][out] */ 
            _Out_opt_  EXCEPINFO *pExcepInfo,
            /* [annotation][out] */ 
            _Out_opt_  UINT *puArgErr);
        
        END_INTERFACE
    } ITTSVtbl;

    interface ITTS
    {
        CONST_VTBL struct ITTSVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITTS_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITTS_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITTS_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITTS_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITTS_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITTS_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITTS_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __ITTS_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CTTSDoc;

#ifdef __cplusplus

class DECLSPEC_UUID("45747DEE-03D5-42F3-A860-AA19A0887724")
CTTSDoc;
#endif
#endif /* __TTS_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


