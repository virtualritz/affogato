/** Gelato Renderer Class.
 *
 *  gelatoRenderer is a renderer-derived singleton class to access the Gelato API
 *
 */

#include <gelatoapi.h>
#include "affogatoGelatoRenderer.h"

namespace affogatoRenderer {
	
	using namespace Gelato;
	using namespace std;
	
	void gelatoRenderer::createRenderer( const string destination ) {
		static GelatoAPI *tmp = GelatoAPI::CreateRenderer( destination.c_str() );
		r = tmp;
	}
		
	const gelatoRenderer* gelatoRenderer::accessRenderer() {		

		return &theRenderer;
	}
	
	void gelatoRenderer::input( const string filename ) {
		r->Input( filename.c_str() );
	}
	
	void gelatoRenderer::input( const string filename, const float *bound ) {
		r->Input( filename.c_str(), bound );
	}
	
	void gelatoRenderer::camera( const string name ) {
		r->Camera( name.c_str() );
	}
	
	void gelatoRenderer::output(	const string name, const  string format,
									const string dataname, const string cameraname ) {
		r->Output( name.c_str(), format.c_str(), dataname.c_str(), cameraname.c_str() );
	}

	void gelatoRenderer::world() {
		r->World();
	}	
	
	void gelatoRenderer::render( const string cameraname ) {
		// This will aquire all data from XSI
		r->Render( cameraname == "" ? NULL : cameraname.c_str() );
		// Clear the attribute cache
		// This implictly calls the destructors of all tokenValues in the cache
		tokenValueCache.clear();
	}
	
	void gelatoRenderer::parameter ( std::vector< affogato::tokenValue > tokenValueArray ) {
		for( unsigned i = 0; i < tokenValueArray.size(); i++ )
			parameter( tokenValueArray[ i ] );
	}
	
	void gelatoRenderer::parameter ( affogato::tokenValue aTokenValue ) {
		// Add this to the cache to make sure the data is available when we do a r->Render()
		tokenValueCache.push_back( aTokenValue );
		r->Parameter( getTokenValueAsString( tokenValueCache.back() ).c_str(), tokenValueCache.back().getData() );
	}
	
	void gelatoRenderer::parameter( string typedname, float value ) {
		r->Parameter( typedname.c_str(), value );
	}
	
	void gelatoRenderer::parameter( string typedname, string value ) {
		r->Parameter( typedname.c_str(), value.c_str() );
	}
	
	void gelatoRenderer::attribute( const string typedname, float value ) {
		r->Attribute( typedname.c_str(), value );
	}
	
	void gelatoRenderer::attribute( const string typedname, string value ) {
		r->Attribute( typedname.c_str(), value.c_str() );
	}
	
	void gelatoRenderer::pushAttributes() {
		r->PushAttributes();
	}
	
	void gelatoRenderer::popAttributes() {
		r->PopAttributes();
	}
	
	void gelatoRenderer::translate( float x, float y, float z ) {
		r->Translate( x, y, z );
	}
	
	void gelatoRenderer::rotate( float angle, float x, float y, float z ) {
		r->Rotate( angle, x, y, z );
	}
	
	void gelatoRenderer::scale( float x, float y, float z ) {
		r->Scale( x, y, z );
	}	
	
	void gelatoRenderer::mesh(	const char *interp, int nfaces,
								const int *nverts, const int *verts ) {
		r->Mesh( interp, nfaces, nverts, verts );
		tokenValueCache.clear();
	}

	// Define static members
	GelatoAPI *gelatoRenderer::r;
	gelatoRenderer gelatoRenderer::theRenderer;
	vector< tokenValue > gelatoRenderer::tokenValueCache;
	
	// Private methods
	string gelatoRenderer::getTokenValueAsString( tokenValue aTokenValue ) {
		
		string tokenValueStr;
		
		switch( aTokenValue.getClass() ) {
			case tokenValue::storageConstant:	 // uniform
				tokenValueStr = "constant";
				break;
			case tokenValue::storagePerPiece:    
				tokenValueStr = "perpiece";
				break;
			case tokenValue::storageLinear:      // varying
				tokenValueStr = "linear";
				break;
			case tokenValue::storageVertex:
				tokenValueStr = "vertex";
				break;
			case tokenValue::storageFaceVarying: // Needs to be translated into linear for Gelato
				tokenValueStr = "linear";
				break;
			case tokenValue::storageFaceVertex:  // Currently unsupported in Gelato
				tokenValueStr = "linear";
				break;
			case tokenValue::storageUndefined:
			default:
				tokenValueStr = "";
		};
			
		switch( aTokenValue.getType() ) {
			case tokenValue::typeFloat:
				tokenValueStr += " float ";
				break;
			case tokenValue::typeInteger:
				tokenValueStr += " int ";
				break;
			case tokenValue::typeColor:
				tokenValueStr += " color ";
				break;
			case tokenValue::typePoint:
				tokenValueStr += " point ";
				break;
			case tokenValue::typeHomogenousPoint:
				tokenValueStr += " hpoint ";
				break;
			case tokenValue::typeVector:
				tokenValueStr += " vector ";
				break;
			case tokenValue::typeNormal:
				tokenValueStr += " normal ";
				break;
			case tokenValue::typeMatrix:
				tokenValueStr += " matrix ";
				break;
			case tokenValue::typeUndefined:
			default:
				tokenValueStr += "";
		}

		tokenValueStr += aTokenValue.getName();
		
		return tokenValueStr;
	}
}