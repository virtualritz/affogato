/** Poly mesh (and subdivison surface) data container class.
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
#include <limits>

// Boost headers
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/case_conv.hpp>

// XSI headers
#include <xsi_application.h>
#include <xsi_cluster.h>
#include <xsi_edge.h>
#include <xsi_point.h>
#include <xsi_polygonface.h>
#include <xsi_polygonnode.h>
#include <xsi_polygonmesh.h>
#include <xsi_primitive.h>
#include <xsi_vertex.h>
#include <xsi_x3dobject.h>

// Affogato headers
#include "affogatoGlobals.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoPolyMeshData.hpp"
#include "affogatoRenderer.hpp"


namespace affogato {

	using namespace XSI;
	using namespace MATH;
	using namespace std;

	polyMeshData::~polyMeshData() {
		// Nothing to destruct
	}

	polyMeshData::polyMeshData( const Primitive &polyMeshPrim, double atTime, bool usePref, double atPrefTime ) {
		const globals& g( globals::access() );

		identifier = getAffogatoName( X3DObject( polyMeshPrim.GetParent() ).GetFullName().GetAsciiString() );

		Geometry meshGeo( polyMeshPrim.GetGeometry( atTime ) );
		PolygonMesh mesh( meshGeo );

		debugMessage( L"Aquiring polyMesh primitive" );

		CRefArray props( X3DObject( polyMeshPrim.GetParent() ).GetProperties() );
		subDivScheme = 1;
		boundary = boundarySharp;
		unsigned exitLoop = 0;
		for( unsigned i = 0; i < ( unsigned )props.GetCount(); i++ ) {
			Property prop( props[ i ] );

			if( isAffogatoProperty( prop ) ) {
				CParameterRefArray params = prop.GetParameters();
				for( unsigned p = 0; p < ( unsigned )params.GetCount(); p++ ) {
					Parameter param( params[ p ] );
					string paramName( param.GetName().GetAsciiString() );
					boost::to_lower( paramName );
					if( "boundarytype" == paramName ) {
						boundary = static_cast< boundaryType >( ( unsigned short )param.GetValue( g.animation.time ) );
						exitLoop++;
						break;
					}
				}
			} else if( CString( siGeomApproxType ) == prop.GetType() ) {
				subDivScheme = ( short int )prop.GetParameterValue( L"gapproxmordrsl" );
				exitLoop++;
			}
			if( 1 < exitLoop )
				break;
		}

		debugMessage( L"Getting vertices and faces" );

		//parameter( "uniform edge crease", 1.0, { 3, 4, 5, 6, 7 } ):

		CPointRefArray vertices( mesh.GetPoints() );
		CFacetRefArray facets( mesh.GetFacets() );
		CPolygonFaceRefArray polys( mesh.GetPolygons () );

		numFaces = facets.GetCount();
		nverts = boost::shared_ptr< int >( new int[ numFaces ], arrayDeleter() );
		verts  = boost::shared_ptr< int >( new int[ mesh.GetPolygons().GetPolygonNodePolygonFaceIndexArray().GetCount() - numFaces ], arrayDeleter() );

		unsigned vertexIndex = 0;

		for( unsigned face = 0; face < ( unsigned )numFaces; face++ ) {
			Facet facet( facets.GetItem( face ) );

			CLongArray points( facet.GetPoints().GetIndexArray() );
			nverts.get()[ face ] = points.GetCount();

			for( unsigned vertex = 0; vertex < ( unsigned )nverts.get()[ face ]; vertex++ ) {
				verts.get()[ vertexIndex + vertex ] = points[ vertex ];
			}
			vertexIndex += nverts.get()[ face ];
		}

		unsigned numVertices( vertices.GetCount() );
		unsigned numVerticesForAllFaces( vertexIndex );

		tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( vertices, "P", tokenValue::storageVertex, tokenValue::typePoint ) ) );

		if( subDivScheme ) {
			// Creases
			debugMessage( L"Doing creases" );

			CEdgeRefArray edges( mesh.GetEdges() );
			if( edges.GetCreaseArray().GetCount() ) {
				for( unsigned i = 0; i < ( unsigned )edges.GetCount(); i++ ) {
					Edge e( edges.GetItem( i ) );
					float creaseVal( ( float )e.GetCrease() );
					if( creaseVal ) {
						if( e.GetIsHard() )
							creaseVal = 1e38f;

						CVertexRefArray verts( e.GetVertices() );
						unsigned count( ( unsigned )verts.GetCount() );
						boost::shared_ptr< int > creases( new int[ count ] );
						for( unsigned j = 0; j < count; j++ ) {
							Vertex v( verts.GetItem( j ) );
							creases.get()[ j ] = v.GetIndex();
						}
						tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( creaseVal, "creasevalue" ) ) );
						tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( creases, count, "crease", tokenValue::storageUniform ) ) );
					}
				}
			}

			// Corners
			debugMessage( L"Doing corners" );

			CVertexRefArray verts( mesh.GetVertices() );
			if( verts.GetCreaseArray().GetCount() ) {
				for( unsigned i = 0; i < ( unsigned )verts.GetCount(); i++ ) {
					Vertex v( verts.GetItem( i ) );
					float creaseVal( ( float )v.GetCrease() );
					if( creaseVal ) {
						tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( creaseVal, "cornervalue" ) ) );
						tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( ( int )v.GetIndex(), "corner" ) ) );
					}
				}
			}
		}

		debugMessage( L"Doing parameters" );
		// Do [motion blurred] parameters
		if( g.motionBlur.geometryParameterBlur || ( g.animation.time == atTime ) ) {
			if( usePref ) {
				Geometry meshGeoPref( polyMeshPrim.GetGeometry( atPrefTime ) );
				PolygonMesh meshPref( meshGeo );
				CPointRefArray verticesPref( meshPref.GetPoints() );
				tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( verticesPref, "__Pref", tokenValue::storageVertex, tokenValue::typePoint ) ) );
			}
			if( !subDivScheme ) // Add normals for polygon mesh
				tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( polys.GetPolygonNodeNormalArray(), "N", tokenValue::storageFaceVarying, tokenValue::typeNormal ) ) );
		}

		debugMessage( L"Doing optional variables" );
		// Do [motion blurred] variables
		if( g.motionBlur.geometryVariableBlur || ( g.animation.time == atTime ) ) {

			CRefArray clusters;
			CStringArray families;
			families.Add( siClusterFamily );
			//siSampledPointCluster
			meshGeo.GetClusters().Filter( CString(), families, CString(), clusters );

			// ----------------

			debugMessage( L"Doing UVs" );

			CLongArray completeNodeIndices( mesh.GetNodes().GetIndexArray() );

			if( clusters.GetCount() ) {
				CLongArray polyNodePerPolygon( mesh.GetPolygons().GetPolygonNodePolygonFaceIndexArray() );

				for( unsigned i( 0 ); i < ( unsigned )clusters.GetCount(); i++ ) {
					Cluster cluster( clusters[ i ] );

					//UVs
					boost::shared_ptr< float > uvCoordinates( new float[ 2 * numVerticesForAllFaces ], arrayDeleter() );

					CRefArray uvProperties;
					cluster.GetProperties().Filter( siClsUVSpaceTxtType, CStringArray(), CString(), uvProperties );

					//To take into account the cluster offset.
					CLongArray clusterOffsetIndices;
					cluster.FindIndices( completeNodeIndices, clusterOffsetIndices );

					for( unsigned uvprops = 0; uvprops < ( unsigned )uvProperties.GetCount(); uvprops++ ) {
						CRef uvNow = ClusterProperty( uvProperties[ uvprops ] ).EvaluateAt( atTime );
						ClusterProperty prop( uvNow );

						CClusterPropertyElementArray uvwElements( prop.GetElements() );
						CDoubleArray uvs( uvwElements.GetArray() );
						unsigned uvwValueSize( uvwElements.GetValueSize() );

						//For each polygon
						unsigned indexInPolyNodePerPolygon( 0 );
						unsigned index( 0 );
						do {
							// For each nodes in a polygon
							// I'm using j as one based because we need to jump after
							// the number of nodes per polygon.
							for( unsigned j = 1; j <= ( unsigned )polyNodePerPolygon[ indexInPolyNodePerPolygon ]; j++ ) {
								long samplePointGeoIndex( polyNodePerPolygon[ indexInPolyNodePerPolygon + j ] );
								long clusterIndex( clusterOffsetIndices[ samplePointGeoIndex ] );
								uvCoordinates.get()[ 2 * index ]		= ( float )uvs[ clusterIndex * uvwValueSize ];
								uvCoordinates.get()[ 2 * index + 1 ]	= 1.0f - ( float )uvs[ clusterIndex * uvwValueSize + 1 ];
								index++;
							}
							indexInPolyNodePerPolygon += 1 + polyNodePerPolygon[ indexInPolyNodePerPolygon ];
						}
						while( indexInPolyNodePerPolygon < ( unsigned )polyNodePerPolygon.GetCount() );

						string setname( prop.GetName().GetAsciiString() );

						if( "Texture_Projection" == setname ) // The default name gets translated to the RMan default name
							setname = "st";

						setname += "[2]";

						tokenValue::storageClass interpolationClass;
						if( subDivScheme )
							interpolationClass = tokenValue::storageFaceVertex; // Subdivs get facevertex
						else
							interpolationClass = tokenValue::storageFaceVarying; // Polys get facevarying

						tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( uvCoordinates, 2 * vertexIndex, setname, interpolationClass, tokenValue::typeFloat ) ) );
					}

					// Vertex colors
					debugMessage( L"Doing vertex colors" );

					boost::shared_ptr< float > vertexColors( new float[ 3 * numVerticesForAllFaces ], arrayDeleter() );

					CRefArray colorProperties;
					cluster.GetProperties().Filter( L"vertexColor", CStringArray(), CString(), colorProperties );

					// Take into account the cluster offset.
					cluster.FindIndices( completeNodeIndices, clusterOffsetIndices );

					for( unsigned colorProp( 0 ); colorProp < ( unsigned )colorProperties.GetCount(); colorProp++ ) {
						CRef colorNow = ClusterProperty( colorProperties[ colorProp ] ).EvaluateAt( atTime );
						ClusterProperty prop( colorNow );

						CClusterPropertyElementArray colorElements( prop.GetElements() );
						CDoubleArray colors( colorElements.GetArray() );
						unsigned colorValueSize( colorElements.GetValueSize() );

						// For each polygon
						unsigned indexInPolyNodePerPolygon( 0 );
						unsigned index( 0 );
						do {
							// For each nodes in a polygon
							// Using j as one based because we need to jump after the number of nodes per polygon.
							for( unsigned j = 1; j <= ( unsigned )polyNodePerPolygon[ indexInPolyNodePerPolygon ]; j++ ) {
								long samplePointGeoIndex( polyNodePerPolygon[ indexInPolyNodePerPolygon + j ] );
								long clusterIndex( clusterOffsetIndices[ samplePointGeoIndex ] );
								vertexColors.get()[ 3 * index ]		= ( float )colors[ clusterIndex * colorValueSize ];
								vertexColors.get()[ 3 * index + 1 ]	= ( float )colors[ clusterIndex * colorValueSize + 1 ];
								vertexColors.get()[ 3 * index + 2 ]	= ( float )colors[ clusterIndex * colorValueSize + 2 ];
								index++;
							}
							indexInPolyNodePerPolygon += 1 + polyNodePerPolygon[ indexInPolyNodePerPolygon ];
						}
						while( indexInPolyNodePerPolygon < ( unsigned )polyNodePerPolygon.GetCount() );

						string setname( prop.GetName().GetAsciiString() );

						if( "Vertex_Color" == setname ) // The default name gets translated to the RMan default name
							setname = "Cs";

						tokenValue::storageClass interpolationClass;
						if( subDivScheme )
							interpolationClass = tokenValue::storageFaceVertex; // Subdivs get facevertex
						else
							interpolationClass = tokenValue::storageFaceVarying; // Polys get facevarying

						tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( vertexColors, 3 * vertexIndex, setname, interpolationClass, tokenValue::typeColor ) ) );
					}
				}
			}

			meshGeo.GetClusters().Filter( siVertexCluster, CStringArray(), CString(), clusters );

			if( clusters.GetCount() ) {

				for( unsigned i( 0 ); i < ( unsigned )clusters.GetCount(); i++ ) {
					Cluster cluster( clusters[ i ] );

					// Weight maps
					debugMessage( L"Doing weight maps" );

					boost::shared_ptr< float > vertexWeights( new float[ numVertices ], arrayDeleter() );

					CRefArray weightProperties;
					cluster.GetProperties().Filter( siWgtMapType, CStringArray(), CString(), weightProperties );

					for( unsigned weightProp( 0 ); weightProp < ( unsigned )weightProperties.GetCount(); weightProp++ ) {
						CRef weightNow = ClusterProperty( weightProperties[ weightProp ] ).EvaluateAt( atTime );
						ClusterProperty prop( weightNow );

						CClusterPropertyElementArray weightElements( prop.GetElements() );
						CDoubleArray weights( weightElements.GetArray() );
						long weightValueSize( weightElements.GetValueSize() );

						unsigned index = 0;
						for( unsigned j = 0; j < numVertices; j++ ) {
							long clusterIndex;
							cluster.FindIndex( j, clusterIndex );
							if( -1 == clusterIndex ) {
								vertexWeights.get()[ index ] = 0.0f;
							} else {
								vertexWeights.get()[ index ] = ( float )weights[ clusterIndex * weightValueSize ];
							}
							index++;
						}

						string setname( prop.GetName().GetAsciiString() );

						tokenValuePtrArray.push_back( boost::shared_ptr< tokenValue >( new tokenValue( vertexWeights, numVertices, setname, tokenValue::storageVertex, tokenValue::typeFloat ) ) );
					}
				}
			}
		}

		debugMessage( L"All done" );

		bound = affogato::getBoundingBox( polyMeshPrim, atTime );
	}

	vector< float > polyMeshData::boundingBox() const {
		return bound;
	}

	void polyMeshData::write() const {
		using namespace ueberMan;
		ueberManInterface theRenderer;

		for( vector< boost::shared_ptr< tokenValue > >::const_iterator it = tokenValuePtrArray.begin(); it < tokenValuePtrArray.end(); it++ ) {
			theRenderer.parameter( **it );
		}
		theRenderer.mesh( subDivScheme ? "catmull-clark" : "linear", numFaces, nverts.get(), verts.get(), boundary == boundarySharp, const_cast< string& >( identifier ) );
	}
}
