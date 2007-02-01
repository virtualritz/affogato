/** Renderer interface class.
 *
 *  This class is designed to call an arbitray number of registered
 *  renderers. The idea was that once can render certain types of
 *  geometry or parts of the scene with a different renderer each.
 *  This would require a mask that selects the renderer per call/per
 *  gprim type, This feature was never implemented so far. :)
 *
 *  @file
 *
 *  @par License:
 *  Copyright (C) 2006 Rising Sun Pictures Pty. Ltd.
 *  @par
 *  This plugin is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later
 *  version.
 *  @par
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *  @par
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *	Boston, MA 02110-1301 USA or point your web browser to
 *	http://www.gnu.org/licenses/lgpl.txt.
 *
 *  @author Moritz Moeller (moritz.moeller@rsp.com.au)
 *
 *  @par Disclaimer:
 *  Rising Sun Pictures Pty. Ltd., hereby disclaims all copyright
 *  interest in the plugin 'Affogato' (a plugin to translate 3D
 *  scenes to a 3D renderer) written by Moritz Moeller.
 *  @par
 *  Any one who uses this code does so completely at their own risk.
 *  Rising Sun Pictures doesn't warrant that this code does anything
 *  at all but if it does something and you don't like it, then we
 *  are not responsible.
 *  @par
 *  Have a nice day!
 */

// Standard headers
#include <vector>
#include <string>
#include <sstream>
#include <stdexcept>

// Boost headers
#include <boost/format.hpp>

// XSI headers
#ifdef __XSI_PLUGIN
	#include <xsi_application.h>
#else
	#include <stdio.h>
#endif

// Affogato headers
#include "affogatoHelpers.hpp"
#include "affogatoRenderer.hpp"
#include "affogatoTokenValue.hpp"

#define ueberManInterfaceCallAll(x) { for( vector< ueberMan* >::iterator it = rendererList.begin(); it < rendererList.end(); it++ ) \
										( *it )->x; }


#ifndef __XSI_PLUGIN
	#define message( x )
#endif

namespace ueberMan {

#ifdef __XSI_PLUGIN
	using namespace XSI;
#endif
	using namespace std;
	using namespace affogato;

	string	getHandle( const string& type, int number );
	string	getLightHandle( const string& handle );
	string	getLookHandle( const string& handle );
	string	getShaderHandle( const string& handle );
	string	getSpaceHandle( const string& handle );
	string	getObjectHandle( const string& handle );
	string	getPrimtiveHandle( const string& handle );

	ueberManInterface::ueberManInterface() {
		debugMessage( L"Constructing ueberManInterface" );
	}

	void ueberManInterface::registerRenderer( const ueberMan& theRenderer ) {

		// Check renderer list here to make sure the renderer doesn't exist already
		for( vector< ueberMan* >::iterator it = rendererList.begin(); it < rendererList.end(); it++ ) {
			if( &theRenderer == *it ) {

				debugMessage( L"UeberMan: Ignored attempt to register renderer twice" );
#ifndef __XSI_PLUGIN
				fprintf( stderr, "Ignored attempt to register renderer twice" );
				fflush( stderr );
#endif
				return;
			}
		}
		rendererList.push_back( const_cast< ueberMan* >( &theRenderer ) );

		debugMessage( L"UeberMan: Registered Renderer No. " + CValue( (long) rendererList.size() ).GetAsText() );
#ifndef __XSI_PLUGIN
		//fprintf( stderr, "Registered Renderer No. %d\n", rendererList.size() );
#endif

	}

	void ueberManInterface::unregisterRenderer( const ueberMan& theRenderer ) {
		for( vector< ueberMan* >::iterator it = rendererList.begin(); it < rendererList.end(); it++ ) {
			if( &theRenderer == *it ) {
				rendererList.erase( it );
				break;
			}
		}
	}

	context ueberManInterface::beginScene( const string& destination, bool useBinary, bool useCompression ) {

		debugMessage( L"UeberMan: BeginScene" );

		// ctxArray; // Contains the returned contexts from all registered renderers

		++contextCounter;
		currentContext = contextCounter;



		contextArrayMap[ currentContext ] = boost::shared_ptr< vector< context > >( new vector< context >( rendererList.size() ) );

		boost::shared_ptr< vector< context > > ctxArray = contextArrayMap[ currentContext ];
		debugMessage( L"UeberMan: BeginScene0.2" );

		unsigned i = 0;
		for( vector< ueberMan* >::iterator it = rendererList.begin(); it < rendererList.end(); it++, i++ ) {
			( *ctxArray )[ i ] = ( *it )->beginScene( destination, useBinary, useCompression );
			debugMessage( L"UeberMan: Beginning Scene " + CValue( ( *ctxArray )[ i ] ).GetAsText() );
		}

		return currentContext;
	}

	void ueberManInterface::switchScene( context ctx ) {
		debugMessage( L"UeberMan: SwitchScene [" + CValue( ctx ).GetAsText() + L"]" );
		if( contextArrayMap.end() != contextArrayMap.find( ctx ) ) {
			boost::shared_ptr< vector< context > > ctxArray = contextArrayMap[ ctx ];
			// TODO: some out-of-bounds checking here for ctx before blindfoldedly entering the loop
			unsigned i = 0;
			for( vector< ueberMan* >::iterator it = rendererList.begin();
					it < rendererList.end();
					it++, i++ )	{
				//debugMessage( L"UeberMan: Switching to " + CValue( ctxArray[ i ] ).GetAsText() );
				( *it )->switchScene( ( *ctxArray )[ i ] );
			}
			currentContext = ctx;
		} else {
			debugMessage( L"UeberMan Error: Trying to switch to non-exising context (" + CValue( ctx ).GetAsText() + L")" );
			//throw( out_of_range( "UeberMan Error: Trying to switch to non-exising context (" + ( format( "%d" ) % ctx ).str() + ")" ) );
		}
	}

	context	ueberManInterface::currentScene() {
		if( !contextArrayMap.empty() )
			return currentContext;
		else
			return contextUndefined;
	}

	void ueberManInterface::endScene( context ctx ) {
		context endContext;
		if( contextUndefined == ctx ) {
			debugMessage( L"UeberMan: EndScene" );
			endContext = currentContext;
		} else {
			endContext = ctx;
			debugMessage( L"UeberMan: EndScene [" + CValue( currentContext ).GetAsText() + L"]" );
		}

		if( contextArrayMap.end() != contextArrayMap.find( endContext ) ) {
			boost::shared_ptr< vector< context > > ctxArray = contextArrayMap[ endContext ];
			unsigned i = 0;
			for( vector< ueberMan* >::iterator it = rendererList.begin();
					it < rendererList.end();
					it++, i++ ) {
				( *it )->endScene( ( *ctxArray )[ i ] );
			}
			// Delete the context list for the passed context
			contextArrayMap.erase( endContext );
			// if there are contexts left, make sure we switch all renderers to the last one in the list
			if( !contextArrayMap.empty() ) {
				currentContext = contextArrayMap.rbegin()->first;

				debugMessage( L"UeberMan: Switching to " + CValue( currentContext ).GetAsText() );
				ctxArray = contextArrayMap[ currentContext ];
				unsigned i = 0;
				for( vector< ueberMan* >::iterator it = rendererList.begin();
					it < rendererList.end();
					it++, i++ ) {
					( *it )->switchScene( ( *ctxArray )[ i ] );
				}
			} else {
				debugMessage( L"UeberMan: No more contexts" );
				// If we endup here, we can do cleanup!!!
				lookSet.clear();
			}
		} else {
			debugMessage( L"UeberMan Error: Trying to end non-exising context (" + CValue( endContext ).GetAsText() + L")" );
			//throw( out_of_range( "UeberMan Error: Trying to end non-exising context (" + ( format( "%d" ) % ctx ).str() + ")" ) ) ;
		}
	}

	void ueberManInterface::input( const string& filename ) {
		ueberManInterfaceCallAll( input( filename ) );
	}

	void ueberManInterface::input( const string& filename, const float *bound ) {
		ueberManInterfaceCallAll( input( filename, bound ) );
	}

	void ueberManInterface::camera( cameraHandle& cameraid ) {
		ueberManInterfaceCallAll( camera( cameraid ) );
	}

	void ueberManInterface::output(	const string& name, const string& format,
									const string& dataname, const cameraHandle& camerid ) {
		ueberManInterfaceCallAll( output( name, format, dataname, camerid ) );
	}

	void ueberManInterface::world() {
		ueberManInterfaceCallAll( world() );
	}

	void ueberManInterface::render( const string& cameraname ) {
		ueberManInterfaceCallAll( render( cameraname ) );
		contextArrayMap.clear();
		lookSet.clear();
	}

	void ueberManInterface::motion( const vector< float >& times ) {
		ueberManInterfaceCallAll( motion( times ) );
	}

	void ueberManInterface::parameter( std::vector< affogato::tokenValue > tokenValueArray ) {
		ueberManInterfaceCallAll( parameter( tokenValueArray ) );
	}

	void ueberManInterface::parameter( const tokenValue& aTokenValue ) {
		ueberManInterfaceCallAll( parameter( aTokenValue ) );
	}

	void ueberManInterface::parameter( const string& typedname, const string& value ) {
		ueberManInterfaceCallAll( parameter( typedname, value ) );
	}

	void ueberManInterface::parameter( const string& typedname, const float value ) {
		ueberManInterfaceCallAll( parameter( typedname, value ) );
	}

	void ueberManInterface::parameter( const string& typedname, const int value ) {
		ueberManInterfaceCallAll( parameter( typedname, value ) );
	}

	void ueberManInterface::parameter( const string& typedname, const bool value ) {
		ueberManInterfaceCallAll( parameter( typedname, value ) );
	}

	void ueberManInterface::variable( const tokenValue& aTokenValue ) {
		ueberManInterfaceCallAll( variable( aTokenValue ) );
	}

	void ueberManInterface::variable( const string& typedname, const string& value ) {
		ueberManInterfaceCallAll( variable( typedname, value ) );
	}

	void ueberManInterface::variable( const string& typedname, const float value ) {
		ueberManInterfaceCallAll( variable( typedname, value ) );
	}

	void ueberManInterface::variable( const string& typedname, const int value ) {
		ueberManInterfaceCallAll( variable( typedname, value ) );
	}

	void ueberManInterface::variable( const string& typedname, const bool value ) {
		ueberManInterfaceCallAll( variable( typedname, value ) );
	}

	void ueberManInterface::attribute( const tokenValue& aTokenValue ) {
		ueberManInterfaceCallAll( attribute( aTokenValue ) );
	}

	void ueberManInterface::attribute( const string& typedname, const string& value ) {
		ueberManInterfaceCallAll( attribute( typedname, value ) );
	}

	void ueberManInterface::attribute( const string& typedname, const float value ) {
		ueberManInterfaceCallAll( attribute( typedname, value ) );
	}

	void ueberManInterface::attribute( const string& typedname, const int value ) {
		ueberManInterfaceCallAll( attribute( typedname, value ) );
	}

	void ueberManInterface::attribute( const string& typedname, const bool value ) {
		ueberManInterfaceCallAll( attribute( typedname, value ) );
	}

	bool ueberManInterface::getAttribute( const string& typedname, float& value ) {
		ueberManInterfaceCallAll( getAttribute( typedname, value ) );
		return true;
	}

	bool ueberManInterface::getAttribute( const string& typedname, int& value ) {
		ueberManInterfaceCallAll( getAttribute( typedname, value ) );
		return true;
	}

	bool ueberManInterface::getAttribute( const string& typedname, string& value ) {
		ueberManInterfaceCallAll( getAttribute( typedname, value ) );
		return true;
	}

	void ueberManInterface::pushAttributes() {
		ueberManInterfaceCallAll( pushAttributes() );
	}

	void ueberManInterface::popAttributes() {
		ueberManInterfaceCallAll( popAttributes() );
	}

	void ueberManInterface::option( const tokenValue &aTokenValue ) {
		ueberManInterfaceCallAll( option( aTokenValue ) );
	}

	void ueberManInterface::option( const string& typedname, const string& value ) {
		ueberManInterfaceCallAll( option( typedname, value ) );
	}

	void ueberManInterface::option( const string& typedname, const float value ) {
		ueberManInterfaceCallAll( option( typedname, value ) );
	}

	void ueberManInterface::option( const string& typedname, const int value ) {
		ueberManInterfaceCallAll( option( typedname, value ) );
	}

	void ueberManInterface::option( const string& typedname, const bool value ) {
		ueberManInterfaceCallAll( option( typedname, value ) );
	}

	void ueberManInterface::pushSpace() {
		ueberManInterfaceCallAll( pushSpace() );
	}

	void ueberManInterface::popSpace() {
		ueberManInterfaceCallAll( popSpace() );
	}

	void ueberManInterface::space( const vector< float >& matrix ) {
		ueberManInterfaceCallAll( space( matrix ) );
	}

	void ueberManInterface::space( const spaceHandle& spacename ) {
		ueberManInterfaceCallAll( space( spacename ) );
	}

	void ueberManInterface::nameSpace( spaceHandle& spacename ) {
		ueberManInterfaceCallAll( nameSpace( spacename ) );
	}

	void ueberManInterface::appendSpace( const vector< float >& matrix ) {
		ueberManInterfaceCallAll( appendSpace( matrix ) );
	}

	void ueberManInterface::translate( const float x, const float y, const float z ) {
		ueberManInterfaceCallAll( translate( x, y, z ) );
	}

	void ueberManInterface::rotate( const float angle, const float x, const float y, const float z ) {
		ueberManInterfaceCallAll( rotate( angle, x, y, z ) );
	}

	void ueberManInterface::scale( const float x, const float y, const float z ) {
		ueberManInterfaceCallAll( scale( x, y, z ) );
	}

	void ueberManInterface::beginLook( lookHandle& lookId ) {
		if( lookSet.end() == lookSet.find( lookId ) ) {
			lookId = getLookHandle( lookId );
			ueberManInterfaceCallAll( beginLook( lookId ) );
			lookSet.insert( lookId );
			//inLook[ currentContext ] = true;
		} /*else {
			throw( out_of_range( "Look id '" + lookId + "' already taken" ) );
		}*/
	}

	void ueberManInterface::endLook() {
		//if( inLook[ currentContext ] ) {
		ueberManInterfaceCallAll( endLook() );
		/*} else {
			//throw( out_of_range( "Invalid context for lookEnd()" ) );
		}*/
	}

	void ueberManInterface::nameLook( lookHandle& lookId ) {
		if( lookSet.end() != lookSet.find( lookId ) ) {
			lookId = getSpaceHandle( lookId );
			ueberManInterfaceCallAll( nameLook( lookId ) );
			lookSet.insert( lookId );
		} else {
			throw( out_of_range( "Look id '" + lookId + "' already taken" ) );
		}
	}

	void ueberManInterface::look( const lookHandle& lookId ) {
		ueberManInterfaceCallAll( look( lookId ) );
	}

	void ueberManInterface::appendLook( const lookHandle& lookId ) {
		ueberManInterfaceCallAll( appendLook( lookId ) );
	}

	set< lookHandle > ueberManInterface::getLooks() {
		return lookSet;
	}

	void ueberManInterface::shader( const string& shadertype, const string& shadername, shaderHandle& shaderId ) {
		shaderId = getShaderHandle( shaderId );
		ueberManInterfaceCallAll( shader( shadertype, shadername, shaderId ) );
	}

	void ueberManInterface::light( const string& shadername, lightHandle& lightId ) {
		lightId = getLightHandle( lightId );
		ueberManInterfaceCallAll( light( shadername, lightId ) );
	}

	void ueberManInterface::switchLight( const lightHandle& lightid, const bool on ) {
		ueberManInterfaceCallAll( switchLight( lightid, on ) );
	}

	void ueberManInterface::points( const string& type, const int numPoints, primitiveHandle &identifier ) {
		identifier = getPrimtiveHandle( identifier );
		ueberManInterfaceCallAll( points( type, numPoints, identifier ) );
	}

	void ueberManInterface::curves( const string& interp, const int ncurves, const int numVertsPerCurve, const bool closed, primitiveHandle &identifier ) {
		identifier = getPrimtiveHandle( identifier );
		ueberManInterfaceCallAll( curves( interp, ncurves, numVertsPerCurve, closed, identifier ) );
	}

	void ueberManInterface::curves( const string& interp, const int ncurves, const vector< int >& numVertsPerCurve, const bool closed, primitiveHandle &identifier ) {
		identifier = getPrimtiveHandle( identifier );
		ueberManInterfaceCallAll( curves( interp, ncurves, numVertsPerCurve, closed, identifier ) );
	}

	void ueberManInterface::curves( const int numCurves, const vector< int >& numVertsPerCurve, const vector< int >& order, const vector< float >& knot, const vector< float >& min, const vector< float >& max, primitiveHandle& identifier ) {
		identifier = getPrimtiveHandle( identifier );
		ueberManInterfaceCallAll( curves( numCurves, numVertsPerCurve, order, knot, min, max, identifier ) );
	}

	void ueberManInterface::patch( const string& interp, int nu, int nv, primitiveHandle &identifier ) {
		identifier = getPrimtiveHandle( identifier );
		ueberManInterfaceCallAll( patch ( interp, nu, nv, identifier ) );
	}

	void ueberManInterface::patch( const int nu, const int uorder, const float *uknot, const float umin, const float umax, const int nv, const int vorder, const float *vknot, const float vmin, const float vmax, primitiveHandle &identifier ) {
		identifier = getPrimtiveHandle( identifier );
		ueberManInterfaceCallAll( patch( nu, uorder, uknot, umin, umax, nv, vorder, vknot, vmin, vmax, identifier ) );
	}

	void ueberManInterface::mesh( const string& interp, const int nfaces, const int *nverts, const int *verts, const bool interpolateBoundary, primitiveHandle &identifier ) {
		identifier = getPrimtiveHandle( identifier );
		ueberManInterfaceCallAll( mesh( interp, nfaces, nverts, verts, interpolateBoundary, identifier ) );
	}

	void ueberManInterface::sphere( const float radius, const float zmin, const float zmax, const float thetamax, primitiveHandle &identifier ) {
		identifier = getPrimtiveHandle( identifier );
		ueberManInterfaceCallAll( sphere( radius, zmin, zmax, thetamax, identifier ) );
	}

	void ueberManInterface::sphere( const float radius, primitiveHandle &identifier ) {
		sphere( radius, -radius, radius, 360, identifier );
	}

	void ueberManInterface::blobby( const int numLeafs, const vector< int >& code, const vector< float >& floatData, const vector< string >& stringData, primitiveHandle& identifier ) {
		identifier = getPrimtiveHandle( identifier );
		ueberManInterfaceCallAll( blobby( numLeafs, code, floatData, stringData, identifier ) );
	}

	void ueberManInterface::makeMap( const string& type ) {
		ueberManInterfaceCallAll( makeMap( type ) );
	}

	string getHandle( const string& type, int number ) {
		stringstream ss;
		ss << "__ueberMan" << type << "Handle" << number;
		return ss.str();
	}

	string getLightHandle( const string& handle ) {
		static int counter = 0;
		if( handle.empty() )
			return getHandle( "Light", counter++ );
		else
			return handle;
	}

	string getLookHandle( const string& handle ) {
		static int counter = 0;
		if( handle.empty() )
			return getHandle( "Look", counter++ );
		else
			return handle;
	}

	string getShaderHandle( const string& handle ) {
		static int counter = 0;
		if( handle.empty() )
			return getHandle( "Shader", counter++ );
		else
			return handle;
	}

	string getSpaceHandle( const string& handle ) {
		static int counter = 0;
		if( handle.empty() )
			return getHandle( "Space", counter++ );
		else
			return handle;
	}

	string getObjectHandle( const string& handle ) {
		static int counter = 0;
		if( handle.empty() )
			return getHandle( "Object", counter++ );
		else
			return handle;
	}

	string getPrimtiveHandle( const string& handle ) {
		static int counter = 0;
		if( handle.empty() )
			return getHandle( "Primtive", counter++ );
		else
			return handle;
	}


	vector< ueberMan* >ueberManInterface::rendererList;
	map< context, boost::shared_ptr< vector< context > > >ueberManInterface::contextArrayMap;
	ueberManInterface ueberManInterface::theUeberManInterface;
	context ueberManInterface::contextCounter = 0;
	context ueberManInterface::currentContext = 0;
	set< lookHandle >ueberManInterface::lookSet;

	spaceHandle
		cameraSpace( "camera" ),
		worldSpace(	 "world" ),
		screenSpace( "screen" ),
		ndcSpace(    "NDC" );
};
