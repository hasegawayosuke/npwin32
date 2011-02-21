#include	<windows.h>

class MemMgr{
private:
	LPVOID _pBase;
	MemMgr();
public:
	static MemMgr& Instance( void ){
		static MemMgr inst;
		return inst;
	}
	~MemMgr();
	LPVOID alloc( DWORD );
	VOID free( LPVOID );
};

#define	my_alloc( size )	( MemMgr::Instance().alloc( size ) )
#define	my_free( p )		( MemMgr::Instance().free( p ) )

