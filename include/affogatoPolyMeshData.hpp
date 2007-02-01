#ifndef affogatoPolyMeshData_H
#define affogatoPolyMeshData_H
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
#include <vector>

// XSI headers
#include <xsi_primitive.h>

// Affogato headers
#include "affogatoData.hpp"


namespace affogato {

	using namespace XSI;

	class polyMeshData : public data {

		public:
								polyMeshData( const Primitive &polyMeshPrim, double atTime, bool usePref = false, double atPrefTime = 0 );
								~polyMeshData();
			void				write() const;
			objectType			type() const { return objectMesh; };
			vector< float >		boundingBox() const;

		private:
			int		numFaces;
			int	 	numPoints;
			boost::shared_ptr< int > nverts;
			boost::shared_ptr< int > verts;
			int		subDivScheme;

			const float	*vertexParam;

			vector< float > bound;

			typedef enum boundaryType{
				boundaryApproximating	= 0,
				boundarySharp			= 1,
				boundaryRounded			= 2
			} boundaryType;

			boundaryType boundary;
	};
}

#endif
