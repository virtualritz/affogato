#ifndef affogatoLens_H
#define affogatoLens_H

#include <map>
#include <string>


namespace affogato {

	class lens {
		public:
			lens( const string name);
			~lens();


			}
		private:
			string name;
			float focalLength;
			float scale;
			string hype;

	};

	class lenses {
		public:
			lenses( const string &xmlFile ) {
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

					lensPtrMap[ lensHash ] = new lens( name, focallength, scale, hype );
				}
			}

			vector< string > getLenses() {
				vector< string >
			}

			float getFullFov( const Camera &camera, const string &lens ) {
				float focalLength = camera.GetParameterValue( "projplanedist" );
				float width = 25.4 * camera.GetParameterValue( "projplanewidth" );
				// Compensate for overscan
				width *= fullResX / resX;
				float fov = 2 * atan( ( width / 2 ) / focalLength );
			}

			float getFov( const Camera &camera, const string &lens ) {
				float focalLength = camera.GetParameterValue( "projplanedist" );
				float width = 25.4 * camera.GetParameterValue( "projplanewidth" );
				// Compensate for overscan
				float fov = 2 * atan( ( width / 2 ) / focalLength );
			}

			float getX() {
				return resX;
			}

			float getY() {
				return resY;
			}

			float getX() {
				return fullResX;
			}

			float getY() {
				return fullResY;
			}

		private:
			vector< *lens > lensPtrArray;
			map< string, *lens > lensPtrMap;
	};
}
#endif
