#include	"memmgr.h"

MemMgr::MemMgr()
{
	SYSTEM_INFO info;
	GetSystemInfo( &info );
	_pBase = NULL;
}

MemMgr::~MemMgr()
{

}

LPVOID MemMgr::alloc( DWORD cbSize )
{
	return VirtualAlloc( NULL, cbSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE );
}

VOID MemMgr::free( LPVOID p )
{
	VirtualFree( p, 0, MEM_DECOMMIT | MEM_RELEASE );
}
