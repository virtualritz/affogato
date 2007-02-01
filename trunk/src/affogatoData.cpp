/** Abstract data container base class.
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
#include <vector>

// XSI headers
#include <xsi_application.h>

// Affogato headers
#include "affogatoData.hpp"
#include "affogatoTokenValue.hpp"


namespace affogato {

	using namespace std;

	data::data() {
		// Nothing to construct
	}

	data::~data() {
		// Nothing to destruct
	}

	data::data( const data &cpy ) {
		data *tmp = const_cast< data* >( &cpy );
		for( vector< boost::shared_ptr< tokenValue > >::iterator it = tmp->tokenValuePtrArray.begin(); it < tmp->tokenValuePtrArray.end(); it++ ) {
			tokenValuePtrArray.push_back( boost::shared_ptr< tokenValue >( new tokenValue( *( *it ) ) ) );
		}
	}

	inline void	data::startGrain() {
	}

	inline void data::writeNextGrain() {
		write();
	}

	inline unsigned	data::granularity() const {
		return 1;
	}

	vector< float > data::boundingBox() const {
		vector< float > bound( 6 );
		bound[ 5 ] = bound[ 3 ] = bound[ 1 ] = numeric_limits< float >::min();
		bound[ 0 ] = bound[ 2 ] = bound[ 4 ] = numeric_limits< float >::max();

		return bound;
	}

}
