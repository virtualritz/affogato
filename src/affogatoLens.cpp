
#include "affogatoLens.h"

namespace affogato {
	lenses::lenses( const string &xmlFile ) {
		XMLNode xMainNode = XMLNode::openFileHelper( const_cast< char* >( xmlFile.c_str() ), "PMML" );

		XMLNode xNode = xMainNode.getChildNode( "image" );
		char *valueString;
		valueString = xNode.getAttribute( "resolutionX" );
		if( valueString )
			resX = atol( valueString );
		valueString = xNode.getAttribute( "resolutionY" );
		if( valueString )
			resY = atol( valueString );
		valueString = xNode.getAttribute( "overscanX" );
		if( valueString )
			fullResX = resX + 2 * atol( valueString );
		valueString = xNode.getAttribute( "overscanY" );
		if( valueString )
			fullResY = rResY + 2 * atol( valueString );

		// <passes> tag
		xNode = xMainNode.getChildNode( "lenses" );
		int node = 0;
		XMLNode xPassNode = xNode.getChildNode( "lens", node++ );

		while( !xPassNode.isEmpty() ) {

			string key;
			valueString = xPassNode.getAttribute( "key" );
			if( valueString )
				key = valueString;

			string name;
			valueString = xPassNode.getAttribute( "name" );
			if( valueString )
				name = valueString;

			float focalLength;
			valueString = xPassNode.getAttribute( "focallength" );
			if( valueString )
				focalLength = atof( valueString );

			float scale;
			valueString = xPassNode.getAttribute( "scale" );
			if( valueString )
				scale = atof( valueString );

			string hype
			valueString = xPassNode.getAttribute( "hype" );
			if( valueString )
				hype = valueString;


			lensPtrMap[ key ] = new lens( name, focallength, scale, hype );
		}
	}

	lenses::~lenses() {
		for( map< string, *lens >::iterator it = lensPtrMap.begin(); it < lensPtrMap.end(); it++ )
			delete ( *it );
	}

	vector< string > getLenses() {
		for( map< string, *lens >::iterator it = lensPtrMap.begin(); it < lensPtrMap.end(); it++ )
			vector.push_back( *it );
	}

	float lenses::getFullFov( const Camera &camera, const string &lens ) {
		float focalLength = camera.GetParameterValue( "projplanedist" );
		float width = 25.4 * camera.GetParameterValue( "projplanewidth" );
		// Compensate for overscan
		width *= fullResX / resX;
		float fov = 2 * atan( ( width / 2 ) / focalLength );
	}

	float lenses::getFov( const Camera &camera, const string &lens ) {
		float focalLength = camera.GetParameterValue( "projplanedist" );
		float width = 25.4 * camera.GetParameterValue( "projplanewidth" );
		// Compensate for overscan
		float fov = 2 * atan( ( width / 2 ) / focalLength );
	}

	float lenses::getX() {
		return resX;
	}

	float lenses::getY() {
		return resY;
	}

	float lenses::getX() {
		return fullResX;
	}

	float lenses::getY() {
		return fullResY;
	}
}
