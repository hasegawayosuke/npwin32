#include    "memmgr.h"
#include    "npobj.h"

MemMgr::MemMgr()
{
    LOGF;
    _hHeap = HeapCreate( HEAP_CREATE_ENABLE_EXECUTE, 0, 0 ); 
}

MemMgr::~MemMgr()
{
    LOGF;
    HeapDestroy( _hHeap );
}

LPVOID MemMgr::alloc( DWORD cbSize )
{
    LOGF;
    return HeapAlloc( _hHeap, 0, cbSize );
}

bool MemMgr::free( LPVOID p )
{
    LOGF;
    return HeapFree( _hHeap, 0, p ) ? TRUE : FALSE;
}

