/** Shader data management class.
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
#include <cctype>
#include <math.h>
#include <string>

// Boost headers
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/tokenizer.hpp>

// XSI headers
#include <xsi_application.h>
#include <xsi_argument.h>
#include <xsi_command.h>
#include <xsi_context.h>
#include <xsi_kinematics.h>
#include <xsi_model.h>
#include <xsi_ppglayout.h>
#include <xsi_selection.h>

// Affogato headers
#include "affogato.hpp"
#include "affogatoGlobals.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoRenderer.hpp"
#include "affogatoShader.hpp"
#include "affogatoWorker.hpp"

// RenderMan headers
#include "slo.h"


namespace affogato {

	using namespace XSI;
	using namespace ueberMan;
	using namespace boost;
	using namespace std;

	shader::shader()
	:	type( shaderUndefined ),
		displacementSphere( 0 ),
		displacementSpace( "shader" )
	{
		//debugMessage( L"Shader constructor" );

	}

	shader::shader( const shader &aShader ) {
		//debugMessage( L"Shader copy constructor." );
		/*for( vector< tokenValue* >::iterator it = tokenValuePtrArray.begin(); it < tokenValuePtrArray.end(); it++ ) {
			delete *it;
		}*/

		tokenValuePtrArray.clear();

		type = aShader.type;
		name = aShader.name;
		//lightHandle = aShader.lightHandle;
		displacementSphere = aShader.displacementSphere;
		displacementSpace = aShader.displacementSpace;

		shader *s = const_cast< shader* >( &aShader );

		tokenValuePtrArray = aShader.tokenValuePtrArray;
/*		for( vector< tokenValue* >::iterator it = s->tokenValuePtrArray.begin(); it < s->tokenValuePtrArray.end(); it++ ) {
			tokenValue x( *( *it ) );
			tokenValuePtrArray.push_back( new tokenValue( x ) );
		}*/
		//debugMessage( L"Done with shader copy constructor." );
	}


	shader::shader( const Parameter &aShader ) {
		//debugMessage( L"calling shader constructor." );
		set( aShader );
		//debugMessage( L"done with shader constructor." );
	}

	shader::shader( const Property &aShader ) {
		//debugMessage( L"calling shader constructor." );
		set( aShader );
		//debugMessage( L"done with shader constructor." );
	}

	shader::shader( shaderType theType, const string &theName )
	:	type( theType ),
		name( theName ),
		displacementSphere( 0 ),
		displacementSpace( "shader" )
	{

	}

	bool shader::isShader( const Property &aShader ) {
		CParameterRefArray params( aShader.GetParameters() );
		return CString( __AFFOGATO_SHADER_ID ) == Parameter( params[ 0 ] ).GetScriptName();
	}

	shader::shaderType shader::getType( const Property &aShader ) {
		if( isShader( aShader ) ) {
			string shaderData( CStringToString( aShader.GetParameterValue( CString( __AFFOGATO_SHADER_ID ) ) ) );
			size_t pos = shaderData.find( ";", 0 );
			return static_cast< shaderType >( atoi( shaderData.substr( 0, pos ).c_str() ) );
		} else
			return shaderUndefined;
	}

	shader::~shader() {
		// nothing to delete
	}

	void shader::set( const Property &aShader ) {
		CParameterRefArray params( aShader.GetParameters() );

		const globals& g( globals::access() );

		debugMessage( L"Testing property " + aShader.GetName() + L" for shader." );

		if( CString( __AFFOGATO_SHADER_ID ) == Parameter( params[ 0 ] ).GetScriptName() ) {
			string shaderData( CStringToString( Parameter( params[ 0 ] ).GetValue() ) );

			size_t pos = shaderData.find( ";", 0 );

			type = static_cast< shaderType >( atoi( shaderData.substr( 0, pos ).c_str() ) );
			name = CStringToString( ( CString )aShader.GetParameterValue( L"___ShaderFile" ) );

			debugMessage( L"Shader type: " + CValue( ( long )type ).GetAsText() + L", " + stringToCString( name ) );

			string shaderParams = CStringToString( ( CString )aShader.GetParameterValue( __AFFOGATO_SHADER_PARAMS ) );

			typedef tokenizer< char_separator< char > > tokenizer;

			char_separator< char > sep( ";" );
			tokenizer tokens( shaderParams, sep );

			// Find the light category
			string category;
			if( shaderLight == type ) {
				for( tokenizer::iterator it = tokens.begin(); it != tokens.end(); it++ ) {
					if( 7 == atoi( ( *( it++ ) ).c_str() ) ) {
						if( ( "__category" == *it )
#ifdef RSP
							|| ( "category" == *it )
#endif
						   ) {
							category = CStringToString( ( CString )aShader.GetParameterValue( stringToCString( *it ) ) );
							break;
						}
					}
				}
			}

			for( tokenizer::iterator it = tokens.begin(); it != tokens.end(); it++ ) {
				switch( atoi( ( *( it++ ) ).c_str() ) ) {
					case 1: // float
						tokenValuePtrArray.push_back( shared_ptr< tokenValue>( new tokenValue( ( float )aShader.GetParameterValue( stringToCString( *it ), g.animation.time ), *it ) ) );
						break;
					case 5: // color
						float color[ 3 ];
						color[ 0 ] = ( float )aShader.GetParameterValue( stringToCString( *it + "R" ), g.animation.time );
						color[ 1 ] = ( float )aShader.GetParameterValue( stringToCString( *it + "G" ), g.animation.time );
						color[ 2 ] = ( float )aShader.GetParameterValue( stringToCString( *it + "B" ), g.animation.time );
						tokenValuePtrArray.push_back( shared_ptr< tokenValue>( new tokenValue( color, 3, *it, tokenValue::storageUndefined, tokenValue::typeColor ) ) );
						break;
					case 7: { // string
						string cleanedValue, value( parseString( CStringToString( ( CString )aShader.GetParameterValue( stringToCString( *it ) ) ) ) );
						cleanedValue = value;
						replace_all( cleanedValue, " ", "" );

						typedef enum autoMapType {
							mapNone,
							mapTexture,
							mapShadow,
							mapDeepShadow,
							mapEnvironment,
							mapReflection,
							mapPhoton,
							mapPoint,
							mapBrick
						} autoMapType;

						autoMapType mapType = mapNone;

						string objectName;
						if( cleanedValue.substr( 0, 4 ) == "auto" ) {
							if( cleanedValue.substr( 4, 7 ) == "shadow(" ) {
								mapType = mapShadow;
								objectName = cleanedValue.substr( 11, cleanedValue.length() - 12 );
							} else
							if( cleanedValue.substr( 4, 11 ) == "deepshadow(" ) {
								mapType = mapDeepShadow;
								objectName = cleanedValue.substr( 15, cleanedValue.length() - 16 );
							} else
							if( cleanedValue.substr( 4, 12 ) == "environment(" ) {
								mapType = mapEnvironment;
								objectName = cleanedValue.substr( 16, cleanedValue.length() - 17 );
							} else
							if( cleanedValue.substr( 4, 11 ) == "reflection(" ) {
								mapType = mapReflection;
								objectName = cleanedValue.substr( 15, cleanedValue.length() - 16 );
							} else
							if( cleanedValue.substr( 4, 7 ) == "photon(" ) {
								mapType = mapReflection;
								objectName = cleanedValue.substr( 11, cleanedValue.length() - 12 );
							}
						}

						switch( mapType ) {
							case mapShadow:
							case mapDeepShadow: {
								debugMessage( L"Doing auto(deep)shadow" );

								CRefArray objects;
								if( objectName.empty() ) {
									objects.Add( aShader.GetParent() );
									objectName = CStringToString( X3DObject( objects[ 0 ] ).GetName() );
									message( L"Using object '" + stringToCString( objectName ) + L"' for auto (deep)shadow map generation", messageInfo );
								} else {
									Application app;
									Model sceneRoot( app.GetActiveSceneRoot() );
									objects.Add( sceneRoot.FindChild( stringToCString( objectName ), siCameraPrimType, CStringArray() ) );
								}

								if( objects.GetCount() ) {

									X3DObject light( aShader.GetParent() );
									X3DObject camera( objects[ 0 ] );

									ueberManInterface theRenderer;
									blockManager& bm( const_cast< blockManager& >( blockManager().access() ) );

									string cameraName( CStringToString( camera.GetName() ) );
									string shadowName( cameraName + ( mapDeepShadow == mapType ? ".deep" : "." ) + "shadow" );

									enum mapGenerationType {
										mapGenIdle = 0,
										mapGenAlways = 1,
										mapGenLazy = 2,
										magGenReferenceOnly = 3
									} mapGeneration = mapGenAlways;

									filesystem::path shadowDir( g.directories.map );
									CRefArray affogatoProps( getAffogatoProperties( camera ) );

									for( unsigned i = 0; i < ( unsigned )affogatoProps.GetCount(); i++ ) {
										Property prop( affogatoProps[ i ] );
										CValue basePath( prop.GetParameterValue( L"basepath" ) );
										if( !basePath.IsEmpty() ) {
											shadowDir = CStringToString( basePath );
										}

										CValue mapGen( prop.GetParameterValue( L"mapgeneration" ) );
										if( !mapGen.IsEmpty() ) {
											mapGeneration = static_cast< mapGenerationType >( ( long )mapGen );
										}
									}

									string jobName( g.name.baseName + "." + shadowName + "." + g.name.currentFrame );
									string jobNameGlobal( g.name.baseName + "." + shadowName + ".####" );
									string shadowMapName( ( shadowDir / jobNameGlobal ).native_file_string() );
									string shadowMapWriteName( getCacheFilePath( shadowDir / jobName, g.directories.caching.mapWrite ).native_file_string() );
									string shadowMapWriteNameGlobal( getCacheFilePath( shadowDir / jobNameGlobal, g.directories.caching.mapWrite ).native_file_string() );
									string shadowMapSourceName( getCacheFilePath( shadowDir / jobName, g.directories.caching.mapSource ).native_file_string() );

									//filesystem::exists(

									string ext;
									if( mapDeepShadow == mapType ) { // We're in a deep shadow pass
										ext = ".dsh";
									} else {
										ext = ".shd";
									}
									shadowMapSourceName		+= ext;

									if( g.data.shadow.shadow ) {

										bm.beginBlock( blockManager::blockShadowScene, true, shadowName );

										context ctx( bm.currentContext() );

										ueberManInterface theRenderer;

										theRenderer.attribute( "shading:rate", g.shading.rate );
										theRenderer.attribute( "subsurface:shadingrate", g.rays.subSurface.rate );
										theRenderer.attribute( "shading:interpolation", string( "smooth" ) ); // string( g.shading.smooth ? "smooth" : "constant" ) );
										theRenderer.attribute( "dicing:hair", ( int ) 1 );
										theRenderer.attribute( "irradiance:nsamples", ( int )g.rays.irradiance.samples );
										theRenderer.attribute( "irradiance:shadingrate", ( float )g.rays.irradiance.shadingRate );

										string category( CStringToString( aShader.GetParameterValue( L"__category" ) ) );
										if( !category.empty() )
											theRenderer.option( "lightcategory", string( category ) );

										string type, data;
										if( mapDeepShadow == mapType ) { // We're in a deep shadow pass
											theRenderer.attribute( "pass", string( "deepshadow" ) );
											theRenderer.option( "pass", string( "deepshadow" ) );
											if( !category.empty() )
												theRenderer.option( "lightcategory", string( category ) );
											// DSM interpolation  0 -discrete, otherwise continuous
											theRenderer.parameter( "volumeinterpretation", string( light.GetParameterValue( L"ShadowMapDetailAccuracy" ) ? "continuous" : "discrete" ) );
											ext = ".dsh";
											type = "dsm";
											data = "rgbaz";
										} else {
											theRenderer.attribute( "pass", string( "shadow" ) );
											theRenderer.option( "pass", string( "shadow" ) );
											if( !category.empty() )
												theRenderer.option( "lightcategory", string( category ) );
											theRenderer.parameter( "zcompression", string( "zip" ) );
											ext = ".shd";
											type = "shadow";
											data = "z";
										}

										shadowMapName			+= ext;
										shadowMapWriteName		+= ext;
										shadowMapWriteNameGlobal += ext;
										cameraHandle camHandle( "shadowcamera" );

										theRenderer.parameter( "filter", string( "box" ) );
										float filter[ 2 ] = { 1, 1 };
										theRenderer.parameter( tokenValue( filter, 2, "filterwidth", tokenValue::storageUniform, tokenValue::typeFloat ) );
										theRenderer.output( shadowMapWriteName, type, data, camHandle );

										theRenderer.attribute( "identifier:name", CStringToString( camera.GetFullName() ) );

										theRenderer.parameter( "projection", string( camera.GetParameterValue( L"proj" ) ? "perspective" : "orthographic" ) );

										bool isPerspective( camera.GetParameterValue( L"proj", g.animation.time ) );
										if( isPerspective ) {
											theRenderer.parameter( "projection", string( "perspective" ) );
										} else {
											theRenderer.parameter( "projection", string( "orthographic" ) );
											float orthoSizeHalf	( 0.5f * ( float )camera.GetParameterValue( L"orthoheight", g.animation.time ) );
											float screen[ 4 ] = { -orthoSizeHalf, +orthoSizeHalf, -orthoSizeHalf, +orthoSizeHalf };
											theRenderer.parameter( tokenValue( screen , 4, "screen", tokenValue::storageUndefined, tokenValue::typeFloat ) );
										}

										theRenderer.parameter( "near", ( float )camera.GetParameterValue( L"near", g.animation.time ) );
										theRenderer.parameter( "far", ( float )camera.GetParameterValue( L"far", g.animation.time ) );

										float timeOrigin( globals::motionBlur::shutterMoving == g.motionBlur.shutterConfiguration ? g.motionBlur.shutterOffset + g.animation.time : g.motionBlur.shutterOffset );
										// If motion blurred shadow maps are disabled, set shutter close = shutter open
										float samples[ 2 ] = {	timeOrigin + ( g.motionBlur.shadowMapBlur ? g.motionBlur.shutterOpen : 0 ),
																timeOrigin + ( g.motionBlur.shadowMapBlur ? g.motionBlur.shutterClose : 0 ) };
										theRenderer.parameter( tokenValue( samples, 2, "shutter", tokenValue::storageUndefined, tokenValue::typeFloat ) );

										float shutterEfficiency[ 2 ] = { g.motionBlur.shutterEfficiency, g.motionBlur.shutterEfficiency };
										theRenderer.parameter( tokenValue( shutterEfficiency, 2, "shutterefficiency", tokenValue::storageUndefined, tokenValue::typeFloat ) );

										theRenderer.parameter( "bucketorder", g.reyes.bucketorder );

										int bucket[ 2 ] = { g.reyes.bucketSize.x, g.reyes.bucketSize.y };
										theRenderer.parameter( tokenValue( bucket, 2, "bucketsize", tokenValue::storageUndefined ) );

										theRenderer.parameter( "texturememory", ( int )g.reyes.textureMemory );

										theRenderer.parameter( "gridsize", ( int )g.reyes.gridSize );

										theRenderer.parameter( "eyesplits", ( int )g.reyes.eyeSplits );

										theRenderer.parameter( "fov", ( float )camera.GetParameterValue( L"fov" ) );

										// Use light's shadow res as a fallback
										// Alternatively look for an affogato property
										// with a 'resolution' parameter that can
										// override it on the camera.
										int shdRes( ( long )light.GetParameterValue( L"ShadowMapRes" ) );
										int shdSmp( ( long )light.GetParameterValue( L"ShadowMapDetailSamples" ) );

										for( unsigned i = 0; i < ( unsigned )affogatoProps.GetCount(); i++ ) {
											Property prop( affogatoProps[ i ] );

											CValue res( prop.GetParameterValue( L"resolution" ) );
											if( !res.IsEmpty() )
												shdRes = ( long )res;

											CValue samples( prop.GetParameterValue( L"samples" ) );
											if( !res.IsEmpty() )
												shdSmp = ( long )samples;
										}

										int res[ 2 ] = { shdRes, shdRes };
										theRenderer.parameter( tokenValue( res, 2, "resolution", tokenValue::storageUndefined ) );

										if( mapDeepShadow == mapType ) { // We're in a deep shadow pass
											float pixsamples[ 2 ] = { ( float )shdSmp, ( float )shdSmp };
											theRenderer.parameter( tokenValue( pixsamples, 2, "pixelsamples", tokenValue::storageUndefined ) );
										} else {
											float pixsamples[ 2 ] = { 1.0f, 1.0f };
											theRenderer.parameter( tokenValue( pixsamples, 2, "pixelsamples", tokenValue::storageUndefined ) );
											theRenderer.parameter( "depthfilter", string( "midpoint" ) );
										}

										theRenderer.camera( camHandle );

										CMatrix4 camflip(   1,  0,  0,  0,
															0,  1,  0,  0,
															0,  0, -1,  0,
															0,  0,  0,  1 );

										if( g.motionBlur.lightBlur ) {
											vector< float > sampletimes( getMotionSamples( g.motionBlur.transformMotionSamples ) );
											theRenderer.motion( remapMotionSamples( sampletimes ) );
											for( unsigned short motion = 0; motion < g.motionBlur.transformMotionSamples; motion++ ) {
												CMatrix4 matrix = camera.GetKinematics().GetGlobal().GetTransform( sampletimes[ motion ] ).GetMatrix4();
												matrix.InvertInPlace();
												matrix.MulInPlace( camflip );
												theRenderer.space( CMatrix4ToFloat( matrix ) );
											}
										} else {
											CMatrix4 matrix = camera.GetKinematics().GetGlobal().GetTransform( g.animation.time ).GetMatrix4();
											matrix.InvertInPlace();
											matrix.MulInPlace( camflip );
											theRenderer.space( CMatrix4ToFloat( matrix ) );
										}

										theRenderer.input( g.data.worldBlockName.native_file_string() );

										bm.addMapRenderIoToCurrentBlock( shadowMapWriteNameGlobal, shadowMapName );


										bm.endBlock( ctx, 300 );
									}

									value = shadowMapSourceName;
								}
								break;
							}
							case mapEnvironment: {

								CRefArray objects;
								if( objectName.empty() ) {
									objects.Add( aShader.GetParent() );
									objectName = CStringToString( X3DObject( objects[ 0 ] ).GetName() );
									message( L"Using object '" + stringToCString( objectName ) + L"' for auto environment map generation", messageInfo );
								} else {
									message( L"blob " + stringToCString( objectName ), messageError );
									Application app;
									Model sceneRoot( app.GetActiveSceneRoot() );
									objects.Add( sceneRoot.FindChild( stringToCString( objectName ), CString(), CStringArray() ) );
								}

								if( objects.GetCount() ) {

									X3DObject object( objects[ 0 ] );

									ueberManInterface theRenderer;

									blockManager& bm( const_cast< blockManager& >( blockManager().access() ) );

									filesystem::path envDir( g.directories.map );

									CRefArray props( object.GetProperties() );

									string envName( objectName + ".environment" );
									string jobName( g.name.baseName + "." + envName + "." + g.name.currentFrame );
									string jobNameGlobal( g.name.baseName + "." + envName + ".####" );
									string envMapSourceName( getCacheFilePath( envDir / jobName, g.directories.caching.mapSource ).native_file_string() );
									string ext( ".env" );
									envMapSourceName += ext;

									for( unsigned i = 0; i < ( unsigned )props.GetCount(); i++ ) {
										Property prop( props[ i ] );
										if( CString( L"AffogatoEnvironmentMapGenerator" ) == prop.GetType() ) {
											if( isVisible( object ) ) {
												vector< string > mapFace;
												vector< string > paramName;
												taskList mapFaceTasks;
												for( unsigned camera = 0; camera < 6; camera++ ) {

													switch( camera ) {
														case 0:
															paramName.push_back( "px" );
															break;
														case 1:
															paramName.push_back( "nx" );
															break;
														case 2:
															paramName.push_back( "py" );
															break;
														case 3:
															paramName.push_back( "ny" );
															break;
														case 4:
															paramName.push_back( "pz" );
															break;
														case 5:
															paramName.push_back( "nz" );
															break;
													}
													envName = objectName + ".environment." + paramName.back();

													jobName = g.name.baseName + "." + envName + "." + g.name.currentFrame;
													jobNameGlobal = g.name.baseName + "." + envName + ".####";
													string envMapName( ( envDir / jobNameGlobal ).native_file_string() );
													string envMapWriteName( getCacheFilePath( envDir / jobName, g.directories.caching.mapWrite ).native_file_string() );
													string envMapWriteNameGlobal( getCacheFilePath( envDir / jobNameGlobal, g.directories.caching.mapWrite ).native_file_string() );

													envMapName				+= ext;
													envMapWriteName			+= ext;
													envMapWriteNameGlobal	+= ext;

													mapFace.push_back( envMapWriteName );

													bm.beginBlock( blockManager::blockMapScene, true, envName );

													context ctx( bm.currentContext() );

													ueberManInterface theRenderer;

													cameraHandle camHandle( "environmentcamera" );

													theRenderer.parameter( "filter", string( "gaussian" ) );
													float filter[ 2 ] = { 2.5f, 2.5f };
													theRenderer.parameter( tokenValue( filter, 2, "filterwidth", tokenValue::storageUniform, tokenValue::typeFloat ) );
													theRenderer.output( envMapWriteName, "file", "rgba", camHandle );

													theRenderer.attribute( "shading:quality", ( float )sqrt( 1 / ( g.shading.rate * g.shading.rate ) ) );
													theRenderer.attribute( "shading:interpolation", string( g.shading.smooth ? "smooth" : "constant" ) );
													theRenderer.attribute( "shading:motionfactor", ( float )g.reyes.motionFactor );
													theRenderer.attribute( "shading:focusfactor", ( float )g.reyes.focusFactor );
													theRenderer.attribute( "shading:backfacing", false );
													theRenderer.attribute( "dicing:hair", ( int )g.shading.hair );
													float oThresh[ 3 ] = { g.reyes.opacityThreshold, g.reyes.opacityThreshold, g.reyes.opacityThreshold };
													theRenderer.option( tokenValue( oThresh, 3, "limits:othreshold", tokenValue::storageUndefined, tokenValue::typeColor ) );

													if( g.rays.enable ) {
														theRenderer.option( "trace:maxdepth", ( int )g.rays.trace.depth );
														theRenderer.attribute( "irradiance:nsamples", ( int )g.rays.irradiance.samples );
														theRenderer.attribute( "irradiance:shadingrate", ( float )g.rays.irradiance.shadingRate );
														theRenderer.attribute( "trace:bias", ( float )g.rays.trace.bias );
														theRenderer.attribute( "trace:samplemotion", g.rays.trace.motion );
														theRenderer.attribute( "trace:displacements", g.rays.trace.displacements );
													} else {
														theRenderer.option( "trace:maxdepth", 0 );
													}
													theRenderer.attribute( "subsurface:shadingrate", ( float )g.rays.subSurface.rate );

													theRenderer.attribute( "identifier:name", CStringToString( object.GetFullName() ) );

													theRenderer.parameter( "near", ( float )prop.GetParameterValue( L"NearClip", g.animation.time ) );
													theRenderer.parameter( "far", ( float )prop.GetParameterValue( L"FarClip", g.animation.time ) );

													float timeOrigin( globals::motionBlur::shutterMoving == g.motionBlur.shutterConfiguration ? g.motionBlur.shutterOffset + g.animation.time : g.motionBlur.shutterOffset );
													float samples[ 2 ] = { timeOrigin + g.motionBlur.shutterOpen, timeOrigin + g.motionBlur.shutterClose };
													theRenderer.parameter( tokenValue( samples, 2, "shutter", tokenValue::storageUndefined, tokenValue::typeFloat ) );

													theRenderer.parameter( "shutteroffset", g.motionBlur.shutterOffset );

													float shutterEfficiency[ 2 ] = { g.motionBlur.shutterEfficiency, g.motionBlur.shutterEfficiency };
													theRenderer.parameter( tokenValue( shutterEfficiency, 2, "shutterefficiency", tokenValue::storageUndefined, tokenValue::typeFloat ) );

													theRenderer.parameter( "jitter", g.reyes.jitter );

													theRenderer.parameter( "extrememotiondof", g.reyes.extremeMotionDepthOfField );

													theRenderer.parameter( "bucketorder", g.reyes.bucketorder );

													int bucket[ 2 ] = { g.reyes.bucketSize.x, g.reyes.bucketSize.y };
													theRenderer.parameter( tokenValue( bucket, 2, "bucketsize", tokenValue::storageUndefined ) );

													theRenderer.parameter( "texturememory", ( int )g.reyes.textureMemory );

													theRenderer.parameter( "gridsize", ( int )g.reyes.gridSize );

													theRenderer.parameter( "eyesplits", ( int )g.reyes.eyeSplits );

													int res[ 2 ];
													res[ 0 ] = res[ 1 ] = ( unsigned long )prop.GetParameterValue( L"Resolution", g.animation.time );
													theRenderer.parameter( tokenValue( res, 2, "resolution", tokenValue::storageUndefined ) );

													theRenderer.parameter( "eyesplits", ( int )g.reyes.eyeSplits );

													theRenderer.parameter( "projection", string( "perspective" ) );

													theRenderer.parameter( "fov", ( float )95.0f );

													theRenderer.camera( camHandle );


													theRenderer.space( CMatrix4ToFloat( object.GetKinematics().GetGlobal().GetTransform( g.animation.time ).GetMatrix4() ) );
													switch( camera ) {
														case 0:
															theRenderer.rotate( -90, 0, 1, 0 ); // +X
															break;
														case 1:
															theRenderer.rotate(  90, 0, 1, 0 ); // -X
															break;
														case 2:
															theRenderer.rotate(  90, 1, 0, 0 ); // +Y
															break;
														case 3:
															theRenderer.rotate( -90, 1, 0, 0 ); // -Y
															break;
														case 5:
															theRenderer.rotate( 180, 0, 1, 0 ); // -Z
															break;
													}

													theRenderer.input( g.data.worldBlockName.native_file_string() );

													bm.addMapRenderIoToCurrentBlock( envMapWriteNameGlobal, envMapName );

													bm.endBlock( ctx, 200 );
												}

												// Create data file to convert texture
												envName = objectName + ".environment";
												string envMapWriteNameGlobal( getCacheFilePath( envDir / jobNameGlobal, g.directories.caching.mapWrite ).native_file_string() );

												bm.beginBlock( blockManager::blockMapScene, true, envName );

												context ctx( bm.currentContext() );

												for( unsigned i = 0; i < 6; i++ ) {
													theRenderer.parameter( paramName[ i ], ( string )mapFace[ i ] );
												}

												theRenderer.parameter( "mapname", envMapSourceName );

												theRenderer.makeMap( "environment" );

												bm.addMapRenderIoToCurrentBlock( envMapWriteNameGlobal, envMapSourceName );

												bm.endBlock( ctx, 101 );

												value = envMapSourceName;
											}
											break;
										}
									} // for( ... props.GetCount() )
								} // if( objects.GetCount() )
								break;
							} // case mapEnvironment:
						} // switch( mapType )

						tokenValuePtrArray.push_back( shared_ptr< tokenValue>( new tokenValue( value, *it ) ) );
						break;
					} // case 7: -- string
				}
			}

			displacementSphere = ( float )aShader.GetParameterValue( L"___DisplacementBound" );
			displacementSpace = CStringToString( aShader.GetParameterValue( L"___DisplacementSpace" ) );


		} else {
			type = shaderUndefined;
		}
	}

	void shader::set( const Parameter &aShader ) {
		globals& g = const_cast< globals& >( globals::access() );

		string paramName( CStringToString( aShader.GetName() ) );
		to_lower( paramName );

		type = shaderUndefined;

		if( CValue::siString == aShader.GetValue( g.animation.time ).m_t ) {
			name = CStringToString( aShader.GetValue( g.animation.time ) );
			if( "surface" == paramName ) {
				type = shaderSurface;
			} else
			if( "displacement" == paramName ) {
				type = shaderDisplacement;
			} else
			if( "volume" == paramName ) {
				type = shaderVolume;
			}
		}
	}

	void shader::addParameter( const tokenValue &aTokenValue ) {
	}

	bool shader::isValid() const {
		return shaderUndefined != type;
	}

	shader::shaderType shader::getType() const {
		return type;
	}

	void shader::write( const string &aHandle ) {
		if( isValid() ) {
			globals& g = const_cast< globals& >( globals::access() );

			ueberManInterface theRenderer;

			if( ( shaderDisplacement == type ) || ( shaderSurface == type ) ) {
				if( 0 <= displacementSphere ) {
					theRenderer.attribute( "displacementbound:coordinatesystem", displacementSpace );
					theRenderer.attribute( "displacementbound:sphere", displacementSphere );
					//message( L"Displacement Bound: " + CValue( displacementSphere ).GetAsText(), messageInfo );
				}
			}

			if( g.data.sections.shaderParameters ) {
				if( !g.data.sections.shaderNumericParameters ) {
					debugMessage( L"Only writing strings" );
					for( tokenValue::tokenValuePtrVector::const_iterator it = tokenValuePtrArray.begin(); it < tokenValuePtrArray.end(); it++ ) {
						if( tokenValue::typeString == ( *it )->type() )
							theRenderer.parameter( *( *it ) );
					}
				} else {
					for( tokenValue::tokenValuePtrVector::const_iterator it = tokenValuePtrArray.begin(); it < tokenValuePtrArray.end(); it++ )
						theRenderer.parameter( *( *it ) );
				}
			}

			Application app;

			string strippedName( name );
			if( !g.searchPath.shaderPaths ) {
				size_t pos( name.rfind( "/" ) );

				if( string::npos != pos )
					strippedName = name.substr( pos + 1 );
			}

			string handle( aHandle );
			if( handle.empty() )
				handle = "noHandle";

			if( shaderLight == type ) {

				theRenderer.light( strippedName, handle );
			} else {
				string shaderTypeName;
				switch( type ) {
					case shaderSurface:
						shaderTypeName = "surface";
						break;
					case shaderDisplacement:
						shaderTypeName = "displacement";
						break;
					case shaderVolume:
						shaderTypeName = "volume";
						break;
					case shaderTransformation:
						shaderTypeName = "transformation";
						break;
					case shaderDeformation:
						shaderTypeName = "deformation";
						break;
					case shaderImager:
						shaderTypeName = "imager";
						break;
				}

				theRenderer.shader( shaderTypeName, strippedName, handle );
			}
		}
	}
}

using namespace XSI;
using namespace affogato;
using namespace boost;
using namespace std;



bool findAndSetSloShader( string& shaderFile ) {
	Application app;

	string shaderNameStr( shaderFile );

	int found( !Slo_SetShader( shaderFile.c_str() ) );

	if( !found ) {
		Property affogatoGlobals;
		CRefArray props( app.GetActiveSceneRoot().GetProperties() );
		for( long k = 0; k < props.GetCount(); k++ ) {
			Property prop( props[ k ] );
			if( CString( L"AffogatoGlobals" ) == prop.GetType() ) {
				affogatoGlobals = prop;
				break;
			}
		}
		if( affogatoGlobals.IsValid() ) {
			string search = CStringToString( affogatoGlobals.GetParameterValue( L"ShaderPath" ) );

			typedef tokenizer< char_separator< char > > tokenizer;

			char_separator< char > sep( ":" );
			tokenizer tokens( search, sep );

			for( tokenizer::iterator it = tokens.begin(); it != tokens.end(); it++ ) {
				string path = *it;
				if( ( path.rfind( "/" ) != path.length() - 1 ) ) {
					path += "/";
				}

				string shaderNameStr( path + shaderFile );

				if( !Slo_SetShader( shaderNameStr.c_str() ) ) {
					found = true;
					break;
				}
			}
		}
	}

	if( !found ) {
		string search( getEnvironment( "SHADERS" ) );
		if( !search.empty() ) {
			typedef tokenizer< char_separator< char > > tokenizer;

#ifdef _WIN32
			char_separator< char > sep( ";" );
#else
			char_separator< char > sep( ":" );
#endif
			tokenizer tokens( search, sep );

			for( tokenizer::iterator it = tokens.begin(); it != tokens.end(); it++ ) {
				string path = *it;
				if( ( path.rfind( "/" ) != path.length() - 1 ) ) {
					path += "/";
				}

				string shaderNameStr( path + shaderFile );

				if( !Slo_SetShader( shaderNameStr.c_str() ) ) {
					found = true;
					break;
				}
			}
		}
	}

	if( !found ) {
		string search( getEnvironment( "DL_SHADERS_PATH" ) );
		if( !search.empty() ) {
			typedef tokenizer< char_separator< char > > tokenizer;

#ifdef _WIN32
			char_separator< char > sep( ";" );
#else
			char_separator< char > sep( ":" );
#endif
			tokenizer tokens( search, sep );

			for( tokenizer::iterator it = tokens.begin(); it != tokens.end(); it++ ) {
				string path = *it;
				if( ( path.rfind( "/" ) != path.length() - 1 ) ) {
					path += "/";
				}

				string shaderNameStr( path + shaderFile );

				if( !Slo_SetShader( shaderNameStr.c_str() ) ) {
					found = true;
					break;
				}
			}
		}
	}

	shaderFile = shaderNameStr;

	return found != 0;
}

XSIPLUGINCALLBACK CStatus AffogatoCreateShader_Init( const XSI::CRef &in_context ) {
	Context ctxt( in_context );
	Command cmd( ctxt.GetSource() );

	cmd.EnableReturnValue( true );

	Application app;
	message( L"Adding " + cmd.GetName() + L" Command.", messageInfo );

	ArgumentArray args = cmd.GetArguments();
	args.Add( L"Shader File Name", L"" );
	args.Add( L"Target Object", L"" );
	args.Add( L"Strip Path", false );
	args.Add( L"Select Shader", false );

	return CStatus::OK;
}



XSIPLUGINCALLBACK CStatus AffogatoCreateShader_Execute( const CRef &inContext ) {

	Application app;

	Context ctx( inContext );
	CValueArray args = ctx.GetAttribute( L"Arguments" );

	string shaderFile;
	try {
#ifdef _WIN32
		filesystem::path shaderFilePath( CStringToString( args[ 0 ] ), filesystem::native );
#else
		filesystem::path shaderFilePath( CStringToString( args[ 0 ] ), filesystem::windows_name );
#endif
		shaderFile = shaderFilePath.string();
	} catch( ... ) {
		shaderFile = CStringToString( args[ 0 ] );
	}

	if( shaderFile.empty() ) {
		message( L"AffogatoCreateShader: Called without specifying a shader file to parse. No shader created.", messageError );
		return CStatus::Fail;
	}

	if( findAndSetSloShader( shaderFile ) ) {

		SceneItem object;
		object = SceneItem( CRef( args[ 1 ] ) );

		if( !object.IsValid() ) {
			Selection selected = app.GetSelection();

			if( !selected.GetCount() ) {
				object = app.GetActiveSceneRoot();
			} else {
				object = selected[ 0 ];
				message( L"AffogatoCreateShader:   Creating shader under " + object.GetFullName(), messageInfo );
			}
		}

		// Build the shader name (strip the path)
		size_t pos0 = shaderFile.rfind( "/" );
		if( string::npos == pos0 )
			pos0 = 0;
		else
			++pos0;

		size_t pos1 = shaderFile.rfind( "." );
		if( ( string::npos == pos1 ) || ( pos1 < pos0 ) )
			pos1 = shaderFile.length();

		string shaderName( shaderFile.substr( pos0, pos1 - pos0 ) );
		string ribShaderName( shaderFile.substr( 0, pos1 ) );

		if( args[ 2 ] ) {
			size_t pos( ribShaderName.rfind( "/" ) );
			if( string::npos != pos )
				ribShaderName = ribShaderName.substr( pos + 1 );
		}

		// Find the shader type
		SLO_TYPE shaderType = Slo_GetType();
		CString shaderTypeName;
		shader::shaderType numShaderType;
		switch( shaderType ) {
			case SLO_TYPE_SURFACE:
				shaderTypeName = L"Surface";
				numShaderType = shader::shaderSurface;
				break;
			case SLO_TYPE_LIGHT:
				shaderTypeName = L"Light";
				numShaderType = shader::shaderLight;
				break;
			case SLO_TYPE_DISPLACEMENT:
				shaderTypeName = L"Displacement";
				numShaderType = shader::shaderDisplacement;
				break;
			case SLO_TYPE_VOLUME:
				shaderTypeName = L"Volume";
				numShaderType = shader::shaderVolume;
				break;
			case SLO_TYPE_TRANSFORMATION:
				shaderTypeName = L"Transformation";
				numShaderType = shader::shaderTransformation;
				break;
			case SLO_TYPE_IMAGER:
				shaderTypeName = L"Imager";
				numShaderType = shader::shaderImager;
				break;
		}

		// Create the custom property
		CustomProperty prop = ( CRef )object.AddProperty( L"Custom_parameter_list", false, stringToCString( shaderName ) );
		Parameter param;
		PPGLayout oLayout = prop.GetPPGLayout();

		// Create the special shader ID parameters
		char typeStr[ 2 ];
		sprintf( typeStr, "%1d", numShaderType );
		prop.AddParameter(	__AFFOGATO_SHADER_ID, CValue::siString, siReadOnly | siPersistable | siNotInspectable,
							L"Affogato Shader ID", L"",
							stringToCString( string( typeStr ) + ";" + shaderName ), param );

		oLayout.AddGroup( L"Shader", true );
		prop.AddParameter(	L"___ShaderFile", CValue::siString, siReadOnly | siPersistable,
							L"Shader File", L"",
							stringToCString( ribShaderName ), param );
		PPGItem item;
		item = oLayout.AddItem( L"___ShaderFile", L"Shader File" );
		item.PutLabelMinPixels( LABEL_WIDTH_WIDE );
		oLayout.EndGroup();

		// Add light specific stuff
		if( SLO_TYPE_LIGHT == shaderType ) {
			oLayout.AddGroup( L"Light", true );
			item = oLayout.AddItem( L"__category", L"Light Category" );
			item.PutLabelMinPixels( LABEL_WIDTH_WIDE );
			oLayout.AddRow();
			item = oLayout.AddItem( L"__nonspecular", L"Specular Fadeout" );
			item.PutAttribute( siUIDecimals, 2l );
			item.PutLabelMinPixels( LABEL_WIDTH_WIDE );
			item = oLayout.AddItem( L"__nondiffuse", L"Diffuse Fadeout" );
			item.PutAttribute( siUIDecimals, 2l );
			item.PutLabelMinPixels( LABEL_WIDTH_WIDE );
			oLayout.EndRow();
			oLayout.EndGroup();
		// Add surface/displacement specific stuff
		} else if( ( SLO_TYPE_DISPLACEMENT == shaderType ) || ( SLO_TYPE_SURFACE == shaderType ) ) {
			oLayout.AddGroup( L"Displacement", true );
			oLayout.AddRow();
			prop.AddParameter(	L"___DisplacementBound", CValue::siFloat, siAnimatable,
								L"Displacement Bound", L"",
								0.0f, -1.0f, 1e38, -1.0f, 1.0f, param );

			item = oLayout.AddItem( L"___DisplacementBound", L"Bound" );
			item.PutLabelMinPixels( LABEL_WIDTH_WIDE );
			item.PutAttribute( siUIThumbWheel, true );

			prop.AddParameter(	L"___DisplacementSpace", CValue::siString, siAnimatable,
								L"Displacement Space", L"",
								L"shader", param );

			item = oLayout.AddItem( L"___DisplacementSpace", L"Space" );
			item.PutLabelMinPixels( LABEL_WIDTH_WIDE );

			oLayout.EndRow();
			oLayout.EndGroup();
		}

		// Create property & layout
		oLayout.AddGroup( L"RenderMan " + shaderTypeName + L" Shader", true );

		CString paramString;
		for( int j = 1; j < Slo_GetNArgs() + 1; j++ ) {
			SLO_VISSYMDEF *parameter = Slo_GetArgById( j );
			if( !parameter || !parameter->svd_valisvalid )
				continue;

			CString paramName = charToCString( parameter->svd_name );
			//CString const char *detail = Slo_DetailtoStr( parameter->svd_detail );

			//const char *type = Slo_TypetoStr( parameter->svd_type );
			unsigned arraylen = parameter->svd_arraylen;

			Parameter subParam;
			if( SLO_STOR_OUTPUTPARAMETER != parameter->svd_storage ) {

				int caps = siAnimatable | siPersistable;

				int num_elements = arraylen == 0 ? 1 : arraylen;
				for( int k = 0; k < num_elements; k++ ) {
					SLO_VISSYMDEF *elem = Slo_GetArrayArgElement( parameter, k );
					unsigned arraylen = parameter->svd_arraylen;

					if( !elem || arraylen ) // we don't handle arrays yet
						continue;

					switch( parameter->svd_type ) {
						case SLO_TYPE_SCALAR:
							if( ( CString( L"__nonspecular" ) == paramName ) || ( CString( L"__nondiffuse" ) == paramName ) ) {
								prop.AddParameter(	paramName, CValue::siFloat, caps,
													L"", L"", *elem->svd_default.scalarval, 0.0f, 1.0f, 0.0f, 1.0f, param );
							} else {
								prop.AddParameter(	paramName, CValue::siFloat, caps,
													L"", L"", *elem->svd_default.scalarval, param );
								item = oLayout.AddItem( paramName, paramName );
								item.PutLabelMinPixels( LABEL_WIDTH_WIDE );
							}

							paramString += L"1;" + paramName + L";";

							break;
						case SLO_TYPE_COLOR: {

							prop.AddParameter( paramName + L"R", CValue::siDouble, caps,
												L"", L"", elem->svd_default.pointval->xval, param );
							prop.AddParameter( paramName + L"G", CValue::siDouble, caps,
												L"", L"", elem->svd_default.pointval->yval, param );
							prop.AddParameter( paramName + L"B", CValue::siDouble, caps,
												L"", L"", elem->svd_default.pointval->zval, param );

							item = oLayout.AddColor( paramName + L"R", paramName );

							paramString += L"5;" + paramName + L";";


							break;
						}
						case SLO_TYPE_STRING:
							prop.AddParameter(	paramName, CValue::siString, siPersistable,
												L"", L"", charToCString( elem->svd_default.stringval ), param );
							if( CString( L"__category" ) != paramName ) {
								item = oLayout.AddItem( paramName, paramName, siControlFilePath );
								item.PutAttribute( siUIOpenFile, true );
								item.PutAttribute( siUIFileFilter, L"Mip-Mapped TIFF Textures (*.tdl)|*.tdl|TIFF Textures (*.tif)|*.tif|All Files (*.*)|*.*||" );
								item.PutLabelMinPixels( LABEL_WIDTH_WIDE );
								//item = oLayout.AddItem( paramName, paramName );
							}

							paramString += L"7;" + paramName + L";";
							break;
						default:
							break;
					}
				}
			}
		}
		Slo_EndShader();

		prop.AddParameter(	__AFFOGATO_SHADER_PARAMS, CValue::siString, siReadOnly | siPersistable | siNotInspectable,
							L"", L"", paramString, param );

		oLayout.EndGroup();

		ctx.PutAttribute( L"ReturnValue", CValue( prop ) );

		if( args[ 3 ] ) {
			CValueArray args( 3 );
			CValue retval;
			args[ 0 ]= CValue( prop );
			args[ 1 ]= CValue();
			args[ 2 ]= CValue();
			CStatus ist = app.ExecuteCommand( L"SelectObj", args, retval );
		}

		return CStatus::OK;
	} else {
		message( L"AffogatoCreateShader: Unable to open shader '" + CString( args[ 0 ] ) + L"'. Exiting.", messageError );
		return CStatus::MemberNotFound;
	}

}


XSIPLUGINCALLBACK CStatus AffogatoReloadShader_Init( const XSI::CRef &in_context ) {
	Context ctxt( in_context );
	Command cmd( ctxt.GetSource() );

	cmd.EnableReturnValue( true );

	Application app;
	message( L"Adding " + cmd.GetName() + L" Command.", messageInfo );

	ArgumentArray args = cmd.GetArguments();
	args.Add( L"AffogatoShaderPropery", L"" );
	args.Add( L"NewShaderPath", L"" );
	args.Add( L"StripPath", false );

	return CStatus::OK;
}

/*
XSIPLUGINCALLBACK CStatus AffogatoReloadShader_Execute( const CRef &inContext ) {
	Application app;

	Context ctx( inContext );
	CValueArray args = ctx.GetAttribute( L"Arguments" );

	SceneItem object;
	object = SceneItem( CRef( args[ 0 ] ) );

	if( !object.IsValid() ) {
		Selection selected = app.GetSelection();

		if( !selected.GetCount() ) {
			object = app.GetActiveSceneRoot();
		} else {
			object = selected[ 0 ];
			message( L"AffogatoCreateShader:   Reloading shader in " + object.GetFullName(), messageInfo );
		}
	}

	Property prop( object );
	if( prop.IsValid() ) {

		string shaderFile = CStringToString( args[ 1 ] );
		if( shaderFile.empty() )
			shaderFile = object.GetParameterValue( "" )

		if( !findAndSetSloShader( shaderFile ) ) {
			app.LogMessage( L"AffogatoCreateShader: Unable to open shader '" + CString( args[ 0 ] ) + L"'. Exiting.", siErrorMsg );
			return CStatus::MemberNotFound;
		}


	}

	return CStatus::OK;
}*/

/*
XSIPLUGINCALLBACK CStatus AffogatoShader_DefineLayout( const CRef &inContext ) {

	Application app;
	CustomProperty prop = Context( inContext ).GetSource();

	Parameter param( prop.GetParameter( L"ShaderFile" ) );
	string name = CStringToString( ( CString )param.GetValue() );

	PPGLayout oLayout( Context( inContext ).GetSource() );
	PPGItem item;

	if( !name.length() ) {

		oLayout.Clear();

		item = oLayout.AddItem( L"ShaderFile", L"Shader File", siControlFilePath );
		item.PutAttribute( siUIFileMustExist, true );
		item.PutAttribute( siUIOpenFile, true );
#ifndef unix
		item.PutAttribute( siUIInitialDir, L"c:\\" );
#else
		item.PutAttribute( siUIInitialDir, L"~" );
#endif
		// This decides what file extentions to display
		item.PutAttribute( siUIFileFilter, L"Shader files (*.sdl)|*.sdl|All Files (*.*)|*.*||" );

		return CStatus::OK;
	}



	return CStatus::OK;
}*/

/*
XSIPLUGINCALLBACK CStatus AffogatoShader_PPGEvent( const CRef& io_Ctx ) {
	// This callback is called when events happen in the user interface
	// This is where you implement the "logic" code.

	// This code only processes the events that it is interested in,
	// other events are ignored

	// If the value of a parameter changes but the UI is not shown then this
	// code will not execute.  Also this code is not re-entrant, so any changes
	// to parameters inside this code will not result in further calls to this function

	Application app;

	// The context object is used to determine exactly what happened
	// We don't use the same "PPG" object that is used from Script-based logic code
	// but through the C++ API we can achieve exactly the same functionality.
	PPGEventContext ctx( io_Ctx );

	PPGEventContext::PPGEvent eventID = ctx.GetEventID();

	if ( PPGEventContext::siOnInit == eventID )	{
		// This event meant that the UI was just created.
		// It gives us a chance to set some parameter values.
		// We could even change the layout completely at this point.

		// For this event Source() of the event is the CustomProperty object

		CustomProperty prop = ctx.GetSource() ;
		prop.PutParameterValue( L"Selection", app.GetSelection().GetCount() > 0l ) ;

		app.LogMessage( L"Inspected object " + prop.GetName() + L" has " +
						CValue( prop.GetParameters().GetCount() ).GetAsText() + L" parameters" ) ;


		prop.PutParameterValue( L"MultiLine", CString( L"From C++ use \\r\\n\r\nto break a string into multiple lines."
								L"\r\n\r\n\t\\t is useful too\r\n"
								L"Use \\\" and \\\' to produce \" and \'" ));

		//Demonstrate how to use the PPGLayout to populate the items inside the ComboBox
		PPGLayout oPPGLayout = prop.GetPPGLayout();
		PPGItem oPPGItem = oPPGLayout.GetItem(L"DynamicCombo");

		//Replacing the Default string that was originally defined at the DefineLayout Callback
		CValueArray newDynamicComboItem( 4 ) ;
			newDynamicComboItem[0] = L"StringLabel1"; newDynamicComboItem[1] = L"StringValue1" ;
			newDynamicComboItem[2] = L"StringLabel2"; newDynamicComboItem[3] = L"StringValue2" ;

		oPPGItem.PutUIItems(newDynamicComboItem);

		//Setting the value for the DynamicCombo to the second value "StringValue2"
		prop.GetParameter(L"DynamicCombo").PutValue(L"StringValue2");

		//Redraw the PPG to show the new combo items
		ctx.PutAttribute( L"Refresh",true );
*//*
	} else if ( PPGEventContext::siParameterChange == eventID ) {
		// For this event the Source of the event is the parameter
		// itself
		Parameter param = ctx.GetSource() ;

		// But we have no trouble getting at the CustomProperty
		// that owns the changed parameter
		CustomProperty prop = param.GetParent() ;
		CString paramName = param.GetScriptName() ;


		// Application.ActiveSceneRoot.AddProperty("AffogatoShader")
		if ( paramName == L"ShaderFile" ) {

			CString name = param.GetValue();

			PPGLayout oLayout = prop.GetPPGLayout();


			if( name.Length() ) {

				oLayout.Clear();

				PPGItem item;

				prop.AddParameter(	L"ShaderFile", CValue::siString, siPersistable,
									L"Shader File", L"",
									name, param );
				item = oLayout.AddItem( L"ShaderFile", L"Shader File", siControlFilePath );

				//oLayout.AddGroup( charToCString( Slo_TypetoStr( Slo_GetType() ) ) + L" " + charToCString( Slo_GetName() ), true );

				int err = Slo_SetShader( CStringToString( name ).c_str() );
				if( err != 0 ) {
					app.LogMessage( L"Affogato: Unable to open shader '" + name + L"'!" );
					return CStatus::Fail;
				}

				//return CStatus::OK;

				int j, k, kk;
				for( int j = 1; j < Slo_GetNArgs() + 1; j++ ) {
					SLO_VISSYMDEF *parameter = Slo_GetArgById( j );
					if( !parameter || !parameter->svd_valisvalid )
						continue;

					CString paramName = charToCString( parameter->svd_name );
					//CString const char *detail = Slo_DetailtoStr( parameter->svd_detail );

					//const char *type = Slo_TypetoStr( parameter->svd_type );
					unsigned arraylen = parameter->svd_arraylen;

					/*if( parameter->svd_storage == SLO_STOR_OUTPUTPARAMETER )
						printf( " \"%s\" \"output %s %s", name, detail, type );
					else
						printf( " \"%s\" \"%s %s", name, detail, type );


					if( arraylen > 0 )
						printf( "[%d]", arraylen );

					printf( "\"\n" );
					printf( "\t\tDefault value: " );
					switch( parameter->svd_type ) {
						case SLO_TYPE_COLOR:
						// Even if color has a different spacename,
						// it has been converted by evaluation.
						printf( "\"rgb\" ");
						break;
						case SLO_TYPE_POINT:
						case SLO_TYPE_VECTOR:
						case SLO_TYPE_NORMAL:
						case SLO_TYPE_MATRIX:
							if( parameter->svd_spacename[0] != (char)0 )
								printf( "\"%s\" ", parameter->svd_spacename );
							break;
						default:
							break;
					}


					if( arraylen > 0 ) {
						printf( "{" );
					}


					int num_elements = arraylen == 0 ? 1 : arraylen;
					for( k = 0; k < num_elements; k++ ) {
						SLO_VISSYMDEF *elem = Slo_GetArrayArgElement( parameter, k );

						if( !elem )	{
							printf( "<error>" );
							continue;
						}

						switch( parameter->svd_type ) {
							case SLO_TYPE_SCALAR:
								param = prop.GetParameter( paramName );
								if( param.GetValue().m_t != CValue::siFloat ) {
									prop.RemoveParameter( param );
									prop.AddParameter(	paramName, CValue::siFloat, siAnimatable,
														L"", L"",
														*elem->svd_default.scalarval, param );
								}

								item = oLayout.AddItem( paramName, paramName );
								item.PutLabelMinPixels( 96 );

								break;
						/*	case SLO_TYPE_POINT:
							case SLO_TYPE_VECTOR:
							case SLO_TYPE_NORMAL:
							case SLO_TYPE_COLOR:
								printf( "[%g %g %g]",
								elem->svd_default.pointval->xval,
								elem->svd_default.pointval->yval,
								elem->svd_default.pointval->zval );
								break;
							case SLO_TYPE_MATRIX:
								printf( "[%g ", elem->svd_default.matrixval[0] );
								for( kk = 1; kk < 15; kk++ )
								printf( "%g ", elem->svd_default.matrixval[kk] );
								printf( "%g]", elem->svd_default.matrixval[15] );
								break;
							case SLO_TYPE_STRING:
								printf( "\"%s\"", elem->svd_default.stringval );
								break;
							default:
								break;
						}
						/*
						if( k != (num_elements-1) )	{
							printf( ", " );
						}
					}
					/*if( arraylen > 0 )
						printf( "}" );

					printf( "\n" );
				}

				oLayout.EndGroup();

				ctx.PutAttribute( L"Refresh", true );
			}
		}

		app.LogMessage( L"AffogatoShader_PPGEvent called for ParameterChange: "
						+ prop.GetName() + L" " + paramName );
	}

	return CStatus::OK;
}


/*
int main( int argc, char ** argv ) {
	int i;
	int exitCode = 0;
	bool RIBDeclare = false;
	bool printAnnotationIndex = false;
	bool printAnnotation = false;
	const char *annotationKey;
	int start = 1;
	// DlDebug::InitErrorSignalsHandling();
	if ( argc <= 1 || !strcmp(argv[1], "-h") || !strcmp(argv[1], "--help") ) {
		printf(
				"Usage: shaderinfo [<option>] [file1 ... fileN].\n"
				"Options\n"
				" -d : Outputs declarations in RIB format\n"
				" -ai : Outputs available annotation keys usable with -a\n"
				" -a <key> : Outputs <key> annotation.\n"
				" -v : Shows version to console.\n"
				" --version : Same as -v.\n"
				" -h : Shows this help.\n\n"
				"Notes:\n"
				"- No arguments is the equivalent of passing -h.\n"
				"- All options are exclusive. Use no more than one.\n"
				"- The options -h, -v, and --version should not be used with shader names.\n"
				);
		return 0;
	}

	if( argc == 2 && (!strcmp(argv[1], "-v") || !strcmp(argv[1], "--version")) ) {
		printf( "shaderinfo version 3.0.\n" );
		printf( "Copyright (c) 1999-2004 The 3Delight Team.\n" );
		return 0;
	}

	if ( argc == 1 ) {
		start++;
	} else if (!strcmp(argv[1], "-d")) {
		RIBDeclare = true;
		start++;
	} else if (!strcmp(argv[1], "-ai"))	{
		printAnnotationIndex = true;
		start++;
	} else if (!strcmp(argv[1], "-a")) {
		printAnnotation = true;
		start++;
		annotationKey = 0;
		if (argc >= 2) {
			annotationKey = argv[2];
			start++;
		} else {
			fprintf(
					stderr,
					"shaderinfo: no annotation key specified\n");
			return 1;
		}
	}

	const char* path = getenv( "DL_SHADERS_PATH" );
	Slo_SetPath( path ? path : "." );
	for( i = start; i < argc; i++ ) {
		int err = Slo_SetShader( argv[i] );
		if( err != 0 ) {
		fprintf(
				stderr,
				"shaderinfo: unable to open shader \"%s\" (error %d).\n",
				argv[i], err );
		exitCode = 1;
		continue;
		}
		printf( "\n%s \"%s\"\n", Slo_TypetoStr(Slo_GetType()), Slo_GetName() );
		if (printAnnotationIndex) {
			int i = 0;
			const char *key;
			key = Slo_GetAnnotationKeyById(i);
			if (!key) {
				fprintf(
						stderr,
						"shaderinfo: no keys\n");
				continue;
			} else {
				while(key) {
					printf( "\t\"%s\"\n", key);
					i++;
					key = Slo_GetAnnotationKeyById(i);
				}
				printf( "\n");
			}
		} else if (printAnnotation)	{
			const char *value = Slo_GetAnnotationByKey(annotationKey);
			if (value) {
				printf("\t\"%s\"\n", value);
			} else {
				fprintf(
						stderr,
						"shaderinfo: unknown key \"%s\"\n",
						annotationKey);
			}
		} else {
			printArgs(RIBDeclare);
		}

		Slo_EndShader();
		printf( "\n" );
	}
	return exitCode;
}*/

