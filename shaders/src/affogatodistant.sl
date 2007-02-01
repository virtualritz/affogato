#include "affogatotools.h"

light affogatodistant(
			float	intensity 				= 1;
			color	lightcolor 				= 1;

	uniform	string	shadowname				= "";
	uniform	string	deepshadowname		 	= "";
			float	shadowbias				= 0.01;
			float	shadowblur				= 0.0;
			float	shadowsamples 			= 16;
	uniform float	shadowfiltersize		= 1;
			color	shadowcolor				= 0;

	output	varying	color	__Cfullshadow	= 0;
	output	varying	color	__Cshadow		= 0;
	output	varying	color	__Cdeepshadow	= 0;
	output	varying	color	__Clunshadowed	= 0;
	output	float			__nondiffuse	= 0;
	output	float			__nonspecular	= 0;
			uniform string	__category		= ""
	)
{
	string pass = "";
	attribute( "user:pass", pass );
	if( 0 == match( "shadow", affogatoGetPass() ) ) {
		solar( vector "shader" ( 0, 0, 1 ), 0 ) {
			if( "" != shadowname )
				__Cshadow = shadow( shadowname, Ps, "samples", shadowsamples, "blur", shadowblur, "bias", shadowbias, "width", shadowfiltersize );
			else
				__Cshadow = 0;

			if( ( 1 != __Cshadow ) && ( "" != deepshadowname ) )
				__Cdeepshadow = shadow( deepshadowname, Ps, "samples", shadowsamples, "blur", shadowblur, "bias", shadowbias, "width", shadowfiltersize );

			color actualShadow = __Cfullshadow = max( __Cshadow, __Cdeepshadow );

			Cl = intensity;
			__Clunshadowed = Cl * lightcolor;

#ifdef DELIGHT
			Cl *= mix( lightcolor, shadowcolor, actualShadow );
#else
			Cl *= mix( lightcolor, shadowcolor, ( comp( actualShadow, 0 ) + comp( actualShadow, 1 ) + comp( actualShadow, 2 ) ) / 3 );
#endif
		}
	} else {
		Cl = 0;
	}
}
