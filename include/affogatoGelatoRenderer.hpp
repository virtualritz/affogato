 
#ifndef gelatoRenderer_H
#define gelatoRenderer_H

#include <gelatoapi.h>
#include <vector>
#include <string>

#include "affogatoRenderer.h"
#include "affogatoTokenValue.h"



namespace affogatoRenderer {

	using namespace Gelato;
	using namespace std;
	
	class gelatoRenderer : renderer {
		public:
									gelatoRenderer() {};
								   ~gelatoRenderer() {};
			void					createRenderer( string destination );
			const	gelatoRenderer*	accessRenderer();

			void	input( const string filename );
			void	input( const string filename, const float *bound );

			void	world();
			void	render( const string cameraname = "" );

			void	camera( const string name );
			void	output( const string name, const  string format,
							const string dataname, const string cameraname );									   
				
			void	parameter( std::vector< affogato::tokenValue > tokenValueArray );
			void	parameter( affogato::tokenValue aTokenValue );
			void	parameter( string typedname, float value );
			void	parameter( string typedname, string value );
									   
			void	attribute( const string typedname, float value );
			void	attribute( const string typedname, string value );

			void	pushAttributes();
			void	popAttributes();

			void	translate( float x, float y, float z );
			void	rotate( float angle, float x, float y, float z );
			void	scale( float x, float y, float z );					   

			void 	mesh(	const char *interp, int nfaces,
							const int *nverts, const int *verts );

		private:
			static 	GelatoAPI *r;
			static	gelatoRenderer theRenderer;
			static	vector< tokenValue > tokenValueCache;
			string	getTokenValueAsString( tokenValue aTokenValue );
	};
	
}

#endif
