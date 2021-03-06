#pragma once

#include    <windows.h>
#include    <atlstr.h>
#include    <atlcoll.h>
#include    <strsafe.h>
#include <npapi.h>
#include <npfunctions.h>
#include <npruntime.h>

class NPObj;

class StringKeyEqualHelper {
public:
    static bool IsEqualKey( const LPCWSTR k1, const LPCWSTR k2 )
    {
        return lstrcmpW( k1, k2 ) == 0 ? true : false;
    }
    static bool IsEqualKey( const LPCSTR k1, const LPCSTR k2 )
    {
        return lstrcmpA( k1, k2 ) == 0 ? true : false;
    }
};

class NPObj : public NPObject {
private:
    static void *p;
    static CAtlMap<NPObject*, NPObj*> _map;
    // static NPObject* _allocate( NPP, NPClass * );
    static void _deallocate( NPObject * );
    static bool _hasMethod( NPObject*, NPIdentifier );
    static bool _invoke( NPObject*, NPIdentifier, const NPVariant*, uint32_t, NPVariant* ); 
    static bool _invokeDefault( NPObject*, const NPVariant*, uint32_t, NPVariant* ); 
    static bool _hasProperty( NPObject*, NPIdentifier );
    static bool _getProperty( NPObject*, NPIdentifier, NPVariant* );
public:
    NPObj( NPP, bool );
    virtual ~NPObj();
    NPObject* getNPObject();
    NPP getNPP();
    static struct NPClass _npo_class;
    static NPObj* lookup( NPObject * );
    virtual bool hasMethod( LPCWSTR methodName );
    virtual bool invoke( LPCWSTR methodName, const NPVariant *args, uint32_t argCount, NPVariant *result);
    virtual bool invokeDefault( const NPVariant *args, uint32_t argCount, NPVariant *result);
    virtual bool hasProperty( LPCWSTR propName );
    virtual bool getProperty( LPCWSTR propName, NPVariant *result);
    virtual bool toString( NPVariant *result );
protected:
    NPObject* _npobject;
    NPP _npp;
};


#pragma warning( push )
#pragma warning( disable : 4100 )
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
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4100 )
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
#pragma warning( pop )

#ifdef DEBUG
class _DEBUG_FUNC_INFO{
private:
    const char *p;
public:
    _DEBUG_FUNC_INFO( const char*funcname, int line )
    {
        p = funcname;
        LOG( "*Enter to %s (line:%d pid:%d tid:%d)", p, line, GetCurrentProcessId(), GetCurrentThreadId() );
    };
    ~_DEBUG_FUNC_INFO()
    {
        LOG( "*Leave from %s", p );
    };
};

//#d efine LOGF        LOG( "Enter %s (pid=%d tid=%d line=%d)", __FUNCTION__, GetCurrentProcessId(), GetCurrentThreadId(), __LINE__ )
#define LOGF    _DEBUG_FUNC_INFO    _debug_func_info( __FUNCTION__, __LINE__ )
#else
#define LOGF
#endif

LPCSTR checkNpArgs( LPCSTR lpszCheck, const NPVariant *args, uint32_t argCount );
LPWSTR Npv2WStr( NPVariant v );
LPSTR Npv2Str( NPVariant v );
int Npv2Int( NPVariant v );
BOOL Npv2Bool( NPVariant v );

NPUTF8* allocUtf8( LPCSTR s );
NPUTF8* allocUtf8( LPCWSTR s );
