surface affogatohair(	float	Diffuse				= 1;
						color	RootColor			= color( .109, .037, .007 );
						color	TipColor			= color( .519, .325, .125 );
						float	Specular			= .35;
						float	SpecularRoughness	= .15;
						color	SpecularColor		= ( color( 1 ) + TipColor ) / 2;
					)
{
    vector T = normalize( dPdv ); /* tangent along length of hair */
    vector V = -normalize( I );   /* V is the view vector */
    color Cspec = 0, Cdiff = 0;   /* collect specular & diffuse light */
    float cosang;

    /* Loop over lights, catch highlights as if this was a thin cylinder */
    illuminance( P ) {
		cosang = cos( abs( acos( T . normalize( L ) ) - acos ( -T . V ) ) );
		Cspec += Cl * v * pow( cosang, 1 / SpecularRoughness );
		Cdiff += Cl * v;
		/* We multipled by v to make it darker at the roots.  This
		 * assumes v=0 at the root, v=1 at the tip.
		 */
    }

    Oi = Os;
    Ci = Oi * ( mix( RootColor, TipColor, v ) * ( ambient() + Diffuse * Cdiff )
                + ( Specular * Cspec * SpecularColor ) );
}
