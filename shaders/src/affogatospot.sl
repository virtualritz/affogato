light affogatospot(
	uniform float	intensity		= 1;
	uniform color	lightcolor		= 1;
	uniform float	coneangle		= radians( 40 );
	uniform float	spreadangle		= radians( 0 );
	uniform float	dropoff			= 0;
	uniform float	exponent		= 0;

	uniform	string	shadowname		= "";
	uniform float	shadowbias		= 0.025;
	uniform float	shadowblur		= 0.01;
	uniform float	shadowsamples		= 16;
	uniform float	shadowfiltersize	= 1;
	uniform color	shadowcolor		= 0;

	output varying	color	__Cshadow	= 0;
	output varying	color	__Clunshadowed	= 0;
			float	__nondiffuse	= 0;
			float	__nonspecular	= 0;
			string	__category	= ""
)
{
	string pass = "";
	attribute( "user:pass", pass );

	if( ( "shadow" != pass ) && ( "deepshadow" != pass ) ) {
		float atten, cosangle;
		uniform float cosoutside, cosinside, angle;

		if( spreadangle < 0 ) {
			angle = coneangle - spreadangle;
			cosoutside = cos( angle );
			cosinside = cos( coneangle );
		} else {
			angle = coneangle;
			cosoutside = cos( coneangle );
			cosinside = cos( coneangle - spreadangle );
		}

		illuminate( point "shader" ( 0, 0, 0 ), vector "shader" ( 0, 0, 1 ), coneangle ) {

			float distance = length( L );

			cosangle = ( L . vector "shader"( 0, 0, 1 ) ) / distance;

			atten = 1 / pow( distance, exponent );
			atten *= pow( cosangle, dropoff );
			atten *= smoothstep( cos(coneangle), cos( coneangle - spreadangle ), cosangle );
			//atten *= smoothstep( cos( -coneangle ), cos( -coneangle + spreadangle ), -cosangle );

			float bias = shadowbias;
			surface( "ShadowBias", bias );

			if( shadowname != "" )
				__Cshadow = shadow( shadowname, Ps, "samples", shadowsamples, "blur", shadowblur, "bias", bias, "width", shadowfiltersize );
			else
				__Cshadow = 0;

			Cl = intensity * atten;

			__Clunshadowed = Cl * lightcolor;
			Cl *= mix( lightcolor, shadowcolor, __Cshadow );
		}

	} else {
		Cl = 0;
	}
}
