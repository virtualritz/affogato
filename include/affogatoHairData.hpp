#ifndef affogatoHairData_H
#define affogatoHairData_H
/** Container class for XSI hair data.
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
#include <vector>

// XSi headers
#include <xsi_hairprimitive.h>
#include <xsi_image.h>
#include <xsi_imageclip2.h>

// Affogato headers
#include "affogatoData.hpp"
#include "affogatoTokenValue.hpp"


namespace affogato {

	using namespace XSI;
	using namespace std;
	/** The hairData class works different than the other affogataData*
	 *  classes. Instead of caching the data, it writes the data out instantly.
	 */
	class hairData : public data {
		public:
								hairData();
								hairData( const Primitive &hairPrim, double atTime, bool caching = false );
							   ~hairData();
			objectType			type() const;
			vector< float >		boundingBox() const;
			unsigned			numberOfChunks();
			void				write() const;
			bool				writeChunk();
		private:
			//CRefArray	getImageClips( CRefArray shaders );
			void				data( bool caching );
			int					ncurves;
			vector< int >		nvertspercurve;
			vector< float > 	bound;
			CRenderHairAccessor rha;
			unsigned			numChunks;
			double				theTime;
			Image				displacementMap;
			float				displacement;
			float				widthScale;
			ImageClip2			clip;
	};
}

#endif
