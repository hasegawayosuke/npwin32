#pragma once

#include    <windows.h>
#include    <atlstr.h>
#include    <atlcoll.h>
#include    <strsafe.h>
#include <npapi.h>
#include <npfunctions.h>
#include <npruntime.h>

class NPObj : public NPObject {
private:
    static void *p;
    static CAtlMap<NPObject*, NPObj*> _map;
    // static NPObject* _allocate( NPP, NPClass * );
    static void _deallocate( NPObject * );
    static bool _hasMethod( NPObject*, NPIdentifier );
    static bool _invoke( NPObject*, NPIdentifier, const NPVariant*, uint32_t, NPVariant* ); 
public:
    NPObj( NPP, bool );
    virtual ~NPObj();
    NPObject* getNPObject();
    NPP getNPP();
    static struct NPClass _npo_class;
    virtual bool hasMethod( LPCWSTR methodName );
    virtual bool invoke( LPCWSTR methodName, const NPVariant *args, uint32_t argCount, NPVariant *result);
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
        LOG( "*Enter to %s (%d)", p, line );
    };
    ~_DEBUG_FUNC_INFO()
    {
        LOG( "*Leave from %s", p );
    };
};

//#d efine LOGF        LOG( "Enter %s (%d)", __FUNCTION__, __LINE__ )
#define LOGF    _DEBUG_FUNC_INFO    _debug_func_info( __FUNCTION__, __LINE__ )
#else
#define LOGF
#endif

LPCSTR checkNpArgs( LPCSTR lpszCheck, const NPVariant *args, uint32_t argCount );
LPWSTR Npv2WStr( NPVariant v );
LPSTR Npv2Str( NPVariant v );
int Npv2Int( NPVariant v );

NPUTF8* allocUtf8( LPCSTR s );
NPUTF8* allocUtf8( LPCWSTR s );
