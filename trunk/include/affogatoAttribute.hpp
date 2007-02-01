#ifndef affogatoAttribute_H
#define affogatoAttribute_H
/** Attrubte abstraction class.
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


// Boost headers
#include <boost/shared_ptr.hpp>

// XSI headers
#include <xsi_material.h>
#include <xsi_parameter.h>

// affogato headers
#include "affogatoTokenValue.hpp"


/*namespace __gnu_cxx {
	using namespace std;

	template<> struct hash< string > {
		size_t operator()( const string& x ) const	{
			return __stl_hash_string( x.c_str() );
		}
	};
}*/

namespace affogato {

	using namespace XSI;
	using namespace std;
	/**
	 *  Handles conversion of XSI properties/Parameters into attributes.
	 *
	 */
	class attribute {
		public:

			attribute();
			attribute( const Parameter &param, bool userType = false );
			attribute( const attribute &attrib );
			attribute( const tokenValue &aTokenValue );
			~attribute();
			attribute& operator=( const attribute &attrib );
			bool operator==( const attribute &comp ) const;
			bool valid() const ;
			void write() const;
			/**
			 * Get the attribute as XML.
			 */
			string xml();
			//void set();
		private:
			enum category {
				categoryUnknown = -1,
				categoryDerivatives,
				categoryDice,
				categoryDisplacementBound,
				categoryGrouping,
				categoryIdentifier,
				categoryIrradiance,
				categoryLight,
				categoryShading,
				categorySubSurface,
				categoryTrimCurve,
				categoryTrace,
				categoryUser,
				categoryVisibility
			} theCategory;
			tokenValue::tokenValuePtr tokenValuePtr;
	};
}

#endif
