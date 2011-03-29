(function( wscript, fso ){
	var src = "win32api.rc0";
	var dst = "win32api.rc";
	var ver = "version.h";

	fso.OpenTextFile( dst, 2, true, false ).Write(
		fso.OpenTextFile( src, 1, false, false ).ReadAll().replace( /\$VERSION/g,
			fso.OpenTextFile( ver, 1, false, false ).ReadAll().match( /"([^\"]*)"/ )[ 1 ].replace( /\./g, "," ) 
		)
	);
})( WScript, new ActiveXObject( "Scripting.FileSystemObject" ) );
