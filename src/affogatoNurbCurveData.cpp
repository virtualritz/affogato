/** NURB curve data container class.
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
#include <vector>

// Boost headers
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/format.hpp>

// XSI headers
#include <xsi_application.h>
#include <xsi_cluster.h>
#include <xsi_controlpoint.h>
#include <xsi_customproperty.h>
#include <xsi_knot.h>
#include <xsi_nurbscurvelist.h>
#include <xsi_nurbscurve.h>
#include <xsi_primitive.h>
#include <xsi_userdatamap.h>
#include <xsi_x3dobject.h>

// Affogato headers
#include "affogatoGlobals.hpp"
#include "affogatoNurbCurveData.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoRenderer.hpp"

using namespace XSI;
using namespace MATH;

/*
inline CVector4 operator+( const CVector4 & v1, const CVector4 & v2 ) {
	CVector4 retval( v1 );
	retval += v2;
	return retval;
}

inline CVector4 operator-( const CVector4 & v1, const CVector4 & v2 ) {
	CVector4 retval( v1 );
	retval -= v2;
	return retval;
}

inline CVector4 operator*( double s, const CVector4& v1 ) {
	CVector4 retval( v1 );
	retval *= s;
	return retval;
}

inline CVector4 operator*( const CVector4& v1, double s ) {
	CVector4 retval( v1 );
	retval *= s;
	return retval;
}

// [ -1/6.,  3/6., -3/6.,  1/6.;  3/6., -6/6.,  3/6.,  0;	-3/6.,  0,     3/6.,  0;  1/6.,  4/6.,  1/6.,  0  ]

CMatrix4 bSplineBasis(	    -1/6.,  3/6., -3/6.,  1/6.,
							 3/6., -6/6.,  3/6.,  0,
							-3/6.,  0,     3/6.,  0,
							 1/6.,  4/6.,  1/6.,  0 );

//[	-1/2.,  3/2., -3/2.,  1/2.;  2/2., -5/2.,  4/2., -1/2.; -1/2.,  0,     1/2.,  0; 0,     2/2.,  0,     0 ]


CMatrix4 catmullRomBasis(	-1/2.,  3/2., -3/2.,  1/2.,
							 2/2., -5/2.,  4/2., -1/2.,
							-1/2.,  0,     1/2.,  0,
							 0,     2/2.,  0,     0 );

CMatrix4 catmullToBSpline( bSplineBasis );
//catmullToBSpline.InvertInPlace();

	/*
	CVector4Array bezierToBSpline( const CVector4Array& p ) {
		CVector4Array pts;

		pts.Add( 6. * p[ 0 ] -  7. * p[ 1 ] + 2. * p[ 2 ]               );
		pts.Add(                2. * p[ 1 ] -      p[ 2 ]               );
		pts.Add(               -1. * p[ 1 ] + 2. * p[ 2 ]               );
		pts.Add(                2. * p[ 1 ] - 7. * p[ 2 ] + 6. * p[ 3 ] );

		return pts;
	}


	CVector4Array catmullRomToBSpline( const CVector4Array& p ) {

		catmull to bezier
		| 0  6  0  0|       P0 =         Pb
		|-1  6  1  0| 1/6   P1 = ( -Pa +6Pb  +Pc     )/6
		| 0  1  6 -1|       P2 = (      +Pb +6Pc -Pd )/6
		| 0  0  6  0|       P3 = (            Pc

		bezier to b-spline
		| 6 -7  2  0|   |Pa|        P0 =  6Pa -7Pb +2Pc
		| 0  2 -1  0| * |Pb|        P1 =      +2Pb  -Pc
		| 0 -1  2  0|   |Pc|        P2 =       -Pb +2Pc
		| 0  2 -7  6|   |Pd|        P3 =      +2Pb -7Pc +6Pd

	}

	*/







namespace affogato {

	using namespace std;

	nurbCurveData::nurbCurveData( const Primitive &curveListPrim, double atTime, bool usePref, double atPrefTime ) {

		const globals& g( globals::access() );

		X3DObject parent( curveListPrim.GetParent() );

		identifier = getAffogatoName( CStringToString( parent.GetFullName() ) );

		float rootWidth( 0.0f );
		float tipWidth( 0.0f );
		float width( 0.0f );

		CRefArray props( getAffogatoProperties( parent ) );
		for( int i = 0; i < props.GetCount(); i++ ) {
			Property prop( props[ i ] );
			CParameterRefArray params( prop.GetParameters() );

			for( int p = 0; p < params.GetCount(); p++ ) {
				Parameter param( params[ p ] );
				string paramName = CStringToString( param.GetName() );

				replace_first( paramName, string( "_" ), string( ":" ) );
				//replace_first( paramName, string( "-" ), string( ":" ) );
				string userParamName = paramName;
				to_lower( paramName );

				if( "curverwidth" == paramName ) {
					if( 0 == width )
						width = ( float )param.GetValue();
				} else
				if( "curveradius" == paramName ) {
					if( 0 == width )
						width = ( float )param.GetValue() * 2;
				} else
				if( "curverootwidth" == paramName ) {
					if( 0 == tipWidth )
						tipWidth = ( float )param.GetValue();
				} else
				if( "curverootradius" == paramName ) {
					if( 0 == tipWidth )
						tipWidth = ( float )param.GetValue() * 2;
				} else
				if( "curvetipwidth" == paramName ) {
					if( 0 == rootWidth )
						rootWidth = ( float )param.GetValue();
				} else
				if( "curvetipradius" == paramName ) {
					if( 0 == rootWidth )
						rootWidth = ( float )param.GetValue() * 2;
				}
			}
			// we can't break here as there might be more than one affogato property
		}

		if( ( 0 != rootWidth ) && ( 0 != tipWidth ) && ( 0 == width ) ) {
			width = 0.5f * ( rootWidth + tipWidth );
		} else
		if( 0 == width ) {
			width = g.geometry.defaultNurbCurveWidth; // Fallback width
		}

		NurbsCurveList curveList( curveListPrim.GetGeometry( atTime ) );
		CNurbsCurveRefArray curves( curveList.GetCurves() );

		vector< float > curveLengths( numCurves );

		numCurves = curves.GetCount();
		if( numCurves ) {

			long numVarying( 0 );
			long numVertex( 0 );

			for( unsigned index( 0 ); index < ( unsigned )numCurves; index++ ) {
				NurbsCurve curve( curves[ index ] );

				double length;
				curve.GetLength( length );
				curveLengths.push_back( ( float )length );

				CVector4Array curvePoints;
				curve.GetControlPoints().GetArray( curvePoints );
				numVertsPerCurve.push_back( curvePoints.GetCount() );

				CKnotArray curveKnot( curve.GetKnots() );
				CDoubleArray curveKnotArray( curveKnot.GetArray() );
				unsigned numKnots( curveKnotArray.GetCount() );

				curveKnot.GetClosed( closed );
				curveKnot.GetDegree( degree );

				order.push_back( degree + 1 );

				numVarying	+= numVertsPerCurve.back() - order.back() + 1;
				numVertex	+= numVertsPerCurve.back();

				unsigned numKnotsCurve( numVertsPerCurve.back() + order.back() );

				vector< float > thisCurveKnots;
				thisCurveKnots.reserve( numKnotsCurve );

				for( unsigned knot( 0 ); knot < ( numKnotsCurve - numKnots ) / 2; knot++ )
					thisCurveKnots.push_back( ( float )curveKnotArray[ 0 ] );

				for( unsigned knot( 0 ); knot < numKnots; knot++ )
					thisCurveKnots.push_back( ( float )curveKnotArray[ knot ] );

				for( unsigned knot( 0 ); knot < ( numKnotsCurve - numKnots ) / 2; knot++ )
					thisCurveKnots.push_back( thisCurveKnots.back() );

				numKnots = numKnotsCurve;

				if( g.geometry.normalizeNurbKnotVector ) {
					float start;
					float scale;

					/*if( closed ) {
						start = thisCurveKnots.front();
						scale = 1 / ( thisCurveKnots.back() - start );
					} else {*/
						start = thisCurveKnots.front();
						scale = 1 / ( thisCurveKnots.back() - start );
					//}
					for( unsigned knot( 0 ); knot < ( unsigned )numKnots; knot++ )
						thisCurveKnots[ knot ] = ( thisCurveKnots[ knot ] - start ) * scale;
				}

				min.push_back( thisCurveKnots[ degree ] );
				max.push_back( thisCurveKnots[ numKnotsCurve - degree - 1 ] );

				knots.insert( knots.end(), thisCurveKnots.begin(), thisCurveKnots.end() );
			}
			numVarying += numCurves;

			CVector4Array curvePoints;
			curveList.GetControlPoints().GetArray( curvePoints );

			if( g.geometry.nonRationalNurbCurve )
				tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( curvePoints, "P", tokenValue::storageVertex, tokenValue::typePoint ) ) );
			else
				tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( curvePoints, "Pw" ) ) );

			if( g.motionBlur.geometryParameterBlur || ( g.animation.time == atTime ) ) {
				if( usePref ) {
					NurbsCurveList curveListAtPrefTime( curveListPrim.GetGeometry( atPrefTime ) );
					curveListAtPrefTime.GetControlPoints().GetArray( curvePoints );
					if( g.geometry.nonRationalNurbCurve )
						tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( curvePoints, "__Pref", tokenValue::storageVertex, tokenValue::typePoint ) ) );
					else
						tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( curvePoints, "__Pref" ) ) );
				}
			}

			tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( &curveLengths[ 0 ], curveLengths.size(), "length", tokenValue::storageUniform, tokenValue::typeFloat ) ) );
			curveLengths.clear();

			// Clusters
			CRefArray clusters;
			curveList.GetClusters().Filter( siVertexCluster, CStringArray(), CString(), clusters );

			bool userDataMapWidth( false );

			for( unsigned i( 0 ); i < ( unsigned )clusters.GetCount(); i++ ) {
				Cluster cluster( clusters[ i ] );
				CRefArray properties( cluster.GetProperties() );
				for( unsigned j( 0 ); j < ( unsigned )properties.GetCount(); j++ ) {
					Property prop( properties[ j ] );
					if( CString( L"UserDataMap" ) == prop.GetType() ) {
						UserDataMap userDataMap( prop );
						CustomProperty userDataTemplate( userDataMap.GetTemplate() );
						CParameterRefArray parms( userDataTemplate.GetParameters() );

						shared_ptr< float > data( new float[ numVertex ], arrayDeleter() );

						for( unsigned p( 0 ); p < ( unsigned )parms.GetCount(); p++ ) {
							Parameter aParm( parms[ p ] );

							if( !closed ) {
								for( long index( 0 ), xsiIndex = 0, dataIndex = 0; index < numCurves; index++ ) {
									for( long vertex( 0 ); vertex < numVertsPerCurve[ index ]; vertex++, dataIndex++ ) {
										long cindex;
										cluster.FindIndex( xsiIndex++, cindex );

										if( -1 < cindex ) {
											const unsigned char *cpData;
											unsigned int cntData;
											userDataMap.GetItemValue( cindex, cpData, cntData );
											userDataTemplate.PutBinaryData( cpData, cntData );

											data.get()[ dataIndex ] = aParm.GetValue();
										} else {
											data.get()[ dataIndex ] = 0;
										}
									}
								}
							} else {
								message( L"Vertex clusters are not supported for closed curves", messageWarning );
							}

							string name( CStringToString( aParm.GetName() ) );
							if( "width" == name ) {
								userDataMapWidth = true;
								tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( data, numVertex, name, tokenValue::storageVertex, tokenValue::typeFloat ) ) );
							} else {
								if( g.motionBlur.geometryParameterBlur || ( g.animation.time == atTime ) )
									tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( data, numVertex, name, tokenValue::storageVertex, tokenValue::typeFloat ) ) );
							}
						}
					}
				}
			}

			// Subcurve Clusters
			clusters.Clear();
			curveList.GetClusters().Filter( siSubCurveCluster, CStringArray(), CString(), clusters );

			for( unsigned i( 0 ); i < ( unsigned )clusters.GetCount(); i++ ) {
				Cluster cluster( clusters[ i ] );
				CRefArray properties( cluster.GetProperties() );
				for( unsigned j( 0 ); j < ( unsigned )properties.GetCount(); j++ ) {
					Property prop( properties[ j ] );
					if( CString( L"UserDataMap" ) == prop.GetType() ) {
						UserDataMap userDataMap( prop );
						CustomProperty userDataTemplate( userDataMap.GetTemplate() );
						CParameterRefArray parms( userDataTemplate.GetParameters() );

						shared_ptr< float > data( new float[ numCurves ], arrayDeleter() );
						for( unsigned p( 0 ); p < ( unsigned )parms.GetCount(); p++ ) {
							Parameter aParm( parms[ p ] );
							string name( CStringToString( aParm.GetName() ) );
							if( ( "width" != name ) || ( !userDataMapWidth && "width" == name ) ){

								for( long index( 0 ); index < numCurves; index++ ) {
									long cindex;
									cluster.FindIndex( index, cindex );
									if( -1 < cindex ) {
										const unsigned char *cpData;
										unsigned int cntData;
										userDataMap.GetItemValue( cindex, cpData, cntData );
										userDataTemplate.PutBinaryData( cpData, cntData );

										data.get()[ index ] = aParm.GetValue();
									} else {
										data.get()[ index ] = 0;
									}
								}
							}

							if( "width" == name ) {
								userDataMapWidth = true;
								tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( data, numCurves, name, tokenValue::storageUniform, tokenValue::typeFloat ) ) );
							} else {
								if( g.motionBlur.geometryParameterBlur || ( g.animation.time == atTime ) )
									tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( data, numCurves, name, tokenValue::storageUniform, tokenValue::typeFloat ) ) );
							}
						}
					}
				}
			}


			if( !userDataMapWidth ) {
				if( ( 0 == rootWidth ) && ( 0 == tipWidth ) && ( 0 != width ) ) {
					tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( width, "constantwidth" ) ) );
				} else {
					shared_ptr< float > widths( new float[ numVarying ], arrayDeleter() );
					unsigned widthIndex( 0 );
					for( unsigned index( 0 ); index < ( unsigned )numCurves; index++ ) {
						for( unsigned pt( 0 ); pt < ( unsigned )numVertsPerCurve[ index ] - order[ index ] + 2; pt++ ) {
							float where( ( ( float )pt / ( numVertsPerCurve[ index ] - order[ index ] + 1 ) ) * 2 );
							if( where < 1 ) {
								widths.get()[ widthIndex++ ] = tipWidth * ( 1 - where ) + width * where;
							} else {
								--where;
								widths.get()[ widthIndex++ ] = width * ( 1 - where ) + rootWidth * where;
							}
						}
					}
					tokenValuePtrArray.push_back( tokenValue::tokenValuePtr( new tokenValue( widths, widthIndex, "width", tokenValue::storageVarying, tokenValue::typeFloat ) ) );
				}
			}

		}

		bound = affogato::getBoundingBox( curveListPrim, atTime );
	}

	vector< float > nurbCurveData::boundingBox() const {
		return bound;
	}

	nurbCurveData::~nurbCurveData() {
	}

	void nurbCurveData::write() const {
		using namespace ueberMan;
		ueberManInterface theRenderer;

		if( numCurves ) {
			for( tokenValue::tokenValuePtrVector::const_iterator it( tokenValuePtrArray.begin() ); it < tokenValuePtrArray.end(); it++ ) {
				theRenderer.parameter( **it );
			}
			theRenderer.curves( numCurves, numVertsPerCurve, order, knots, min, max, const_cast< string& >( identifier ) );
		} else {
			theRenderer.translate( 0, 0, 0 );
		}
	}
}
