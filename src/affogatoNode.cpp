/** Affogato node class stores data for a single scene graph item.
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *  @par
 *  You should have received a copy of the GNU Lesser General Public
 *  License (http://www.gnu.org/licenses/lgpl.txt) along with this
 *  library; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

// Boost headers
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/filesystem/operations.hpp>

// XSI headers
#include <xsi_application.h>
#include <xsi_geometry.h>
#include <xsi_group.h>
#include <xsi_ref.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>
#include <xsi_material.h>
#include <xsi_model.h>
#include <xsi_parameter.h>
#include <xsi_shader.h>
#include <xsi_string.h>
#include <xsi_value.h>
#include <xsi_x3dobject.h>

// Affohato headers
#include "affogatoAttribute.hpp"
#include "affogatoData.hpp"
#include "affogatoGlobals.hpp"
#include "affogatoHairData.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoSphereData.hpp"
#include "affogatoNode.hpp"
#include "affogatoNurbCurveData.hpp"
#include "affogatoNurbMeshData.hpp"
#include "affogatoParticleData.hpp"
#include "affogatoPolyMeshData.hpp"
#include "affogatoRenderer.hpp"
#include "affogatoShader.hpp"


// RenderMan headers
#include <ri.h> // For RiArchiveRecord() :(

namespace affogato {

	using namespace XSI;
	using namespace ueberMan;
	using namespace std;
	using namespace boost;

	context node::lookContext;

	void node::setLookContext( const context& ctx ) {
		lookContext = ctx;
	}

	void node::scanForAttributes( const Property& prop, map< string, tokenValue::tokenValuePtr >& attribMap, shared_ptr< shader >& surface, shared_ptr< shader >& displacement, shared_ptr< shader >& volume ) {
		const globals& g( globals::access() );

		if( !g.defaultShader.overrideAll && shader::isShader( prop ) ) {
			shared_ptr< shader > tmpShader( new shader( prop ) );
			switch( tmpShader->getType() ) {
				case shader::shaderSurface:
					if( !surface )
						surface =tmpShader;
					else
						message( L"More than one surface shader in group", messageWarning );
					break;
				case shader::shaderDisplacement:
					if( !displacement )
						displacement = tmpShader;
					else
						message( L"More than one displacement shader in group", messageWarning );
					break;
				case shader::shaderVolume:
					if( !volume )
						volume = tmpShader;
					else
						message( L"More than one volume shader in group", messageWarning );
					break;
			}
		} else {

			CParameterRefArray params = prop.GetParameters();

			for( unsigned p( 0 ); p < ( unsigned )params.GetCount(); p++ ) {
				Parameter param( params[ p ] );
				string paramName( CStringToString( param.GetName() ) );

				string userParamName( paramName );
				replace_first( paramName, string( "_" ), string( ":" ) );
				to_lower( paramName );

				debugMessage( L"Scanning for special parameters" );

				if( "transformsegments" == paramName ) {
					if( transformMotionSamples == g.motionBlur.transformMotionSamples ) {
						transformMotionSamples = ( long )param.GetValue( g.animation.time ) + 1;
						if( transformMotionSamples > MAXMOTIONSAMPLES ) {
							transformMotionSamples = MAXMOTIONSAMPLES;
						}
					}
				} else
				if( "deformsegments" == paramName ) {
					if( deformMotionSamples == g.motionBlur.deformMotionSamples ) {
						deformMotionSamples = ( long )param.GetValue( g.animation.time ) + 1;
						if( deformMotionSamples > MAXMOTIONSAMPLES ) {
							deformMotionSamples = MAXMOTIONSAMPLES;
						}
					}
				} else
				if( "surface" == paramName ) {
					if( NULL == surface ) {
						shader tmpShader( param );
						if( tmpShader.isValid() )
							surface = shared_ptr< shader >( new shader( tmpShader ) );
					}
				} else
				if( "displacement" == paramName ) {
					if( NULL == displacement ) {
						shader tmpShader( param );
						if( tmpShader.isValid() )
							displacement = shared_ptr< shader >( new shader( tmpShader ) );
					}
				} else
				if( "volume" == paramName ) {
					if( NULL == volume ) {
						shader tmpShader( param );
						if( tmpShader.isValid() )
							volume = shared_ptr< shader >( new shader( tmpShader ) );
					}
				} else
#ifdef RSP
				if( g.data.doHub && ( "hdb" == paramName ) ) {
					if( !hdb ) {
						hdb = ( long )param.GetValue( g.animation.time );
						message( L"getting HDB: " +  CValue( (long) hdb ).GetAsText(), messageInfo );
					}
					type = nodeHub;
				} else
				if( g.data.doHub && ( "hub" == paramName ) ) {
					if( !hub ) {
						hub = ( long )param.GetValue( g.animation.time );
						message( L"getting HUB: " +  CValue( (long) hub ).GetAsText(), messageInfo );
					}
					type = nodeHub;
				} else
#endif
				if( "archive" == paramName ) {
					// Handle archive stuff
					if( archive.empty() ) {
						param = prop.GetParameter( L"archive" );
						if( param.GetValue( g.animation.time ).m_t == CValue::siString ) {
							string value = CStringToString( param.GetValue( g.animation.time ) );
							value = parseString( value );
							if( !value.empty() ) {
#ifdef RSP
								if( value.substr( value.length() - 4, 4 ) == ".hub" )
									archive = "hub -f " + value + " -s " + CStringToString( CValue( g.animation.time ).GetAsText() ) + "-bbox";
								else
#endif
									archive = value;
							}
							type = nodeArchive;
						}
					}
				} else
				if( "databox" == paramName ) {
					databox = CStringToString( param.GetValue( g.animation.time ) ) + "\n";
					databox = parseString( databox );
				} else
				if( "preftime" == paramName ) {
					if( !usePref ) {
						debugMessage( L"Using __Pref" );
						usePref = true;
						prefTime = ( float )param.GetValue( g.animation.time );
					}
				} else
				if( "staticframe" == paramName ) {
					if( !isStatic ) {
						isStatic = true;
						staticFrame = ( long )param.GetValue( g.animation.time );
					}
				} else
				if( "grouping:membership" == paramName ) {

					groupName += ',' + getAffogatoName( parseString ( CStringToString( param.GetValue( g.animation.time ) ) ) );
				}
				else {
					debugMessage( L"Found other parameter " + stringToCString( userParamName ) );

					if( attribMap.end() == attribMap.find( userParamName ) ) {
						CValue value( param.GetValue( g.animation.time ) );
						switch( value.m_t ) {
							case CValue::siBool:
							case CValue::siInt1:
							case CValue::siInt2:
							case CValue::siInt4:
							case CValue::siUInt1:
							case CValue::siUInt2:
							case CValue::siUInt4:
								debugMessage( L"Adding: int " + stringToCString( userParamName ) );
								replace_first( userParamName, string( "_" ), string( ":" ) );
								attribMap[ userParamName ] = tokenValue::tokenValuePtr( new tokenValue( ( int )( ( long )value ), userParamName ) );
								break;
							case CValue::siFloat:
							case CValue::siDouble: {
								char endChar = userParamName[ userParamName.length() - 1 ];
								if( ( 'R' == endChar ) || ( 'G' == endChar ) || ( 'B' == endChar ) ) {
									string actualParamName( userParamName.substr( 0, userParamName.length() - 1 ) );
									string xsiParamName( actualParamName );
									replace_first( actualParamName, string( "_" ), string( ":" ) );
									if( attribMap.end() == attribMap.find( actualParamName ) ) {
										CValue cr, cg, cb;
										cr = prop.GetParameterValue( stringToCString( xsiParamName + "R" ), g.animation.time );
										cg = prop.GetParameterValue( stringToCString( xsiParamName + "G" ), g.animation.time );
										cb = prop.GetParameterValue( stringToCString( xsiParamName + "B" ), g.animation.time );
										if( !cr.IsEmpty() && !cg.IsEmpty() && !cb.IsEmpty() ) {
											float color[ 3 ] = { ( float )cr, ( float )cg, ( float )cb, };
											debugMessage( L"Adding: color " + stringToCString( actualParamName ) );
											attribMap[ actualParamName ] = tokenValue::tokenValuePtr( new tokenValue( color, 3, actualParamName, tokenValue::storageUndefined, tokenValue::typeColor ) );
										} else {
											debugMessage( L"Adding: float " + stringToCString( userParamName ) );
											replace_first( userParamName, string( "_" ), string( ":" ) );
											attribMap[ userParamName ] = tokenValue::tokenValuePtr( new tokenValue( ( float )value, userParamName ) );
										}
									}
								} else {
									debugMessage( L"Adding: float " + stringToCString( userParamName ) );
									replace_first( userParamName, string( "_" ), string( ":" ) );
									attribMap[ userParamName ] = tokenValue::tokenValuePtr( new tokenValue( ( float )value, userParamName ) );
								}
								break;
							}
							case CValue::siString:
							case CValue::siWStr:
								debugMessage( L"Adding: string " + stringToCString( userParamName ) );
								replace_first( userParamName, string( "_" ), string( ":" ) );
								attribMap[ userParamName ] = tokenValue::tokenValuePtr( new tokenValue( parseString( CStringToString( value ) ), userParamName ) );
								break;
							// Todo: Array support!!!
						}
					}
				}
			}
		}
	}

	node::node( const X3DObject& obj )
	:	surface(),
		displacement(),
		volume(),
		bound( 6 ),
		staticFrame( 1 ),
#ifdef RSP
		hdb( 0 ),
		hub( 0 ),
#endif
		isStatic( false ),
		usePref( false ),
		prefTime( 0 )
	{
		globals& g = const_cast< globals& >( globals::access() );
		Application app;

		transformSampleTimes.reserve( MAXMOTIONSAMPLES );
		deformSampleTimes.reserve( MAXMOTIONSAMPLES );

		message( L"Found object '" + obj.GetFullName() + L"'", messageInfo );

		// Make name MtoR/Maya/Liquid compatible
		fileName = CStringToString( obj.GetFullName() );
		name = getAffogatoName( fileName );
		groupName = name; // for recording into trace membership groups

		attributeMap[ "identifier:name" ] = tokenValue::tokenValuePtr( new tokenValue( name, "identifier:name" ) );

		siClassID primID( obj.GetActivePrimitive().GetGeometry().GetRef().GetClassID() );
		if( siGeometryID == primID )
			/* Okl, XSI's cro-magnon unworthy API forces us to do this:
			   In case of hair and particles, the primID will be 22 now (for both!)
			   Below line gets us the primitve ID of those. We can't omit going to
			   the primtive there either, or else we get 72 (X3DObject) for hair
			   (aka if doing an primID = obj.GetRef().GetClassID() )
			   -- craptacular!!!
			 */
			primID = obj.GetActivePrimitive().GetRef().GetClassID();

		transformMotionSamples = g.motionBlur.transformMotionSamples;
		deformMotionSamples = g.motionBlur.deformMotionSamples;

		if( g.data.sections.attributes ) {

			X3DObject scanObj, test;
			test = obj;

			do {
				scanObj = test;
				CRefArray props( scanObj.GetProperties() );
				// Collect all looks. Each look lists all (affogato) properties that make up this look

				for( int i = 0; i < props.GetCount(); i++ ) {
					Property prop( props[ i ] );

					if( isAffogatoProperty( prop ) || ( !g.defaultShader.overrideAll && shader::isShader( prop ) ) ) {
						CRefArray owners( prop.GetOwners() );
						CRefArray groups;
						owners.Filter( siGroup, CStringArray(), CString(), groups );
						if( groups.GetCount() )
							lookVectorMap[ getAffogatoName( CStringToString( Group( groups[ 0 ] ).GetFullName() ) ) ].push_back( shared_ptr< Property >( new Property( prop ) ) );
						else // This is not on a group
							scanForAttributes( prop, attributeMap, surface, volume, displacement );
					} else {
						debugMessage( L"Doing property '" + prop.GetName() + L"'" );

						if( CString( L"visibility" ) == prop.GetType() ) {
							if( attributeMap.end() == attributeMap.find( "visibility:camera" ) )
								attributeMap[ "visibility:camera" ] = tokenValue::tokenValuePtr( new tokenValue( ( int )( long )prop.GetParameterValue( L"primray", g.animation.time ), "visibility:camera" ) );

							if( g.rays.enable && ( 22 != primID ) ) { // If we're /not/ dealing with hair...
								if( attributeMap.end() == attributeMap.find( "visibility:trace" ) )
									attributeMap[ "visibility:trace" ] = tokenValue::tokenValuePtr( new tokenValue( ( int )( long )prop.GetParameterValue( L"scndray", g.animation.time ), "visibility:trace" ) );
							}

							if( attributeMap.end() == attributeMap.find( "user:castsshadows" ) )
								attributeMap[ "user:castsshadows" ] = tokenValue::tokenValuePtr( new tokenValue( ( int )( long )prop.GetParameterValue( L"shdw", g.animation.time ), "user:castsshadows" ) );
						} else
						if( CString( L"geomapprox" ) == prop.GetType() ) {
							if( attributeMap.end() == attributeMap.find( "displacementbound:sphere" ) ) {
								attributeMap[ "displacementbound:sphere" ] = tokenValue::tokenValuePtr( new tokenValue( ( float )prop.GetParameterValue( L"gapproxmaxdisp", g.animation.time ), "displacementbound:sphere" ) );
								attributeMap[ "displacementbound:coordinatesystem" ] = tokenValue::tokenValuePtr( new tokenValue( "shader", "displacementbound:coordinatesystem" ) );
							}
						}
#ifdef RSP
						else if( CString( L"AffogatoHUB" ) == prop.GetType() ) {
							switch( ( long )prop.GetParameterValue( L"Type" ) ) {
								case 1: // HUB
									hub = ( long )prop.GetParameterValue( L"Type" );
									break;
								case 2: // HDB
									hdb = ( long )prop.GetParameterValue( L"Type" );
									break;
							}
							type = nodeHub;
						}
#endif
					}
				}

				test = scanObj.GetParent();

			} while( g.data.hierarchical && ( test != scanObj ) ); // until GetParent() returned the objevct itself -> aka the scene root or if we don't scan hierarchical upwards

			// Find groups
			/*CRefArray owners( obj.GetOwners() );
			CRefArray groups;
			owners.Filter( siGroup, CStringArray(), CString(), groups );

			for( int i = 0; i < groups.GetCount(); i++ ) {
				string name = CStringToString( groups[ i ].GetAsText() );
				string test = name.substr( 0, 7 );
				if( ( "Layers." != test ) && ( "Passes." != test ) )
					groupName += ',' + getAffogatoName( CStringToString( groups[ i ].GetAsText() ) );
			}*/
			attributeMap[ "grouping:membership" ] = tokenValue::tokenValuePtr( new tokenValue( groupName, "grouping:membership" ) );

			if( g.rays.enable && ( 22 != primID ) ) { // If we're /not/ dealing with hair...
				if( attributeMap.end() == attributeMap.find( "visibility:transmission" ) )
					attributeMap[ "visibility:transmission" ] = tokenValue::tokenValuePtr( new tokenValue( "opaque", "visibility:transmission" ) );
			}

			if( g.data.sections.looks ) {
				ueberManInterface theRenderer;

				set< string > looks( theRenderer.getLooks() );
				for( map< string, vector< shared_ptr< Property > > >::iterator it = lookVectorMap.begin(); it != lookVectorMap.end(); it++ ) {
					debugMessage( L"Group: " + stringToCString( it->first ) );

					if( looks.end() == looks.find( it->first ) ) { // If this look doesn't exist yet

						map< string, tokenValue::tokenValuePtr > lookAttributeMap;
						shared_ptr< shader > lookSurface, lookDisplacement, lookVolume;

						groupName = '+' + it->first;

						for( vector< shared_ptr< Property > >::iterator vit = it->second.begin(); vit != it->second.end(); vit++ ) {
							scanForAttributes( **vit, lookAttributeMap, lookSurface, lookDisplacement, lookVolume );

							if( !databox.empty() ) // Add ueberMan callback for this!!!
								RiArchiveRecord( RI_VERBATIM, const_cast< RtString >( databox.c_str() ) );
						}

						lookAttributeMap[ "grouping:membership" ] = tokenValue::tokenValuePtr( new tokenValue( groupName, "grouping:membership" ) );

						/*if( !g.rays.enable ) {
							message( L"Disabling ray-tracing attributes", messageInfo );

							deleteAttributeMapElement( "grouping:membership" );
							deleteAttributeMapElement( "irradiance:nsamples" );
							deleteAttributeMapElement( "irradiance:maxerror" );
							deleteAttributeMapElement( "irradiance:shadingrate" );
							deleteAttributeMapElement( "light:samples" );
							deleteAttributeMapElement( "light:shadows" );
							deleteAttributeMapElement( "subsurface:absorption" );
							deleteAttributeMapElement( "subsurface:scattering" );
							deleteAttributeMapElement( "subsurface:meanfreepath" );
							deleteAttributeMapElement( "subsurface:reflectance" );
							deleteAttributeMapElement( "subsurface:refractionindex" );
							deleteAttributeMapElement( "subsurface:scale" );
							deleteAttributeMapElement( "subsurface:shadingrate" );
							deleteAttributeMapElement( "trace:bias" );
							deleteAttributeMapElement( "trace:displacements" );
							deleteAttributeMapElement( "trace:samplemotion" );
							deleteAttributeMapElement( "visibility:subsurface" );
							deleteAttributeMapElement( "visibility:trace" );
						}*/

						context savedContext( theRenderer.currentScene() );

						debugMessage( L"Setting Renderer context " + CValue( lookContext ).GetAsText() );
						theRenderer.switchScene( lookContext );

						lookHandle lookId( it->first );
						theRenderer.beginLook( lookId );

						for( map< string, tokenValue::tokenValuePtr >::iterator lit = lookAttributeMap.begin(); lit != lookAttributeMap.end(); lit++ )
							theRenderer.attribute( *( lit->second ) );

						if( lookSurface )
							lookSurface->write();

						if( lookDisplacement )
							lookDisplacement->write();

						if( lookVolume )
							lookVolume->write();

						theRenderer.endLook();

						theRenderer.switchScene( savedContext );
					}
				}
			}
		}

		if( g.data.sections.geometry ) {

			Primitive prim( obj.GetActivePrimitive() );
			CTransformation tmp;

			if( g.motionBlur.geometryBlur ) {
				transformSampleTimes = getMotionSamples( transformMotionSamples );
				for( unsigned short motion = 0; motion < transformMotionSamples; motion++ ) {
					transformSamples.push_back( shared_ptr< CMatrix4 >( new CMatrix4( obj.GetKinematics().GetGlobal().GetTransform( transformSampleTimes[ motion ] ).GetMatrix4() ) ) );
				}
			} else {
				transformSamples.push_back( shared_ptr< CMatrix4 >( new CMatrix4( obj.GetKinematics().GetGlobal().GetTransform( g.animation.time ).GetMatrix4() ) ) );
			}

			// Get bounding box
			double centerX, centerY, centerZ, extendX, extendY, extendZ;
			prim.GetGeometry( g.animation.time ).GetBoundingBox( centerX, centerY, centerZ, extendX, extendY, extendZ, tmp );

			if( ( 22 == primID ) || ( !extendX && !extendY && !extendZ ) ) {
				bound[ 0 ] = bound[ 2 ] = bound[ 4 ] = std::numeric_limits< float >::min();
				bound[ 1 ] = bound[ 3 ] = bound[ 5 ] = std::numeric_limits< float >::max();
			} else {
				extendX *= 0.5;
				extendY *= 0.5;
				extendZ *= 0.5;

				bound[ 0 ] = centerX - extendX;
				bound[ 1 ] = centerX + extendX;
				bound[ 2 ] = centerY - extendY;
				bound[ 3 ] = centerY + extendY;
				bound[ 4 ] = centerZ - extendZ;
				bound[ 5 ] = centerZ + extendZ;
			}

			if( !isArchive()
#ifdef RSP
				&& !( hub || hdb )
#endif
			) {

				debugMessage( L"Primitive Id is '" + CValue( ( long )primID ).GetAsText() + L"'." );

				if( g.motionBlur.geometryBlur ) {
					deformSampleTimes = getMotionSamples( deformMotionSamples );
					for( unsigned short motion = 0; motion < deformMotionSamples; motion++ ) {
						switch( primID ) {
							case siPolygonMeshID:
								geometrySamples.push_back( shared_ptr< polyMeshData >( new polyMeshData( prim, deformSampleTimes[ motion ], usePref, prefTime ) ) );
								type = nodeMesh;
								break;
							case siNurbsSurfaceMeshID:
								geometrySamples.push_back( shared_ptr< nurbMeshData >( new nurbMeshData( prim, deformSampleTimes[ motion ], usePref, prefTime ) ) );
								type = nodeNurb;
								break;
							case siNurbsCurveListID:
								geometrySamples.push_back( shared_ptr< nurbCurveData >( new nurbCurveData( prim, deformSampleTimes[ motion ], usePref, prefTime ) ) );
								type = nodeCurves;
								break;
							case 174: //siHairPrimitiveID:
								geometrySamples.push_back( shared_ptr< hairData >( new hairData( prim, deformSampleTimes[ motion ], true ) ) );
								type = nodeHair;
								break;
							case siParticleCloudPrimitiveID:
								/*CValueArray args( 2 );
								args[ 0 ] = L"PlayControl.Current";
								args[ 1 ] = CValue( round( deformSampleTimes.back() ) );
								CValue retVal;
								Application app;
								app.ExecuteCommand( L"SetValue", args, retVal );
								prim = obj.GetActivePrimitive();*/
								geometrySamples.push_back( shared_ptr< particleData >( new particleData( prim, deformSampleTimes[ motion ], true ) ) );
								type = nodeParticle;
								break;
							case 0:
								geometrySamples.push_back( shared_ptr< sphereData >( new sphereData( obj, deformSampleTimes[ motion ] ) ) );
								type = nodeSphere;
								break;
						}
					}
				} else {
					switch( primID ) {
						case siPolygonMeshID:
							geometrySamples.push_back( shared_ptr< polyMeshData >( new polyMeshData( prim, g.animation.time, usePref, prefTime ) ) );
							type = nodeMesh;
							break;
						case siNurbsSurfaceMeshID:
							geometrySamples.push_back( shared_ptr< nurbMeshData >( new nurbMeshData( prim, g.animation.time, usePref, prefTime ) ) );
							type = nodeNurb;
							break;
						case siNurbsCurveListID:
							geometrySamples.push_back( shared_ptr< nurbCurveData >( new nurbCurveData( prim, g.animation.time, usePref, prefTime ) ) );
							type = nodeCurves;
							break;
						case 174: //siHairPrimitiveID:
							geometrySamples.push_back( shared_ptr< hairData >( new hairData( prim, g.animation.time, true ) ) );
							type = nodeHair;
							break;
						case siParticleCloudPrimitiveID:
							geometrySamples.push_back( shared_ptr< particleData >( new particleData( prim, g.animation.time, usePref, prefTime ) ) );
							type = nodeParticle;
							break;
						case 0:
							geometrySamples.push_back( shared_ptr< sphereData >( new sphereData( obj, g.animation.time ) ) );
							type = nodeSphere;
							break;
					}
				}
			} else {
				if( siNullID == primID )
					type = nodeNull;
			}
		}
	}

	node::~node() {

	/*
		debugMessage( L"Destructing node" );
		transformSampleTimes.clear();
		debugMessage( L"Destructing node 0.1" );
		deformSampleTimes.clear();
		debugMessage( L"Destructing node 0.2" );
		bound.clear();

		debugMessage( L"Destructing node 0.3" );
		attributeMap.clear();

		debugMessage( L"Destructing node 1" );
		surface.reset();
		displacement.reset();
		volume.reset();

		debugMessage( L"Destructing node 2" );
		transformSamples.clear();
		debugMessage( L"Destructing node 2.1" );
		geometrySamples.clear();
		debugMessage( L"Destructing node 2.2" );
		lookVectorMap.clear();
		debugMessage( L"Destructing node 3" );*/
	}

	void node::writeAttributes() const {
		if( nodeUndefined != type ) {

			globals& g = const_cast< globals& >( globals::access() );
			ueberManInterface theRenderer;

			/*switch( g.data.attributeMode ) {
				case globals::data::attributeModeLook:
					context savedContext = theRenderer.currentScene();
					theRenderer.switchScene( lookContext );

					theRenderer.switchScene( savedContext );
					theRenderer.appendLook( objectLook );

				case globals::data::attributeModeInline: {*/

					if( g.data.sections.attributes ) {

						ueberManInterface theRenderer;

						switch( g.data.attributeScanningOrder ) {
							case globals::data::scanningOrderGroups: {
								for( map< string, tokenValue::tokenValuePtr >::const_iterator it = attributeMap.begin();it != attributeMap.end(); it++ )
									theRenderer.attribute( *( it->second ) );

								if( surface )
									surface->write();

								if( displacement )
									displacement->write();

								if( volume )
									volume->write();

								for( map< string, vector< shared_ptr< Property > > >::const_iterator it = lookVectorMap.begin(); it != lookVectorMap.end(); it++ )
									theRenderer.appendLook( it->first );

								if( !databox.empty() ) // Add ueberMan callback for this!!!
									RiArchiveRecord( RI_VERBATIM, const_cast< RtString >( databox.c_str() ) );

								break;
							}
							case globals::data::scanningOrderObjects: {
								for( map< string, vector< shared_ptr< Property > > >::const_iterator it = lookVectorMap.begin(); it != lookVectorMap.end(); it++ )
									theRenderer.appendLook( it->first );

								for( map< string, tokenValue::tokenValuePtr >::const_iterator it = attributeMap.begin();it != attributeMap.end(); it++ )
									theRenderer.attribute( *( it->second ) );

								if( surface )
									surface->write();

								if( displacement )
									displacement->write();

								if( volume )
									volume->write();

								if( !databox.empty() ) // Add ueberMan callback for this!!!
									RiArchiveRecord( RI_VERBATIM, const_cast< RtString >( databox.c_str() ) );

								break;
							}
						}
					}
				/*}
			}*/
		}
	}

	void node::writeTransform() const {
		debugMessage( L"Writing transform" );
		if( ( nodeUndefined != type ) && ( transformSamples.size() ) ) {
			const globals& g( globals::access() );
			ueberManInterface theRenderer;
			if( transformSamples.size() > 1 ) {
				theRenderer.motion( remapMotionSamples( transformSampleTimes ) );
				for( vector< shared_ptr< CMatrix4 > >::const_iterator it = transformSamples.begin(); it < transformSamples.end(); it++ )
					if( g.data.relativeTransforms )
						theRenderer.appendSpace( CMatrix4ToFloat( *( *it ) ) );
					else
						theRenderer.space( CMatrix4ToFloat( *( *it ) ) );
			} else {
				if( g.data.relativeTransforms )
					theRenderer.appendSpace( CMatrix4ToFloat( *( transformSamples[ 0 ] ) ) );
				else
					theRenderer.space( CMatrix4ToFloat( *( transformSamples[ 0 ] ) ) );
			}
		}
		debugMessage( L"Done writing transforms" );
	}

	void node::writeGeometry() const {

		debugMessage( L"Writing geometry" );
		const globals& g( globals::access() );

		if( ( nodeUndefined != type ) && ( g.data.sections.geometry ) ) {
			Application app;

			ueberManInterface theRenderer;
			if( isArchive() ) {
				debugMessage( L"Doing archive" );
				theRenderer.input( archive );
			} else {
#ifdef RSP
				if( hub ) {
					using namespace filesystem;

					debugMessage( L"Doing hub: " + CValue( ( long )hub ).GetAsText() + L"." );

					string frame;
					if( isStatic )
						frame = "";
					else
						frame = "." + g.name.currentFrame;

					path fileLocation( g.directories.hub / filesystem::path( g.name.baseName + "." + fileName + frame + ".hub" ) );

					if( !isStatic || ( isStatic && staticFrame == g.animation.time ) ) {
						// 0 Don't write HUBs -- write curves into RIB
						// 1 Always write HUBs
						// 2 Only write HUBs if they do not exist
						// 3 Don't wite HUBs but refer to them (presume they exist)
						if( ( hub != 3 ) || ( !exists( fileLocation ) && ( hub == 2 ) ) ) {

							CValueArray args( 10 );

							args[ 0 ] = CValue( stringToCString( fileLocation.native_file_string() ) );
							args[ 1 ] = CValue( stringToCString( fileName ) );
							args[ 2 ] = CValue( false );						// no appending -- one hub per obejct per frame for now
							args[ 3 ] = CValue( false );						// always overwrite
							args[ 4 ] = 0l;										// sample
							args[ 5 ] = CValue( ( double )g.animation.time + g.motionBlur.shutterOpen );  // shutter open
							args[ 6 ] = CValue( ( double )g.animation.time + g.motionBlur.shutterClose ); // shutter close
							args[ 7 ] = CValue( ( long )g.motionBlur.deformMotionSamples - 1 );
							args[ 8 ] = CValue( false );						// No transform blur -- transform stuff will be in the RIB
							args[ 9 ] = CValue( false );						// Do we motion blur ?

							CValue retVal;
							app.ExecuteCommand( L"ExportHUB", args, retVal );
						} else

					theRenderer.input( "hub -f " + fileLocation.native_file_string() + " -s 0", &( bound[ 0 ] ) ); // + CStringToString( CValue( ( long )g.animation.time ).GetAsText() ) );
					}
				} else
				if( hdb ) {
					using namespace filesystem;

					debugMessage( L"Doing hdb: " + CValue( ( long )hub ).GetAsText() + L"." );

					string frame;
					if( isStatic )
						frame = "";
					else
						frame = "." + g.name.currentFrame;

					path fileLocation( g.directories.hub / filesystem::path( fileName + frame + ".hdb" ) );

					if( !isStatic || ( isStatic && staticFrame == g.animation.time ) ) {
						if( ( hdb != 3 ) || ( !exists( fileLocation ) && ( hdb == 2 ) ) ) {
							CValueArray args( 6 );

							args[ 0 ] = stringToCString( fileName );
							args[ 1 ] = CValue( stringToCString( g.directories.hub.native_file_string() ) );
							args[ 2 ] = CValue( ( double )g.animation.time + g.motionBlur.shutterOpen );  // shutter open
							args[ 3 ] = CValue( ( double )g.animation.time + g.motionBlur.shutterClose ); // shutter close
							args[ 4 ] = CValue( ( long )g.motionBlur.deformMotionSamples - 1 );
							args[ 5 ] = 10000l;

							CValue retVal;
							app.ExecuteCommand( L"Hairarchy", args, retVal );
						}
					}
					theRenderer.input( "renderHairarchy " + fileLocation.native_file_string(), &( bound[ 0 ] ) ); // + CStringToString( CValue( ( long )g.animation.time ).GetAsText() ) );
				} else
#endif
				{
					if( !geometrySamples.empty() ) {
						debugMessage( L"Writing real geo" );
						if( 1 < geometrySamples.size() ) {
							// Iterate over all grains
							for( unsigned i( 0 ); i < geometrySamples[ 0 ]->granularity(); i++ ) {
								theRenderer.motion( remapMotionSamples( deformSampleTimes ) );
								// Iterate over all samples of a grain
								for( vector< shared_ptr< data > >::const_iterator it( geometrySamples.begin() ); it < geometrySamples.end(); it++ ) {
									( *it )->writeNextGrain();
								}
							}
						} else {
							geometrySamples[ 0 ]->write();
						}
					}
				}
			}
		}
		debugMessage( L"Done writing geo" );
	}

	void node::write() const {
		debugMessage( L"Writing node" );

		writeTransform();
		writeAttributes();
		writeGeometry();

		debugMessage( L"Done writing node" );
	}

	vector< float > node::getBoundingBox() const {
		if( !geometrySamples.empty() )
			return geometrySamples[ 0 ]->boundingBox();
		else
			return bound;
	}

	bool node::isArchive() const {
		return !archive.empty();
	}

	bool node::isDataBox() const {
		return !databox.empty();
	}

	void node::deleteAttributeMapElement( const string& element ) {
		map< string, tokenValue::tokenValuePtr >::iterator it;
		it = attributeMap.find( element );
		if( attributeMap.end() != it )
			attributeMap.erase( it );
	}
}

