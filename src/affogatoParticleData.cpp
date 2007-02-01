/** Particle data container class.
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
#include <limits>
#include <math.h>

// Boost headers
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/case_conv.hpp>

// XSI Headers
#include <xsi_application.h>
#include <xsi_cluster.h>
#include <xsi_geometry.h>
#include <xsi_particleattribute.h>
#include <xsi_particlecloudprimitive.h>
#include <xsi_primitive.h>
#include <xsi_time.h>
#include <xsi_userdatamap.h>
#include <xsi_x3dobject.h>

// Affogato Headers
#include "affogatoGlobals.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoParticleData.hpp"
#include "affogatoRenderer.hpp"
#include "affogatoTokenValue.hpp"


namespace affogato {

	using namespace XSI;
	using namespace MATH;
	using namespace std;
	using namespace boost;

	particleData::~particleData() {
		// Nothing to destruct
	}

	particleData::particleData( const Primitive &particlePrim, double atTime, bool usePref, double atPrefTime ) {
		globals& g = const_cast< globals& >( globals::access() );

		isMultiGroupBlob = false;
		typeStr = "particle";

		CRefArray affogatoProps( getAffogatoProperties( particlePrim.GetParent() ) );

		for( unsigned i( 0 ); i < affogatoProps.GetCount(); i++ ) {
			Property prop( affogatoProps[ i ] );

			CParameterRefArray params( prop.GetParameters() );

			for( unsigned p( 0 ); p < params.GetCount(); p++ ) {
				Parameter param( params[ p ] );
				string paramName = CStringToString( param.GetName() );
				if( "particle_type" == paramName ) {
					switch( ( long )param.GetValue() ) {
						default:
						case 0: // points
							typeStr = "particle";
							break;
						case 1: // discs
							typeStr = "disc";
							break;
						case 2: // spheres
							typeStr = "sphere";
							break;
						case 3: // blobbies
							typeStr = "blobby";
							break;
						case 4: // sprites
							typeStr = "patch";
							break;
					}
				}
			}
		}

		bool blobbyIdSplitting( true );

		identifier = getAffogatoName( CStringToString( X3DObject( particlePrim.GetParent() ).GetFullName() ) );

		Geometry particleGeo( particlePrim.GetGeometry( atTime ) );
		ParticleCloudPrimitive particles( particleGeo.GetParent() );

		CDoubleArray positions;
		particles.GetPositionArray( positions );

		numParticles = particles.GetCount();
		unsigned numTriples( numParticles * 3 );

		double timeDelta( atTime - floor( atTime ) );
		// If we are at a subframe, approximate positions by using the velocity
		// Why: because ueber-lame XSI evaluates particles only at integer frames!
		if( 0.0001 < fabs( timeDelta ) ) {
			CDoubleArray velocities;
			particles.GetVelocityArray( velocities );
			CTime theTime;
			float frameRate( ( float )theTime.GetFrameRate() );
			timeDelta /= frameRate;
			shared_ptr< float > pos( new float[ numTriples ], arrayDeleter() );
			for( unsigned i = 0; i < ( unsigned )positions.GetCount(); i++ )
				pos.get()[ i ] = ( float )( positions[ i ] + ( timeDelta * velocities[ i ] ) );
			tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( pos.get(), numTriples, "P", tokenValue::storageVertex, tokenValue::typePoint ) ) );
		} else {
			tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( positions, "P", tokenValue::storageVertex, tokenValue::typePoint ) ) );
		}

		if( g.motionBlur.geometryParameterBlur || ( g.animation.time == atTime ) ) {
			CDoubleArray colors;
			particles.GetColorArray( colors );
			shared_ptr< float > cs( new float[ numTriples ], arrayDeleter() );
			shared_ptr< float > os( new float[ numTriples ], arrayDeleter() );
			for( unsigned i = 0, j = 0; i < numTriples; i += 3, j += 4 ) {
				cs.get()[ i ]     = ( float )colors[ j ];
				cs.get()[ i + 1 ] = ( float )colors[ j + 1 ];
				cs.get()[ i + 2 ] = ( float )colors[ j + 2 ];
				os.get()[ i ] = os.get()[ i + 1 ] = os.get()[ i + 2 ] = ( float )colors[ j + 3 ];
			}
			tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( cs, numTriples, "Cs", tokenValue::storageVarying, tokenValue::typeColor ) ) );
			tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( os, numTriples, "Os", tokenValue::storageVarying, tokenValue::typeColor ) ) );

			CDoubleArray uvws;
			particles.GetUVWArray( uvws );
			shared_ptr< float > stBase( new float[ numTriples ], arrayDeleter() );
			for( unsigned i = 0, j = 0; i < numTriples; i += 3, j += 2 ) {
				stBase.get()[ j ] = ( float )uvws[ i ];
				stBase.get()[ j + 1 ] = ( float )uvws[ i + 1 ];
			}
			tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( stBase, numParticles * 2, "stbase[2]", tokenValue::storageVarying, tokenValue::typeFloat ) ) );

			CLongArray age;
			particles.GetAgeArray( age );
			tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( age, "age", tokenValue::storageVarying, tokenValue::typeInteger ) ) );
		}

		if( g.motionBlur.geometryParameterBlur || ( g.animation.time == atTime ) || blobbyIdSplitting ) {

			// Temporary array
			vector< tokenValue::tokenValuePtr > attributeArray;

			// Clusters
			CRefArray clusters;
			particleGeo.GetClusters().Filter( siVertexCluster, CStringArray(), CString(), clusters );

			for( unsigned i( 0 ); i < ( unsigned )clusters.GetCount(); i++ ) {
				Cluster cluster( clusters[ i ] );
				CRefArray properties( cluster.GetProperties() );
				for( unsigned j( 0 ); j < ( unsigned )properties.GetCount(); j++ ) {
					Property prop( properties[ j ] );
					if( CString( L"UserDataMap" ) == prop.GetType() ) {
						UserDataMap userDataMap( prop );
						CustomProperty userDataTemplate( userDataMap.GetTemplate() );
						CParameterRefArray parms( userDataTemplate.GetParameters() );

						for( unsigned p( 0 ); p < ( unsigned )parms.GetCount(); p++ ) {
							Parameter aParm( parms[ p ] );
							string name( CStringToString( aParm.GetName() ) );
							if( !name.empty() ) {
								switch( aParm.GetValueType() ) {
									case siEmpty:
									case siFloat:
									case siDouble:
										attributeArray.push_back( tokenValue::tokenValuePtr( new tokenValue( numParticles, tokenValue::typeFloat ) ) );
										break;
									//case siInt1:
									case siInt2:
									case siInt4:
									//case siUInt1:
									case siUInt2:
									case siUInt4:
									case siBool:
										attributeArray.push_back( tokenValue::tokenValuePtr( new tokenValue( numParticles, tokenValue::typeInteger ) ) );
										break;
								}
								attributeArray.back()->setName( name );
								attributeArray.back()->setClass( tokenValue::storageVarying );
							}
						}

						for( unsigned p( 0 ); p < ( unsigned )parms.GetCount(); p++ ) {
							Parameter aParm( parms[ p ] );
							string name( CStringToString( aParm.GetName() ) );
							if( !name.empty() ) {
								unsigned xsiIndex = 0, dataIndex = 0;

								Parameter aParm( parms[ p ] );
								for( unsigned particle = 0; particle < ( unsigned )numParticles; particle++, dataIndex++ ) {
									long cindex;
									cluster.FindIndex( xsiIndex++, cindex );

									if( -1 < cindex ) {
										const unsigned char *cpData;
										unsigned int cntData;
										userDataMap.GetItemValue( cindex, cpData, cntData );
										userDataTemplate.PutBinaryData( cpData, cntData );

										switch( attributeArray[ p ]->type() ) {
											case tokenValue::typeFloat: {
												CValue out( aParm.GetValue() );
												*( ( float* )( *( attributeArray[ p ].get() ) )[ dataIndex ] ) = float( out );
												break;
											}
											case tokenValue::typeInteger: {
												CValue out( aParm.GetValue() );
												*( ( int* )( *( attributeArray[ p ].get() ) )[ dataIndex ] ) = ( int )long( out );
												break;
											}
										}
									} else {
										switch( attributeArray[ p ]->type() ) {
											case tokenValue::typeFloat: {
												CValue out( aParm.GetValue() );
												*( ( float* )( *( attributeArray[ p ].get() ) )[ dataIndex ] ) = 0.0f;
												break;
											}
											case tokenValue::typeInteger: {
												CValue out( aParm.GetValue() );
												*( ( int* )( *( attributeArray[ p ].get() ) )[ dataIndex ] ) = 0l;
												break;
											}
										}
									}
								}
							}
						}
					}
				}
			}

			// PP attributes
			Particle p0( particles.GetParticle( 0 ) );
			CRefArray p0Attributes( p0.GetAttributes() );
			unsigned numAttributes( p0.GetAttributes().GetCount() );
			for( unsigned a( 0 ); a < numAttributes; a++ ) {
				string name( CStringToString( ParticleAttribute( p0Attributes[ a ] ).GetName() ) );
				if( !name.empty() ) {
					switch( ParticleAttribute( p0Attributes[ a ] ).GetAttributeType() ) {
						case siPAUndefined:
						case siPAFloat:
							attributeArray.push_back( tokenValue::tokenValuePtr( new tokenValue( numParticles, tokenValue::typeFloat ) ) );
							break;
						case siPAInt:
						case siPAULong:
						case siPAUShort:
						case siPABool:
							attributeArray.push_back( tokenValue::tokenValuePtr( new tokenValue( numParticles, tokenValue::typeInteger ) ) );
							break;
						case siPAVector3:
							attributeArray.push_back( tokenValue::tokenValuePtr( new tokenValue( numParticles, tokenValue::typePoint ) ) );
							break;
						case siPAVector4:
							attributeArray.push_back( tokenValue::tokenValuePtr( new tokenValue( numParticles, tokenValue::typeHomogenousPoint ) ) );
							break;
					}
					attributeArray.back()->setName( name );
					attributeArray.back()->setClass( tokenValue::storageVarying );
				}
			}

			// Aquire PP data from XSI
			for( unsigned a( 0 ); a < numAttributes; a++ ) {
				string name( CStringToString( ParticleAttribute( p0Attributes[ a ] ).GetName() ) );
				if( !name.empty() ) {
					for( unsigned i( 0 ); i < numParticles; i++ ) {
						Particle particle( particles.GetParticle( i ) );
						CRefArray attributes( particle.GetAttributes() );
						ParticleAttribute attribute( attributes[ a ] );
						switch( attributeArray[ a ]->type() ) {
							case tokenValue::typeFloat: {
								CValue out;
								attribute.GetValue( out );
								*( ( float* )( *( attributeArray[ a ].get() ) )[ i ] ) = float( out );
								break;
							}
							case tokenValue::typeInteger: {
								CValue out;
								attribute.GetValue( out );
								*( ( int* )( *( attributeArray[ a ].get() ) )[ i ] ) = ( int )long( out );
								break;
							}
							case tokenValue::typePoint: {
								CVector3 out;
								attribute.GetValue( out );
								*( ( float* )( *( attributeArray[ a ].get() ) )[ 3 * i     ] ) = ( float )( out.GetX() );
								*( ( float* )( *( attributeArray[ a ].get() ) )[ 3 * i + 1 ] ) = ( float )( out.GetY() );
								*( ( float* )( *( attributeArray[ a ].get() ) )[ 3 * i + 2 ] ) = ( float )( out.GetZ() );
								break;
							}
							case tokenValue::typeHomogenousPoint: {
								CVector4 out;
								attribute.GetValue( out );
								*( ( float* )( *( attributeArray[ a ].get() ) )[ 3 * i     ] ) = ( float )( out.GetX() );
								*( ( float* )( *( attributeArray[ a ].get() ) )[ 3 * i + 1 ] ) = ( float )( out.GetY() );
								*( ( float* )( *( attributeArray[ a ].get() ) )[ 3 * i + 2 ] ) = ( float )( out.GetZ() );
								*( ( float* )( *( attributeArray[ a ].get() ) )[ 3 * i + 3 ] ) = ( float )( out.GetW() );
								break;
							}
						}
					}
				}
			}

			tokenValue::tokenValuePtr blobbyIdMap;
			for( vector< tokenValue::tokenValuePtr >::iterator it = attributeArray.begin(); it < attributeArray.end(); it++ ) {
				if( ( "blobby" == typeStr ) && ( "blobbyid" == ( *it )->name() ) ) {
					blobbyIdMap = *it;
					debugMessage( L"Blobby ID!" );
					//tokenValuePtrArray.erase( it );
					isMultiGroupBlob = true;
					break;
				}
			}

			bool noSizeOverride( true );
			for( vector< tokenValue::tokenValuePtr >::iterator it = attributeArray.begin(); it < attributeArray.end(); it++ ) {
				if( "size" == ( *it )->name() ) {
					( *it )->setName( "width" );
					noSizeOverride = false;
					debugMessage( L"Size!" );
					// We always push width!
					tokenValuePtrArray.push_back( *it );
					// Ensure we don't push the parameter twice l8r
					attributeArray.erase( it );
					break;
				}
			}

			if( g.motionBlur.geometryParameterBlur || ( g.animation.time == atTime ) )
				tokenValuePtrArray.insert( tokenValuePtrArray.end(), attributeArray.begin(), attributeArray.end() );

			attributeArray.clear();

			if( noSizeOverride ) {
				CDoubleArray sizes;
				particles.GetSizeArray( sizes );
				tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( sizes, "width", tokenValue::storageVarying, tokenValue::typeFloat ) ) );
			}

			// Deprecated splitting code. Might be useful for st. in the future
			/*if( blobbyIdMap ) {
				// Split everything By ID. Potentially slow.
				splitById( blobbyIdMap );
			}*/

			if( isMultiGroupBlob ) {

				tokenValue::tokenValuePtr widths;
				float width( 0 );
				for( vector< tokenValue::tokenValuePtr >::iterator it = tokenValuePtrArray.begin(); it < tokenValuePtrArray.end(); it++ ) {
					if( "width" == ( *it )->name() ) {
						if( tokenValue::typeFloat == ( *it )->type() ) {
							switch( ( *it )->storage() ) {
								case tokenValue::storageConstant:
								case tokenValue::storageUniform:
									width = *( float* )( *it )->data();
									break;
								case tokenValue::storageVarying:
								case tokenValue::storageVertex:
									widths = *it;
							}
							tokenValuePtrArray.erase( it );
						}
					}
				}

				tokenValue::tokenValuePtr P;
				for( vector< tokenValue::tokenValuePtr >::iterator it = tokenValuePtrArray.begin(); it < tokenValuePtrArray.end(); it++ ) {
					if( "P" == ( *it )->name()) {
						if( tokenValue::typePoint == ( *it )->type() ) {
							switch( ( *it )->storage() ) {
								case tokenValue::storageVarying:
								case tokenValue::storageVertex:
									P = *it;
							}
							tokenValuePtrArray.erase( it );
						}
					}
				}

				if( P ) {
					int index( 0 );
					map< int, vector< int > > idGroups;
					ppos.reserve( numParticles * 16 );
					for( unsigned i( 0 ); i < numParticles; i++ ) {
						code.push_back( 1001 );
						code.push_back( index );

						float theWidth;
						if( widths )
							theWidth = *( float* )( ( *widths )[ i ] );
						else
							theWidth = width;

						ppos.push_back( theWidth );
						ppos.push_back( 0 );
						ppos.push_back( 0 );
						ppos.push_back( 0 );

						ppos.push_back( 0 );
						ppos.push_back( theWidth );
						ppos.push_back( 0 );
						ppos.push_back( 0 );

						ppos.push_back( 0 );
						ppos.push_back( 0 );
						ppos.push_back( theWidth );
						ppos.push_back( 0 );

						ppos.push_back( *( float* )( *P )[ i * 3     ] );
						ppos.push_back( *( float* )( *P )[ i * 3 + 1 ] );
						ppos.push_back( *( float* )( *P )[ i * 3 + 2 ] );
						ppos.push_back( 1 );

						index += 16;

						// Find which particles are in which group
						int blobId( *( ( int* )( *blobbyIdMap.get() )[ i ] ) );
						idGroups[ blobId ].push_back( i );
					}

					for( map< int, vector< int > >::const_iterator it = idGroups.begin(); it != idGroups.end(); it++ ) {
						code.push_back( 0 ); // Sum
						code.push_back( it->second.size() ); // Number of operands
						for( vector< int >::const_iterator jt = it->second.begin(); jt < it->second.end(); jt++ ) {
							code.push_back( *jt ); // Operand
						}
					}

					code.push_back( 2 ); // Maximum
					code.push_back( idGroups.size() ); // Number of operands
					for( unsigned i = 0; i < ( unsigned )idGroups.size(); i++ ) {
						code.push_back( numParticles + i ); // Operand
					}
				}
			}
		}
	}

	// split all toke values by blobbyID
	/*void particleData::splitById( tokenValue::tokenValuePtr blobbyIdMap ) {

		// Find number of particles per blob
		for( unsigned i( 0 ); i < numParticles; i++ ) {
			int blobId( *( ( int* )( *blobbyIdMap.get() )[ i ] ) );
			if( numParticlesMap.end() == numParticlesMap.find( blobId ) )
				numParticlesMap[ blobId ] = 0;
			numParticlesMap[ *(	( int* )( *blobbyIdMap.get() )[ i ] ) ]++;
		}

		// Reserve space for all the new blobs
		for( map< int, unsigned >::const_iterator jt = numParticlesMap.begin(); jt != numParticlesMap.end(); jt++ ) {
			int blobId( jt->first );

			for( vector< tokenValue::tokenValuePtr >::const_iterator it( tokenValuePtrArray.begin() ); it < tokenValuePtrArray.end(); it++ ) {
				tokenValuePtrArrayMap[ blobId ].push_back( tokenValue::tokenValuePtr( new tokenValue( jt->second, ( *it )->getType() ) ) );
				tokenValuePtrArrayMap[ blobId ].back()->setName( ( *it )->getName() );
				tokenValuePtrArrayMap[ blobId ].back()->setClass( ( *it )->getClass() );
			}
			message( CValue( ( long )blobId ).GetAsText() + L": " + CValue( ( long )jt->second ).GetAsText(), messageError );
		}

		// loop over blob groups
		map< int, unsigned > counter;
		for( map< int, unsigned >::const_iterator jt = numParticlesMap.begin(); jt != numParticlesMap.end(); jt++ ) {
			int blobId( jt->first );
			counter[ blobId ] = 0;
			for( vector< tokenValue::tokenValuePtr >::iterator source = tokenValuePtrArray.begin(), dest = tokenValuePtrArrayMap[ blobId ].begin(); source < tokenValuePtrArray.end(); source++, dest++ ) {
				for( unsigned i = 0; i < numParticles; i++ ) {
					int sourceBlobId = *( ( int* )( *blobbyIdMap.get() )[ i ] );
					if( sourceBlobId == blobId ) {
						switch( ( *source )->getType() ) {
							case tokenValue::typeFloat: {
								*( ( float* ) ( *( dest->get() ) )[ counter[ jt->first ]++ ] ) = *( ( float* ) ( *( source->get() ) )[ i ] );
								break;
							}
							case tokenValue::typeInteger: {
								*( ( int* ) ( *( dest->get() ) )[ counter[ jt->first ]++ ] ) = *( ( int* ) ( *( source->get() ) )[ i ] );
								break;
							}
							case tokenValue::typePoint: {
								*( ( float* ) ( *( dest->get() ) )[ 3 * counter[ jt->first ]     ] ) = *( ( float* ) ( *( source->get() ) )[ 3 * i     ] );
								*( ( float* ) ( *( dest->get() ) )[ 3 * counter[ jt->first ] + 1 ] ) = *( ( float* ) ( *( source->get() ) )[ 3 * i + 1 ] );
								*( ( float* ) ( *( dest->get() ) )[ 3 * counter[ jt->first ] + 2 ] ) = *( ( float* ) ( *( source->get() ) )[ 3 * i + 2 ] );
								counter[ jt->first ]++;
								break;
							}
							case tokenValue::typeHomogenousPoint: {
								*( ( float* ) ( *( dest->get() ) )[ 4 * counter[ jt->first ]     ] ) = *( ( float* ) ( *( source->get() ) )[ 4 * i     ] );
								*( ( float* ) ( *( dest->get() ) )[ 4 * counter[ jt->first ] + 1 ] ) = *( ( float* ) ( *( source->get() ) )[ 4 * i + 1 ] );
								*( ( float* ) ( *( dest->get() ) )[ 4 * counter[ jt->first ] + 2 ] ) = *( ( float* ) ( *( source->get() ) )[ 4 * i + 2 ] );
								*( ( float* ) ( *( dest->get() ) )[ 4 * counter[ jt->first ] + 3 ] ) = *( ( float* ) ( *( source->get() ) )[ 4 * i + 3 ] );
								counter[ jt->first ]++;
								break;
							}
						}
					}
				}
			}
		}

		message( L"---", messageError );
		message( CValue( ( long )tokenValuePtrArrayMap.size() ).GetAsText(), messageError );
		message( L"---", messageError );
	}*/

	void particleData::write() const {
		using namespace ueberMan;
		ueberManInterface theRenderer;

		for( tokenValue::tokenValuePtrVector::const_iterator it = tokenValuePtrArray.begin(); it < tokenValuePtrArray.end(); it++ )
			theRenderer.parameter( **it );

		if( numParticles ) {
			if( isMultiGroupBlob ) {
				vector< string > str;
				theRenderer.blobby( numParticles, code, ppos, str, const_cast< string& >( identifier ) );
			} else {
				theRenderer.points( typeStr, numParticles, const_cast< string& >( identifier ) );
			}
		} else {
			theRenderer.translate( .0f, .0f, .0f );
		}
	}

/*
	void particleData::startGrain() {
		numParticlesIt	= numParticlesMap.begin();
		tokenValueIt	= tokenValuePtrArrayMap.begin();
	}

	unsigned particleData::granularity() const {
		return numParticlesMap.size();
	}

	void particleData::writeNextGrain() {
		if( numParticlesIt != numParticlesMap.end() ) {

			using namespace ueberMan;
			ueberManInterface theRenderer;

			for( vector< tokenValue::tokenValuePtr >::const_iterator it = tokenValueIt->second.begin(); it < tokenValueIt->second.end(); it++ ) {
				theRenderer.parameter( **it );
				message( stringToCString( ( *it )->name() ), messageError );
				message( CValue( ( long )( *it )->size() ).GetAsText(), messageError );
			}

			theRenderer.points( "blobby", numParticlesIt->second, const_cast< string& >( identifier ) );

			theRenderer.translate( 0, 0, 0 );

			message( CValue( ( long )tokenValuePtrArrayMap.size() ).GetAsText(), messageError );
			message( CValue( ( long )numParticlesMap.size() ).GetAsText(), messageError );

			++numParticlesIt;
			//++tokenValueIt++;
		}
	}*/

	vector< float > particleData::boundingBox() const {
		return bound;
	}
}
