#ifndef affogatoNurbCurveData_H
#define affogatoNurbCurveData_H
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
#include <vector>

// Boost headers
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>

// XSI headers
#include <xsi_primitive.h>
#include <xsi_nurbscurve.h>

// affogato header
#include "affogatoData.hpp"


namespace affogato {

	using namespace XSI;

	class nurbCurveData : public data {

		public:
								nurbCurveData( const Primitive &curvePrim, double atTime, bool usePref, double atPrefTime );
								~nurbCurveData();
			void				write() const;
			objectType			type() const { return objectCurve; };
			vector< float >		boundingBox() const;

		private:
			long				degree;
			bool				closed;
			int					numCurves;
			vector< int >		numVertsPerCurve;
			vector< int >		order;
			vector< float >		knots;
			vector< float >		min;
			vector< float >		max;
			vector< float >		bound;


	};
}

#endif

