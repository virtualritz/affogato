string affogatoGetPass() {
	uniform string pass = "unknown";
	if( 0 == option( "user:pass", pass ) )
		attribute( "user:pass", pass );
	return pass;
}

float affogatoReceivesShadows() {
	uniform float receive = 1;
	attribute( "user:receivesshadows", receive );
	return clamp( receive, 0, 1 );
}
