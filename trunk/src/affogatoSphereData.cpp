/** Sphere data container class.
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
#include <set>

// Boost headers
#include <boost/algorithm/string/case_conv.hpp>

// XSI headers
#include <xsi_application.h>
#include <xsi_group.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>
#include <xsi_model.h>
#include <xsi_sceneitem.h>

// Affogato headers
#include "affogatoGlobals.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoSphereData.hpp"
#include "affogatoRenderer.hpp"


namespace affogato {

	using namespace XSI;
	using namespace MATH;
	using namespace std;

	CRefArray getGroupMembers( CString groupName ) {
		Application app;
		Model sceneRoot( app.GetActiveSceneRoot() );
		CRefArray objects( sceneRoot.FindChildren( CString(), siSpherePrimType, CStringArray() ) );
		CRefArray members;

		for( unsigned i = 0; i < ( unsigned )objects.GetCount(); i++ ) {
			X3DObject x( objects[ i ] );
			if( isVisible( x ) ) {
				CRefArray owners( x.GetOwners() );
				CRefArray groups;
				owners.Filter( siGroup, CStringArray(), CString(), groups );
				for( unsigned j = 0; j < ( unsigned )groups.GetCount(); j++ ) {
					if( groupName == Group( groups[ j ] ).GetFullName() ) {
						members.Add( objects[ i ] );
					}
				}
			}
		}
		return members;
	}

	CRefArray filterForBlobbies( const long id ) {
		CRefArray blobs;

		Application app;

		Model sceneRoot = app.GetActiveSceneRoot();
		CRefArray spheres( sceneRoot.FindChildren( CString(), siSpherePrimType, CStringArray() ) );

		for( unsigned i = 0; i < ( unsigned )spheres.GetCount(); i++ ) {
			X3DObject x( spheres[ i ] );
			if( isVisible( x ) ) {
				CRefArray props( x.GetProperties() );
				for( unsigned property = 0; property < ( unsigned )props.GetCount(); property++ ) {
					Property prop( props[ property ] );
					if( isAffogatoProperty( prop ) ) {
						CValue blobbyId( prop.GetParameterValue( L"blobbyid" ) );
						long sphereId( blobbyId );
						if( !blobbyId.IsEmpty() && ( sphereId == id ) ) {
							blobs.Add( spheres[ i ] );
						}
					}
				}
			}
		}

		return blobs;
	}


	sphereData::sphereData( const X3DObject &sphere, double atTime ) {
		globals& g = const_cast< globals& >( globals::access() );

		radius = sphere.GetParameterValue( L"radius", atTime );

		CMatrix4 mat( sphere.GetKinematics().GetGlobal().GetTransform( atTime ).GetMatrix4() );
		mat.InvertInPlace();
		matrix = CMatrix4ToFloat( mat );

		rType = renderTypeSphere;

		CRefArray newMembers;

		// We find all properties
		CRefArray props( sphere.GetProperties() );
		for( unsigned property = 0; property < ( unsigned )props.GetCount(); property++ ) {

			Property prop( props[ property ] );
			if( isAffogatoProperty( prop ) ) {
				CValue blobbyId( prop.GetParameterValue( L"blobbyid" ) );
				long id( blobbyId );

				if( !blobbyId.IsEmpty() ) {
					if( g.blobbyGroupsMap[ ( float )atTime ].end() == g.blobbyGroupsMap[ ( float )atTime ].find( id ) ) {
						g.blobbyGroupsMap[ ( float )atTime ].insert( id ); // record this blobby group

						rType = renderTypeBlobby;

						CRefArray members( filterForBlobbies( id ) );

						newMembers += members;
					}
				}
			}
		}

		numBlobs = 0;
		numBlobs = newMembers.GetCount();

		if( numBlobs ) {
			vector< int > codeIndices;
			codeIndices.reserve( newMembers.GetCount() );
			floatData.reserve( floatData.size() + newMembers.GetCount() * 16 );
			code.reserve( code.size() + newMembers.GetCount() * 2 );

			int floatOn = 0;

			for( unsigned member = 0; member < numBlobs; member++ ) {
				X3DObject obj( newMembers[ member ] );

				codeIndices.push_back( member );

				code.push_back( 1001 ); // Ellipsoid
				code.push_back( floatOn ); // Float array index for transformations

				CTransformation trans( obj.GetKinematics().GetGlobal().GetTransform( atTime ) );
				CVector3 scale( trans.GetScaling() );
				scale.ScaleInPlace( 2 * ( float )obj.GetParameterValue( L"radius", atTime ) );
				trans.SetScaling( scale );

				const vector< float >& m = CMatrix4ToFloat( trans.GetMatrix4() );
				floatData.insert( floatData.end(), m.begin(), m.end() );

				floatOn += 16;
			}

			code.push_back( 0 ); // Sum
			code.push_back( numBlobs );

			code.insert( code.end(), codeIndices.begin(), codeIndices.end() );
		}

		bound.resize( 6 );
		if( renderTypeSphere == rType ) {
			bound[ 0 ] = bound[ 2 ] = bound[ 4 ] = -radius;
			bound[ 5 ] = bound[ 3 ] = bound[ 1 ] =  radius;
		} else {
			bound[ 5 ] = bound[ 3 ] = bound[ 1 ] = numeric_limits< float >::max();
			bound[ 0 ] = bound[ 2 ] = bound[ 4 ] = numeric_limits< float >::min();
		}
	}

/* // Old code -- uses groups for more flexibility but is heaps slow
   // -- probably because XSI < 5.0 doesn't have Group::GetMembers()
   // So I had to roll my own.
	sphereData::sphereData( const X3DObject &sphere, double atTime ) {
		globals& g = const_cast< globals& >( globals::access() );

		radius = sphere.GetParameterValue( L"radius" );

		CMatrix4 mat( sphere.GetKinematics().GetGlobal().GetTransform( atTime ).GetMatrix4() );
		mat.InvertInPlace();
		matrix = CMatrix4ToFloat( mat );

		numBlobs = 0;
		int floatOn = 0;
		rType = renderTypeSphere;
		int codeIndex = 0;
		vector< int > maxBlobs;

		maxBlobs.push_back( 0 );
		maxBlobs.push_back( 0 );

		map< string, int > blobSet;

		// We find all properties
		CRefArray props( sphere.GetProperties() );
		for( unsigned property = 0; property < props.GetCount(); property++ ) {
			Property prop( props[ property ] );
			// Is this a blobby sphere?
			if( isAffogatoProperty( prop ) && ( bool )prop.GetParameterValue( L"blobby" ) ) {
				CRefArray owners( prop.GetOwners() );
				CRefArray groups;
				owners.Filter( siGroup, CStringArray(), CString(), groups );
				// Go through all groups this sphere is a member of
				for( unsigned group = 0; group < groups.GetCount(); group++ ) {
					CString groupName = Group( groups[ group ] ).GetFullName();
					string groupNameStr = CStringToString( groupName );
					// If we haven't written out the blobby for this group yet
					if( g.blobbyGroupsMap[ atTime ].end() == g.blobbyGroupsMap[ atTime ].find( groupNameStr ) ) {
						g.blobbyGroupsMap[ atTime ].insert( groupNameStr );
						CRefArray members( getGroupMembers( groupName ) );
						CRefArray tmp;
						members.Filter( siSpherePrimType, CStringArray(), CString(), tmp );
						members = tmp;

						int numAddedBlobs = 0;

						if( members.GetCount() ) {

							rType = renderTypeBlobby;

							vector< int > codeIndices;
							codeIndices.reserve( members.GetCount() );
							floatData.reserve( floatData.size() + members.GetCount() * 16 );
							code.reserve( code.size() + members.GetCount() * 2 );

							for( unsigned member = 0; member < members.GetCount(); member++ ) {
								X3DObject obj( members[ member ] );

								string name( CStringToString( obj.GetFullName() ) );
								if( blobSet.end() == blobSet.find( name ) ) {
									// add the sphere to the blob
									blobSet[ name ] = codeIndex + numAddedBlobs;
									codeIndices.push_back( codeIndex + numAddedBlobs );
									numAddedBlobs++;

									code.push_back( 1001 ); // Ellipsoid
									code.push_back( floatOn ); // Float array index for transformations

									CTransformation trans( obj.GetKinematics().GetGlobal().GetTransform( atTime ) );
									CVector3 scale( trans.GetScaling() );
									scale.ScaleInPlace( 2 * ( float )obj.GetParameterValue( L"radius", atTime ) );
									trans.SetScaling( scale );

									const vector< float >& m = CMatrix4ToFloat( trans.GetMatrix4() );
									floatData.insert( floatData.end(), m.begin(), m.end() );

									floatOn += 16;
								} else {
									// We already recorde the blob -> find the index
									codeIndices.push_back( blobSet[ name ] );
								}
							}

							code.push_back( 0 ); // Sum
							code.push_back( members.GetCount() );

							code.insert( code.end(), codeIndices.begin(), codeIndices.end() );

							numBlobs += numAddedBlobs;
							codeIndex += numAddedBlobs;
							maxBlobs.push_back( codeIndex );
							codeIndex++;
						}
					} else {
						rType = renderTypeIgnore;
					}
				}
			}
		}

		maxBlobs[ 1 ] = maxBlobs.size() - 2;
		code.insert( code.end(), maxBlobs.begin(), maxBlobs.end() );

#ifdef RSP
		/*
		isBlobby = false;

		for( unsigned i = 0; i < props.GetCount(); i++ ) {
			Property prop( props[ i ] );

			if( isAffogatoProperty( prop ) ) {
				CParameterRefArray params = prop.GetParameters();

				for( int p = 0; p < params.GetCount(); p++ ) {
					Parameter param( params[ p ] );
					string paramName( CStringToString( param.GetName() ) );
					boost::to_lower( paramName );
					if( "blobby" == paramName.substr( 0, 6 ) ) {
						isBlobby = true;
						switch( paramName[ paramName.length() - 1 ] ) {
							case 'x':
								blobs[ paramName.substr( 6, paramName.length() - 8 ) ].x = param.GetValue( g.animation.time );
								break;
							case 'y':
								blobs[ paramName.substr( 6, paramName.length() - 8 ) ].y = param.GetValue( g.animation.time );
								break;
							case 'z':
								blobs[ paramName.substr( 6, paramName.length() - 8 ) ].z = param.GetValue( g.animation.time );
								break;
							case 'r':
								blobs[ paramName.substr( 6, paramName.length() - 8 ) ].size = 2 * ( long )param.GetValue( g.animation.time );
								break;
						}
					}
				}
			}
		}

		if( isBlobby ) {

			blobs[ "rspTopSecretMotherBlob" ].x =
			blobs[ "rspTopSecretMotherBlob" ].y =
			blobs[ "rspTopSecretMotherBlob" ].z = 0;
			blobs[ "rspTopSecretMotherBlob" ].size = radius;

			int floatOn = 0;

			for( map< string, blob >::const_iterator it = blobs.begin(); it != blobs.end(); it++ ) {

				// add the particle to the list.
				code.push_back( 1001 );
				code.push_back( floatOn );

				float size = it->second.size;

				floatData.push_back( size );
				floatData.push_back( 0.0 );
				floatData.push_back( 0.0 );
				floatData.push_back( 0.0 );

				floatData.push_back( 0.0 );
				floatData.push_back( size );
				floatData.push_back( 0.0 );
				floatData.push_back( 0.0 );

				floatData.push_back( 0.0 );
				floatData.push_back( 0.0 );
				floatData.push_back( size );
				floatData.push_back( 0.0 );

				floatData.push_back( it->second.x );
				floatData.push_back( it->second.y );
				floatData.push_back( it->second.z );
				floatData.push_back( 1.0 );

				floatOn += 16;
			}

			if( !blobs.empty() ) {
				code.push_back( 0 );
				code.push_back( blobs.size() );

				for( int k = 0; k < blobs.size(); k++ )
					code.push_back( k );
			}
		}*/
/*#endif

		bound.resize( 6 );
		bound[ 0 ] = bound[ 2 ] = bound[ 4 ] = -radius;
		bound[ 5 ] = bound[ 3 ] = bound[ 1 ] = radius;
	}

/*
Application.AddProp("Custom_parameter_list", "", "", "affogato")
Application.SIAddCustomParameter("sphere.affogato", "blobby666x", "siDouble", 0, 0, 1, "", 5, 0, 1, "", "")
Application.SIAddCustomParameter("sphere.affogato", "blobby666y", "siDouble", 0, 0, 1, "", 5, 0, 1, "", "")
Application.SIAddCustomParameter("sphere.affogato", "blobby666z", "siDouble", 0, 0, 1, "", 5, 0, 1, "", "")
Application.SIAddCustomParameter("sphere.affogato", "blobby666r", "siDouble", 0, 0, 1, "", 5, 0, 1, "", "")
*/

	void sphereData::writeGeometry() const {
		using namespace ueberMan;
		ueberManInterface theRenderer;

		if( renderTypeIgnore != rType ) {

			for( vector< boost::shared_ptr< tokenValue > >::const_iterator it = tokenValuePtrArray.begin(); it < tokenValuePtrArray.end(); it++ )
				theRenderer.parameter( **it );

			switch( rType ) {
				case renderTypeSphere: {
					theRenderer.sphere( radius, const_cast< string& >( identifier ) );
					break;
				}
				case renderTypeBlobby: {
					theRenderer.blobby( numBlobs, code, floatData, stringData, const_cast< string& >( identifier ) );
					break;
				}
			}
		} else {
			// Make sure we write st. neutral out to fill the motion block we're likely in
			theRenderer.translate( 0, 0, 0 );
		}
	}

	void sphereData::writeTransform() const {
		using namespace ueberMan;
		ueberManInterface theRenderer;

		switch( rType ) {
			case renderTypeSphere: {
				theRenderer.rotate( 90, 1, 0, 0 );
				break;
			}
			case renderTypeBlobby: {
				theRenderer.appendSpace( matrix );
				break;
			}
		}
	}

	void sphereData::write() const {
		writeTransform();
		writeGeometry();
	}

	void sphereData::startGrain() {
		counter = 0;
	}

	void sphereData::writeNextGrain() {
		if( counter++ ) {
			writeGeometry();
			counter = 0;
		}
		else {
			writeTransform();
		}
	}

	unsigned sphereData::getGranularity() const {
		return 2;
	}

}
