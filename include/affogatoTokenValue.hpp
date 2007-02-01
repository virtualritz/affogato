#ifndef tokenValue_H
#define tokenValue_H
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
#include <vector>

// Boost headers
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

// XSI headers
#ifdef __XSI_PLUGIN
	#include <xsi_floatarray.h>
	#include <xsi_doublearray.h>
	#include <xsi_longarray.h>
	#include <xsi_point.h>
	#include <xsi_vector3.h>
	#include <xsi_vector4.h>
#endif



#define EMPTY "unnamed"

namespace affogato {

#ifdef __XSI_PLUGIN
	using namespace XSI;
	using namespace MATH;
#endif
	using namespace std;
	using namespace boost;

	class tokenValue {
		public:
			typedef enum storageClass {
				storageUndefined   = -1,
				storageConstant    =  0,
				storagePerPiece    =  1, // We allow both the RMan & the
				storageUniform     =  1, // Gelato names to be used
				storageLinear      =  2,
				storageVarying     =  2,
				storageVertex      =  3,
				storageFaceVarying =  4, // Needs to be translated into linear
										 // for Gelato
				storageFaceVertex  =  5  // Currently unsupported in Gelato
			};

			typedef enum parameterType {
				typeUndefined = -1,
				typeBoolean   = 0,
				typeInteger   = 1,
				typeFloat     = 2,
				typeColor     = 3,
				typePoint     = 4,
				typeHomogenousPoint = 5,
				typeHPoint    = 5,
				typeVector    = 6,
				typeNormal    = 7,
				typeMatrix    = 8,
				typeString    = 9
			};

			typedef shared_ptr< tokenValue > tokenValuePtr;
			typedef vector< tokenValuePtr > tokenValuePtrVector;

			tokenValue( const size_t theSize, const parameterType theType );
			tokenValue();
			tokenValue( const tokenValue &src );
			tokenValue & operator=( const tokenValue &src );
			// Three value constructor omitting the actual data
			tokenValue(
				const string& theName,
				const storageClass theClass = storageUndefined,
				const parameterType theType = typeUndefined );

#ifdef __XSI_PLUGIN
			// Four value constructors
			tokenValue(
				const CFloatArray &floats,
				const string& theName = EMPTY,
				const storageClass theClass = storageVertex,
				const parameterType theType = typeFloat );

			tokenValue(
				const CDoubleArray& doubles,
				const string& theName = EMPTY,
				const storageClass theClass = storageVertex,
				const parameterType theType = typeFloat );

			tokenValue(
				const CLongArray &longs,
				const string& theName = EMPTY,
				const storageClass theClass = storageVertex,
				const parameterType theType = typeInteger );

			tokenValue(
				const CVector4Array& vertices,
				const string& theName = EMPTY,
				const storageClass theClass = storageVertex,
				const parameterType theType = typeHPoint );

			tokenValue(
				const CVector3Array& vertices,
				const string& theName = EMPTY,
				const storageClass theClass = storageVertex,
				const parameterType theType = typePoint );

			tokenValue(
				const CPointRefArray &vertices,
				const string& theName = EMPTY,
				const storageClass theClass = storageVertex,
				const parameterType theType = typePoint );
#endif

			tokenValue(
				const string& value,
				const string& theName = EMPTY );

			tokenValue(
				float value,
				const string& theName = EMPTY );

			tokenValue(
				int value,
				const string& theName = EMPTY );

			tokenValue(
				const float* values,
				const size_t theSize = 1,
				const string& theName = EMPTY,
				const storageClass theClass = storageUndefined,
				const parameterType theType = typeFloat );

			tokenValue(
				const int* values,
				const size_t theSize = 1,
				const string& theName = EMPTY,
				const storageClass theClass = storageUndefined );

			// The next two methods assume ownership of the data is transferred to the
			// resp. tokenvalue instance.
			// Aka: it's the APIs user's responsibility that data passed in through
			// 'values' doesn't get altered after the tokenValue got constrcuted
			tokenValue(
				shared_ptr< float > values,
				const size_t theSize = 1,
				const string& theName = EMPTY,
				const storageClass theClass = storageUndefined,
				const parameterType theType = typeFloat );

			tokenValue(
				shared_ptr< int > values,
				const size_t theSize = 1,
				const string& theName = EMPTY,
				const storageClass theClass = storageUndefined );

			// Methods to alter a tokenValue after creation
			void setName( const string& theName );
			void setClass( const storageClass theClass );
			void setType( const parameterType theType );
#ifdef __XSI_PLUGIN
			void setData( const CFloatArray& floats );
			void setData( const CDoubleArray& doubles );
			void setData( const CLongArray& doubles );
			void setData( const CVector4Array& vertices );
			void setData( const CVector3Array& vertices );
			void setData( const CPointRefArray& vertices );
#endif
			void setData( const float value );
			void setData( const float* values, size_t theSize );
			void setData( const int* values, size_t theSize );
			void setData( shared_ptr< float > values, size_t theSize );
			void setData( shared_ptr< int > values, size_t theSize );
			void setData( const int value );
			void setData( const string& value );

			void* operator[]( unsigned index ) const;

			void resize( size_t size );

			// Acccess methods for the renderer API
			string name() const;
			storageClass storage() const;
			parameterType type() const;
			string typeAsString() const;
			const void* data() const;
			string dataAsString() const;
			size_t size() const;
			size_t byteSize() const;
			bool valid() const;
			bool empty() const;

		private:
			size_t			multiplier() const;
			string			_name;
			size_t			_size;
			storageClass    _storClass;
			parameterType   _type;
			shared_ptr< void > _data;
	};
}

#endif
