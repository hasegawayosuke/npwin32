#include    "npobj.h"

#define MIME_TYPES_DESCRIPTION      "application/x-win32api-dynamic-call"

#ifdef __cplusplus
extern "C" {
#endif
NPNetscapeFuncs *npnfuncs;
#ifdef __cplusplus
}
#endif

class dllfunc;

class MyVariant {
private:
    typedef enum MY_VARIANT_TYPE{
        MVT_EMPTY,
        MVT_INT32,  // INT
        MVT_ASTR,   // LPSTR
        MVT_WSTR,   // LPWSTR
        MVT_BOOL,   // BOOL
    };
    MY_VARIANT_TYPE _vt;
    union{
        int _vInt;
        LPSTR _vAstr;
        LPWSTR _vWstr;
    };
public:
    void Clear( void )
    {
        if( _vt == MVT_ASTR ) delete _vAstr; else
        if( _vt == MVT_WSTR ) delete _vWstr;
        _vt = MVT_EMPTY;

    };
    MyVariant() 
    {
        LOGF;
        _vt = MVT_EMPTY;
    };
    ~MyVariant()
    {
        LOGF;
        Clear();
    };
    MyVariant &operator=(int value)
    {
        Clear();
        _vt = MVT_INT32;
        _vInt = value;
        return *this;
    };
    MyVariant &operator=(LPSTR value)
    {
        size_t len = lstrlenA( value ) + 1;
        Clear();
        _vt = MVT_ASTR;
        _vAstr = new CHAR[ len ];
        StringCchCopyA( _vAstr, len, value );
        return *this;
    };
    MyVariant &operator=(LPWSTR value)
    {
        size_t len = lstrlenW( value ) + 1;
        Clear();
        _vt = MVT_WSTR;
        _vWstr = new WCHAR[ len ];
        StringCchCopyW( _vWstr, len, value );
        return *this;
    };
    operator int()
    {
        return _vInt;
    }
    operator LPSTR()
    {
        return _vAstr;
    }
    operator LPWSTR()
    {
        return _vWstr;
    }
    void ToNPVariant( NPVariant *variant )
    {
        switch( _vt ){
        case MVT_EMPTY:
            VOID_TO_NPVARIANT( *variant );
            break;
        case MVT_INT32:
            INT32_TO_NPVARIANT( _vInt, *variant );
            break;
        case MVT_ASTR:
            if( _vAstr == NULL ){
                NULL_TO_NPVARIANT( *variant );
            }else{
                NPUTF8 *s = allocUtf8( _vAstr );
                STRINGZ_TO_NPVARIANT( s, *variant );
            }
            break;
        case MVT_WSTR:
            if( _vWstr == NULL ){
                NULL_TO_NPVARIANT( *variant );
            }else{
                NPUTF8 *s = allocUtf8( _vWstr );
                STRINGZ_TO_NPVARIANT( s, *variant );
            }
            break;
        }
    }
};

class win32api : public NPObj {
private:
    CAtlMap<LPCWSTR,HMODULE> _hDll;
public:
    win32api( NPP instance ) : NPObj( instance, true ){ LOGF; };
    ~win32api();
    bool hasMethod( LPCWSTR methodName );
    bool invoke( LPCWSTR methodName, const NPVariant *args, uint32_t argCount, NPVariant *result);
    dllfunc* import( const NPVariant *args, LPCSTR* pszErrMsg );
};

class dllfunc : public NPObj {
private:
    void *_addr; 
    LPWSTR _dll;
    LPSTR _func;
    LPWSTR _argType;
    WCHAR _resultType;
    CAtlArray<MyVariant> _argBuffer;
public:
    dllfunc( NPP , LPVOID );
    ~dllfunc();
    bool setNames( LPCWSTR, LPCSTR, LPCWSTR, WCHAR );
    bool hasMethod( LPCWSTR methodName );
    bool invoke( LPCWSTR methodName, const NPVariant *args, uint32_t argCount, NPVariant *result);
    bool call( const NPVariant *args, DWORD argCount, NPVariant *result, LPCSTR* pszErrMsg );
    bool arg( const NPVariant *args, DWORD argCount, NPVariant *result, LPCSTR* pszErrMsg );
};

static LPCSTR W32E_CANNOT_LOAD_DLL = "cannot load dll";
static LPCSTR W32E_CANNOT_GET_ADDR = "cannot get proc addr";
static LPCSTR W32E_INVALID_ARGUMENT= "invalid argument";
static LPCSTR W32E_CANNOT_ALLOCATE_MEM = "cannot allocate memory";

win32api::~win32api()
{
    POSITION pos;
    HMODULE h;

    LOGF;
    pos = _hDll.GetStartPosition();
    while( pos != NULL ){
        h = _hDll.GetNextValue( pos );
        FreeLibrary( h );
    }
}

bool win32api::hasMethod( LPCWSTR methodName )
{
    LOGF;
    LOG( L"methodName=%s", methodName );
    return true;
}

bool win32api::invoke( LPCWSTR methodName, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    LOGF;
    LOG( L"methodName=%s", methodName );
    LPCSTR e;
    NPObject *obj;

    if( lstrcmpW( methodName, L"import" ) == 0 ){
        dllfunc* pfunc = NULL;
        pfunc = import( args, &e );
        if( !pfunc ){
            LOG( "pfunc=NULL" );
            if( !e ) e = "";
            npnfuncs->setexception( getNPObject(), e );
            return false;
        }
        obj = pfunc->getNPObject();
        OBJECT_TO_NPVARIANT( obj, *result );
        return true;
    }
    return false;
}

dllfunc* win32api::import( const NPVariant *args, LPCSTR* pszErrMsg )
{
    LPCWSTR szDLL = NULL, szArgs = NULL, szResult = NULL;
    LPCSTR szFunc = NULL;
    HMODULE h;
    LPVOID pProc;
    dllfunc *pfunc = NULL;

    LOGF;
    __try{
        pszErrMsg = NULL;
        szDLL = Npv2WStr( args[ 0 ] );
        szFunc = Npv2Str( args[ 1 ] );
        szArgs = Npv2WStr( args[ 2 ] );
        szResult = Npv2WStr( args[ 3 ] );
        LOG( L"dll=%s arg=%s result=%s", szDLL, szArgs, szResult );
        LOG( "func=%s", szFunc );
        if( !_hDll.Lookup( szDLL, h ) ){
            LOG( L"Loading DLL:%s", szDLL );
            h = LoadLibraryW( szDLL );
            if( h == NULL ){
                *pszErrMsg = W32E_CANNOT_LOAD_DLL;
                __leave;
            }
            _hDll[ szDLL ] = h;
        }
        pProc = GetProcAddress( h, szFunc );
        if( pProc == NULL ){
            LOG( "cannot find %s", szFunc );
            *pszErrMsg = W32E_CANNOT_GET_ADDR;
            __leave;
        }
        // TODO: check arg type and result type
        pfunc = new dllfunc( getNPP(), pProc );
        pfunc->setNames( szDLL, szFunc, szArgs, *szResult );
    }
    __finally{
        delete szDLL;
        delete szFunc;
        delete szArgs;
        delete szResult;
    }
    return pfunc;

}

dllfunc::dllfunc( NPP instance, LPVOID pProc ) : NPObj( instance, false )
{
    LOGF;
    _addr = pProc;
    _dll = NULL;
    _func = NULL;
    _argType = NULL;
}

dllfunc::~dllfunc()
{
    LOGF;
    delete _dll;
    delete _func;
}

bool dllfunc::setNames( LPCWSTR szDll, LPCSTR szFunc, LPCWSTR szArgs, WCHAR cResult )
{
    LOGF;
    size_t s1, s2, s3;

    delete _dll;
    delete _func;
    delete _argType;

    s1 = lstrlenW( szDll ) + 1;
    s2 = lstrlenA( szFunc ) + 1;
    s3 = lstrlenW( szArgs ) + 1;
    _dll = new WCHAR[ s1 ];
    _func = new CHAR[ s2 ];
    _argType = new WCHAR[ s3 ];
    StringCchCopyW( _dll, s1, szDll );
    StringCchCopyA( _func, s2, szFunc );
    StringCchCopyW( _argType, s3, szArgs );

    _argBuffer.SetCount( s3 - 1, s3 - 1 );

    _resultType = cResult;

    return true;
}

bool dllfunc::hasMethod( LPCWSTR methodName )
{
    LOGF;
    LOG( L"method=%s", methodName );
    return true;
}

bool dllfunc::invoke( LPCWSTR methodName, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    bool r = false;;
    LPCSTR e = NULL;

    LOGF;
    if( lstrcmpW( methodName, L"call" ) == 0 ){
        r = call( args, argCount, result, &e );
        if( !r ){
            if( !e ){
                e = W32E_INVALID_ARGUMENT;
            }
            npnfuncs->setexception( getNPObject(), e );
        }
    }else if( lstrcmpW( methodName, L"arg" ) == 0 ){
        r = arg( args, argCount, result, &e );
        if( !r ){
            if( !e ){
                e = W32E_INVALID_ARGUMENT;
            }
            npnfuncs->setexception( getNPObject(), e );
        }
    }
    return r;
}

#pragma warning( push )
#pragma warning( disable : 4100 )
inline void DUMP( LPBYTE p, DWORD len )
{
#ifdef DEBUG
    LPTSTR buf, s;
    buf = new CHAR[ len * 3 + 1 ];
    s = buf;

    while( len ){
        StringCchPrintf( s, 4, "%2.2x ", *p );
        s += 3;
        p++;
        len--;
    }
    LOG( buf );
    delete buf;
#endif
}
#pragma warning( pop )

bool dllfunc::call( const NPVariant *args, DWORD argCount, NPVariant *result, LPCSTR* pszErrMsg )
{
    int i;
    DWORD n, dwSize;
    LPBYTE p0 = NULL, p;
    bool Result = false;
    typedef void (WINAPI *voidproc)(void);
    typedef DWORD (WINAPI *dwproc)(void);

    LOGF;
    LOG( L" %s", _dll );
    LOG( " %s", _func );

    *pszErrMsg = NULL;

    n = lstrlenW( _argType );
    if( argCount < n ){
        *pszErrMsg = W32E_INVALID_ARGUMENT;
        return false;
    }

    dwSize = n * 5 + 10;
    p0 = p = reinterpret_cast<LPBYTE>(VirtualAlloc( NULL, dwSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE ) );
    if( p0 == NULL ){
        *pszErrMsg = W32E_CANNOT_ALLOCATE_MEM;
        return false;
    }
    __try{
        DWORD dw;
        LPSTR sa;
        LPWSTR sw;
        //voidproc pf;
        dwproc pf;
        DWORD r;


#ifdef JITDEBUG
        *p++ = 0xcc;
#endif
        for( i = n - 1; i >= 0; i-- ){
            *p++ = 0x68;
            switch( _argType[ i ] ){
            case 'N': // 32bit number
                dw = static_cast<DWORD>( Npv2Int( args[ i ] ) );
                LOG( L"%d : %c : %lu", i, _argType[ i ], dw );
                _argBuffer[ i ] = dw;
                *((LPDWORD)p) = dw;
                p += sizeof( DWORD );
                break;
            case 'A' : // LPSTR
                sa = Npv2Str( args[ i ] );
                LOG( L"%d : %c ", i, _argType[ i ] );
                _argBuffer[ i ] = sa;
                *((LPDWORD)p) = (DWORD)(LPSTR)( _argBuffer[ i ] );
                delete sa;
                p += (int)(sizeof( LPSTR ) );
                break;
            case 'W' : // LPWSTR
                sw = Npv2WStr( args[ i ] );
                _argBuffer[ i ] = sw;
                LOG( L"%d : %c : %s", i, _argType[ i ], sw );
                *((LPDWORD)p) = (DWORD)(LPWSTR)( _argBuffer[ i ] );
                delete sw;
                p += (int)(sizeof( LPWSTR ));
                break;
            }
        }
        // mov eax, func
        *p++ = 0xb8;
        *((LPDWORD)p) = (DWORD)_addr;
        p += sizeof( DWORD);
        // call eax
        *p++ = 0xff;
        *p++ = 0xd0;
        // ret
        *p++ = 0xc3; 

        pf = reinterpret_cast<dwproc>(p0);
        DUMP( p0, dwSize );
        LOG( "calling api" );
#ifdef JITDEBUG
        {
            CHAR buf[ 1024 ];
            int n = 0;
            StringCchPrintfA( buf, _countof( buf ), "vsjitdebugger.exe -p %d", GetCurrentProcessId() );
            WinExec( buf, SW_SHOW );
            while( IsDebuggerPresent() == 0 && n < 50){
                Sleep( 100 );
                n++;
            }
        }
#endif
        r = pf();
        LOG( "called" );

        // set api result to *result
        switch( _resultType ){
        case L'N': // 32bit
            INT32_TO_NPVARIANT( r, *result );
            break;
        case L'I': // 16bit
            r &= 0xffff;
            INT32_TO_NPVARIANT( r, *result );
            break;
        case L'C': // 8bit
            r &= 0xff;
            INT32_TO_NPVARIANT( r, *result );
            break;
        case L'A': // LPSTR
            if( r == 0 ){
                NULL_TO_NPVARIANT( *result );
            }else{
                NPUTF8 *s = allocUtf8( reinterpret_cast<LPCSTR>(r) );
                STRINGZ_TO_NPVARIANT( s, *result );
            }
            break;
        case L'W': // LPWSTR
            if( r == 0 ){
                NULL_TO_NPVARIANT( *result );
            }else{
                NPUTF8 *s = allocUtf8( reinterpret_cast<LPCWSTR>(r) );
                STRINGZ_TO_NPVARIANT( s, *result );
            }
            break;
        default:
            VOID_TO_NPVARIANT( *result );
            break;
        }

        Result = true;
    }
    __finally{
        VirtualFree( p0, 0, MEM_DECOMMIT | MEM_RELEASE );
    }
    return Result;
}

bool dllfunc::arg( const NPVariant *args, DWORD argCount, NPVariant *result, LPCSTR* pszErrMsg )
{
    LOGF;
    unsigned int i;
    if( argCount != 1 ){
        *pszErrMsg = W32E_INVALID_ARGUMENT;
        return false;
    }
    i = Npv2Int( args[ 0 ] );
    if( i < 0 || _argBuffer.GetCount() <= i ){
        LOG( "invalid argument" );
        *pszErrMsg = W32E_INVALID_ARGUMENT;
        return false;
    }

    LOG( "index=%d", i );
    _argBuffer[ i ].ToNPVariant( result );
    {
        LPSTR s = Npv2Str( *result );
        LOG("%s", s );
        delete s;
    }
    return true;


}

static win32api *_plugin;

#ifdef __cplusplus
extern "C" {
#endif



NPError OSCALL NP_GetEntryPoints( NPPluginFuncs *pluginFuncs )
{
    LOGF;
    LOG( L"PID=%lu", GetCurrentProcessId() );
    pluginFuncs->version = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
    pluginFuncs->size = sizeof( pluginFuncs );
    pluginFuncs->newp = NPP_New;
    pluginFuncs->destroy = NPP_Destroy;
    pluginFuncs->getvalue = NPP_GetValue;
    return NPERR_NO_ERROR;
}

NPError OSCALL NP_Initialize( NPNetscapeFuncs *aNPNFuncs )
{
    LOGF;
    npnfuncs = aNPNFuncs;
    return NPERR_NO_ERROR;
}

NPError OSCALL NP_Shutdown( void )
{
    LOGF;
    return NPERR_NO_ERROR;
}

char* NP_GetMIMEDescription( void )
{
    LOGF;
    return( MIME_TYPES_DESCRIPTION );
}

#pragma warning( push )
#pragma warning( disable : 4100 )
NPError NPP_New( NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char *argn[], char *argv[], NPSavedData *saved )
{
    LOGF;
    return NPERR_NO_ERROR;
}
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4100 )
NPError NPP_Destroy( NPP instance, NPSavedData **save )
{
    LOGF;
    if( _plugin ){
        delete _plugin;
        _plugin = NULL;
    }
    return NPERR_NO_ERROR;
}
#pragma warning( pop )

NPError NPP_GetValue( NPP instance, NPPVariable variable, void *value )
{
    LOGF;
    switch( variable ){
    case NPPVpluginNameString:
        LOG( "NPPVpluginNameString" );
        *((char**)value) = "plugin test";
        return NPERR_NO_ERROR;
        break;
    case NPPVpluginScriptableNPObject:
        LOG( "NPPVpluginScriptableNPObject" );
        if( !_plugin ){
            LOG( "creating instance" );
            _plugin = new win32api( instance );
            LOG( "_plugin=%8.8x", (DWORD)_plugin );
            LOG( "created instance" );
        }
        *(NPObject**)value = _plugin->getNPObject();
        LOG( "npobject=%8.8x", (DWORD)(_plugin->getNPObject() ) );

        LOG( "return" );
        return NPERR_NO_ERROR;
        break;
    default:
        LOG( "%d", variable );
        return NPERR_GENERIC_ERROR;
    }

}



#ifdef __cplusplus
}
#endif



