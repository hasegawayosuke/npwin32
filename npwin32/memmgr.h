#pragma once
#include    <windows.h>
#include    <tchar.h>
#include    <atlcoll.h>
#include    <atlsimpcoll.h>


class MemMgr{
private:
    HANDLE _hHeap;
    MemMgr();
public:
    static MemMgr& Instance( void ){
        static MemMgr inst;
        return inst;
    }
    ~MemMgr();
    LPVOID alloc( DWORD );
    bool free( LPVOID );
};

#define my_alloc( size )    ( MemMgr::Instance().alloc( size ) )
#define my_free( p )        ( MemMgr::Instance().free( p ) )

