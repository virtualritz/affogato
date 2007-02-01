surface backplane( string texturename = "" ) {

	Oi = 0;

	string pass = "";
	attribute( "user:pass", pass );
	if( ( "" != texturename ) && ( 0 == match( "shadow", pass ) ) )
		Ci = texture( texturename );
	else
		Ci = 0;
}
