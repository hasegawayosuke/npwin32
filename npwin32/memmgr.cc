#include    "memmgr.h"
#include    "npobj.h"

MemMgr::MemMgr()
{
    LOGF;
    SYSTEM_INFO info;
    GetSystemInfo( &info );
    _dwPageSize = info.dwPageSize;
    _dwAllocationGranularity = info.dwAllocationGranularity;
}

MemMgr::~MemMgr()
{
    LOGF;
    int  i;
    for( i = _regions.GetSize() -1; i >= 0; i-- ){
        VirtualFree( _regions[ i ]->pBase, 0, MEM_RELEASE );
        delete _regions[ i ];
    }
}

MemMgr::LPMEMREGION MemMgr::Grow()
{
    DWORD i, n;

    LOGF;
    MemMgr::LPMEMREGION p;
    p = new MEMREGION();
    p->dwFreePage = 0;
    p->pBase = VirtualAlloc( NULL, _dwAllocationGranularity, MEM_RESERVE | MEM_TOP_DOWN, PAGE_EXECUTE_READWRITE );
    i = 0xffffffe0;

    n = _dwAllocationGranularity / _dwPageSize;
    for( i = 0; i < n; i++ ){
        p->dwFreePage |= 1 << i;
    }
    _regions.Add( p );
    return p;
}

LPVOID MemMgr::alloc( DWORD cbSize )
{
    int i, j, n, m;
    LPMEMREGION pRegion;
    LPVOID p = NULL;

    LOGF;
    if( cbSize > _dwPageSize ) return NULL;

    n = _regions.GetSize();
    m = _dwAllocationGranularity / _dwPageSize;
    for( i = 0; i < n; i++ ){
        pRegion = _regions[ i ];
        for( j = 0; j < m; j++ ){
            if( pRegion->dwFreePage & (1 << j) ){
                p = reinterpret_cast<LPVOID>( reinterpret_cast<DWORD>( pRegion->pBase ) + j * _dwPageSize );
                pRegion->dwFreePage &= ~(1 << j);
                break;
            }
        }
    };
    if( p == NULL ){
        pRegion = Grow();
        p = pRegion->pBase;
        pRegion->dwFreePage &= ~1; 
    }
    return VirtualAlloc( p, _dwPageSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
}

bool MemMgr::free( LPVOID p )
{
    LOGF;
    int i, j, n, m;
    LPMEMREGION pRegion;

    if( p == NULL ) return true;
    n = _regions.GetSize();
    m = _dwAllocationGranularity / _dwPageSize;
    for( i = 0; i < n; i++ ){
        pRegion = _regions[ i ];
        if( pRegion->pBase <= p && p <= reinterpret_cast<LPVOID>( reinterpret_cast<DWORD>( pRegion->pBase ) + _dwAllocationGranularity ) ){
            for( j = 0; j < m; j++ ){
                DWORD pAddr = reinterpret_cast<DWORD>( pRegion->pBase ) + j * _dwPageSize;
                if( reinterpret_cast<DWORD>( p ) == pAddr ){
                    pRegion->dwFreePage |= (1 << j);
                    VirtualFree( p, _dwPageSize, MEM_DECOMMIT );
                    return true;
                }
            }
        }
    }
    return false;
}

