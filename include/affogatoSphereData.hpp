#ifndef affogatoSphereData_H
#define affogatoSphereData_H
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
#include <vector>

// Boost headers
#include <boost/shared_ptr.hpp>

// XSI headers
#include <xsi_x3dobject.h>

// Affogato headers
#include "affogatoData.hpp"


namespace affogato {

	using namespace XSI;

#ifdef RSP
	struct blob {
		float x, y, z;
		float size;
		blob() {
			x = y = z = 0;
			size = 1;
		}
	};
#endif

	class sphereData : public data {

		public:
								sphereData( const X3DObject &sphere, double atTime );
			void				write() const;
			void				startGrain(); // Initalize writing of a primitve in parts
			void				writeNextGrain(); // write the next part
			unsigned			getGranularity() const; // get the number of parts the primtive consists of
			objectType			type() const { return objectSphere; };
			//vector< float >	getBoundingBox() const;

		private:
			void				writeGeometry() const;
			void				writeTransform() const;
			float				radius;
			vector< float >		bound;
			unsigned			counter;
			typedef enum {
				renderTypeIgnore,
				renderTypeSphere,
				renderTypeBlobby
			} renderType;
			renderType rType;
			vector< int > code;
			vector< float > floatData;
			vector< string > stringData;
			//map< string, blob > blobs;
			vector< float > matrix;
			unsigned numBlobs;
	};
}

#endif
