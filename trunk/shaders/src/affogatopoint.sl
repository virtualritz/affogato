light affogatopoint(
    uniform float intensity	= 1;
    uniform color lightcolor	= 1;
    uniform float dropoff	= 0;
    uniform float exponent	= 0;

    uniform string shadowname	= "";
    uniform float  shadowbias	= 1;
    uniform float  shadowblur	= 0.01;
    uniform float  shadowsamples = 16;
    uniform float  shadowfiltersize = 1;
    uniform color  shadowcolor	= 0;

    output varying color __Cshadow = 0;
    output varying color __Clunshadowed = 0;
    float __nondiffuse		= 0;
    float __nonspecular		= 0;
    string	__category	= ""
)
{
	float shadowBias = shadowbias;
	surface( "ShadowBias", shadowBias );

	string pass = "";
	attribute( "user:pass", pass );

	if( ( "shadow" != pass ) && ( "deepshadow" != pass ) ) {
		float atten;

		illuminate( point "shader" ( 0, 0, 0 ) ) {

			float distance = length( L );
			//cosangle = ( L . vector "shader" ( 0, 0, 1) ) / distance;

			atten = 1 / pow( distance, exponent );
			//atten *= pow( cosangle, dropoff );
			//atten *= smoothstep( cosoutside, cosinside, cosangle );

			if( shadowname != "" )
				__Cshadow = shadow( shadowname, Ps, "samples", shadowsamples, "blur", shadowblur, "bias", shadowbias, "width", shadowfiltersize );
			else
				__Cshadow = 0;
		}

		Cl = intensity * atten;
		__Clunshadowed = Cl * lightcolor;
		Cl *= mix( lightcolor, shadowcolor, __Cshadow );
	} else {
		Cl = 0;
	}
}
