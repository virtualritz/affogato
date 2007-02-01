/** Token-value type class.
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
#include <sstream>
#include <memory>
#include <wchar.h>

// XSI headers
#ifdef __XSI_PLUGIN
	#include <xsi_application.h>
	#include <xsi_point.h>
	#include <xsi_vector3.h>
	#include <xsi_vector4.h>
#endif

// Affogato headers
#include "affogatoTokenValue.hpp"
#include "affogatoHelpers.hpp"


//#define ADEBUGPRINTF(x) { debugMessage(x); }
#define ADEBUGPRINTF(x)

namespace affogato {

#ifdef __XSI_PLUGIN
	using namespace XSI;
#endif
	using namespace std;

	tokenValue::tokenValue() {
		ADEBUGPRINTF( L"Entering Default Constructor" );
		_size		= 0;
		_type		= tokenValue::typeUndefined;
		_storClass	= tokenValue::storageUndefined;
		ADEBUGPRINTF( L"Leaving Default Constructor" );
	}

	tokenValue::tokenValue( const size_t theSize, const parameterType theType ) {
		_type = theType;
		_size = theSize;
		size_t multiplier;
		switch( _type ) {
			case typeFloat:
			case typeInteger:
			case typeString:
				multiplier = 1;
			case typePoint:
			case typeColor:
			case typeVector:
			case typeNormal:
				multiplier = 3;
			case typeHomogenousPoint:
				multiplier = 14;
			case typeMatrix:
				multiplier = 16;
		}
		_size *= multiplier;
		switch( _type ) {
			case typeFloat:
			case typePoint:
			case typeColor:
			case typeHomogenousPoint:
			case typeVector:
			case typeNormal:
			case typeMatrix:
				_data.reset( new float[ _size ], arrayDeleter() );
				break;
			case typeInteger:
				_data.reset( new int[ _size ], arrayDeleter() );
				break;
			case typeString:
				_data.reset( new char[ _size ], arrayDeleter() );
				break;
			case typeUndefined:
			default:
				_data.reset( new short[ _size ], arrayDeleter() );
		}
	}

	tokenValue::tokenValue( const tokenValue &src ) {
		ADEBUGPRINTF( L"Entering Copy Constructor" );
		_name = src._name;
		setClass( src._storClass );
		setType( src._type );
		_size = src._size;
		/*if( size ) {
			size_t multiplier;
			switch( type ) {
				case typeFloat:
				case typePoint:
				case typeColor:
				case typeHomogenousPoint:
				case typeVector:
				case typeNormal:
				case typeMatrix:
					multiplier = sizeof( float );
					_data.reset( new float[ size ], arrayDeleter() );
					break;
				case typeInteger:
					multiplier = sizeof( int );
					_data.reset( new int[ size ], arrayDeleter() );
					break;
				case typeString:
					multiplier = sizeof( char );
					_data.reset( new char[ size ], arrayDeleter() );
					break;
				case typeUndefined:
				default:
					// Assume the worst case
					// We should throw an exception here rather
					_data.reset( new short[ size ], arrayDeleter() );
					multiplier = 1;
			}
			memcpy( _data.get(), src._data.get(), size * multiplier );
		}*/
		_data = src._data;
		ADEBUGPRINTF( L"Leaving Copy Constructor" );
	}

	tokenValue& tokenValue::operator = ( const tokenValue &src ) {
		setClass( src._storClass );
		_name = src._name;
		setType( src._type );
		_size = src._size;
		/*if( size ) {
			size_t multiplier;
			switch( type ) {
				case typeFloat:
				case typePoint:
				case typeColor:
				case typeHomogenousPoint:
				case typeVector:
				case typeNormal:
				case typeMatrix:
					multiplier = sizeof( float );
					_data.reset( new float[ size ], arrayDeleter() );
					break;
				case typeInteger:
					multiplier = sizeof( int );
					_data.reset( new int[ size ], arrayDeleter() );
					break;
				case typeString:
					multiplier = sizeof( char );
					_data.reset( new char[ size ], arrayDeleter() );
					break;
				case typeUndefined:
				default:
					// Assume the worst case
					// We should throw an exception here rather
					_data.reset( new short[ size ], arrayDeleter() );
					multiplier = 1;
			}
			memcpy( _data.get(), src._data.get(), size * multiplier );
		}*/
		_data = src._data;
		return *this;
	}

	tokenValue::tokenValue(
		const string& theName,
		const storageClass theClass,
		const parameterType theType )
	{
		setClass( theClass );
		setType ( theType );
		setName( theName );
	}

#ifdef __XSI_PLUGIN
	tokenValue::tokenValue(
		const CFloatArray& floats,
		const string& theName,
		const storageClass theClass,
		const parameterType theType )
	{
		ADEBUGPRINTF( L"Entering 4 Param Constructor" );
		setClass( theClass );
		setType ( theType );
		setData( floats );
		setName( theName );
		ADEBUGPRINTF( L"Leaving 4 Param Constructor" );
	}

	tokenValue::tokenValue(
		const CDoubleArray& doubles,
		const string& theName,
		const storageClass theClass,
		const parameterType theType )
	{
		ADEBUGPRINTF( L"Entering 4 Param Constructor" );
		setClass( theClass );
		setType ( theType );
		setData( doubles );
		setName( theName );
		ADEBUGPRINTF( L"Leaving 4 Param Constructor" );
	}

	tokenValue::tokenValue(
		const CLongArray& longs,
		const string& theName,
		const storageClass theClass,
		const parameterType theType )
	{
		ADEBUGPRINTF( L"Entering 4 Param Constructor" );
		setClass( theClass );
		setType ( theType );
		setData( longs );
		setName( theName );
		ADEBUGPRINTF( L"Leaving 4 Param Constructor" );
	}

	tokenValue::tokenValue(
		const CVector4Array& vertices,
		const string& theName,
		const storageClass theClass,
		const parameterType theType )
	{
		ADEBUGPRINTF( L"Entering 4 Param Constructor" );
		setClass( theClass );
		setType ( theType );
		setData( vertices );
		setName( theName );
		ADEBUGPRINTF( L"Leaving 4 Param Constructor" );
	}

	tokenValue::tokenValue(
		const CVector3Array& vertices,
		const string& theName,
		const storageClass theClass,
		const parameterType theType )
	{
		ADEBUGPRINTF( L"Entering 4 Param Constructor" );
		setClass( theClass );
		setType ( theType );
		setData( vertices );
		setName( theName );
		ADEBUGPRINTF( L"Leaving 4 Param Constructor" );
	}

	tokenValue::tokenValue(
		const CPointRefArray& vertices,
		const string& theName,
		const storageClass theClass,
		const parameterType theType )
	{
		ADEBUGPRINTF( L"Entering 4 Param Constructor" );
		setClass( theClass );
		setType ( theType );
		setData( vertices );
		setName( theName );
		ADEBUGPRINTF( L"Leaving 4 Param Constructor" );
	}
#endif

	tokenValue::tokenValue(
		const string& value,
		const string& theName )
	{
		ADEBUGPRINTF( L"Entering 2 Param String Constructor" );
		setClass( storageConstant );
		setType( typeString );
		setData( value );
		setName( theName );
		ADEBUGPRINTF( L"Leaving 2 Param String Constructor" );
	}

	tokenValue::tokenValue(
		const float value,
		const string& theName )
	{
		setClass( storageConstant );
		setType( typeFloat );
		setData( value );
		setName( theName );
	}

	tokenValue::tokenValue(
		const int value,
		const string& theName )
	{
		setClass( storageConstant );
		setType( typeInteger );
		setData( value );
		setName( theName );
	}

	tokenValue::tokenValue(
		const float* values,
		const size_t theSize,
		const string& theName,
		const storageClass theClass,
		const parameterType theType )
	{
		ADEBUGPRINTF( L"Entering 3 Param Float Constructor" );
		setClass( theClass );
		setType( theType );
		setData( values, theSize );
		setName( theName );
		ADEBUGPRINTF( L"Leaving 3 Param Float Constructor" );
	}

	tokenValue::tokenValue(
		const int* values,
		const size_t theSize,
		const string& theName,
		const storageClass theClass )
	{
		ADEBUGPRINTF( L"Entering 3 Param Int Constructor" );
		setClass( theClass );
		setType( typeInteger );
		setData( values, theSize );
		setName( theName );
		ADEBUGPRINTF( L"Leaving 3 Param Int Constructor" );
	}

	tokenValue::tokenValue(
		shared_ptr< float > values,
		const size_t theSize,
		const string& theName,
		const storageClass theClass,
		const parameterType theType )
	{
		ADEBUGPRINTF( L"Entering 3 Param shared_ptr Float Constructor for " + stringToCString( theName ) );
		setClass( theClass );
		setType( theType );
		setData( values, theSize );
		setName( theName );
		ADEBUGPRINTF( L"Leaving 3 Param shared_ptr Float Constructor" );
	}

	tokenValue::tokenValue(
		shared_ptr< int > values,
		const size_t theSize,
		const string& theName,
		const storageClass theClass )
	{
		ADEBUGPRINTF( L"Entering 3 Param shared_ptr Int Constructor " + stringToCString( theName ) );
		setClass( theClass );
		setType( typeInteger );
		setData( values, theSize );
		setName( theName );
		ADEBUGPRINTF( L"Leaving 3 Param shared_ptr Int Constructor" );
	}

	void tokenValue::setName( const string& tokenName ) {
		ADEBUGPRINTF( L"Start Setting Name" );
		_name = tokenName;
		ADEBUGPRINTF( L"Done Setting Name" );
	}

	void tokenValue::setClass( const storageClass theClass ) {
		_storClass = theClass;
	}

	void tokenValue::setType( const parameterType theType ) {
		_type = theType;
	}


	void tokenValue::setData( const float value ) {
		_size = 1;
		_data.reset( new float );
		*( ( float* )_data.get() ) = value;

	//	float *tmp = static_cast< float* >( data );
		// If we want tokenValue to be more flexible (allow altering data after creation)
		// We need to do a lot more error checking, as e.g. below
		/*if( ( typeString == type ) || ( typeInteger == type ) || ( typeUndefined == type ) ) {
			type = typeFloat;
			throw "tokenValue::setData( float value ): redefining data type";
		}*/
	}

	void tokenValue::setData( const float* values, const size_t theSize ) {
		_size = theSize;
		if( _size ) {
			_data = shared_ptr< void >( new float[ _size ], arrayDeleter() );
			memcpy( _data.get(), values, _size * sizeof( float ) );
		}
	}

	void tokenValue::setData( const int* values, const size_t theSize ) {
		_size = theSize;
		if( _size ) {
			_data = shared_ptr< void >( new int[ _size ], arrayDeleter() );
			memcpy( _data.get(), values, _size * sizeof( int ) );
		}
	}

	void tokenValue::setData( shared_ptr< float > values, const size_t theSize ) {
		_size = theSize;
		_data = values;
		//shared_ptr< void >( new float[ theSize ], arrayDeleter() );
		//memcpy( _data.get(), values.get(), size * sizeof( float ) );
	}

	void tokenValue::setData( shared_ptr< int > values, const size_t theSize ) {
		_size = theSize;
		_data = values;
		//data = shared_ptr< void >( new int[ theSize ], arrayDeleter() );
		//memcpy( _data.get(), values.get(), size * sizeof( int ) );
	}

	void tokenValue::setData( const int value ) {
		_size = 1;
		_data.reset( new int );
		*( ( int* )_data.get() ) = value;
	}

	void tokenValue::setData( const string& value ) {
		_size = value.length() + 1;
		_data.reset( new char[ _size ], arrayDeleter() );
		memcpy( ( char* )_data.get(), value.c_str(), _size );
	}

#ifdef __XSI_PLUGIN
	void tokenValue::setData( const CFloatArray &floats ) {
		ADEBUGPRINTF( L"Start Copying CFloatArray data" );

		_size = floats.GetCount();
		_data = shared_ptr< void >( new float[ _size ], arrayDeleter() );
		float *tmp( ( float* )_data.get() );
		for( unsigned i( 0 ); i < _size; i++ ) {
			tmp[ i ] = floats[ i ];
		}

		ADEBUGPRINTF( L"Done Copying CFloatArray data" );
	}

	void tokenValue::setData( const CDoubleArray &doubles ) {
		ADEBUGPRINTF( L"Start Copying CDoubleArray data" );

		_size = doubles.GetCount();
		_data = shared_ptr< void >( new float[ _size ], arrayDeleter() );
		float *tmp( ( float* )_data.get() );
		for( unsigned i( 0 ); i < _size; i++ ) {
			tmp[ i ] = ( float )doubles[ i ];
		}

		ADEBUGPRINTF( L"Done Copying CDoubleArray data" );
	}

	void tokenValue::setData( const CLongArray &longs ) {
		ADEBUGPRINTF( L"Start Copying CLongArray data" );

		_size = longs.GetCount();
		_data = shared_ptr< void >( new int[ _size ], arrayDeleter() );
		int *tmp( ( int* )_data.get() );
		for( unsigned i( 0 ); i < _size; i++ ) {
			tmp[ i ] = longs[ i ];
		}

		ADEBUGPRINTF( L"Done Copying CLongArray data" );
	}

	void tokenValue::setData( const CVector4Array &vertices ) {
		ADEBUGPRINTF( L"Start Copying CVector4Array data" );

		long nPts( vertices.GetCount() );
		if( typeHomogenousPoint == type() ) {
			_size = nPts * 4;
			_data = shared_ptr< void >( new float[ _size ], arrayDeleter() );
			float* tmp = ( float* )_data.get();
			for( long i( 0 ); i < nPts; i++ ) {
				CVector4 v = vertices[ i ];
				tmp[ i * 4 ]     = ( float )v.GetX();
				tmp[ i * 4 + 1 ] = ( float )v.GetY();
				tmp[ i * 4 + 2 ] = ( float )v.GetZ();
				tmp[ i * 4 + 3 ] = ( float )v.GetW(); // weight;
			}
		} else {
			_size = nPts * 3;
			_data = shared_ptr< void >( new float[ _size ], arrayDeleter() );
			float* tmp = ( float* )_data.get();
			for( long i( 0 ); i < nPts; i++ ) {
				CVector4 v = vertices[ i ];
				float weight     = ( float )v.GetW();
				tmp[ i * 3 ]     = ( float )v.GetX() / weight;
				tmp[ i * 3 + 1 ] = ( float )v.GetY() / weight;
				tmp[ i * 3 + 2 ] = ( float )v.GetZ() / weight;
			}
		}

		ADEBUGPRINTF( L"Done Copying CVector4Array data" );
	}

	void tokenValue::setData( const CVector3Array &vertices ) {
		ADEBUGPRINTF( L"Start Copying CVector3Array data" );

		long nPts( vertices.GetCount() );
		if( typeHomogenousPoint == type() ) {
			_size = nPts * 4;
			_data = shared_ptr< void >( new float[ _size ], arrayDeleter() );
			float* tmp = ( float* )_data.get();
			for( long i( 0 ); i < nPts; i++ ) {
				CVector3 v = vertices[ i ];
				tmp[ i * 4 ]     = ( float )v.GetX();
				tmp[ i * 4 + 1 ] = ( float )v.GetY();
				tmp[ i * 4 + 2 ] = ( float )v.GetZ();
				tmp[ i * 4 + 3 ] = 1.0f;
			}
		} else {
			_size = nPts * 3;
			_data = shared_ptr< void >( new float[ _size ], arrayDeleter() );
			float* tmp = ( float* )_data.get();
			for( long i( 0 ); i < nPts; i++ ) {
				CVector3 v = vertices[ i ];
				tmp[ i * 3 ]     = ( float )v.GetX();
				tmp[ i * 3 + 1 ] = ( float )v.GetY();
				tmp[ i * 3 + 2 ] = ( float )v.GetZ();
			}
		}
		ADEBUGPRINTF( L"Done Copying CVector3Array data" );
	}

	void tokenValue::setData( const CPointRefArray &vertices ) {
		ADEBUGPRINTF( L"Start Copying CPointRefArray data" );

		long nPts = vertices.GetCount();
		if( typeHomogenousPoint == type() ) {
			_size = nPts * 4;
			_data = shared_ptr< void >( new float[ _size ], arrayDeleter() );
			float* tmp = ( float* )_data.get();
			for( long i = 0; i < nPts; i++ ) {
				CVector3 v = Point( vertices[ i ] ).GetPosition();
				tmp[ i * 4 ]     = ( float )v.GetX();
				tmp[ i * 4 + 1 ] = ( float )v.GetY();
				tmp[ i * 4 + 2 ] = ( float )v.GetZ();
				tmp[ i * 4 + 3 ] = 1;
			}
		} else {
			_size = nPts * 3;
			_data = shared_ptr< void >( new float[ _size ], arrayDeleter() );
			float* tmp = ( float* )_data.get();
			for( long i = 0; i < nPts; i++ ) {
				CVector3 v = Point( vertices[ i ] ).GetPosition();
				tmp[ i * 3 ]     = ( float )v.GetX();
				tmp[ i * 3 + 1 ] = ( float )v.GetY();
				tmp[ i * 3 + 2 ] = ( float )v.GetZ();
			}
		}
		ADEBUGPRINTF( L"Done Copying CPointRefArray data" );
	}
#endif

	string tokenValue::name() const {
		return _name;
	}

	tokenValue::storageClass tokenValue::storage() const {
		return _storClass;
	}

	tokenValue::parameterType tokenValue::type() const {
		return _type;
	}

	string tokenValue::typeAsString() const {
		switch( _type ) {
			case typeFloat:
				return "float";
			case typeInteger:
				return "int";
			case typeColor:
				return "color";
			case typePoint:
				return "point";
			case typeHomogenousPoint:
				return "hpoint";
			case typeVector:
				return "vector";
			case typeNormal:
				return "normal";
			case typeMatrix:
				return "matrix";
			case typeString:
				return "string";
		}
		return "";
	}

	const void* tokenValue::data() const {
		return _data.get();
	}

	string tokenValue::dataAsString() const {
		stringstream retStr;
		switch( _type ) {

			case typeFloat: {
				float* a( ( float* )_data.get() );
				if( _size > 1 ) {
					for( unsigned i = 0; i < _size - 1; i++ )
						retStr << a[ i ] << " ";
					retStr << a[ _size - 1 ];
				} else
					retStr << *a;
				break;
			}
			case typeInteger: {
				int* a( ( int* )_data.get() );
				if( _size > 1 ) {
					for( unsigned i = 0; i < _size - 1; i++ )
						retStr << a[ i ] << " ";
					retStr << a[ _size - 1 ];
				} else
					retStr << *a;
				break;
			}
			case typeHomogenousPoint:
				retStr << "empty";
				break;
			case typePoint:
			case typeVector:
			case typeNormal:
			case typeColor:
			case typeMatrix: {
				float* a( ( float* )_data.get() );
				for( unsigned i = 0; i < _size - 1; i++ )
					retStr << a[ i ] << " ";
				retStr << a[ _size - 1 ];
				break;
			}
			case typeString:
				retStr << ( char* )( _data.get() );
				break;
			case typeUndefined:
			default:
				retStr << "empty";
		}

		return retStr.str();
	}

	size_t tokenValue::size() const {
		switch( _type ) {
			case typeFloat:
			case typeInteger:
			case typeString:
				return _size;
			case typeHomogenousPoint:
				return _size / 4;
			case typeColor:
			case typePoint:
			case typeVector:
			case typeNormal:
				return _size / 3;
			case typeMatrix:
				return _size / 16;
		}
		return 0;
	}

	size_t tokenValue::byteSize() const {
		return _size * multiplier();
	}

	bool tokenValue::valid() const {
		return ( bool )_data;
	}

	bool tokenValue::empty() const {
		return 0 == _size;
	}

	void* tokenValue::operator[]( unsigned index ) const {
		switch( _type ) {
			case typeFloat:
			case typePoint:
			case typeColor:
			case typeHomogenousPoint:
			case typeVector:
			case typeNormal:
			case typeMatrix:
				return ( void* )( ( float* )_data.get() + index );
				break;
			case typeInteger:
				return ( void* )( ( float* )_data.get() + index );
				break;
			case typeString:
				return ( void* )( ( float* )_data.get() + index );
				break;
			default:
			case typeUndefined:
				return _data.get();
		}
	}

	size_t tokenValue::multiplier() const {
		size_t multiplier;
		switch( _type ) {
			case typeFloat:
			case typePoint:
			case typeColor:
			case typeHomogenousPoint:
			case typeVector:
			case typeNormal:
			case typeMatrix:
				multiplier = sizeof( float );
				break;
			case typeInteger:
				multiplier = sizeof( int );
				break;
			case typeString:
				multiplier = sizeof( char );
				break;
			case typeUndefined:
				multiplier = 0;
		}
		return multiplier;
	}
}
