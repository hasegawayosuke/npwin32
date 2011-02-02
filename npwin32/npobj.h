#pragma once

#include	<windows.h>
#include	<atlstr.h>
#include	<atlcoll.h>
#include	<strsafe.h>
#include <npapi.h>
#include <npfunctions.h>
#include <npruntime.h>

class NPObj : public NPObject {
private:
	static void *p;
	static CAtlMap<NPObject*, NPObj*> _map;
	static bool _hasMethod( NPObject*, NPIdentifier );
	static bool _invoke( NPObject*, NPIdentifier, const NPVariant*, uint32_t, NPVariant*); 
public:
	NPObj( NPP );
	~NPObj();
	NPObject* getNPObject();
	NPP getNPP();
	static struct NPClass _npo_class;
	virtual bool hasMethod( LPCWSTR methodName );
	virtual bool invoke( LPCWSTR methodName, const NPVariant *args, uint32_t argCount, NPVariant *result);
protected:
	NPObject* _npobject;
	NPP _npp;
};


inline void LOG( LPCSTR lpszFormat, ... )
{
#ifdef DEBUG
	CHAR buf[ 4096 ];
	va_list ap;

	va_start( ap, lpszFormat );
	StringCchVPrintfA( buf, _countof( buf ), lpszFormat, ap );
	OutputDebugStringA( buf );
#endif
}

inline void LOG( LPCWSTR lpszFormat, ... )
{
#ifdef DEBUG
	WCHAR buf[ 4096 ];
	va_list ap;

	va_start( ap, lpszFormat );
	StringCchVPrintfW( buf, _countof( buf ), lpszFormat, ap );
	OutputDebugStringW( buf );
#endif
}

LPCSTR checkNpArgs( LPCSTR lpszCheck, const NPVariant *args, uint32_t argCount );
LPWSTR Npv2WStr( NPVariant v );
LPSTR Npv2Str( NPVariant v );
int Npv2Int( NPVariant v );

#ifdef DEBUG
#define	LOGF		LOG( "Enter %s (%d)", __FUNCTION__, __LINE__ )
#else
#define	LOGF
#endif
