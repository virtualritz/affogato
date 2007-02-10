/** Container class for XSI hair data.
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
#include <string>
#include <limits>
#include <math.h>

// XSI headers
#include <xsi_application.h>
#include <xsi_color.h>
#include <xsi_floatarray.h>
#include <xsi_hairprimitive.h>
#include <xsi_imageclip2.h>
#include <xsi_longarray.h>
#include <xsi_material.h>
#include <xsi_ref.h>
#include <xsi_renderhairaccessor.h>
#include <xsi_sceneitem.h>
#include <xsi_shader.h>
#include <xsi_x3dobject.h>

// Affogato headers
#include "affogatoGlobals.hpp"
#include "affogatoHairData.hpp"
#include "affogatoRenderer.hpp"
#include "affogatoTokenValue.hpp"
#include "affogatoHelpers.hpp"

// RenderMan headers
#include "rx.h" // For RxTextureV()

#define CURVE_DATA_CHUNK_SIZE 65636

namespace affogato {

	using namespace XSI;

	hairData::hairData()
		: bound( 6 ), numChunks( 0 )
	{
		bound[ 5 ] = bound[ 3 ] = bound[ 1 ] = numeric_limits< float >::min();
		bound[ 0 ] = bound[ 2 ] = bound[ 4 ] = numeric_limits< float >::max();
	}

	hairData::~hairData() {
		// Nothing to delete
	}

	/*CRefArray hairData::getImageClips( CRefArray shaders ) {
		CRefArray daClips;
		Application app;
		app.LogMessage( L"Moint" );
		for( long i = 0; i < shaders.GetCount(); i++ ) {
			app.LogMessage( L"Shader " + Shader( shaders[ i ] ).GetName() );
			daClips += Shader( shaders[ i ] ).GetImageClips();
			CRefArray subShaders = Shader( shaders[ i ] ).GetShaders();
			if( subShaders.GetCount() )
				daClips += getImageClips( subShaders );
		}
		return daClips;
	}*/

	hairData::hairData( const Primitive &hairPrim, double atTime, bool caching )
		: bound( 6 ), widthScale( 1.0f )
	{
		identifier = getAffogatoName( CStringToString( X3DObject( hairPrim.GetParent() ).GetFullName() ) );

		theTime = atTime;

		bound[ 5 ] = bound[ 3 ] = bound[ 1 ] = numeric_limits< float >::min();
		bound[ 0 ] = bound[ 2 ] = bound[ 4 ] = numeric_limits< float >::max();

		HairPrimitive theHairPrimitive( const_cast< Primitive& >( hairPrim ) );
		long numHairs = hairPrim.GetParameterValue( L"TotalHairs", floor( theTime ) );
		// By flooring time, we make sure the number of hairs doesn't vary if we export subframe stuff
		// Warning: this will break when the shutter is centered on frame!!!!
		double startTime = floor( theTime );
		numHairs = ( long )( numHairs * ( ( double )hairPrim.GetParameterValue( L"RenderPercentage", startTime ) / 100.0 ) );

		CRefArray affogatoProps( getAffogatoProperties( hairPrim.GetParent() ) );
		for( int i = 0; i < affogatoProps.GetCount(); i++ ) {
			Property prop( affogatoProps[ i ] );

			CParameterRefArray params( prop.GetParameters() );

			for( int p = 0; p < params.GetCount(); p++ ) {
				Parameter param( params[ p ] );
				string paramName = CStringToString( param.GetName() );
				if( "hairpercentage" == paramName ) {
					numHairs = ( numHairs * 10 * ( long )param.GetValue( startTime ) + 5 ) / 1000;
				} else
				if( "hairwidthscale" == paramName ) {
					widthScale = ( float )param.GetValue( atTime );
				}
			}
		}

		Application app;
		message( L"Aquiring " + CValue( numHairs ).GetAsText() + L" hairs" + ( numHairs < 3333 ? L"" : L" -- remember that patience is a virtue..." ), messageInfo );

		if( caching ) {
			if( numHairs > INT_MAX )
				numHairs = INT_MAX;
			rha = theHairPrimitive.GetRenderHairAccessor( numHairs, numHairs, theTime );
			rha.Next();
		} else {
			rha = theHairPrimitive.GetRenderHairAccessor( numHairs, CURVE_DATA_CHUNK_SIZE, theTime );
			numChunks = ( ( rha.GetRequestedHairCount() * 10. / CURVE_DATA_CHUNK_SIZE ) + 10. ) / 10.;
		}

		/*if( rha.GetUVCount() ) { // We have UV sets -- search for a displacement texture
			CRefArray shaders = SceneItem( hairPrim.GetParent() ).GetMaterial().GetShaders();
			CRefArray imageClips;
			for( long i = 0; i < shaders.GetCount(); i++ ) {
				//app.LogMessage( L"Shader " + Shader( shaders[ i ] ).GetName() );
				imageClips += Shader( shaders[ i ] ).GetImageClips();
			}

			for( long i = 0; i < imageClips.GetCount(); i++ ) {
				ImageClip2 iClip = imageClips[ i ];
				if( CString( L"displacement" ) == iClip.GetName() ) {
					displacementMap = iClip.GetImage( theTime );
					clip = iClip;
					break;
				}
			}

			CRefArray props( X3DObject( hairPrim.GetParent() ).GetProperties() );
			displacement = 0;
			for( long i = 0; i < props.GetCount(); i++ ) {
				Property prop( props[ i ] );
				if( CString( L"geomapprox" ) == prop.GetType() ) {
					displacement = 0.5f * ( float )prop.GetParameterValue( L"gapproxmaxdisp", floor( theTime ) );
				}
			}
		}*/

		data( caching );
	}

	data::objectType hairData::type() const {
		return objectHair;
	}

	unsigned hairData::numberOfChunks() {
		return numChunks;
	}

	void hairData::write() const {
		using namespace ueberMan;
		ueberManInterface theRenderer;

		debugMessage( L"I'm wriiiiting!" );
		for( vector< boost::shared_ptr< tokenValue > >::const_iterator it = tokenValuePtrArray.begin(); it < tokenValuePtrArray.end(); it++ ) {
			theRenderer.parameter( **it );
		}
		debugMessage( L"Writing curves" );

		theRenderer.curves( "b-spline", ncurves, nvertspercurve, false, const_cast< string& >( identifier ) );
		debugMessage( L"Done writing curves" );
	}

	bool hairData::writeChunk() {
		using namespace ueberMan;
		ueberManInterface theRenderer;

		message( L"Writing Chunks!", messageInfo );

		tokenValuePtrArray.clear();

		bool finished = false;
		if( rha.Next() ) {
			debugMessage( L"Doing Chunk " );
			data( false );
		} else {
			finished = true;
			rha.Reset();
		}

		theRenderer.translate( 0, 0, 0 );
		//theRenderer.curves( "b-spline", ncurves, nvertspercurve, false, identifier );

		return finished;
	}

	void hairData::data( bool caching ) {
		using namespace ueberMan;
		ueberManInterface theRenderer;
		const globals& g( globals::access() );

		debugMessage( L"Getting Data!" );


		long nChunkSize = rha.GetRequestedChunkSize();
		debugMessage( L"Hair chunk size: " + CValue( nChunkSize ).GetAsText() );
		long nReqHairCount = rha.GetRequestedHairCount();
		debugMessage( L"Hair count: " + CValue( nReqHairCount ).GetAsText() );
		long nHairCount = rha.GetChunkHairCount();
		debugMessage( L"Chunk hair count: " + CValue( nHairCount ).GetAsText() );
		long nUVs = rha.GetUVCount();
		debugMessage( L"Hair UV set count: " + CValue( nUVs ).GetAsText() );


		// get the number of vertices for each render hair
		// note: this array is used for iterating over the render hair position
		// and radius values
		//message( L"Getting Vertex Count Array" );
		CLongArray verticesCountArray;
		rha.GetVerticesCount( verticesCountArray );

		// Number of curves in that chunk
		//message( L"Getting Number of Curves" );
		ncurves = verticesCountArray.GetCount();
		// Number of vertixes per curve in the current chunk
		nvertspercurve.reserve( ncurves );

		long size( 0 );
		for( long curve( 0 ); curve < ncurves; curve++ ) {
			nvertspercurve.push_back( verticesCountArray[ curve ] + 2 ); // We add tangents!
			size += nvertspercurve[ curve ];
		}


		debugMessage( L"Getting hair positions" );
		// get the render hair positions
		CFloatArray posVals, normVals;
		rha.GetVertexPositions( posVals );
		rha.GetHairSurfaceNormalValues( normVals );

		boost::shared_ptr< float > vertices( new float[ size * 3 ], arrayDeleter() );
		boost::shared_ptr< float > baseP( new float[ ncurves * 3 ], arrayDeleter() );


		if( displacementMap.IsValid() && nUVs ) {
			debugMessage( L"Displacing hair" );
			CFloatArray uvVals;
			rha.GetUVValues( 0, uvVals );

			for( long loop( 0 ), index( 0 ), index2( 0 ), origindex( 0 ); loop < ncurves / 1000; loop++ ) {
				Image displacementMap = clip.GetScaledDownImage( siImageRatio1x1 );
				long start	= loop * 1000;
				long end	= start + 1000;
				end = end > ncurves ? ncurves : end;
				for( long curve = start; curve < end; curve++ ) {
					CColor dispColor;
					float s(       uvVals[ curve * 3 ]       * displacementMap.GetResX() );
					float t( ( 1 - uvVals[ curve * 3 + 1 ] ) * displacementMap.GetResY() );
					displacementMap.GetPixelValue( s, t, dispColor );

					debugMessage( L"Curve: " + CValue( curve ).GetAsText() );
					float disp( ( ( float )dispColor.r - 0.5f ) * displacement );
					// 50% Gray is neutral
					float offsetX( disp * normVals[ curve * 3 ] );
					float offsetY( disp * normVals[ curve * 3 + 1 ] );
					float offsetZ( disp * normVals[ curve * 3 + 2 ] );
					// Double up root
					float x( posVals[ origindex ] );
					float y( posVals[ origindex + 1 ] );
					float z( posVals[ origindex + 2 ] );
					vertices.get()[ index++ ] = offsetX + x + x - posVals[ origindex + 3 ];
					vertices.get()[ index++ ] = offsetY + y + y - posVals[ origindex + 4 ];
					vertices.get()[ index++ ] = offsetZ + z + z - posVals[ origindex + 5 ];
					// Save root
					baseP.get()[ index2++ ] = x;
					baseP.get()[ index2++ ] = y;
					baseP.get()[ index2++ ] = z;
					for( long vertex = 0; vertex < verticesCountArray[ curve ]; vertex++ ) {
						vertices.get()[ index++ ] = offsetX + posVals[ origindex++ ];
						vertices.get()[ index++ ] = offsetY + posVals[ origindex++ ];
						vertices.get()[ index++ ] = offsetZ + posVals[ origindex++ ];
					}
					// Double up tip
					x = posVals[ origindex - 3 ];
					y = posVals[ origindex - 2 ];
					z = posVals[ origindex - 1 ];
					vertices.get()[ index++ ] = offsetX + x + x - posVals[ origindex - 6 ];
					vertices.get()[ index++ ] = offsetY + y + y - posVals[ origindex - 5 ];
					vertices.get()[ index++ ] = offsetZ + z + z - posVals[ origindex - 4 ];
				}
				displacementMap.ResetObject();
			}
		} else {
			for( long curve = 0, index = 0, index2 = 0, origindex = 0; curve < ncurves; curve++ ) {
				// Double up root
				float x( posVals[ origindex ] );
				float y( posVals[ origindex + 1 ] );
				float z( posVals[ origindex + 2 ] );
				vertices.get()[ index++ ] = x + x - posVals[ origindex + 3 ];
				vertices.get()[ index++ ] = y + y - posVals[ origindex + 4 ];
				vertices.get()[ index++ ] = z + z - posVals[ origindex + 5 ];
				// Save root
				baseP.get()[ index2++ ] = x;
				baseP.get()[ index2++ ] = y;
				baseP.get()[ index2++ ] = z;
				for( long vertex = 0; vertex < verticesCountArray[ curve ]; vertex++ ) {
					vertices.get()[ index++ ] = posVals[ origindex++ ];
					vertices.get()[ index++ ] = posVals[ origindex++ ];
					vertices.get()[ index++ ] = posVals[ origindex++ ];
				}
				// Double up tip
				x = posVals[ origindex - 3 ];
				y = posVals[ origindex - 2 ];
				z = posVals[ origindex - 1 ];
				vertices.get()[ index++ ] = x + x - posVals[ origindex - 6 ];
				vertices.get()[ index++ ] = y + y - posVals[ origindex - 5 ];
				vertices.get()[ index++ ] = z + z - posVals[ origindex - 4 ];
			}
		}

		debugMessage( L"Calculating Bounding Box" );
		// XSI is too stupid to calculate a correct BB for hair -- we do out own
		if( theTime == g.animation.time ) {
			for( unsigned i = 0; i < ( unsigned )posVals.GetCount(); i += 3 ) {
				if( posVals[ i     ] < bound[ 0 ] )
					bound[ 0 ] = posVals[ i ];
				else
				if( posVals[ i     ] > bound[ 1 ] )
					bound[ 1 ] = posVals[ i ];

				if( posVals[ i + 1 ] < bound[ 2 ] )
					bound[ 2 ] = posVals[ i + 1 ];
				else
				if( posVals[ i + 1 ] > bound[ 3 ] )
					bound[ 3 ] = posVals[ i + 1 ];

				if( posVals[ i + 2 ] < bound[ 4 ] )
					bound[ 4 ] = posVals[ i + 2 ];
				else
				if( posVals[ i + 2 ] > bound[ 5 ] )
					bound[ 5 ] = posVals[ i + 2 ];
			}
		}
		posVals.Clear();

		debugMessage( L"Pushing hair positions" );
		if( caching ) {
			tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( vertices, size * 3, "P", tokenValue::storageVertex, tokenValue::typePoint ) ) );
			if( g.motionBlur.geometryParameterBlur || ( theTime == g.animation.time ) ) {
				tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( baseP, ncurves * 3, "Pbase", tokenValue::storageUniform, tokenValue::typePoint ) ) );
				tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( normVals, "Nbase", tokenValue::storageUniform, tokenValue::typeNormal ) ) );
				normVals.Clear();
			}
		} else {
			theRenderer.parameter( tokenValue( vertices, size * 3, "P", tokenValue::storageVertex, tokenValue::typePoint ) );
			vertices.reset();
			if( g.motionBlur.geometryParameterBlur || ( theTime == g.animation.time ) ) {
				theRenderer.parameter( tokenValue( baseP, ncurves * 3, "Pbase", tokenValue::storageUniform, tokenValue::typePoint ) );
				baseP.reset();
				theRenderer.parameter( tokenValue( normVals, "Nbase", tokenValue::storageUniform, tokenValue::typeNormal ) );
				normVals.Clear();
			}
		}

		// Get the render hair radii
		debugMessage( L"Getting hair radii" );
		CFloatArray radVals;
		rha.GetVertexRadiusValues( radVals );

		// We just need widths for the 'visible' CVs.
		// Thus the number of widths is ncvs-2 per curve
		int widthsize( size - 2 * ncurves );
		boost::shared_ptr< float > widths( new float[ widthsize ], arrayDeleter() );

		for( long curve( 0 ), index( 0 ); curve < ncurves; curve++ ) {
			for ( long vertex = 0; vertex < verticesCountArray[ curve ]; vertex++ ) {
				widths.get()[ index ] = widthScale * 2 * radVals[ index ];
				index++;
			}
		}
		radVals.Clear();

		debugMessage( L"Pushing hair widths" );
		if( caching ) {
			tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( widths, widthsize, "width", tokenValue::storageVarying, tokenValue::typeFloat ) ) );
		} else {
			theRenderer.parameter( tokenValue( widths, widthsize, "width", tokenValue::storageVarying, tokenValue::typeFloat ) );
			widths.reset();
		}

		debugMessage( L"Done pushing hair widths" );

		// Get other data
		if( g.motionBlur.geometryParameterBlur || ( theTime == g.animation.time ) ) {
			// Get the uv values
			//app.LogMessage( L"Getting UV Sets" );
			for ( long uvset = 0; uvset < nUVs; uvset++ ) {
				boost::shared_ptr< float > uvs( new float[ ncurves * 2 ], arrayDeleter() );

				debugMessage( L"Adding data for hair UV set " +  rha.GetUVName( uvset ) );
				CFloatArray uvVals;
				rha.GetUVValues( uvset, uvVals );

				for( long uv = 0, index = 0; index < ncurves * 2; uv += 3 ) {
					uvs.get()[ index++ ] = uvVals[ uv ];
					uvs.get()[ index++ ] = 1 - uvVals[ uv + 1 ];
				}

				uvVals.Clear(); // Free memory before pushing (and thus copying) data

				string setname = CStringToString( rha.GetUVName( uvset ) );
				if( "Texture_Projection" == setname ) // The default name gets translated to the RMan default name
					setname = "stbase";

				debugMessage( L"Pushing" );

				if( caching )
					tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( uvs, ncurves * 2, setname + "[2]", tokenValue::storageUniform, tokenValue::typeFloat ) ) );
				else
					theRenderer.parameter( tokenValue( uvs, ncurves * 2, setname + "[2]", tokenValue::storageUniform, tokenValue::typeFloat ) );

				debugMessage( L"Done Pushing" );
			}

			// Create the curve ids
			boost::shared_ptr< float > ids( new float[ ncurves ], arrayDeleter() );

			for( long curve = 0, index = 0; curve < ncurves; curve++ )
				ids.get()[ curve ] = ( float )curve;

			debugMessage( L"Adding hair ID data" );


			if( caching )
				tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( ids, ncurves, "id", tokenValue::storageUniform ) ) );
			else
				theRenderer.parameter( tokenValue( ids, ncurves, "id", tokenValue::storageUniform ) );
		}
		debugMessage( L"All done" );
	}

	vector< float > hairData::boundingBox() const {
		return bound;
	}

}




