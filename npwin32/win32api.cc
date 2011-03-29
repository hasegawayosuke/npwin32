#include    "npobj.h"
#include    "memmgr.h"
#include    "version.h"

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
    MY_VARIANT_TYPE type( void )
    {
        return _vt;
    };
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
    operator LPDWORD()
    {
        return reinterpret_cast<LPDWORD>( &_vInt );

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

typedef enum {
    AVT_VOID,
    AVT_LPVOID,
    AVT_BOOL,
    AVT_LPBOOL,
    AVT_CHAR,
    AVT_BYTE,
    AVT_LPBYTE,
    AVT_SHORT,
    AVT_LPSHORT,
    AVT_WORD,
    AVT_LPWORD,
    AVT_LONG,
    AVT_LPLONG,
    AVT_DWORD,
    AVT_LPDWORD,
    AVT_ASTR,
    AVT_WSTR,
    AVT_CALLBACK,
    AVT_STRUCT,
    AVT_LPSTRUCT,
}APIVARTYPE, *LPAPIVARTYPE;

const struct {
    APIVARTYPE avtType;
    LPCWSTR szType;
} AvtTables[] = {
    { AVT_VOID,     L"VOID" },
    { AVT_LPVOID,   L"LPVOID" },
    { AVT_BOOL,     L"BOOL" },
    { AVT_LPBOOL,   L"LPBOOL" },
    { AVT_CHAR,     L"CHAR" },
    { AVT_BYTE,     L"BYTE" },
    { AVT_LPBYTE,   L"LPBYTE" },
    { AVT_SHORT,    L"SHORT" },
    { AVT_LPSHORT,  L"LPSHORT" },
    { AVT_WORD,     L"WORD" },
    { AVT_LPWORD,   L"LPWORD" },
    { AVT_LONG,     L"INT" },
    { AVT_LPLONG,   L"LPINT" },
    { AVT_LONG,     L"LONG" },
    { AVT_LPLONG,   L"LPLONG" },
    { AVT_DWORD,    L"DWORD" },
    { AVT_LPDWORD,  L"LPDWORD" },
    { AVT_DWORD,    L"UINT" },
    { AVT_LPDWORD,  L"LPUINT" },
    { AVT_ASTR,     L"LPSTR" },
    { AVT_ASTR,     L"LPCSTR" },
    { AVT_WSTR,     L"LPWSTR" },
    { AVT_WSTR,     L"LPCWSTR" },
    { AVT_CALLBACK, L"CALLBACK" },
    { AVT_STRUCT,   L"STRUCT" },
    { AVT_LPSTRUCT, L"LPSTRUCT" },
};

class win32api : public NPObj {
private:
    CAtlMap<CAtlStringW, HMODULE> _hDll;
public:
    win32api( NPP instance ) : NPObj( instance, true ){ LOGF; };
    ~win32api();
    bool hasMethod( LPCWSTR methodName );
    bool invoke( LPCWSTR methodName, const NPVariant *args, uint32_t argCount, NPVariant *result);
    bool hasProperty( LPCWSTR propName );
    bool getProperty( LPCWSTR propName, NPVariant *result );
    bool import( const NPVariant *args, DWORD argCount, NPVariant *result, LPCSTR* pszErrMsg );
    bool callback( const NPVariant *args, DWORD argCount, NPVariant *result, LPCSTR* pszErrMsg );
    bool struct_( const NPVariant *args, DWORD argCount, NPVariant *result, LPCSTR* pszErrMsg );
};

class dllfunc : public NPObj {
private:
    void *_addr; 
    LPWSTR _dll;
    LPSTR _func;
    LPAPIVARTYPE _argType;
    DWORD _argCount;
    APIVARTYPE _resultType;
    CAtlArray<MyVariant> _argBuffer;
    LPBYTE _pCode;
public:
    static dllfunc* create( NPP, HMODULE, LPCWSTR );
    dllfunc( NPP );
    ~dllfunc();
    bool hasMethod( LPCWSTR methodName );
    bool invoke( LPCWSTR methodName, const NPVariant *args, uint32_t argCount, NPVariant *result);
    bool invokeDefault( const NPVariant *args, uint32_t argCount, NPVariant *result );
    bool call( const NPVariant *args, DWORD argCount, NPVariant *result, LPCSTR* pszErrMsg );
    bool arg( const NPVariant *args, DWORD argCount, NPVariant *result, LPCSTR* pszErrMsg );
    DWORD argCount() { return _argCount; };
};

// callback object
class dllcbk : public NPObj {
private:
    NPObject *_jsfunc;
    LPAPIVARTYPE _argType;
    APIVARTYPE _resultType;
    DWORD _argCount;
    LPBYTE _pCode;
    static DWORD WINAPI callbackThunk( dllcbk*, DWORD );
public:
    dllcbk( NPP );
    ~dllcbk();
    DWORD addr() { return reinterpret_cast<DWORD>( _pCode ); };
    static dllcbk* create( NPP instance, NPObject*, LPCWSTR szDeclaration );
    bool setArgs( NPObject*, LPCWSTR, WCHAR );
    DWORD callback( DWORD ESP );
};

// struct object
class dllsct : public NPObj {
private:
    typedef struct {
        MyVariant buf;
        APIVARTYPE avt;
        DWORD offset;
    } SCTMEMBER, *LPSCTMEMBER;
    LPBYTE _addr;
    CSimpleMap<LPCWSTR, LPSCTMEMBER> _members;
public:
    dllsct( NPP );
    static dllsct* create( NPP instance, LPCWSTR szDeclaration );
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
    _hDll.RemoveAll();
}

bool win32api::hasMethod( LPCWSTR methodName )
{
    LOGF;
    LOG( L"methodName=%s", methodName );
    if( lstrcmpW( methodName, L"import" ) == 0 ){
        return true;
    }
    if( lstrcmpW( methodName, L"callback" ) == 0 ){
        return true;
    }
    if( lstrcmpW( methodName, L"struct" ) == 0 ){
        return true;
    }
    if( lstrcmpW( methodName, L"toString" ) == 0 ){
        return true;
    }
    return false;
}

bool win32api::invoke( LPCWSTR methodName, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    LOGF;
    LOG( L"methodName=%s argCount=%d", methodName, argCount );
    LPCSTR e;
    bool r = false;

    if( lstrcmpW( methodName, L"import" ) == 0 ){
        r = import( args, argCount, result, &e );
        if( !r ){
            if( !e ){
                e = W32E_INVALID_ARGUMENT;
            }
            npnfuncs->setexception( getNPObject(), e );
        }
    }else if( lstrcmpW( methodName, L"callback" ) == 0 ){
        r = callback( args, argCount, result, &e );
        if( !r ){
            if( !e ){
                e = W32E_INVALID_ARGUMENT;
            }
            npnfuncs->setexception( getNPObject(), e );
        }
    }else if( lstrcmpW( methodName, L"struct" ) == 0 ){
        r = struct_( args, argCount, result, &e );
        if( !r ){
            if( !e ){
                e = W32E_INVALID_ARGUMENT;
            }
            npnfuncs->setexception( getNPObject(), e );
        }
    }else if( lstrcmpW( methodName, L"toString" ) == 0 ){
        r = toString( result );
    }
    return r;
}

bool win32api::hasProperty( LPCWSTR propName )
{
    if( lstrcmpW( propName, L"version" ) == 0 ){
        return true;
    }
    return false;
}

bool win32api::getProperty( LPCWSTR propName, NPVariant *result )
{
    LOGF;
    if( lstrcmpW( propName, L"version" ) == 0 ){
        const char s[] = VERSION;
        NPUTF8* p = reinterpret_cast<NPUTF8*>( npnfuncs->memalloc( sizeof( s ) ) );

        RtlCopyMemory( p, s, sizeof( s ) );
        STRINGN_TO_NPVARIANT( p, sizeof( s ), *result );
        return true;
    }
    return false;
}

bool win32api::import( const NPVariant *args, DWORD argCount, NPVariant *result, LPCSTR* pszErrMsg )
{
    LPCWSTR szDLL = NULL;
    LPCWSTR szDeclaration = NULL;
    HMODULE h = NULL;
    dllfunc *pfunc = NULL;
    bool r = false;

    LOGF;
    *pszErrMsg = checkNpArgs( "SS", args, argCount );
    if( *pszErrMsg ) LOG( "err=%s", *pszErrMsg );
    if( *pszErrMsg ) return false;

    __try{
        szDLL = Npv2WStr( args[ 0 ] );
        szDeclaration = Npv2WStr( args[ 1 ] );
        LOG( L"dll=\'%s\' func=%s", szDLL, szDeclaration );
        LOG( "DLLs=%d", _hDll.GetCount() );
        {
            POSITION pos;
            pos = _hDll.GetStartPosition();

        }
        if( !_hDll.Lookup( szDLL, h ) ){
            LOG( L"Loading DLL:%s", szDLL );
            h = LoadLibraryW( szDLL );
            if( h == NULL ){
                LOG( "cannot load dll" );
                *pszErrMsg = W32E_CANNOT_LOAD_DLL;
                __leave;
            }
            LOG( L"Loaded:%s(%lu)", szDLL, h );
            _hDll[ szDLL ] = h;
        }else{
            LOG( L"Loaded:%s(%lu)", szDLL, h );
        }

        pfunc = dllfunc::create( getNPP(), h, szDeclaration );
        if( pfunc != NULL ){
            OBJECT_TO_NPVARIANT( pfunc->getNPObject(), *result );
            r = true;
        }else{
            *pszErrMsg = W32E_INVALID_ARGUMENT;
        }
    }
    __finally{
        //delete szDLL;
        delete szDeclaration;
    }
    return r;

}

bool win32api::callback( const NPVariant *args, DWORD argCount, NPVariant *result, LPCSTR* pszErrMsg )
{
    LPCWSTR szDeclaration = NULL;
    bool r = false;
    dllcbk *cbk;

    LOGF;
    *pszErrMsg = checkNpArgs( "OS", args, argCount );
    if( *pszErrMsg ){
        LOG( "err=%s", *pszErrMsg );
        return false;
    }

    szDeclaration = Npv2WStr( args[ 1 ] );
    cbk = dllcbk::create( getNPP(), NPVARIANT_TO_OBJECT( args[ 0 ] ), szDeclaration );
    if( cbk != NULL ){
        OBJECT_TO_NPVARIANT( cbk->getNPObject(), *result );
        r = true;
    }else{
        *pszErrMsg = W32E_INVALID_ARGUMENT;
    }
    delete szDeclaration;
    return r;
}

bool win32api::struct_( const NPVariant *args, DWORD argCount, NPVariant *result, LPCSTR* pszErrMsg )
{
    LPCWSTR szDeclaration = NULL;
    bool r = false;
    LOGF;
    dllsct *sct;

    *pszErrMsg = checkNpArgs( "S", args, argCount );

    if( *pszErrMsg ){
        LOG( "err=%s", *pszErrMsg );
        return false;
    }

    szDeclaration = Npv2WStr( args[ 0 ] );
    sct = dllsct::create( getNPP(), szDeclaration );
    if( sct != NULL ){
        OBJECT_TO_NPVARIANT( sct->getNPObject(), *result );
        r = true;
    }else{
        *pszErrMsg = W32E_INVALID_ARGUMENT;
    }
    delete szDeclaration;
    return r;
}

#define is_token( c )       ( IsCharAlphaNumericW( c ) || (c == L'_' ) )
#define SkipSpace( s )      while( *s==L' ' || *s==L'\t' || *s==L'\r' || *s==L'\n' )s++
#define SkipToken( s )      while( is_token( *s ) )s++

// TODO: bin search
BOOL GetApiVarType( LPCWSTR &s, APIVARTYPE &type )
{
    int i, n, len;
    n = _countof( AvtTables );

    for( i = 0; i < n; i++ ){
        len = lstrlenW( AvtTables[ i ].szType );
        if( StrCmpNIW( s, AvtTables[ i ].szType, len ) == 0 ){
            if( !is_token( *( s + len) ) ){
                s += len;
                type = AvtTables[ i ].avtType;
                return TRUE;
            }
        }
    }
    return FALSE;
}

LPCWSTR GetAvtString( APIVARTYPE avt )
{
    int i, n;
    n = _countof( AvtTables );
    static LPCWSTR s = L"";

    for( i = 0; i < n; i++ ){
        if( AvtTables[ i ].avtType == avt ){
            return AvtTables[ i ].szType;
        }
    }
    return s;
}


dllfunc::dllfunc( NPP instance ) : NPObj( instance, false )
{
    LOGF;
    _addr = NULL;
    _dll = NULL;
    _func = NULL;
    _argType = NULL;
    _pCode = NULL;
}

dllfunc::~dllfunc()
{
    LOGF;
    delete _dll;
    delete _func;
    delete _argType;
    my_free( _pCode );
}


dllfunc* dllfunc::create( NPP instance, HMODULE hDll, LPCWSTR szDeclaration )
{
    APIVARTYPE avtResult;
    LPCWSTR wszFuncName;
    DWORD dwFuncNameLen = 0;
    LPSTR aszFuncName = NULL;
    LPVOID pProc;
    int w;
    int i;
    dllfunc* pfunc = NULL;
    int argCount = 0;
    LPCWSTR wszArgStart;
    APIVARTYPE avt;

    LOGF;
    SkipSpace( szDeclaration );
    if( !GetApiVarType( szDeclaration, avtResult ) ){
        LOG( L"invalid result type" );
        return NULL;
    }
    SkipSpace( szDeclaration );
    wszFuncName = szDeclaration;
    while( *szDeclaration != L' ' && *szDeclaration != L'(' ){
        szDeclaration++;
        dwFuncNameLen++;
    }

    w = WideCharToMultiByte( CP_ACP, 0, wszFuncName, dwFuncNameLen, NULL, 0, NULL, NULL );
    aszFuncName = new CHAR[ w + 1 ];
    WideCharToMultiByte( CP_ACP, 0, wszFuncName, dwFuncNameLen, aszFuncName, w + 1, NULL, NULL );
    aszFuncName[ w ] = '\0';
    LOG( "funcname=\'%s\'", aszFuncName );

    pProc = GetProcAddress( hDll, aszFuncName );
    if( pProc == NULL ){
        LOG( "cannot get proc address(%d)", GetLastError() );
        return NULL;
    }

    SkipSpace( szDeclaration );
    if( *szDeclaration == L'(' ) szDeclaration++;

    wszArgStart = szDeclaration;
    while( *szDeclaration ){
        SkipSpace( szDeclaration );
        LOG( L"%d \'%s\'", argCount, szDeclaration );
        if( *szDeclaration == L')' ) break;
        if( !GetApiVarType( szDeclaration, avt ) || avt == AVT_VOID ) {
            LOG( L"invalid arg type:%s", szDeclaration );
            return NULL;
        }
        argCount++;
        SkipSpace( szDeclaration );
        SkipToken( szDeclaration );
        SkipSpace( szDeclaration );
        if( *szDeclaration == L',' ){
            *szDeclaration++;
        }
    }

    pfunc = new dllfunc( instance );
    pfunc->_resultType = avtResult;
    pfunc->_addr = pProc;
    pfunc->_func = aszFuncName;
    if( argCount ){
        pfunc->_argType = new APIVARTYPE[ argCount ];
        pfunc->_argBuffer.SetCount( argCount );
    }
    pfunc->_argCount = argCount;
    pfunc->_pCode = reinterpret_cast<LPBYTE>( my_alloc( 5 * argCount + 10 ) );
    if( pfunc->_pCode == NULL ){
        delete pfunc;
        return NULL;
    }

    i = 0;
    while( *wszArgStart ){
        SkipSpace( wszArgStart );
        if( *wszArgStart == L')' ) break;
        GetApiVarType( wszArgStart, avt );
        LOG( L"arg[%d] type=%d", i, avt );
        pfunc->_argType[ i++ ] = avt;

        SkipSpace( wszArgStart );
        SkipToken( szDeclaration );
        SkipSpace( szDeclaration );
        if( *wszArgStart == L',' ){
            *wszArgStart++;
        }
    }

    return pfunc;

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
    LOG( L"methodName=%s argCount=%d", methodName, argCount );
#ifdef DEBUG
    {
        DWORD i;
        for( i = 0; i < argCount; i++ ){
            if( NPVARIANT_IS_INT32( args[ i ] ) ){
                LOG( "args[%d] = 0d%lu", i, NPVARIANT_TO_INT32( args[ i ] ) );
            }else if( NPVARIANT_IS_VOID( args[ i ] ) ){
                LOG( "args[%d] = VOID", i );
            }else if( NPVARIANT_IS_NULL( args[ i ] ) ){
                LOG( "args[%d] = NULL", i );
            }else if( NPVARIANT_IS_BOOLEAN( args[ i ] ) ){
                LOG( "args[%d] = BOOLEAN", i );
            }else if( NPVARIANT_IS_DOUBLE( args[ i ] ) ){
                LOG( "args[%d] = DOUBLE", i );
            }else if( NPVARIANT_IS_STRING( args[ i ] ) ){
                LOG( "args[%d] = STRING", i );
            }else if( NPVARIANT_IS_OBJECT( args[ i ] ) ){
                LOG( "args[%d] = OBJECT", i );
            }
        }
    }
#endif
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
    }else if( lstrcmpW( methodName, L"toString" ) == 0 ){
        r = toString( result );
    }
    return r;
}

bool dllfunc::invokeDefault( const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    return invoke( L"call", args, argCount, result );
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
    LOG( " %s", _func );

    *pszErrMsg = NULL;

    n = _argCount;
    if( argCount < n ){
        *pszErrMsg = W32E_INVALID_ARGUMENT;
        return false;
    }

    dwSize = n * 5 + 10;

    p0 = p = _pCode;

    {
        DWORD dw;
        LPSTR sa;
        LPWSTR sw;
        //voidproc pf;
        dwproc pf;
        DWORD r;


#ifdef JITDEBUG
        *p++ = 0xcc;    // break point for debugger
#endif
        for( i = n - 1; i >= 0; i-- ){
            *p++ = 0x68;
            // TODO:
            LOG( L"arg[ %d ] type=%s", i, GetAvtString( _argType[ i ] ) ); 
            switch( _argType[ i ] ){
            case AVT_LONG:  // 32bit number
            case AVT_DWORD:
                dw = static_cast<DWORD>( Npv2Int( args[ i ] ) );
                LOG( L"%d : %d : %lu", i, _argType[ i ], dw );
                _argBuffer[ i ] = dw;
                *((LPDWORD)p) = dw;
                p += sizeof( DWORD );
                LOG( "# %d", __LINE__ );
                break;
            case AVT_ASTR:  // LPSTR
                sa = Npv2Str( args[ i ] );
                LOG( L"%d : %d ", i, _argType[ i ] );
                _argBuffer[ i ] = sa;
                *((LPDWORD)p) = (DWORD)(LPSTR)( _argBuffer[ i ] );
                delete sa;
                p += (int)(sizeof( LPSTR ) );
                break;
            case AVT_WSTR: // LPWSTR
                sw = Npv2WStr( args[ i ] );
                _argBuffer[ i ] = sw;
                LOG( L"%d : %d : %s", i, _argType[ i ], sw );
                *((LPDWORD)p) = (DWORD)(LPWSTR)( _argBuffer[ i ] );
                delete sw;
                p += (int)(sizeof( LPWSTR ));
                break;
            case AVT_BOOL:
                dw = static_cast<DWORD>( Npv2Bool( args[ i ] ) );
                _argBuffer[ i ] = static_cast<DWORD>( dw );
                *((LPDWORD)p) = dw;
                p += sizeof( DWORD );
                break;
            case AVT_LPLONG:
            case AVT_LPDWORD:
                dw = static_cast<DWORD>( Npv2Int( args[ i ] ) );
                _argBuffer[ i ] = dw;
                *((LPDWORD)p) = reinterpret_cast<DWORD>(static_cast<LPDWORD>( _argBuffer[ i ] ));
                p += sizeof( LPDWORD );
                break;
            case AVT_CALLBACK:
                {
                    NPObject* npobject;
                    dllcbk* cbk;
                    if( !NPVARIANT_IS_OBJECT( args[ i ] ) ){
                        LOG( L"invalid arg type." );
                        return false;
                    }
                    npobject = NPVARIANT_TO_OBJECT( args[ i ] );
                    if( ( cbk = static_cast<dllcbk*>( NPObj::lookup( npobject )) ) == NULL ){
                        LOG( L"invalid arg.obj==NULL" );
                        return false;
                    }
                    dw = static_cast<DWORD>( cbk->addr() );
                    LOG(L"===");
                    DUMP( (LPBYTE)cbk->addr(), 40 );
                    _argBuffer[ i ] = dw;
                    *((LPDWORD)p) = dw;
                    p += sizeof( DWORD );
                    break;
                }
            default:
                LOG( L"invalid arg type:%d", _argType[ i ] );
                *pszErrMsg = W32E_INVALID_ARGUMENT;
                return false;
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
            while( IsDebuggerPresent() == 0 && n < 100){
                Sleep( 200 );
                n++;
            }
        }
#endif
        r = pf();
        LOG( "called:%lu", r );

        // set api result to *result
        LOG( L"result type=%s", GetAvtString( _resultType ) );
        switch( _resultType ){
        case AVT_BOOL:
            BOOLEAN_TO_NPVARIANT( r != 0, *result );
            break;
        case AVT_LONG:
        case AVT_DWORD:
            // fix negative range
            INT32_TO_NPVARIANT( r, *result );
            break;
        case AVT_SHORT:
        case AVT_WORD:
            // TODO: fix negative range
            r &= 0xffff;
            INT32_TO_NPVARIANT( r, *result );
            break;
        case AVT_CHAR:
        case AVT_BYTE:
            // TODO: fix negative range
            r &= 0xff;
            INT32_TO_NPVARIANT( r, *result );
            break;
        case AVT_ASTR:
            if( r == 0 ){
                NULL_TO_NPVARIANT( *result );
            }else{
                NPUTF8 *s = allocUtf8( reinterpret_cast<LPCSTR>(r) );
                STRINGZ_TO_NPVARIANT( s, *result );
            }
            break;
        case AVT_WSTR:
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

    LOG( "index=%d type=%d", i, _argBuffer[ i ].type() );
    _argBuffer[ i ].ToNPVariant( result );
    return true;


}

DWORD WINAPI dllcbk::callbackThunk( dllcbk* _this, DWORD ESP )
{
    LOGF;
    return _this->callback( ESP );
}

dllcbk::dllcbk (NPP instance ) : NPObj( instance, false )
{
    _jsfunc = NULL;
    _argType = NULL;
    _argCount = 0;
    _pCode = NULL;
}

dllcbk::~dllcbk ()
{
    LOGF;
    if( _jsfunc ) npnfuncs->releaseobject( _jsfunc ); // decrease ref counter of JS function
    delete _argType;
    my_free( _pCode );
}

dllcbk* dllcbk::create( NPP instance, NPObject* jsfunc, LPCWSTR szDeclaration )
{
    APIVARTYPE avtResult;
    int i;
    dllcbk* pcbk = NULL;
    int argCount = 0;
    LPCWSTR wszArgStart;
    APIVARTYPE avt;
    LPBYTE pCode;

    LOGF;
    SkipSpace( szDeclaration );
    if( !GetApiVarType( szDeclaration, avtResult ) ) return NULL;
    SkipSpace( szDeclaration );

    if( *szDeclaration != L'(' ) return NULL;
    szDeclaration++;
    SkipSpace( szDeclaration );

    wszArgStart = szDeclaration;
    while( *szDeclaration ){
        SkipSpace( szDeclaration );
        LOG( L"%d \'%s\'", argCount, szDeclaration );
        if( *szDeclaration == L')' ) break;
        if( !GetApiVarType( szDeclaration, avt ) || avt == AVT_VOID ) return NULL;
        argCount++;
        SkipSpace( szDeclaration );
        SkipToken( szDeclaration );
        SkipSpace( szDeclaration );
        if( *szDeclaration == L',' ){
            *szDeclaration++;
        }
    }

    pcbk = new dllcbk( instance );
    pcbk->_resultType = avtResult;
    if( argCount ){
        pcbk->_argType = new APIVARTYPE[ argCount ];
    }
    pcbk->_argCount = argCount;
    pcbk->_jsfunc = jsfunc;
    pCode = static_cast<LPBYTE>( my_alloc( 50 ) );
    pcbk->_pCode = pCode;
    npnfuncs->retainobject( jsfunc ); // increase ref counter of JS function

    i = 0;
    while( *wszArgStart ){
        SkipSpace( wszArgStart );
        if( *wszArgStart == L')' ) break;
        GetApiVarType( wszArgStart, avt );
        LOG( L"arg[%d] type=%d", i, avt );
        pcbk->_argType[ i++ ] = avt;

        SkipSpace( wszArgStart );
        SkipToken( wszArgStart );
        SkipSpace( wszArgStart );
        if( *wszArgStart == L',' ){
            *wszArgStart++;
        }
    }

#ifdef JITDEBUG
    {
        CHAR buf[ 1024 ];
        int n = 0;
        StringCchPrintfA( buf, _countof( buf ), "vsjitdebugger.exe -p %d", GetCurrentProcessId() );
        WinExec( buf, SW_SHOW );
        while( IsDebuggerPresent() == 0 && n < 100){
            Sleep( 200 );
            n++;
        }
    }
    *pCode++ = 0xcc;        // break point for debugger
#endif
    // push ESP
    *pCode++ = 0x54;        
    // push this
    *pCode++ = 0x68;    
    *(reinterpret_cast<LPDWORD>(pCode)) = reinterpret_cast<DWORD>( pcbk );
    pCode += sizeof( DWORD );
    // mov eax, callbackThunk
    *pCode++ = 0xb8;
    *(reinterpret_cast<LPDWORD>(pCode)) = reinterpret_cast<DWORD>( callbackThunk );
    pCode += sizeof( DWORD );
    // call eax
    *pCode++ = 0xff;
    *pCode++ = 0xd0;
    // ret n
    *pCode++ = 0xc2;
    *(reinterpret_cast<LPWORD>(pCode)) = static_cast<WORD>( argCount * 4 );

    return pcbk;
}

DWORD dllcbk::callback( DWORD ESP )
{
    NPVariant *args;
    NPVariant result;
    DWORD r = 0;
    DWORD i;

    LOGF;
    args = reinterpret_cast<NPVariant*>( npnfuncs->memalloc( sizeof( NPVariant ) * _argCount ) );
    for( i = 0; i < _argCount; i++ ){
        DWORD dw = *reinterpret_cast<LPDWORD>( ESP + i * 4 + 4 );
        LOG( L"arg type %d = %s", i, GetAvtString( _argType[ i ] ) );
        // TODO:
        switch( _argType[ i ] ){
        case AVT_BOOL:
            BOOLEAN_TO_NPVARIANT( dw != 0, args[ i ] );
            break;
        case AVT_DWORD:
        case AVT_LONG:
            INT32_TO_NPVARIANT( dw, args[ i ] );
            break;
        default:
            VOID_TO_NPVARIANT( args[ i ] );
            break;
        }
    }

    if( !npnfuncs->invokeDefault( getNPP(), _jsfunc, args, _argCount, &result ) ){
        LOG( L"fail to call jsfunc" );
        npnfuncs->memfree( args );
        return 0;
    }

    LOG( L"result type=%s", GetAvtString( _resultType ) );
    // TODO:
    switch( _resultType ){
    case AVT_BOOL:
        LOG( L"callback result type=bool" );
        r = Npv2Bool( result );
        break;
    case AVT_LONG:
    case AVT_DWORD:
        r = Npv2Int( result );
        break;
    }
    LOG( L"callback result=%d", r );
    npnfuncs->memfree( args );

    return r;
}

dllsct::dllsct (NPP instance ) : NPObj( instance, false )
{

}

dllsct* dllsct::create( NPP instance, LPCWSTR szDeclaration )
{
    LPCWSTR wszArgStart;
    DWORD dwMemberCount = 0;
    APIVARTYPE avt;
    DWORD offset = 0;
    dllsct* sct;
    LPSCTMEMBER member;
    LPCWSTR wszNameStart;
    LPWSTR wszName;
    DWORD dwNameLen;

    LOGF;

    SkipSpace( szDeclaration );
    if( *szDeclaration != L'{' ) return NULL;

    wszArgStart = szDeclaration;
    while( *szDeclaration ){
        SkipSpace( szDeclaration );
        if( *szDeclaration == L'}' ) break;
        if( !GetApiVarType( szDeclaration, avt ) || avt == AVT_VOID ) {
            LOG( L"invalid arg type:%s", szDeclaration );
            return NULL;
        }
        dwMemberCount++;
        SkipSpace( szDeclaration );
        SkipToken( szDeclaration );
        SkipSpace( szDeclaration );
        if( *szDeclaration == L';' ){
            *szDeclaration++;
        }else{
            LOG( L"invalid token:%s", szDeclaration );
            return NULL;
        }
    }

    sct = new dllsct( instance );
    while( *wszArgStart ){
        SkipSpace( wszArgStart );
        if( *wszArgStart == L'}' ) break;
        GetApiVarType( wszArgStart, avt );
        dwMemberCount++;
        SkipSpace( wszArgStart );
        wszNameStart = wszArgStart;
        SkipToken( wszArgStart );
        dwNameLen = wszArgStart - wszNameStart;
        SkipSpace( wszArgStart );
        if( *wszArgStart == L';' ) *wszArgStart++;

        wszName = new WCHAR[ dwNameLen + 1 ];
        StringCchCopyW( wszName, dwNameLen + 1, wszNameStart );

        switch( avt ){
        case AVT_DWORD:
            member = new SCTMEMBER();
            member->avt = avt;
            member->offset = offset;
            offset += sizeof( DWORD );
            sct->_members.Add( wszName, member );

            break;
        }
    }

    return NULL; // debug
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



