#ifndef affogatoParticleData_H
#define affogatoParticleData_H
/** Particle data container class.
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

	class particleData : public data {

		public:
								particleData( const Primitive &particlePrim, double atTime, bool usePref = false, double atPrefTime = 0 );
								~particleData();
			void				write() const;
			/*void				startGrain();
			void				writeNextGrain();
			unsigned			getGranularity() const;*/
			objectType			type() const { return objectParticle; };
			vector< float >		boundingBox() const;

		private:
			//void				splitById( tokenValue::tokenValuePtr blobbyIdMap );
			vector< float >		bound;
			unsigned			numParticles;
			string				typeStr;
			vector< int >		code;
			vector< float >		ppos;
			bool				isMultiGroupBlob;
			//map< int, unsigned >							numParticlesMap;
			//map< int, vector< tokenValue::tokenValuePtr > >	tokenValuePtrArrayMap;
			//map< int, unsigned >::iterator					numParticlesIt;
			//map< int, vector< tokenValue::tokenValuePtr > >::iterator tokenValueIt;

	};
}

#endif
