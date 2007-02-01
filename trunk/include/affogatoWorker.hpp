#ifndef affogatoWorker_H
#define affogatoWorker_H
/** The Affogato worker class manages the whole process of generating
 *  data for the renderer..
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
#include <stack>
#include <string>

// Boost headers
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/shared_ptr.hpp>

// XSI headers
#include <xsi_light.h>
#include <xsi_property.h>
#include <xsi_ref.h>
#include <xsi_string.h>

// affogatHeaders
#include "affogatoData.hpp"
#include "affogatoJob.hpp"
#include "affogatoJobEngine.hpp"
#include "affogatoRenderer.hpp"
#include "affogatoHelpers.hpp"


namespace affogato {

	using namespace XSI;
	using namespace std;
	using namespace ueberMan;
	using namespace boost;

	class worker {

		public:
					worker();
			void	work( const string& globalsString = string(), const CRefArray& objectList = CRefArray(), const string& destination = string() );
			void	archive( const CRefArray& objectList, const string& dest );

		private:

			void	scene( const CRefArray& objectList );
			void	options( const string& dest = string() );
			void	camera( const string& dest = string() );
			void	globalAttributes( const string& dest = string() );
			void	spaces( const CRefArray& objectList, const string& dest = string() );
			void	shadow( const Light& light );
			void	lights( const CRefArray& objectList, const string& dest = string() );
			void	frontAndBackPlane();
			void	nulls( const CRefArray& objectList, const string& dest = string() );
			void	geometry( const CRefArray& objectList, const string& dest = "" );

			//void	doWork( const string& globalsString, bool selectedOnly, void ( worker::*callfunc )( const bool ) );

			filesystem::path worldBlockName;
			//CRefArray objectList;
	};


	class blockManager {

		friend class worker;

		public:
			typedef enum blockType {
				blockUndefined = -1,
				blockScene,
				blockMapScene,
				blockBrickScene,
				blockShadowScene,
				blockMap,
				blockOptions,
				blockCamera,
				blockWorld,
				blockLights,
				blockSpaces,
				blockLooks,
				blockGeometry,
				blockObject,
				blockAttributes
			} blockType;

						blockManager();
					   ~blockManager();
				void	reset();
				void	deleteJobs();

				filesystem::path	beginBlock( blockType theBlockType, bool startScene = true, const string& blockName = string(), bool isStatic = false, const vector< float >& bound = vector< float >() );
				void	endBlock( const context& ctx, unsigned priority = 0 );
				context	currentContext();
				context	renderContext( const context& ctx );
				void 	switchBlock( const context& ctx );
		static	const	blockManager& access();
				void	addMapRenderIoToCurrentBlock( const string& from, const string& to );
				void	addImageRenderIoToCurrentBlock( const string& from, const string& to );
				void 	processJobChunk( int frameCounter, int chunkNo );
				void	processJob( int chunkNo );

		private:
			struct block {
				block();
				block( blockType aBlockType, context aContext, context aParentContext, bool wasStarted, const string& blockName = "empty" );
				block( const block& cpy );
				blockType type;
				context renderContext;
				bool started;
				string name;
				string jobName;
				string fileName;
				context parentBlockContext;
				task preTask; // stuff we do before the block
				task postTask;  // stuff we do after the block
			};

			map< context, shared_ptr< block > > blockPtrTracker;
			vector< shared_ptr< job > > jobPtrStack;
			taskManager tasks;

			vector< task* > currentPreTaskStack;
			vector< task* > currentPostTaskStack;

			static context activeContext;
			static context internalContext;
	};

	class timeStats {
		public:
			timeStats() {
				times.push_back( timeSample( "start", clock() ) );
			}
			~timeStats() {}

			void takeTime( const string& name ) {
				times.push_back( timeSample( name, clock() ) );
			}
			void printTimes() {
				Application app;
				app.LogMessage( L"Stopwatch:", siInfoMsg );
				for( unsigned i( 1 ); i < times.size(); i++ ) {
					app.LogMessage( stringToCString( ( format( "    %-16s %3.2f seconds" )
									% ( times[ i ].name + ":" )
									% ( ( times[ i ].deltaTime - times[ i - 1 ].deltaTime ) / ( float )CLOCKS_PER_SEC ) ).str() ),
									siInfoMsg );
				}
				app.LogMessage( stringToCString( ( format( "    %-16s %3.2f seconds" )
								% "Total:"
								% ( ( times.back().deltaTime - times.front().deltaTime ) / ( float )CLOCKS_PER_SEC ) ).str() ),
								siInfoMsg );
			}
			void reset() {
				times.clear();
				times.push_back( timeSample( "start", clock() ) );
			}
		private:
			clock_t startTime;
			struct timeSample {
				timeSample( const string& aName, clock_t aTime )
				:	name( aName ),
					deltaTime( aTime )
				{}
				string name;
				clock_t deltaTime;
			};
			vector< timeSample > times;
	};

}

#endif
