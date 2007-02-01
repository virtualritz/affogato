/** NURB mesh data container class.
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
#include <stdio.h>
#include <vector>

// XSI headers
#include <xsi_primitive.h>
#include <xsi_nurbsdata.h>
#include <xsi_nurbssurfacemesh.h>

// Affogato headers
#include "affogatoGlobals.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoNurbMeshData.hpp"
#include "affogatoRenderer.hpp"


namespace affogato {

	using namespace XSI;
	using namespace std;

	nurbMeshData::~nurbMeshData() {
		if( NULL != uKnot )
			delete[] uKnot;
		if( NULL != vKnot )
			delete[] vKnot;
	}

	nurbMeshData::nurbMeshData( const Primitive &nurbMeshPrim, double atTime, bool usePref, double atPrefTime ) {
		globals& g = const_cast< globals& >( globals::access() );

		NurbsSurfaceMesh nurbSurfaceMesh( nurbMeshPrim.GetGeometry( atTime ) );

		identifier = getAffogatoName( CStringToString( X3DObject( nurbMeshPrim.GetParent() ).GetFullName() ) );

		// Add two surfaces to the mesh.
		CNurbsSurfaceDataArray nurbSurfaceDataArray;
		nurbSurfaceMesh.Get( siIGESNurbs, nurbSurfaceDataArray );

		// set the type of geometry
		if( g.geometry.nonRationalNurbSurface )
			tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( nurbSurfaceDataArray[ 0 ].m_aControlPoints, "P", tokenValue::storageVertex, tokenValue::typePoint ) ) );
		else
			tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( nurbSurfaceDataArray[ 0 ].m_aControlPoints, "Pw" ) ) );

		if( usePref ) {
			NurbsSurfaceMesh nurbSurfaceMeshPref = nurbMeshPrim.GetGeometry( atPrefTime );
			nurbSurfaceMeshPref.Get( siIGESNurbs, nurbSurfaceDataArray );
			if( g.geometry.nonRationalNurbSurface )
				tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( nurbSurfaceDataArray[ 0 ].m_aControlPoints, "__Pref", tokenValue::storageVertex, tokenValue::typePoint ) ) );
			else
				tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( nurbSurfaceDataArray[ 0 ].m_aControlPoints, "__Pref" ) ) );
		}

		// grab the order information
		uOrder = nurbSurfaceDataArray[ 0 ].m_lUDegree + 1;
		vOrder = nurbSurfaceDataArray[ 0 ].m_lVDegree + 1;

		numCVsU = nurbSurfaceDataArray[ 0 ].m_lNbUControlPoints;
		numCVsV = nurbSurfaceDataArray[ 0 ].m_lNbVControlPoints;

		int numKnotsU = nurbSurfaceDataArray[ 0 ].m_aUKnots.GetCount();
		uKnot = new float[ numKnotsU ];
		for( int knot = 0; knot < numKnotsU; knot++ )
			uKnot[ knot ] = ( float )nurbSurfaceDataArray[ 0 ].m_aUKnots[ knot ];


		int numKnotsV = nurbSurfaceDataArray[ 0 ].m_aVKnots.GetCount();
		vKnot = new float[ numKnotsV ];
		for( int knot = 0; knot < numKnotsV; knot++ )
			vKnot[ knot ] = ( float )nurbSurfaceDataArray[ 0 ].m_aVKnots[ knot ];

		if( g.geometry.normalizeNurbKnotVector ) {
			float start;
			float scale;

			// U
			if( nurbSurfaceDataArray[ 0 ].m_bUClosed ) {
				start = uKnot[ 3 ];
				scale = 1 / ( uKnot[ numKnotsU - 4 ] - start );
			} else {
				start = uKnot[ 0 ];
				scale = 1 / ( uKnot[ numKnotsU - 1 ] - start );
			}
			for( int knot = 0; knot < numKnotsU; knot++ )
				uKnot[ knot ] = ( uKnot[ knot ] - start ) * scale;

			// V
			if( nurbSurfaceDataArray[ 0 ].m_bVClosed ) {
				start = vKnot[ 3 ];
				scale = 1 / ( vKnot[ numKnotsV - 4 ] - start );
			} else {
				start = vKnot[ 0 ];
				scale = 1 / ( vKnot[ numKnotsV - 1 ] - start );
			}
			for( int knot = 0; knot < numKnotsV; knot++ )
				vKnot[ knot ] = ( vKnot[ knot ] - start ) * scale;
		}

		uMin = uKnot[ uOrder - 1 ];
		uMax = uKnot[ numKnotsU - uOrder ];

		vMin = vKnot[ vOrder - 1 ];
		vMax = vKnot[ numKnotsV - vOrder ];

		if( g.animation.time == atTime ) {
			bound = affogato::getBoundingBox( nurbMeshPrim, atTime );
		} else {
			bound.resize( 6 );
			bound[ 5 ] = bound[ 3 ] = bound[ 1 ] = numeric_limits< float >::max();
			bound[ 0 ] = bound[ 2 ] = bound[ 4 ] = numeric_limits< float >::min();
		}
	}

	vector< float > nurbMeshData::boundingBox() const {
		return bound;
	}

	void nurbMeshData::write() const {
		using namespace ueberMan;
		ueberManInterface theRenderer;

		for( tokenValue::tokenValuePtrVector::const_iterator it = tokenValuePtrArray.begin(); it < tokenValuePtrArray.end(); it++ ) {
			theRenderer.parameter( **it );
		}
		theRenderer.patch( numCVsU, uOrder, uKnot, uMin, uMax, numCVsV, vOrder, vKnot, vMin, vMax, const_cast< string& >( identifier ) );
	}

}
