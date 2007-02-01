/** The Affogato worker class manages the whole process of generating
 *  data for the renderer.
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
#include <fstream>
#include <math.h>
#include <string>
#include <time.h>

// Boost headers
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>

// XSI headers
#include <xsi_argument.h>
#include <xsi_application.h>
#include <xsi_color.h>
#include <xsi_command.h>
#include <xsi_context.h>
#include <xsi_decl.h>
#include <xsi_desktop.h>
#include <xsi_geometry.h>
#include <xsi_hairprimitive.h>
#include <xsi_layout.h>
#include <xsi_kinematics.h>
#include <xsi_kinematicstate.h>
#include <xsi_light.h>
#include <xsi_menuitem.h>
#include <xsi_menu.h>
#include <xsi_model.h>
#include <xsi_parameter.h>
#include <xsi_pluginitem.h>
#include <xsi_pluginregistrar.h>
#include <xsi_plugin.h>
#include <xsi_primitive.h>
#include <xsi_progressbar.h>
#include <xsi_property.h>
#include <xsi_projectitem.h>
#include <xsi_ref.h>
#include <xsi_sceneitem.h>
#include <xsi_shader.h>
#include <xsi_siobject.h>
#include <xsi_status.h>
#include <xsi_string.h>
#include <xsi_transformation.h>
#include <xsi_uitoolkit.h>
#include <xsi_value.h>
#include <xsi_vertex.h>
#include <xsi_vector3.h>
#include <xsi_view.h>
#include <xsi_x3dobject.h>

// Affogato headers
#include "affogato.hpp"
#include "affogatoExecute.hpp"
#include "affogatoGlobals.hpp"
#include "affogatoHairData.hpp"
#include "affogatoHelpers.hpp"
#include "affogatoNode.hpp"
#include "affogatoPolyMeshData.hpp"
#include "affogatoRenderer.hpp"
//#include "affogatoDummyRenderer.hpp"
#include "affogatoRiRenderer.hpp"
#include "affogatoXmlRenderer.hpp"
#include "affogatoShader.hpp"
#include "affogatoWorker.hpp"


namespace affogato {

	using namespace XSI;
	using namespace XSI::MATH;
	using namespace boost;
	using namespace std;
	using namespace ueberMan;

	context blockManager::internalContext = 0;
	context blockManager::activeContext = 1;

	blockManager::block::block()
	:
		type( blockUndefined ),
		started( false ),
		renderContext( contextUndefined ),
		name( "empty" ),
		parentBlockContext( contextUndefined )
	{
		debugMessage( L"Constructing Block" );
	}

	blockManager::block::block( blockType aBlockType, context aContext, context aParentContext, bool wasStarted, const string &blockName ) {
		debugMessage( L"Init-constructing Block" );
		type = aBlockType;
		started = wasStarted;
		renderContext = aContext;
		name = blockName;
		parentBlockContext = aParentContext;
	}

	blockManager::block::block( const block &cpy ) {
		debugMessage( L"Copy-constructing Block" );
		type = cpy.type;
		started = cpy.started;
		renderContext = cpy.renderContext;
		name = cpy.name;
		parentBlockContext = cpy.parentBlockContext;
	}

	filesystem::path blockManager::beginBlock( blockType theBlockType, bool startScene, const string &blockName, bool isStatic, const vector< float >& bound ) {
		globals& g( const_cast< globals& >( globals::access() ) );
		ueberManInterface theRenderer;

		debugMessage( L"Beginning Block " + stringToCString( blockName ) );

		filesystem::path sceneName;
		string dottedBlockName;

		if( !blockName.empty() )
			dottedBlockName = "." + blockName;

		// Set context in case we don't begin a new scene
		context ctx( theRenderer.currentScene() );

		shared_ptr< block > currentBlock( shared_ptr< block >( new block() ) );
		debugMessage( L"Searching for context " + CValue( activeContext ).GetAsText() );
		if( blockPtrTracker.end() != blockPtrTracker.find( activeContext ) ) {
			debugMessage( L"Setting current block to this context" );
			currentBlock = blockPtrTracker[ activeContext ];
		}

		string frameJobName( g.name.baseName + dottedBlockName );

		++internalContext;
		blockPtrTracker[ internalContext ] = shared_ptr< block>( new block( theBlockType, ctx, activeContext, startScene, blockName ) );

		switch( g.data.granularity ) {
			case globals::data::granularityAttributes: {
				if( blockAttributes == theBlockType ) {
					dottedBlockName += ".attributes";
					string jobName( g.name.baseName + dottedBlockName + "." + g.name.currentFrame );
					debugMessage( L"Writing attributes, parentBlock is " + stringToCString( currentBlock->name ) );
					filesystem::path fileName( g.directories.attribute / jobName );
					if( blockObject == currentBlock->type )
						theRenderer.input( getCacheFilePath( fileName, g.directories.caching.dataSource ).native_file_string() );
					if( startScene ) {
						message( L"Writing sub-object block to '" + stringToCString( getCacheFilePath( fileName, g.directories.caching.dataWrite ).native_file_string() ) + L"'", messageInfo );
						ctx = theRenderer.beginScene( getCacheFilePath( fileName, g.directories.caching.dataWrite ).native_file_string(), g.data.binary, g.data.compress );
					}
					sceneName = getCacheFilePath( fileName, g.directories.caching.dataSource ).native_file_string();

/*					if( ( g.directories.attributeSource != g.directories.attributeDestination ) && ( stageMap.end() == stageMap.find( frameJobName ) ) {
						stage newStage( frameJobName );
						newStage.addAction( ioManager::stage::action( ( g.directories.attributeSource / jobName ).native_file_string() + ".rib", ( g.directories.attributeDestination / jobName ).native_file_string() + ".rib" ) );
						stageMap[ frameJobName ] = newStage;
					}*/
					blockPtrTracker[ internalContext ]->jobName = g.name.baseName + dottedBlockName;
				}
			}
			case globals::data::granularityObjects: {
				if( blockObject == theBlockType ) {
					string jobName( g.name.baseName + dottedBlockName + "." + g.name.currentFrame );
					filesystem::path fileName( g.directories.object / jobName );
					if( blockGeometry == currentBlock->type ) {
						if( !bound.empty() && g.data.delay && g.data.subSectionParentTransforms ) {
							theRenderer.attribute( "visibility:trace", true );
							theRenderer.attribute( "visibility:subsurface", string( "__dummy" ) );
							theRenderer.input( getCacheFilePath( fileName, g.directories.caching.dataSource ).native_file_string(), &( bound[ 0 ] ) );
						} else {
							theRenderer.input( getCacheFilePath( fileName, g.directories.caching.dataSource ).native_file_string() );
						}
					}
					if( startScene ) {
						message( L"Writing sub-section block to '" + stringToCString( getCacheFilePath( fileName, g.directories.caching.dataWrite ).native_file_string() ) + L"'", messageInfo );
						ctx = theRenderer.beginScene( getCacheFilePath( fileName, g.directories.caching.dataWrite ).native_file_string(), g.data.binary, g.data.compress );
					}
					sceneName = getCacheFilePath( fileName, g.directories.caching.dataSource ).native_file_string();

/*					if( ( g.directories.objectSource != g.directories.objectDestination ) && ( stageMap.end() == stageMap.find( frameJobName ) ) {
						stage newStage( frameJobName );
						newStage.addAction( ioManager::stage::action( ( g.directories.objectSource / jobName ).native_file_string() + ".rib", ( g.directories.objectDestination / jobName ).native_file_string() + ".rib" ) );
						stageMap[ frameJobName ] = newStage;
					}*/

					blockPtrTracker[ internalContext ]->jobName = g.name.baseName + dottedBlockName;
				}
			}
			case globals::data::granularitySections: {
				if( ( blockAttributes != theBlockType ) &&
					( blockObject != theBlockType ) &&
					( blockWorld != theBlockType ) &&
					( blockScene != theBlockType ) &&
					( blockMapScene != theBlockType ) &&
					( blockBrickScene != theBlockType ) &&
					( blockShadowScene != theBlockType )
					) {
					bool frame( false ), world( false );
					if( blockName.empty() ) {
						switch( theBlockType ) {
							case blockOptions:
								dottedBlockName = ".options";
								frame = true;
								break;
							case blockCamera:
								dottedBlockName = ".camera";
								frame = true;
								break;
							case blockLights:
								dottedBlockName = ".lights";
								world = true;
								break;
							case blockSpaces:
								dottedBlockName = ".spaces";
								world = true;
								break;
							case blockLooks:
								dottedBlockName = ".looks";
								world = true;
								break;
							case blockGeometry:
								dottedBlockName = ".geometry";
								world = true;
								break;
							default:
								dottedBlockName = ".undefined";
						}
					}
					string jobName( g.name.baseName + dottedBlockName + "." + g.name.currentFrame );
					filesystem::path fileName( g.directories.data / jobName );
					if( ( world && ( blockWorld == currentBlock->type ) ) || ( frame && ( blockScene == currentBlock->type ) ) )
						theRenderer.input( getCacheFilePath( fileName, g.directories.caching.dataSource ).native_file_string() );
					if( startScene ) {
						message( L"Writing section block to '" + stringToCString( getCacheFilePath( fileName, g.directories.caching.dataWrite ).native_file_string() ) + L"'", messageInfo );
						ctx = theRenderer.beginScene( getCacheFilePath( fileName, g.directories.caching.dataWrite ).native_file_string(), g.data.binary, g.data.compress );
					}
					sceneName = getCacheFilePath( fileName, g.directories.caching.dataSource ).native_file_string();

/*					if( ( g.directories.dataSource != g.directories.dataDestination ) && ( stageMap.end() == stageMap.find( frameJobName ) ) {
						stage newStage( frameJobName );
						newStage.addAction( ioManager::stage::action( ( g.directories.dataSource / jobName ).native_file_string() + ".rib", ( g.directories.dataDestination / jobName ).native_file_string() + ".rib" ) );
						stageMap[ frameJobName ] = newStage;
					}*/
					blockPtrTracker[ internalContext ]->jobName = g.name.baseName + dottedBlockName;
				}
			}
			case globals::data::granularitySubFrame: {
				if( blockWorld == theBlockType ) {
					if( blockName.empty() )
						dottedBlockName = ".world";
					string jobName( g.name.baseName + dottedBlockName + "." + g.name.currentFrame );
					filesystem::path fileName( g.directories.data / jobName );
					if( blockScene == currentBlock->type )
						theRenderer.input( getCacheFilePath( fileName, g.directories.caching.dataSource ).native_file_string() );
					if( startScene ) {
						message( L"Writing sub-frame block to '" + stringToCString( getCacheFilePath( fileName, g.directories.caching.dataWrite ).native_file_string() ) + L"'", messageInfo );
						ctx = theRenderer.beginScene( getCacheFilePath( fileName, g.directories.caching.dataWrite ).native_file_string(), g.data.binary, g.data.compress );

					}
					sceneName = getCacheFilePath( fileName, g.directories.caching.dataSource ).native_file_string();

/*					if( ( g.directories.dataSource != g.directories.dataDestination ) && ( stageMap.end() == stageMap.find( frameJobName ) ) {
						stage newStage( frameJobName );
						newStage.addAction( ioManager::stage::action( ( g.directories.dataSource / jobName ).native_file_string() + ".rib", ( g.directories.dataDestination / jobName ).native_file_string() + ".rib" ) );
						stageMap[ frameJobName ] = newStage;
					}*/

					blockPtrTracker[ internalContext ]->jobName = g.name.baseName + dottedBlockName;
				}
			}
			case globals::data::granularityFrame: {
				if( ( blockScene == theBlockType ) ||
					( blockMapScene == theBlockType ) ||
					( blockBrickScene == theBlockType ) ||
					( blockShadowScene == theBlockType )
					) {
					string jobName( g.name.baseName + dottedBlockName + "." + g.name.currentFrame );
					filesystem::path fileName( g.directories.data / jobName );
					if( startScene ) {
						message( L"Writing frame to '" + stringToCString( getCacheFilePath( fileName, g.directories.caching.dataWrite ).native_file_string() ) + L"'", messageInfo );

						if( g.data.directToRenderer )
							ctx = theRenderer.beginScene( "direct", g.data.binary, g.data.compress );
						else
							ctx = theRenderer.beginScene( getCacheFilePath( fileName, g.directories.caching.dataWrite ).native_file_string(), g.data.binary, g.data.compress );

						string version( string( "Builder Affogato " ) + AFFOGATOVERSION );
						RiArchiveRecord( RI_STRUCTURE, const_cast< RtString >( version.c_str() ) );

						theRenderer.option( "searchpath:shader", g.searchPath.shader );
						theRenderer.option( "searchpath:texture", g.searchPath.texture );
						theRenderer.option( "searchpath:archive", g.searchPath.archive );
						theRenderer.option( "searchpath:procedural", g.searchPath.procedural );

						switch( g.jobGlobal.cache ) {
							case globals::jobGlobal::cacheTypeReadsAndWrites:
							case globals::jobGlobal::cacheTypeWrites:
								theRenderer.option( "netcache:writecache", ( int )g.jobGlobal.cacheSize );
							case globals::jobGlobal::cacheTypeReads:
								theRenderer.option( "netcache:cachedir", g.jobGlobal.cacheDir );
								theRenderer.option( "netcache:cachesize", ( int )g.jobGlobal.cacheSize );
						}
					}

					if( globals::jobGlobal::launchRenderer == g.jobGlobal.launch ) {
						jobPtrStack.push_back( shared_ptr< job >( new job( jobName,
						g.renderer.command + " -p " + ( format( "%d " ) % g.jobGlobal.numCPUs ).str() + getCacheFilePath( fileName, g.directories.caching.dataSource ).native_file_string() + ".rib" ) ) );
						debugMessage( stringToCString( "Adding job: '" + g.renderer.command + " -p " + ( format( "%d " ) % g.jobGlobal.numCPUs ).str() + sceneName.native_file_string() + ".rib'" ) );
					} else {
						jobPtrStack.push_back( shared_ptr< job >( new job( jobName,
						g.renderer.command + " -stats1 -Progress -p " + ( format( "%d " ) % g.jobGlobal.numCPUs ).str() + getCacheFilePath( fileName, g.directories.caching.dataSource ).native_file_string() + ".rib" ) ) );
						debugMessage( stringToCString( "Adding job '" + g.renderer.command + " -stats1 -Progress -p " + ( format( "%d " ) % g.jobGlobal.numCPUs ).str() + sceneName.native_file_string() + ".rib'" ) );
					}
					jobName = g.name.baseName + dottedBlockName;

					currentPreTaskStack.push_back( &( blockPtrTracker[ internalContext ]->preTask ) ); // set the task array for sub section blocks to add to;
					currentPreTaskStack.back()->setTitle( jobName + ".####" );
					currentPostTaskStack.push_back( &( blockPtrTracker[ internalContext ]->postTask ) ); // set the task array for sub section blocks to add to;
					currentPostTaskStack.back()->setTitle( jobName + ".####" );

					if( blockShadowScene == theBlockType ) {

						parameter param( fileName.native_file_string() + ".####.rib", string() );
						parameter::array params;
						params.push_back( param );
						//tasks.addSubTask( "delight", jobName, ( int )round( g.animation.time ), params );

						//currentFrameSuperTask = blockSuperTask;
						//currentPreTaskStack.push_back( &( blockPtrTracker[ internalContext ]->preTasks ) ); // set the task array for sub section blocks to add to;
						//currentPostTaskStack.push_back( &( blockPtrTracker[ internalContext ]->postTasks ) ); // set the task array for sub section blocks to add to;

					} else {
						parameter param( fileName.native_file_string() + ".####.rib", string() );
						parameter::array params;
						params.push_back( param );
						//task tmpTask( "delight", jobName, ( int )round( g.animation.time ), params );
						/*if( !g.jobGlobal.preFrameCommand.empty() )
							tmpTask.addParameter( parseString( g.jobGlobal.preFrameCommand ), "precommand" );
						if( !g.jobGlobal.postFrameCommand.empty() )
							tmpTask.addParameter( parseString( g.jobGlobal.postFrameCommand ), "postcommand" );*/
						//tasks.addTask( tmpTask );
						//currentFrameSuperTask = blockSuperTask;


					}

					sceneName = getCacheFilePath( fileName, g.directories.caching.dataSource ).native_file_string();

					blockPtrTracker[ internalContext ]->jobName = g.name.baseName + dottedBlockName;
				}
			}
		}

		debugMessage( L"Registering context " + CValue( internalContext ).GetAsText() );
		debugMessage( L"Render context is " + CValue( ctx ).GetAsText() );
		blockPtrTracker[ internalContext ]->renderContext = ctx;
		debugMessage( L"Begun Block " + stringToCString( dottedBlockName ) );
		activeContext = internalContext;

		return sceneName;
	}

	void blockManager::reset() {
		internalContext = 0;
		activeContext = 1;
		jobPtrStack.clear();
		blockPtrTracker.clear();
		tasks.clear();
	}

	context blockManager::currentContext() {
		return activeContext;
	}

	context blockManager::renderContext( const context& ctx ) {
		if( blockPtrTracker.end() != blockPtrTracker.find( ctx ) ) {
			return blockPtrTracker[ ctx ]->renderContext;
		} else {
			debugMessage( L"Can't switch to unknown block" );
			throw( out_of_range( "Can't return render context for unknown block" ) );
		}
	}

	void blockManager::switchBlock( const context& ctx ) {
		if( blockPtrTracker.end() != blockPtrTracker.find( ctx ) ) {
			ueberManInterface theRenderer;
			debugMessage( L"Switching to context " + CValue( ctx ).GetAsText() );
			debugMessage( L"Switching to render context " + CValue( blockPtrTracker[ ctx ]->renderContext ).GetAsText() );
			theRenderer.switchScene( blockPtrTracker[ ctx ]->renderContext );
			activeContext = ctx;
		} else {
			debugMessage( L"Can't switch to unknown block" );
			throw( out_of_range( "Can't switch to unknown block" ) );
		}
	}

	void blockManager::addMapRenderIoToCurrentBlock( const string& from, const string& to ) {
		const globals& g( globals::access() );
		//block& currentBlock( *blockPtrTracker[ internalContext ] );
		if( blockPtrTracker.end() == blockPtrTracker.find( internalContext ) ) {
			debugMessage( L"No active block to add task to" );
			throw( out_of_range( "Can't end unknown block" ) );
		} else
		if( g.directories.caching.mapWrite && g.directories.caching.mapCopy ) { //&& !currentBlock.jobName.empty() ) {

			//tasks.
			currentPreTaskStack.back()->setClass( "touchfile" );
			//currentPreTaskStack.back()->addFrame( ( int )round( g.animation.time ) );
			currentPreTaskStack.back()->addParameter( to );


			currentPostTaskStack.back()->setClass( "renderio" );
			//currentPostTaskStack.back()->addFrame( ( int )round( g.animation.time ) );
			currentPostTaskStack.back()->addParameter( from, "from" );
			currentPostTaskStack.back()->addParameter( to, "to" );
		}
	}

	void blockManager::addImageRenderIoToCurrentBlock( const string& from, const string& to ) {
		const globals& g( globals::access() );
		//block& currentBlock( *blockPtrTracker[ internalContext ] );
		if( blockPtrTracker.end() == blockPtrTracker.find( internalContext ) ) {
			debugMessage( L"No active block to add task to" );
			throw( out_of_range( "Can't end unknown block" ) );
		} else
		if( g.directories.caching.imageWrite && g.directories.caching.imageCopy ) { // && !currentBlock.jobName.empty() ) {
			currentPreTaskStack.back()->setClass( "touchfile" );
			//currentPreTaskStack.back()->addFrame( ( int )round( g.animation.time ) );
			currentPreTaskStack.back()->addParameter( to );

			currentPostTaskStack.back()->setClass( "renderio" );
			//currentPostTaskStack.back()->addFrame( ( int )round( g.animation.time ) );
			currentPostTaskStack.back()->addParameter( from, "from" );
			currentPostTaskStack.back()->addParameter( to, "to" );
		}
	}

	void blockManager::endBlock( const context& ctx, unsigned priority ) {
		globals& g( const_cast< globals& >( globals::access() ) );
		ueberManInterface theRenderer;

		// use current context to find block data
		if( blockPtrTracker.end() == blockPtrTracker.find( ctx ) ) {
			debugMessage( L"Can't end unknown block" );
			throw( out_of_range( "Can't end unknown block" ) );
		}

		block& currentBlock( *blockPtrTracker[ ctx ] );

		Application app;
		debugMessage( L"Ending Block " + stringToCString( currentBlock.name ) );

		if( currentBlock.started ) {
			if( g.directories.caching.dataWrite && g.directories.caching.dataCopy && !currentBlock.jobName.empty() ) { // if we write write to the cache and copy back, we need to create touch files
				currentPreTaskStack.back()->setClass( "touchfile" );
				currentPreTaskStack.back()->setTitle( currentBlock.jobName );
				//currentPreTaskStack.back()->addFrame( ( int )round( g.animation.time ) );
				currentPreTaskStack.back()->addParameter( ( g.directories.data / currentBlock.jobName ).native_file_string() + ".####.rib" );

				currentPostTaskStack.back()->setClass( "renderio" );
				currentPostTaskStack.back()->setTitle( currentBlock.jobName );
				//currentPostTaskStack.back()->addFrame( ( int )round( g.animation.time ) );
				currentPostTaskStack.back()->addParameter( ( getCacheFilePath( g.directories.data / currentBlock.jobName ) ).native_file_string() + ".####.rib", "from" );
				currentPostTaskStack.back()->addParameter( ( g.directories.data / currentBlock.jobName ).native_file_string() + ".####.rib", "to" );
			}

			//currentFrameSuperTask.setTitle( currentBlock.jobName );

			switch( g.data.granularity ) {
				case globals::data::granularityAttributes: {
					if( blockAttributes == currentBlock.type ) {
						debugMessage( L"Ending attribute context " + CValue( ctx ).GetAsText() );
						theRenderer.endScene( currentBlock.renderContext );
					}
				}
				case globals::data::granularityObjects: {
					if( blockObject == currentBlock.type ) {
						debugMessage( L"Ending object context " + CValue( ctx ).GetAsText() );
						theRenderer.endScene( currentBlock.renderContext );
					}
				}
				case globals::data::granularitySections: {
					if( ( blockAttributes != currentBlock.type ) && ( blockObject != currentBlock.type ) && ( blockWorld != currentBlock.type ) && ( blockScene != currentBlock.type ) && ( blockShadowScene != currentBlock.type ) ) {
						debugMessage( L"Ending section context " + CValue( ctx ).GetAsText() );
						theRenderer.endScene( currentBlock.renderContext );
					}
				}
				case globals::data::granularitySubFrame: {
					if( blockWorld == currentBlock.type ) {
						debugMessage( L"Ending sub-frame context " + CValue( ctx ).GetAsText() );
						theRenderer.endScene( currentBlock.renderContext );
					}
				}
				case globals::data::granularityFrame: {
					if( ( blockScene == currentBlock.type ) ||
						( blockMapScene == currentBlock.type ) ||
						( blockBrickScene == currentBlock.type ) ||
						( blockShadowScene == currentBlock.type )
						) {
						debugMessage( L"Ending frame context " + CValue( ctx ).GetAsText() );
						theRenderer.endScene( currentBlock.renderContext );

						//tasks.tasksToSubTasks();
						// Get this scene's job
						//shared_ptr< job > tmpJob( jobPtrStack.back() );
						job tmpJob( jobPtrStack.back()->getTitle() );

						//tmpJob = jobPtrStack.back();

						if( !g.jobGlobal.preFrameCommand.empty() )
							tmpJob.addCommand( parseString( g.jobGlobal.preFrameCommand ) );

						tmpJob.merge( *( jobPtrStack.back().get() ) );
						// pop the stack's top
						jobPtrStack.pop_back();

						if( !g.jobGlobal.postFrameCommand.empty() )
							tmpJob.addCommand( parseString( g.jobGlobal.postFrameCommand ) );

						// add the job to the subjob list of the new stack top
						jobPtrStack.back()->addSubJob( tmpJob );
						//jobPtrStack.back()->addSubJob( *( tmpJob.get() ) );

						// addSubJob copies the job, so the shared_ptr going out
						// of scope and getting deleted after this line is ok

						task thisBlockTask( "delight", currentBlock.jobName + ".####" );
						thisBlockTask.addFrame( ( int )( g.animation.time + 0.5 ) );

						thisBlockTask.addParameter( getCacheFilePath( g.directories.data / currentBlock.jobName, g.directories.caching.dataSource ).native_file_string() + ".####.rib", string() );

						//message( L"Adding Task!!!!", messageError );

						if( !( currentBlock.preTask.empty() || currentBlock.postTask.empty() ) ) {
							taskList thisTaskList( currentBlock.jobName + ".####", taskList::typeSuperTask );
							thisTaskList.addFrame( ( int )( g.animation.time + 0.5 ) );

							thisTaskList.addTask( currentBlock.preTask );

							thisTaskList.addTask( thisBlockTask );

							thisTaskList.addTask( currentBlock.postTask );

							tasks.addTaskList( priority, thisTaskList );

						} else {
							tasks.addTask( priority, thisBlockTask );
						}

						currentPreTaskStack.pop_back();
						currentPostTaskStack.pop_back();
					}
				}
			}
		}
		activeContext = currentBlock.parentBlockContext;
		blockPtrTracker.erase( ctx );
		debugMessage( L"Ended Block" );
	}

	blockManager::blockManager() {
		debugMessage( L"Constructing BlockManager" );
	}

	blockManager::~blockManager() {
		debugMessage( L"Destructing Block" );
		deleteJobs();
	}

	void blockManager::deleteJobs() {
		/*while( !jobPtrStack.empty() ) {
			delete jobPtrStack.top();
			jobPtrStack.pop();
		}*/

		//jobPtrStack.clear();
	}

	void blockManager::processJobChunk( int frameCounter, int chunkNo ) {
		const globals& g( globals::access() );

		if( g.jobGlobal.jobScript.chunkSize ) {
			if( !( frameCounter % g.jobGlobal.jobScript.chunkSize ) ) {
				string chunkName( ( format( ".%03d" ) % ( chunkNo + 1 ) ).str() );
				jobPtrStack.back()->setTitle( g.name.baseName + chunkName );

				filesystem::path jobFileName( g.directories.temp / ( g.name.baseName + ".job" + g.name.blockName + chunkName + ".xml" ) );

				if( globals::jobGlobal::jobScript::jobScriptXML == g.jobGlobal.jobScript.type ) {
					message( L"Spitting out XML job file to '" + stringToCString( jobFileName.native_file_string() ) + L"'", messageInfo );
					jobPtrStack.back()->writeXML( jobFileName.native_file_string() );

					if( ( globals::jobGlobal::launchJobInterpreter == g.jobGlobal.launch ) && ( !g.jobGlobal.jobScript.interpreter.empty() ) ) {
						message( L"Launching XML job file '" + stringToCString( jobFileName.native_file_string() ) + L"'...", messageInfo );
						Application app;
						bool interactive( app.IsInteractive() );
//#ifndef DEBUG
						execute( g.jobGlobal.jobScript.interpreter, jobFileName.native_file_string(), ".", !interactive );
//#endif
					}
				}

				if( globals::jobGlobal::launchRenderer == g.jobGlobal.launch )
					jobPtrStack.back()->run( g.jobGlobal.launchSub ? 99999 : 1 );

				chunkNo++;

				jobPtrStack.clear();
				jobPtrStack.push_back( shared_ptr< job >( new job() ) );
			}
		}
	}

	void blockManager::processJob( int chunkNo ) {
		const globals& g( globals::access() );

		if( ( !g.jobGlobal.jobScript.chunkSize ) || ( ( g.animation.times.size() - 1 ) % g.jobGlobal.jobScript.chunkSize ) ) { // if we have some tasks to write out left
			string chunkName = chunkNo ? ( format( ".%03d" ) % ( chunkNo + 1 ) ).str() : "";
			jobPtrStack.back()->setTitle( g.name.baseName + chunkName );

			filesystem::path jobFileName( g.directories.temp / ( g.name.baseName + ".job" + g.name.blockName + chunkName + ".xml" ) );

			if( globals::jobGlobal::jobScript::jobScriptXML == g.jobGlobal.jobScript.type ) {
				message( L"Spitting out XML job file to '" + stringToCString( jobFileName.native_file_string() ) + L"'", messageInfo );
				jobPtrStack.back()->writeXML( jobFileName.native_file_string() );
				if( ( globals::jobGlobal::launchJobInterpreter == g.jobGlobal.launch ) && ( !g.jobGlobal.jobScript.interpreter.empty() ) ) {
					message( L"Launching XML job file '" + stringToCString( jobFileName.native_file_string() ) + L"'...", messageInfo );
					Application app;
					bool interactive( app.IsInteractive() );
//#ifndef DEBUG
					execute( g.jobGlobal.jobScript.interpreter, jobFileName.native_file_string(), ".", !interactive );
//#endif
				}
			}

			if( globals::jobGlobal::launchRenderer == g.jobGlobal.launch )
				jobPtrStack.back()->run( g.jobGlobal.launchSub ? 99999 : 1 );
		}

		if( globals::jobGlobal::jobScript::jobScriptJobEngineXML == g.jobGlobal.jobScript.type ) {
//			taskListJobEngineFormatter formatter;
			string jobName( writeJobEngineXML( ( g.directories.temp / ( g.name.baseName + g.name.blockName ) ).native_file_string(), tasks.getTaskHierarchy() ) );
			message( L"Spitting out jobEngine file to '" + stringToCString( jobName ) + L"'", messageInfo );
			if( ( globals::jobGlobal::launchJobInterpreter == g.jobGlobal.launch ) && ( !g.jobGlobal.jobScript.interpreter.empty() ) ) {
				message( L"Launching jobEngine file '" + stringToCString( jobName ) + L"'...", messageInfo );
				Application app;
				bool interactive( app.IsInteractive() );
				execute( g.jobGlobal.jobScript.interpreter, jobName, ".", !interactive );
			}
		}
	}

	const blockManager& blockManager::access() {
		static blockManager theManager;
		return theManager;
	}

	worker::worker() {
		debugMessage( L"Constructing Worker" );
	}

	void worker::options( const string &dest ) {
		ueberManInterface theRenderer;
		const globals& g( globals::access() );
		blockManager& bm( const_cast< blockManager& >( blockManager::access() ) );

		debugMessage( L"Options" );
		bm.beginBlock( blockManager::blockOptions, g.data.sections.options );
		context ctx( bm.currentContext() );

		if( g.data.sections.options ) {
			theRenderer.attribute( "shading:quality", ( float )sqrt( 1 / ( g.shading.rate * g.shading.rate ) ) );
			theRenderer.attribute( "shading:interpolation", string( g.shading.smooth ? "smooth" : "constant" ) );
			theRenderer.attribute( "shading:motionfactor", ( float )g.reyes.motionFactor );
			theRenderer.attribute( "shading:focusfactor", ( float )g.reyes.focusFactor );
			theRenderer.attribute( "shading:backfacing", false );
			theRenderer.attribute( "dicing:hair", ( int )g.shading.hair );
			float oThresh[ 3 ] = { g.reyes.opacityThreshold, g.reyes.opacityThreshold, g.reyes.opacityThreshold };
			theRenderer.option( tokenValue( oThresh, 3, "limits:othreshold", tokenValue::storageUndefined, tokenValue::typeColor ) );

			if( g.rays.enable ) {
				theRenderer.option( "trace:maxdepth", ( int )g.rays.trace.depth );
				theRenderer.attribute( "irradiance:nsamples", ( int )g.rays.irradiance.samples );
				theRenderer.attribute( "irradiance:shadingrate", ( float )g.rays.irradiance.shadingRate );
				theRenderer.attribute( "trace:bias", ( float )g.rays.trace.bias );
				theRenderer.attribute( "trace:samplemotion", g.rays.trace.motion );
				theRenderer.attribute( "trace:displacements", g.rays.trace.displacements );
			} else {
				theRenderer.option( "trace:maxdepth", 0 );
			}
			theRenderer.attribute( "subsurface:shadingrate", ( float )g.rays.subSurface.rate );

			if( !g.passPtrArray.empty() ) {
				g.writePasses();
				const vector< filesystem::path > passNames( g.getPassNames() );
				for( vector< filesystem::path >::const_iterator it = passNames.begin(); it < passNames.end(); it++ ) {
					string imageSourceName( getCacheFilePath( *it ).native_file_string() );
					string imageDestinationName( it->native_file_string() );
					bm.addImageRenderIoToCurrentBlock( imageSourceName, imageDestinationName );
				}
			}
			if( g.baking.bake ) {
				theRenderer.attribute( "shading:sides", 2 );
				//float opacity[ 3 ] = { 0.1, 0.1, 0.1 };
				//theRenderer.attribute( tokenValue( opacity, 3, "shading:opacity", tokenValue::storageUndefined, tokenValue::typeColor ) );
				theRenderer.attribute( "dicing:rasterorient", false );
				theRenderer.attribute( "cull:hidden", false );
				theRenderer.attribute( "cull:backfacing", false );
				theRenderer.attribute( "pass", string( "bake" ) );
			}

			if( globals::feedback::previewNone != g.feedback.previewDisplay ) {
				switch( g.feedback.previewDisplay ) {
					case globals::feedback::previewIDisplay: {
						message( L"Adding preview 'i-display'", messageInfo );
						float quantize[ 4 ] = { 0, 65535, 0, 65535 };
						theRenderer.parameter( tokenValue( quantize, 4, "quantize", tokenValue::storageUndefined ) );
						float dither = 0.5;
						theRenderer.parameter( tokenValue( dither, "dither" ) );
						theRenderer.output( g.name.baseName + "." + g.name.currentFrame, "idisplay", "rgba", "testcam" );
						break;
					}
					case globals::feedback::previewHoudini: {
						message( L"Adding preview 'Houdini' display", messageInfo );
						theRenderer.output( g.name.baseName + "." + g.name.currentFrame, "houdini", "rgba", "testcam" );
						break;
					}
					case globals::feedback::previewIt: {
						message( L"Adding preview 'it' display", messageInfo );
						theRenderer.output( g.name.baseName + "." + g.name.currentFrame, "it", "rgba", "testcam" );
						break;
					}
					case globals::feedback::previewFramebuffer: {
						message( L"Adding preview 'framebuffer' display", messageInfo );
						// Make sure we output 16 bit to preview displays
						float quantize[ 4 ] = { 0, 65535, 0, 65535 };
						theRenderer.parameter( tokenValue( quantize, 4, "quantize", tokenValue::storageUndefined ) );
						float dither = 0.5;
						theRenderer.parameter( tokenValue( dither, "dither" ) );
						theRenderer.output( g.name.baseName + "." + g.name.currentFrame, "framebuffer", "rgb", "testcam" );
						break;
					}
				}
			}

			Application app;
			Model sceneRoot = app.GetActiveSceneRoot();
			CRefArray objects = sceneRoot.FindChildren( L"__options*", siNullPrimType, CStringArray() );
			if( objects.GetCount() ) {
				if( isVisible( objects[ 0 ] ) ) {
					debugMessage( L"Found global option null." );

					CRefArray props( ( X3DObject( objects[ 0 ] ).GetProperties() ) );
					CRefArray affogatoProps;
					props.Filter( CString(), CStringArray(), L"affogato*", affogatoProps );

					for( int i = 0; i < affogatoProps.GetCount(); i++ ) {
						Property prop( affogatoProps[ i ] );

						CParameterRefArray params( prop.GetParameters() );

						for( int p = 0; p < params.GetCount(); p++ ) {
							Parameter param( params[ p ] );
							string paramName = CStringToString( param.GetName() );

							replace_first( paramName, string( "_" ), string( ":" ) );

							CValue value( param.GetValue( g.animation.time ) );
							switch( value.m_t ) {
								case CValue::siBool:
								case CValue::siInt1:
								case CValue::siInt2:
								case CValue::siInt4:
								case CValue::siUInt1:
								case CValue::siUInt2:
								case CValue::siUInt4:
									debugMessage(L"Adding: int " + stringToCString( paramName ) );
									theRenderer.option( paramName, ( int )( ( long )value ) );
									break;
								case CValue::siFloat:
								case CValue::siDouble:
									debugMessage(L"Adding: int " + stringToCString( paramName ) );
									theRenderer.option( paramName, ( float )value );
									break;
								case CValue::siString:
								case CValue::siWStr:
									debugMessage(L"Adding: int " + stringToCString( paramName ) );
									theRenderer.option( paramName, parseString( CStringToString( value ) ) );
									break;
								// Todo: Array support!!!
							}
						}
					}

					for( int i = 0; i < props.GetCount(); i++ ) {
						shader tmpShader( ( Property( props[ i ] ) ) );
						tmpShader.write();
					}
				}
			}
		}

		bm.endBlock( ctx );
	}

	void worker::camera( const string &dest ) {
		ueberManInterface theRenderer;
		globals& g = const_cast< globals& >( globals::access() );
		blockManager& bm = const_cast< blockManager& >( blockManager::access() );

		Application app;
		debugMessage( L"Camera" );
		bm.beginBlock( blockManager::blockCamera, g.data.sections.camera );
		context ctx = bm.currentContext();

		if( g.data.sections.camera ) {

			Model sceneRoot = app.GetActiveSceneRoot();

			// Search for the cameraName passed -- if it doesn't exist, use the first camera found
			/*long cam = 0;
			CString cameraName = stringToCString( g.camera.cameraName );
			for( long i = 0; i < cameras.GetCount(); i++ ) {
				if( cameraName == X3DObject( cameras[ i ] ).GetFullName() ) {
					cam = i;
					break;
				}
			}
			X3DObject camera( cameras[ cam ] );*/

			X3DObject camera( sceneRoot.FindChild( stringToCString( g.camera.cameraName ), siCameraPrimType, CStringArray() ) );
			if( !camera.IsValid() ) {
				CRefArray cameras( sceneRoot.FindChildren( L"", siCameraPrimType, CStringArray() ) );
				camera = cameras[ 0 ];
			}

			//for( long i = 0; i < cameras.GetCount(); i++ ) {
			message( L"Found: " + camera.GetName(), messageInfo );

			theRenderer.attribute( "identifier:name", CStringToString( camera.GetFullName() ) );

			g.camera.nearClip = camera.GetParameterValue( L"near", g.animation.time );
			g.camera.farClip = camera.GetParameterValue( L"far", g.animation.time );
			if( g.camera.freezeScale ) {
				float zScale = ( float )camera.GetKinematics().GetGlobal().GetTransform( g.animation.time ).GetScaling().GetZ();
				g.camera.nearClip *= zScale;
				g.camera.farClip *= zScale;
			}
			theRenderer.parameter( "near", g.camera.nearClip );
			theRenderer.parameter( "far", g.camera.farClip );

			float timeOrigin( globals::motionBlur::shutterMoving == g.motionBlur.shutterConfiguration ? g.motionBlur.shutterOffset + g.animation.time : g.motionBlur.shutterOffset );
			float samples[ 2 ] = { timeOrigin + g.motionBlur.shutterOpen, timeOrigin + g.motionBlur.shutterClose };
			theRenderer.parameter( tokenValue( samples, 2, "shutter", tokenValue::storageUndefined, tokenValue::typeFloat ) );

			theRenderer.parameter( "shutteroffset", g.motionBlur.shutterOffset );

			float shutterEfficiency[ 2 ] = { g.motionBlur.shutterEfficiency, g.motionBlur.shutterEfficiency };
			theRenderer.parameter( tokenValue( shutterEfficiency, 2, "shutterefficiency", tokenValue::storageUndefined, tokenValue::typeFloat ) );

			theRenderer.parameter( "jitter", g.reyes.jitter );

			theRenderer.parameter( "samplemotion", g.reyes.sampleMotion );

			theRenderer.parameter( "extrememotiondof", g.reyes.extremeMotionDepthOfField );

			theRenderer.parameter( "bucketorder", g.reyes.bucketorder );

			int bucket[ 2 ] = { g.reyes.bucketSize.x, g.reyes.bucketSize.y };
			theRenderer.parameter( tokenValue( bucket, 2, "bucketsize", tokenValue::storageUndefined ) );

			theRenderer.parameter( "texturememory", ( int )g.reyes.textureMemory );

			theRenderer.parameter( "gridsize", ( int )g.reyes.gridSize );

			theRenderer.parameter( "eyesplits", ( int )g.reyes.eyeSplits );

			int res[ 2 ] = { g.resolution.x, g.resolution.y };

			if( g.camera.useAspect )
				g.camera.aspect = ( float )camera.GetParameterValue( L"aspect", g.animation.time );
			else
				g.camera.aspect = float( res[ 0 ] ) / float( res[ 1 ] );

			g.camera.fieldOfView = ( float )camera.GetParameterValue( L"fov", g.animation.time );

			unsigned fovType( ( long )camera.GetParameterValue( L"fovtype", g.animation.time ) );
			// If the fov is vertical (0) and the aspect < 1 or the fov is horizontal (1) and the aspect > 1
			if( ( ( 0 == fovType ) && ( 1 > g.camera.aspect ) ) || ( ( 1 == fovType ) && ( 1 < g.camera.aspect ) ) ) {
				g.camera.fieldOfView = ( 360.0f * atan( ( 1.0f / g.camera.aspect ) * tan( g.camera.fieldOfView * PI / 360.0f ) ) ) / PI;
			}

			float focalLength( ( float )camera.GetParameterValue( L"projplanedist", g.animation.time ) );

			CRefArray props( camera.GetProperties() );
			if( g.camera.hypeOverscan ) {
				bool foundHype( false );
				for( long i = 0; i < props.GetCount(); i++ ) {
					Property prop( props[ i ] );
					if( CString( L"rspCameraProperty" ) == prop.GetType() ) {

						res[ 0 ] = ( unsigned long )prop.GetParameterValue( L"adjustedRenderResX" );
						res[ 1 ] = ( unsigned long )prop.GetParameterValue( L"adjustedRenderResY" );
						float scale = ( float )prop.GetParameterValue( L"HypeScalarValueForThisLens" );

						if( res[ 0 ] > res[ 1 ] ) { // Landscape
							// Da fukin film back is in fukin inchez!
							float height = 25.4f * ( float )prop.GetParameterValue( L"originalFilmBackY" );
							// Compensate for overscan
							height *= ( res[ 1 ] / ( ( unsigned long )prop.GetParameterValue( L"origRenderResY" ) / scale ) ) / scale;
							g.camera.fieldOfView = ( 360.0f * atan( ( height / 2.0f ) / focalLength ) ) / PI;
						} else { // Portrait
							// Da fukin film back is in fukin inchez!
							float width = 25.4f * ( float )prop.GetParameterValue( L"originalFilmBackY" );
							// Compensate for overscan
							width *= ( res[ 1 ] / ( ( unsigned long )prop.GetParameterValue( L"origRenderResY" ) / scale ) ) / scale;
							g.camera.fieldOfView = ( 360.0f * atan( ( width / 2.0f ) / focalLength ) ) / PI;
						}

						foundHype = true;
						break;
					}
				}

				if( !foundHype )
					message( L"Hype camera property not found! Hype overscan will NOT be used.", messageWarning );
			}
			theRenderer.parameter( "fov", g.camera.fieldOfView );

			res[ 0 ] = int( res[ 0 ] * g.resolution.multiplier + 0.5f );
			res[ 1 ] = int( res[ 1 ] * g.resolution.multiplier + 0.5f );
			theRenderer.parameter( tokenValue( res, 2, "resolution", tokenValue::storageUndefined ) );

			props.Filter( CString(), CStringArray(), L"affogato*", props );
			for( int i = 0; i < props.GetCount(); i++ ) {
				Property prop( props[ i ] );

				if( g.camera.depthOfField ) {
					CValue fStop = prop.GetParameterValue( L"fstop", g.animation.time );
					CValue focalDistance = prop.GetParameterValue( L"focaldistance", g.animation.time );
					if( !fStop.IsEmpty() && !focalDistance.IsEmpty() ) {
						g.camera.fStop = ( float )fStop;
						g.camera.focalDistance = ( float )focalDistance;
						theRenderer.parameter( "fstop", g.camera.fStop );
						theRenderer.parameter( "focallength", focalLength );
						theRenderer.parameter( "focaldistance", g.camera.focalDistance );
					}
					break;
				}
			}

			float screen[ 4 ] = { -1, 1, -1, 1 };
			switch( g.camera.rotoViewStyle ) {
				case globals::camera::rotoViewStyleFillScreen: {
					message( L"Zoom", messageError );
					screen[ 0 ] = ( 1 <= g.camera.aspect ? g.camera.aspect : 1.0 ) * ( 2.0f * ( float )camera.GetParameterValue( L"subfrustumleft", g.animation.time ) - 1.0f );
					screen[ 1 ] = ( 1 <= g.camera.aspect ? g.camera.aspect : 1.0 ) * ( 2.0f * ( float )camera.GetParameterValue( L"subfrustumright", g.animation.time ) - 1.0f );
					screen[ 2 ] = ( 1 > g.camera.aspect ? 1.0 / g.camera.aspect : 1.0 ) * ( 2.0f * ( ( float )camera.GetParameterValue( L"subfrustumbottom", g.animation.time ) ) - 1.0f );
					screen[ 3 ] = ( 1 > g.camera.aspect ? 1.0 / g.camera.aspect : 1.0 ) * ( 2.0f * ( ( float )camera.GetParameterValue( L"subfrustumtop", g.animation.time ) ) - 1.0f );
					theRenderer.parameter( tokenValue( screen , 4, "screen", tokenValue::storageUndefined, tokenValue::typeFloat ) );
					break;
				}
				case globals::camera::rotoViewStyleCrop: {
					float crop[ 4 ] = { camera.GetParameterValue( L"subfrustumleft", g.animation.time ),
										camera.GetParameterValue( L"subfrustumright", g.animation.time ),
										1.0f - ( float )camera.GetParameterValue( L"subfrustumbottom", g.animation.time ),
										1.0f - ( float )camera.GetParameterValue( L"subfrustumtop", g.animation.time ) };
					theRenderer.parameter( tokenValue( crop , 4, "crop", tokenValue::storageUndefined, tokenValue::typeFloat ) );
				}
				default: {
					// Set values in case we're using an orthographic camera
					screen[ 0 ] = -g.camera.aspect;
					screen[ 1 ] = +g.camera.aspect;
				}
			}

			// If this is a perspective projection
			if( camera.GetParameterValue( L"proj", g.animation.time ) ) {
				theRenderer.parameter( "projection", string( "perspective" ) );
			} else {
				theRenderer.parameter( "projection", string( "orthographic" ) );
				float orthoSizeHalf( 0.5f * ( float )camera.GetParameterValue( L"orthoheight", g.animation.time ) );
				screen[ 0 ] *= orthoSizeHalf;
				screen[ 1 ] *= orthoSizeHalf;
				screen[ 2 ] *= orthoSizeHalf;
				screen[ 3 ] *= orthoSizeHalf;
				theRenderer.parameter( tokenValue( screen , 4, "screen", tokenValue::storageUndefined, tokenValue::typeFloat ) );
			}

			if( g.baking.bake ) {
				float pixsamples[ 2 ] = { 1.0f, 1.0f };
				theRenderer.parameter( tokenValue( pixsamples, 2, "pixelsamples", tokenValue::storageUndefined ) );
			} else {
				float pixsamples[ 2 ] = { ( float )g.sampling.x, ( float )g.sampling.y };
				theRenderer.parameter( tokenValue( pixsamples, 2, "pixelsamples", tokenValue::storageUndefined ) );
			}

			cameraHandle camHandle( "defaultcamera" );
			theRenderer.camera( camHandle );

			MATH::CMatrix4 flip( 	1,  0,  0,  0,
									0,  1,  0,  0,
									0,  0, -1,  0,
									0,  0,  0,  1 );

			if( g.motionBlur.cameraBlur ) {
				vector< float > sampletimes( getMotionSamples( g.motionBlur.transformMotionSamples ) );
				theRenderer.motion( remapMotionSamples( sampletimes ) );

				for( unsigned short motion = 0; motion < g.motionBlur.transformMotionSamples; motion++ ) {
					CTransformation transform( camera.GetKinematics().GetGlobal().GetTransform( sampletimes[ motion ] ) );
					if( g.camera.freezeScale ) {
						transform.SetScaling( CVector3( 1.0, 1.0, 1.0 ) );
					}
					CMatrix4 matrix( transform.GetMatrix4() );
					matrix.InvertInPlace();
					matrix.MulInPlace( flip );
					theRenderer.space( CMatrix4ToFloat( matrix ) );
				}
			} else {
				CMatrix4 matrix = camera.GetKinematics().GetGlobal().GetTransform( g.animation.time ).GetMatrix4();
				matrix.InvertInPlace();
				matrix.MulInPlace( flip );
				theRenderer.space( CMatrix4ToFloat( matrix ) );
			}
		}

		bm.endBlock( ctx );
	}

	void worker::globalAttributes( const string &dest ) {
		ueberManInterface theRenderer;
		globals& g = const_cast< globals& >( globals::access() );
		blockManager& bm = const_cast< blockManager& >( blockManager::access() );

		debugMessage( L"Global Attributes" );

		if( g.data.sections.world ) {
			Application app;
			Model sceneRoot = app.GetActiveSceneRoot();
			CRefArray objects = sceneRoot.FindChildren( L"__attributes*", siNullPrimType, CStringArray() );
			if( objects.GetCount() ) {
				debugMessage( L"Found global attribute null." );

				if( isVisible( objects[ 0 ] ) ) {
					CRefArray props( ( X3DObject( objects[ 0 ] ).GetProperties() ) );
					CRefArray affogatoProps;
					props.Filter( CString(), CStringArray(), L"affogato*", affogatoProps );

					for( int i = 0; i < affogatoProps.GetCount(); i++ ) {

						Property prop( affogatoProps[ i ] );

						CParameterRefArray params = prop.GetParameters();

						for( int p = 0; p < params.GetCount(); p++ ) {
							Parameter param( params[ p ] );
							string paramName = CStringToString( param.GetName() );

							replace_first( paramName, string( "_" ), string( ":" ) );

							CValue value( param.GetValue( g.animation.time ) );
							switch( value.m_t ) {
								case CValue::siBool:
								case CValue::siInt1:
								case CValue::siInt2:
								case CValue::siInt4:
								case CValue::siUInt1:
								case CValue::siUInt2:
								case CValue::siUInt4:
									debugMessage(L"Adding: int " + stringToCString( paramName ) );
									theRenderer.attribute( paramName, ( int )( ( long )value ) );
									break;
								case CValue::siFloat:
								case CValue::siDouble:
									debugMessage(L"Adding: int " + stringToCString( paramName ) );
									theRenderer.attribute( paramName, ( float )value );
									break;
								case CValue::siString:
								case CValue::siWStr:
									debugMessage(L"Adding: int " + stringToCString( paramName ) );
									theRenderer.attribute( paramName, parseString( CStringToString( value ) ) );
									break;
								// Todo: Array support!!!
							}
						}
					}

					for( int i = 0; i < props.GetCount(); i++ ) {
						shader tmpShader( ( Property( props[ i ] ) ) );
						tmpShader.write();
					}
				}
			}

			if( !g.defaultShader.surface.empty() ) {
				shader surface( shader::shaderSurface, g.defaultShader.surface );
				surface.write();
			}
			if( !g.defaultShader.displacement.empty() ) {
				shader displacement( shader::shaderDisplacement, g.defaultShader.displacement );
				displacement.write();
			}
			if( !g.defaultShader.volume.empty() ) {
				shader volume( shader::shaderVolume, g.defaultShader.volume );
				volume.write();
			}
		}
	}

	void worker::spaces( const CRefArray &objectList, const string &dest ) {
		globals& g = const_cast< globals& >( globals::access() );

		blockManager& bm = const_cast< blockManager& >( blockManager::access() );

		debugMessage( L"Spaces" );

		bm.beginBlock( blockManager::blockSpaces, g.data.sections.spaces );
		context ctx = bm.currentContext();
		// Write out coordinate systems

		if( g.data.sections.spaces ) {
			double atTime( 0 );
			ueberManInterface theRenderer;

			Application app;

			Model sceneRoot = app.GetActiveSceneRoot();
			CRefArray objects;
			objectList.Filter( siNullPrimType, CStringArray(), CString(), objects );

			for( long i = 0; i < objects.GetCount(); i++ ) {

				X3DObject obj( objects[ i ] );

				string objName = getAffogatoName( CStringToString( obj.GetFullName() ) );

				if( isVisible( obj ) && ( "__options" != objName ) && ( "__attributes" != objName ) && ( "__backplane" != objName ) && ( "__frontplane" != objName ) ) {
					theRenderer.pushSpace();

					if( g.data.relativeTransforms )
						theRenderer.appendSpace( CMatrix4ToFloat( obj.GetKinematics().GetGlobal().GetTransform( g.animation.time ).GetMatrix4() ) );
					else
						theRenderer.space( CMatrix4ToFloat( obj.GetKinematics().GetGlobal().GetTransform( g.animation.time ).GetMatrix4() ) );
					theRenderer.nameSpace( objName );
					theRenderer.popSpace();
				}
			}

			X3DObject camera( sceneRoot.FindChild( stringToCString( g.camera.cameraName ), siCameraPrimType, CStringArray() ) );
			if( !camera.IsValid() ) {
				CRefArray cameras( sceneRoot.FindChildren( L"", siCameraPrimType, CStringArray() ) );
				camera = cameras[ 0 ];
			}



			CRefArray props( getAffogatoProperties( camera ) );

			for( int i = 0; i < props.GetCount(); i++ ) {
				Property prop( props[ i ] );
				CValue value( prop.GetParameterValue( L"__frozencamera", g.animation.time ) );
				if( !value.IsEmpty() )
					atTime = value;
			}

			theRenderer.pushSpace();
			if( g.data.relativeTransforms )
				theRenderer.appendSpace( CMatrix4ToFloat( camera.GetKinematics().GetGlobal().GetTransform( atTime ).GetMatrix4() ) );
			else
				theRenderer.space( CMatrix4ToFloat( camera.GetKinematics().GetGlobal().GetTransform( atTime ).GetMatrix4() ) );
			spaceHandle s( "__affogatofrozencamera" );
			theRenderer.nameSpace( s );
			theRenderer.popSpace();

		}

		bm.endBlock( ctx );
	}

	void worker::nulls( const CRefArray &objectList, const string &dest ) {
		globals& g = const_cast< globals& >( globals::access() );
		blockManager& bm = const_cast< blockManager& >( blockManager::access() );

		context ctx = bm.currentContext();
		// Write out coordinate systems with attributes

		if( g.data.sections.geometry ) {
			ueberManInterface theRenderer;
			Application app;

			Model sceneRoot = app.GetActiveSceneRoot();
			CRefArray objects;
			objectList.Filter( siNullPrimType, CStringArray(), CString(), objects );

			for( unsigned  i = 0; i < ( unsigned )objects.GetCount(); i++ ) {

				X3DObject obj( objects[ i ] );

				string objName = getAffogatoName( CStringToString( obj.GetFullName() ) );

				debugMessage( stringToCString( objName ) );

				if( isVisible( obj ) && ( "__options" != objName ) && ( "__attributes" != objName ) && ( "__backplane" != objName ) && ( "__frontplane" != objName ) ) {
					node object( obj );
					if( object.isArchive() || object.isDataBox() ) {
						theRenderer.pushAttributes();
						object.writeTransform();
						object.writeAttributes();
						object.writeGeometry();
						theRenderer.popAttributes();
					}
				}
			}
		}
	}

	void worker::shadow( const Light &light ) {
		ueberManInterface theRenderer;
		const globals& g( globals::access() );

		vector< string >shadowNameArray;

		CParameterRefArray lightShaderParams( ProjectItem( light.GetShaders()[ 0 ] ).GetParameters() );
		string objName( CStringToString( light.GetFullName() ) );

		if( ( bool )Parameter( lightShaderParams.GetItem( L"shadow" ) ).GetValue( g.animation.time ) ) {
			if( light.GetParameterValue( L"ShadowMap", g.animation.time ) ) {
				if( !worldBlockName.empty() ) {

					float shadowSamples;

					bool deepShadow( light.GetParameterValue( L"ShadowMapDetail", g.animation.time ) );

					blockManager& bm( const_cast< blockManager& >( blockManager::access() ) );

					string jobName( g.name.baseName + "." + objName + "." + g.name.currentFrame );
					string jobNameGlobal( g.name.baseName + "." + objName + ".####" );
					string shadowMapName( ( g.directories.map / jobNameGlobal ).native_file_string() );
					string shadowMapWriteName( getCacheFilePath( g.directories.map / jobName, g.directories.caching.mapWrite ).native_file_string() );
					string shadowMapWriteNameGlobal( getCacheFilePath( g.directories.map / jobNameGlobal, g.directories.caching.mapWrite ).native_file_string() );
					string shadowMapSourceName( getCacheFilePath( g.directories.map / jobName, g.directories.caching.mapSource ).native_file_string() );

					if( !g.data.shadow.shadow ) {
						// We just call beginBlock to get the shadow RIB name to reference-in the shader.
						//shadowName = shadowMapName = bm.beginBlock( blockManager::blockShadowScene, false, objName + ".shadow" );
						//bm.endBlock( bm.currentContext() );

						if( deepShadow ) {
							//shadowSamples = 4; // Deep shadow maps should use four samples (not any more!)
							shadowMapName				+= ".dsh";
							shadowMapWriteName 			+= ".dsh";
							shadowMapWriteNameGlobal	+= ".dsh";
							shadowMapSourceName			+= ".dsh";
						} else {
							//shadowSamples = light.GetParameterValue( L"ShadowMapSamp" );
							shadowMapName				+= ".shd";
							shadowMapWriteName			+= ".shd";
							shadowMapWriteNameGlobal	+= ".shd";
							shadowMapSourceName			+= ".shd";
						}

						shadowSamples = light.GetParameterValue( L"ShadowMapSamp", g.animation.time );
					} else {
						bm.beginBlock( blockManager::blockShadowScene, true, objName + ".shadow" );

						context ctx = bm.currentContext();

						CRefArray props( light.GetLocalProperties() );
						for( unsigned i = 0; i < ( unsigned )props.GetCount(); i++ ) {
							Property prop( props[ i ] );
							CParameterRefArray params( prop.GetParameters() );
							if( shader::shaderLight == shader::getType( prop ) ) {
								string category( CStringToString( prop.GetParameterValue( L"__category" ) ) );
								if( !category.empty() ) {
									theRenderer.option( "lightcategory", string( category ) );
									break;
								}
							}
						}

						theRenderer.attribute( "shading:rate", ( float ) g.shading.shadowRate );
						theRenderer.attribute( "shading:interpolation", string( "smooth" ) ); //deepShadow ? "smooth" : "constant" ) );
						theRenderer.attribute( "dicing:hair", 1 );

						theRenderer.attribute( "identifier:name", objName + ".camera" );

						theRenderer.parameter( "projection", string( 2 == ( long )light.GetParameterValue( L"Type" ) ? "perspective" : "orthographic" ) );

						theRenderer.parameter( "fov", ( float )light.GetParameterValue( L"LightCone", g.animation.time ) );

						// I don't think that grabbing the clipping planes from the scene camera is the right thing to do here.
						// Let's set the defaults to 0.1->1e6 and allow overrides in an "affogato" custom property on the light.

						props = getAffogatoProperties( light );

						unsigned short motionsamples = g.motionBlur.transformMotionSamples;
						float nearClip = 0.1f;
						float farClip = 1000000.0f;

						for( long k = 0; k < props.GetCount(); k++ ) {
							Property prop( props[ k ] );

							CValue value( prop.GetParameterValue( L"motionsamples", g.animation.time ) );
							if( !value.IsEmpty() ) {
								motionsamples = value;
								if( motionsamples > MAXMOTIONSAMPLES )
									motionsamples = MAXMOTIONSAMPLES;
							}

							value = prop.GetParameterValue( L"nearclip", g.animation.time );
							if( !value.IsEmpty() )
								nearClip = value;

							value = prop.GetParameterValue( L"farclip", g.animation.time );
							if( !value.IsEmpty() )
								farClip = value;
						}

						theRenderer.parameter( "near", nearClip );
						theRenderer.parameter( "far", farClip );

						int shdRes = ( long )light.GetParameterValue( L"ShadowMapRes", g.animation.time );
						int res[ 2 ] = { shdRes, shdRes };
						theRenderer.parameter( tokenValue( res, 2, "resolution", tokenValue::storageUndefined ) );

						float timeOrigin( globals::motionBlur::shutterMoving == g.motionBlur.shutterConfiguration ? g.motionBlur.shutterOffset + g.animation.time : g.motionBlur.shutterOffset );
						// If motion blurred shadow maps are disabled, set shutter close = shutter open
						float samples[ 2 ] = {	timeOrigin + ( g.motionBlur.shadowMapBlur ? g.motionBlur.shutterOpen : 0 ),
												timeOrigin + ( g.motionBlur.shadowMapBlur ? g.motionBlur.shutterClose : 0 ) };
						theRenderer.parameter( tokenValue( samples, 2, "shutter", tokenValue::storageUndefined, tokenValue::typeFloat ) );
						float shutterEfficiency[ 2 ] = { g.motionBlur.shutterEfficiency, g.motionBlur.shutterEfficiency };
						theRenderer.parameter( tokenValue( shutterEfficiency, 2, "shutterefficiency", tokenValue::storageUndefined, tokenValue::typeFloat ) );
						theRenderer.parameter( "bucketorder", g.reyes.bucketorder );
						theRenderer.parameter( "eyesplits", ( int )g.reyes.eyeSplits );


						shadowSamples = light.GetParameterValue( L"ShadowMapSamp", g.animation.time );

						cameraHandle camHandle( "shadowcamera" );

						if( deepShadow ) { // We're in a deep shadow pass
							shadowMapName		+= ".dsh";
							shadowMapWriteName	+= ".dsh";
							shadowMapWriteNameGlobal += ".dsh";
							shadowMapSourceName	+= ".dsh";

							theRenderer.attribute( "pass", string( "deepshadow" ) );
							theRenderer.option( "pass", string( "deepshadow" ) );

							int shdSmp = ( long )light.GetParameterValue( L"ShadowMapDetailSamples", g.animation.time );
							float pixsamples[ 2 ] = { ( float )shdSmp, ( float )shdSmp };
							theRenderer.parameter( tokenValue( pixsamples, 2, "pixelsamples", tokenValue::storageUndefined ) );
							theRenderer.camera( camHandle );

							theRenderer.parameter( "volumeinterpretation", string( light.GetParameterValue( L"ShadowMapDetailAccuracy", g.animation.time ) ? "continuous" : "discrete" ) ); // DSM interpolation  0 -discrete, otherwise continuous

							theRenderer.parameter( "filter", string( "box" ) );
							float filter[ 2 ] = { 1, 1 };
							theRenderer.parameter( tokenValue( filter, 2, "filterwidth", tokenValue::storageUniform, tokenValue::typeFloat ) );
							theRenderer.output( shadowMapWriteName, "dsm", "rgbaz", camHandle );
						} else {
							shadowMapName		+= ".shd";
							shadowMapWriteName	+= ".shd";
							shadowMapWriteNameGlobal += ".shd";
							shadowMapSourceName	+= ".shd";

							theRenderer.attribute( "pass", string( "shadow" ) );
							theRenderer.option( "pass", string( "shadow" ) );

							float pixsamples[ 2 ] = { 1.0f, 1.0f };
							theRenderer.parameter( tokenValue( pixsamples, 2, "pixelsamples", tokenValue::storageUndefined ) );
							theRenderer.parameter( "depthfilter", string( "midpoint" ) );
							theRenderer.camera( camHandle );

							theRenderer.parameter( "filter", string( "box" ) );
							float filter[ 2 ] = { 1, 1 };
							theRenderer.parameter( tokenValue( filter, 2, "filterwidth", tokenValue::storageUniform, tokenValue::typeFloat ) );
							theRenderer.parameter( "zcompression", string( "zip" ) );
							theRenderer.output( shadowMapWriteName, "shadow", "z", camHandle );
						}

						CMatrix4 camflip(   1,  0,  0,  0,
											0,  1,  0,  0,
											0,  0, -1,  0,
											0,  0,  0,  1 );

						if( g.motionBlur.lightBlur ) {
							vector< float > sampletimes( getMotionSamples( motionsamples ) );

							theRenderer.motion( remapMotionSamples( sampletimes ) );
							for( unsigned short motion = 0; motion < motionsamples; motion++ ) {
								CMatrix4 matrix = light.GetKinematics().GetGlobal().GetTransform( sampletimes[ motion ] ).GetMatrix4();
								matrix.InvertInPlace();
								matrix.MulInPlace( camflip );

								theRenderer.space( CMatrix4ToFloat( matrix ) );
							}
						} else {
							CMatrix4 matrix = light.GetKinematics().GetGlobal().GetTransform( g.animation.time ).GetMatrix4();
							matrix.InvertInPlace();
							matrix.MulInPlace( camflip );
							theRenderer.space( CMatrix4ToFloat( matrix ) );
						}

						theRenderer.input( worldBlockName.native_file_string() );

						bm.addMapRenderIoToCurrentBlock( shadowMapWriteNameGlobal, shadowMapName );

						bm.endBlock( ctx, 300 );
					}

					theRenderer.parameter( "shadowname", string( shadowMapSourceName ) );
					theRenderer.parameter( "shadowsamples", ( float )shadowSamples );
					float bias = ( float )light.GetParameterValue( L"ShadowMapBias", g.animation.time );
					theRenderer.parameter( "shadowbias", float( bias == 0.0 ? 0.01f : bias ) );
					theRenderer.parameter( "shadowblur", ( float )light.GetParameterValue( L"ShadowMapSoft", g.animation.time ) );

				} else {
					message( L"World block does not exist! Omitting writing of shadow map RIB.", messageError );
				}
			} else if( g.rays.enable ) {
				theRenderer.parameter( "shadowsamples", ( float )light.GetParameterValue( L"ShadowMapSamp", g.animation.time ) );
				float bias = ( float )light.GetParameterValue( L"ShadowMapBias" );
				theRenderer.parameter( "shadowbias", float( bias == 0.0 ? 0.01f : bias ) );
				theRenderer.parameter( "shadowblur", ( float )light.GetParameterValue( L"ShadowMapSoft", g.animation.time ) );
				theRenderer.parameter( "shadowname", string( "raytrace" ) );
			}
		}
	}

	void worker::lights( const CRefArray &objectList, const string &dest ) {
		globals& g = const_cast< globals& >( globals::access() );
		blockManager& bm = const_cast< blockManager& >( blockManager::access() );

		debugMessage( L"Lights" );
		bm.beginBlock( blockManager::blockLights, g.data.sections.lights );
		context ctx = bm.currentContext();

		if( g.data.sections.lights ) {
			Application app;
			ueberManInterface theRenderer;

			/*Model sceneRoot = app.GetActiveSceneRoot();
			CRefArray objects = sceneRoot.FindChildren( L"", siLightPrimType, CStringArray() );*/

			// Filter lights

			Model sceneRoot = app.GetActiveSceneRoot();
			CRefArray objects;
			objectList.Filter( siLightPrimType, CStringArray(), CString(), objects );

			for( long i = 0; i < objects.GetCount(); i++ ) {

				X3DObject obj( objects[ i ] );

				message( L"Found light: " + objects[ i ].GetAsText(), messageInfo );

				if( isVisible( obj ) ) {

					theRenderer.pushAttributes();

					string category;
					CRefArray props( obj.GetProperties() );

					string objName( CStringToString( X3DObject( objects[ i ] ).GetFullName() ) );
					theRenderer.attribute( "identifier:name", objName );

					unsigned short motionsamples( g.motionBlur.transformMotionSamples );

					for( unsigned k = 0; k < ( unsigned )props.GetCount(); k++ ) {
						Property prop( props[ k ] );
						if( isAffogatoProperty( prop ) ) {
							motionsamples = prop.GetParameterValue( L"motionsamples", g.animation.time );
							if( motionsamples ) {
								if( motionsamples > MAXMOTIONSAMPLES )
									motionsamples = MAXMOTIONSAMPLES;
								break;
							}
						}
					}

					CMatrix4 zscale(    1,  0,  0,  0,
										0,  1,  0,  0,
										0,  0, -1,  0,
										0,  0,  0,  1 );

					if( g.motionBlur.lightBlur ) {
						vector< float > sampletimes( getMotionSamples( motionsamples ) );
						theRenderer.motion( remapMotionSamples( sampletimes ) );
						for( unsigned short motion = 0; motion < motionsamples; motion++ ) {
							CMatrix4 matrix = obj.GetKinematics().GetGlobal().GetTransform( sampletimes[ motion ] ).GetMatrix4();
							zscale.MulInPlace( matrix );
							if( g.data.relativeTransforms )
								theRenderer.appendSpace( CMatrix4ToFloat( zscale ) );
							else
								theRenderer.space( CMatrix4ToFloat( zscale ) );

						}
					} else {
						CMatrix4 matrix = obj.GetKinematics().GetGlobal().GetTransform( g.animation.time ).GetMatrix4();
						zscale.MulInPlace( matrix );
						if( g.data.relativeTransforms )
							theRenderer.appendSpace( CMatrix4ToFloat( zscale ) );
						else
							theRenderer.space( CMatrix4ToFloat( zscale ) );
					}

					string shaderName;
					long lightType( ( long )obj.GetParameterValue( L"Type" ) );
					switch( lightType ) {
						case 0: // Point
							shaderName = "affogatopoint";
							break;
						case 1: // Distant/Infinte
							shaderName = "affogatodistant";
							break;
						case 2: // Spot
							shaderName = "affogatospot";
							break;
					}

					CParameterRefArray lightShaderParams = ProjectItem( Light( obj ).GetShaders()[ 0 ] ).GetParameters();

					shadow( Light( obj ) );
					shared_ptr< shader > light;

					for( unsigned k = 0; k < ( unsigned )props.GetCount(); k++ ) {
						Property prop( props[ k ] );

						if( isAffogatoProperty( prop ) ) {
							CParameterRefArray params( prop.GetParameters() );

							for( int p = 0; p < params.GetCount(); p++ ) {
								Parameter param( params[ p ] );
								string paramName( CStringToString( param.GetName() ) );

								replace_first( paramName, string( "_" ), string( ":" ) );
								string userParamName( paramName );
								to_lower( paramName );

								if( "category" == paramName ) {
									category = CStringToString( param.GetValue() );
								} else
								if( "light" == paramName ) {
									shaderName = CStringToString( param.GetValue() );
								} else {
									debugMessage( L"found other parameter" );

									CValue value( param.GetValue( g.animation.time ) );
									switch( value.m_t ) {
										case CValue::siBool:
										case CValue::siInt1:
										case CValue::siInt2:
										case CValue::siInt4:
										case CValue::siUInt1:
										case CValue::siUInt2:
										case CValue::siUInt4:
											debugMessage(L"Adding: int " + stringToCString( userParamName ) );
											theRenderer.attribute( userParamName, ( int )( ( long )value ) );
											break;
										case CValue::siFloat:
										case CValue::siDouble:
											debugMessage(L"Adding: float " + stringToCString( userParamName ) );
											theRenderer.attribute( userParamName, ( float )value );
											break;
										case CValue::siString:
										case CValue::siWStr:
											debugMessage(L"Adding: string " + stringToCString( userParamName ) );
											theRenderer.attribute( userParamName, parseString( CStringToString( value ) ) );
											break;
										// Todo: Array support!!!
									}
								}
							}
							//break;
						}

						if( shader::shaderLight == shader::getType( prop ) ) {
							light = shared_ptr< shader >( new shader( prop ) );
						} else {
							debugMessage( L"Found invalid shader." );
						}

					}

					if( light ) {
						light->write( objName );
					} else {

						if( !obj.GetParameterValue( L"SpecularContribution", g.animation.time ) )
							theRenderer.parameter( "__nonspecular", 1.0f );

						if( !obj.GetParameterValue( L"DiffuseContribution", g.animation.time ) )
							theRenderer.parameter( "__nondiffuse" , 1.0f );

						if( !category.empty() )
							theRenderer.parameter( "__category" , category );

						Parameter lightColor = lightShaderParams.GetItem( L"color" );
						float lightcolor[ 3 ] = {	lightColor.GetParameter( L"red"   ).GetValue( g.animation.time ),
													lightColor.GetParameter( L"green" ).GetValue( g.animation.time ),
													lightColor.GetParameter( L"blue"  ).GetValue( g.animation.time ) };
						theRenderer.parameter( tokenValue( lightcolor, 3, "lightcolor", tokenValue::storageUniform, tokenValue::typeColor ) );

						theRenderer.parameter( "intensity", ( float )Parameter( lightShaderParams.GetItem( L"intensity" ) ).GetValue( g.animation.time ) );

						if( lightType != 1 ) {
							switch( ( long )Parameter( lightShaderParams.GetItem( L"atten" ) ).GetValue( g.animation.time ) ) {
								case 0:
									theRenderer.parameter( "exponent", 0.0f );
									break;
								case 1:
									//switch( Parameter( lightShaderParams.GetItem( L"mode" ) ).GetValue( g.animation.time ) )
									theRenderer.parameter( "exponent", ( float )obj.GetParameterValue( L"LightExponent", g.animation.time ) );
									break;
							}
						}

						if( 1 < lightType ) {
							theRenderer.parameter( "coneangle", ( float )( ( float )obj.GetParameterValue( L"LightCone", g.animation.time ) / 2 * PI / 180 ) );
							theRenderer.parameter( "spreadangle", ( float )( ( float )Parameter( lightShaderParams.GetItem( L"spread" ) ).GetValue( g.animation.time ) / 2 * PI / 180 ) );
						}

						theRenderer.light( shaderName, objName );
					}

					//theRenderer.shader( "surface", "defaultsurface" );
					//RiCone( 2, .1, 360, RI_NULL);

					theRenderer.popAttributes();

					theRenderer.switchLight( objName );
				} // Aquired data from XSI gets destructed here as we're leaving scope
			}
		}
		bm.endBlock( ctx );
	}

	void worker::frontAndBackPlane() {
		const globals& g = const_cast< globals& >( globals::access() );

		if( g.data.sections.geometry ) {

			ueberManInterface theRenderer;

			Application app;

			if( g.camera.backPlane ) {
				Model sceneRoot = app.GetActiveSceneRoot();
				CRefArray objects = sceneRoot.FindChildren( L"__backplane", siNullPrimType, CStringArray() );

				if( objects.GetCount() ) {
					debugMessage( L"Found backplane null." );

					if( isVisible( objects[ 0 ] ) ) {

						theRenderer.pushAttributes();

						theRenderer.attribute( "identifier:name", string( "__backplane" ) );
						theRenderer.attribute( "grouping:membership", string( "__backplane" ) );
						//theRenderer.space( cameraSpace );
						theRenderer.space( spaceHandle( "camera" ) );
						theRenderer.translate( 0, 0, ( float )g.camera.farClip );
						// flip normal
						theRenderer.scale( 1,  1, -1 );

						CRefArray props = X3DObject( objects[ 0 ] ).GetProperties();

						for( int i = 0; i < props.GetCount(); i++ ) {
							if( shader::isShader( ( Property )props[ i ] ) ) {
								shader tmpShader( ( Property )props[ i ] );
								tmpShader.write();
							}
						}

						float ywidth = tan( g.camera.fieldOfView * ( float )PI / 360.0f ) * g.camera.farClip;
						float xwidth = ywidth * g.camera.aspect;

						float P[] = {	-xwidth, +ywidth, 0, +xwidth, +ywidth, 0,
										-xwidth, -ywidth, 0, +xwidth, -ywidth, 0 };

						theRenderer.parameter( tokenValue( P, 12, "P", tokenValue::storageVertex, tokenValue::typePoint ) );
						primitiveHandle identifier( "__backplane" );
						theRenderer.patch( "linear", 2, 2, identifier );
						theRenderer.popAttributes();
					}
				}
			}

			if( g.camera.frontPlane ) {
				Model sceneRoot = app.GetActiveSceneRoot();
				CRefArray objects = sceneRoot.FindChildren( L"__frontplane", siNullPrimType, CStringArray() );

				if( objects.GetCount() ) {
					debugMessage( L"Found frontplane null." );

					if( isVisible( objects[ 0 ] ) ) {

						theRenderer.pushAttributes();

						theRenderer.attribute( "identifier:name", string( "__frontplane" ) );
						theRenderer.attribute( "grouping:membership", string( "__frontplane" ) );
						//theRenderer.space( cameraSpace );
						theRenderer.space( spaceHandle( "camera" ) );
						theRenderer.translate( 0, 0, ( float )g.camera.nearClip );
						// flip normal
						theRenderer.scale( 1,  1, -1 );

						CRefArray props = X3DObject( objects[ 0 ] ).GetProperties();

						for( int i = 0; i < props.GetCount(); i++ ) {
							if( shader::isShader( ( Property )props[ i ] ) ) {
								shader tmpShader( ( Property )props[ i ] );
								tmpShader.write();
							}
						}

						float ywidth = tan( g.camera.fieldOfView * ( float )PI / 360.0f ) * g.camera.nearClip;
						float xwidth = ywidth * g.camera.aspect;
						float P[] = {	-xwidth, +ywidth, 0, +xwidth, +ywidth, 0,
										-xwidth, -ywidth, 0, +xwidth, -ywidth, 0 };

						theRenderer.parameter( tokenValue( P, 12, "P", tokenValue::storageVertex, tokenValue::typePoint ) );
						primitiveHandle identifier( "__frontplane" );
						theRenderer.patch( "linear", 2, 2, identifier );
						theRenderer.popAttributes();
					}
				}
			}
		}
	}

	void worker::geometry( const CRefArray &objectList, const string &dest ) {
		ueberManInterface theRenderer;
		const globals& g = const_cast< globals& >( globals::access() );
		blockManager& bm = const_cast< blockManager& >( blockManager::access() );

		debugMessage( L"Geometry" );
		bm.beginBlock( blockManager::blockGeometry, g.data.sections.geometry );
		context ctxGeo = bm.currentContext();

		//message( L"Received " + CValue( objectList.GetCount() ).GetAsText() + L" objects", messageError );

		if( g.data.sections.geometry || g.data.sections.attributes && objectList.GetCount() ) {

			Application app;

			frontAndBackPlane();
			nulls( objectList );

			CRefArray objects;
			CRefArray found;
			objectList.Filter( siPolyMeshType,    CStringArray(), CString(), found );
			objects += found;
			found.Clear();
			objectList.Filter( siSrfMeshPrimType, CStringArray(), CString(), found );
			objects += found;
			found.Clear();
			objectList.Filter( siCrvListPrimType, CStringArray(), CString(), found );
			objects += found;
			found.Clear();
			objectList.Filter( siHairKeyword,     CStringArray(), CString(), found );
			objects += found;
			found.Clear();
			objectList.Filter( siCloudPrimType,   CStringArray(), CString(), found );
			objects += found;
			found.Clear();
			objectList.Filter( siSpherePrimType,  CStringArray(), CString(), found );
			objects += found;

			//message( L"Found " + CValue( objects.GetCount() ).GetAsText() + L" objects", messageError );

			ProgressBar bar;

			/* We have to fuckin (sorry) check if bloody XSI is in interactive
			 * mode or else the progress bar thingy will screw the export up
			 * (in batch mode)
			 */
			bool interactive( app.IsInteractive() );
			if( interactive && ( 1 < objects.GetCount() ) ) {
				UIToolkit kit( app.GetUIToolkit() );
				bar = kit.GetProgressBar();
				bar.PutMaximum( objects.GetCount() );
				bar.PutStep( 1 );
				bar.PutCaption( L"Affogato Geometry" );
				bar.PutVisible( true );
			}

			for( long i = 0; i < objects.GetCount(); i++ ) {

				debugMessage( L"Next object" );

				X3DObject obj( objects[ i ] );

				string objName( CStringToString( X3DObject( obj ).GetFullName() ) );

				if( interactive )
					bar.PutStatusText( L"Object '" + stringToCString( objName ) + L"'" );

				debugMessage( L"Testing visibiliy" );
				if( isVisible( obj ) ) {

					debugMessage( L"Constructing node for " + stringToCString( objName ) );
					node object( obj );

					// Whether we're in Sub-Section granularity mode and a transforms should be stored in the world block
					bool transforms = g.data.subSectionParentTransforms && ( globals::data::granularityObjects >= g.data.granularity );
					if( transforms ) {
						theRenderer.pushSpace();
						object.writeTransform();
					}

					if( g.data.sections.geometry ) {
						bm.beginBlock( blockManager::blockObject, g.data.sections.geometry, objName, !obj.IsAnimated(), object.getBoundingBox() );
						context ctxObj = bm.currentContext();

						//theRenderer.pushAttributes();

						//if( !transforms )
							theRenderer.pushAttributes();

						if( !transforms )
							object.writeTransform();

						bm.beginBlock( blockManager::blockAttributes, g.data.sections.attributes, objName );
						context ctxAttr = bm.currentContext();
						object.writeAttributes();
						bm.endBlock( ctxAttr );

						object.writeGeometry();

						//if( !transforms )
							theRenderer.popAttributes();

						bm.endBlock( ctxObj );

					} else if( g.data.sections.attributes ) {
						bm.beginBlock( blockManager::blockAttributes, g.data.sections.attributes, objName );
						context ctxAttr = bm.currentContext();
						object.writeAttributes();
						bm.endBlock( ctxAttr );
					}

					theRenderer.switchScene( ctxGeo );

					if( transforms ) {
						theRenderer.popSpace();
					}

					debugMessage( L"Done with object" );
				}

				if( interactive ) {
					if( bar.IsCancelPressed() )
						break;
					bar.Increment();
				}
			}

			if( interactive )
				bar.PutVisible( false );
		}

		bm.endBlock( ctxGeo );
	}


	void worker::archive( const CRefArray &objectList, const string &destination ) {

		Application app;

		ueberManInterface theRenderer;

#ifndef DEBUG
		theRenderer.registerRenderer( ueberManRiRenderer::accessRenderer() );
		//theRenderer.registerRenderer( ueberManXmlRenderer::accessRenderer() );
#endif

		globals& g( const_cast< globals& >( globals::access() ) );

		g.animation.time = g.animation.times[ 0 ];
		g.data.shadow.shadow = false;
		g.data.granularity = globals::data::granularitySubFrame;

		blockManager& bm = const_cast< blockManager& >( blockManager::access() ); // Real instance
		bm.jobPtrStack.push_back( shared_ptr< job >( new job() ) );

		context ctx( theRenderer.beginScene( parseString( destination ), g.data.binary, g.data.compress ) );

		timeStats masterHora;

		spaces( objectList );

		masterHora.takeTime( "Spaces" );

		lights( objectList );

		masterHora.takeTime( "Lights" );

		geometry( objectList );

		masterHora.takeTime( "Objects" );

		theRenderer.endScene( ctx );

		if( g.feedback.stopWatch )
			masterHora.printTimes();

#ifndef DEBUG
		theRenderer.unregisterRenderer( ueberManRiRenderer::accessRenderer() );
		//theRenderer.unregisterRenderer( ueberManXmlRenderer::accessRenderer() );
#endif
	}

	void worker::work( const string &globalsString, const CRefArray &objects, const string& destination ) {

		static bool notDone( true );
		if( notDone ) { // Have to do this or else filesystem throws exceptions like crazy
#ifdef _WIN32
			filesystem::path::default_name_check( filesystem::native );
#else
			filesystem::path::default_name_check( filesystem::windows_name );
#endif
			notDone = false;
		}

		Application app;

		CRefArray objectList( objects );

		const_cast< globals& >( globals::access() ).passPtrArray.clear();

		try {
			if( globalsString.empty() ) {

				Model sceneRoot = app.GetActiveSceneRoot();

				Property affogatoGlobals;

				bool noGlobals( true );
				CRefArray props( sceneRoot.GetProperties() );
				for( long k = 0; k < props.GetCount(); k++ ) {
					Property prop( props[ k ] );
					if( CString( L"AffogatoGlobals" ) == prop.GetType() ) {
						affogatoGlobals = prop;
						noGlobals = false;
						break;
					}
				}

				if( noGlobals )
					affogatoGlobals = sceneRoot.AddProperty( L"AffogatoGlobals" );
				else
					affogatoGlobals = updateGlobals( affogatoGlobals );

				debugMessage( L"Setting globals" );

				const_cast< globals& >( globals::access() ).set( affogatoGlobals );

				debugMessage( L"Done setting globals" );

				if( destination.empty() )
					scene( objects );
				else
					archive( objects, destination );
			} else {

				string trimmedName( trim_copy( globalsString ) );
				string ext( trimmedName.substr( globalsString.length() - 4, globalsString.length() - 1 ) );
				to_lower( ext );

				if( ".xml" == ext ) {

					Model sceneRoot = app.GetActiveSceneRoot();

					Property affogatoGlobals;

					bool noGlobals( true );
					CRefArray props( sceneRoot.GetProperties() );
					for( long k = 0; k < props.GetCount(); k++ ) {
						Property prop( props[ k ] );
						if( CString( L"AffogatoGlobals" ) == prop.GetType() ) {
							affogatoGlobals = prop;
							noGlobals = false;
							break;
						}
					}

					if( noGlobals )
						affogatoGlobals = sceneRoot.AddProperty( L"AffogatoGlobals" );
					else
						affogatoGlobals = updateGlobals( affogatoGlobals );

					fstream test( trimmedName.c_str() );
					if( !test.is_open() ) {
						message( L"Affogato: File '" + stringToCString( trimmedName ) + L"' not found. Exiting.", messageError );
						return;
					}

					globals& g = const_cast< globals& >( globals::access() );
					g.aquire( affogatoGlobals );
					g.set( trimmedName ); // Overwrite whatever we found with the stuff from the XML file
					if( g.directories.base.empty() )
						throw( "I refuse to do any work without a base path." );

					if( destination.empty() )
						scene( objects );
					else
						archive( objects, destination );
				}
				else { // Search for given globals property in scene
					//Model sceneRoot = app.GetActiveSceneRoot();

					CRef obj;

					if( CStatus::OK == obj.Set( stringToCString( trimmedName ) ) ) {

						Property affogatoGlobals( obj );

						affogatoGlobals = updateGlobals( affogatoGlobals );

						globals& g = const_cast< globals& >( globals::access() );
						g.set( affogatoGlobals );
						if( destination.empty() )
							scene( objects );
						else
							archive( objects, destination );
					} else
						message( L"Could not find Affogato Globals property '" + stringToCString( trimmedName ) + L"'. Exiting.", messageError );
				}
			}
		}
		catch( runtime_error& err ) {
			message( stringToCString( err.what() ), messageError );
			message( L"Aborting", messageError );
		}
	}

	void worker::scene( const CRefArray &objectList ) {
		try {
			Application app;

			ueberManInterface theRenderer;

#ifdef DEBUG
//			shared_ptr< ueberManDummyRenderer > dummy = shared_ptr< ueberManDummyRenderer >( new ueberManDummyRenderer );
//			theRenderer.registerRenderer( dummy->accessRenderer() );
			theRenderer.registerRenderer( ueberManRiRenderer::accessRenderer() );
#else
			//shared_ptr< ueberManRiRenderer > delight = shared_ptr< ueberManRiRenderer >( new ueberManRiRenderer );
			theRenderer.registerRenderer( ueberManRiRenderer::accessRenderer() );
#endif

			//theRenderer.registerRenderer( ueberManXmlRenderer::accessRenderer() );
			//theRenderer.registerRenderer( ueberManGelatoRenderer::accessRenderer() );

			globals& g( const_cast< globals& >( globals::access() ) );

			blockManager& bm( const_cast< blockManager& >( blockManager::access() ) ); // Real instance
			bm.reset();
			//bm.tasks.setTitle( g.name.baseName );

			bm.jobPtrStack.push_back( shared_ptr< job >( new job() ) );

			Model sceneRoot = app.GetActiveSceneRoot();
			CRefArray allSpaces( sceneRoot.FindChildren( CString(), siNullPrimType,  CStringArray() ) );
			CRefArray allLights( sceneRoot.FindChildren( CString(), siLightPrimType, CStringArray() ) );

			timeStats masterHora;

			ProgressBar bar;

			/* We have to fuckin (sorry) check if bloody XSI is in interactive
			 * mode or else the progress bar thingy will screw the export up
			 * (in batch mode)
			 */
			bool interactive( app.IsInteractive() );
			if( interactive && ( 1 < g.animation.times.size() ) ) {
				UIToolkit kit( app.GetUIToolkit() );
				bar = kit.GetProgressBar();
				bar.PutMaximum( g.animation.times.size() );
				bar.PutStep( 1 );
				bar.PutCaption( L"Affogato Render Output" );
				bar.PutVisible( true );
			}

			tidyUpCache();

			int frameCounter = 0, chunkNo = 0;
			do {
				message( L"Rendering Frame " + stringToCString( g.name.currentFrame ), messageInfo );
				masterHora.reset();

				if( interactive )
					bar.PutStatusText( L"Frame " + stringToCString( g.name.currentFrame ) );

				//g.name.currentFrame = ( format( "%04d" ) % g.animation.time ).str();

				if( g.data.directToRenderer && !g.jobGlobal.preFrameCommand.empty() ) {
					string cmd( parseString( g.jobGlobal.preFrameCommand ) );
					size_t splitPos = cmd.find( ' ' );
					execute( cmd.substr( 0, splitPos ), cmd.substr( splitPos )
#ifdef unix
						+ " 2>/var/tmp/affogatoout.log"
#endif
						, g.directories.base.native_file_string(), true );
				}

				bm.beginBlock( blockManager::blockScene, g.data.granularity > globals::data::granularitySubFrame ? g.data.frame : true );
				context ctxScene = bm.currentContext();
				{
					Model sceneRoot = app.GetActiveSceneRoot();

					message( L"Working...", messageInfo );

					masterHora.takeTime( "Pre Cmd" );
					options();
					masterHora.takeTime( "Options" );
					camera();
					masterHora.takeTime( "Camera" );

					debugMessage( L"World" );
					g.data.worldBlockName = worldBlockName = bm.beginBlock( blockManager::blockWorld, true );
					context ctxWorld = bm.currentContext();
					{
						theRenderer.world();

						spaces( allSpaces );
						masterHora.takeTime( "Spaces" );

						globalAttributes();
						masterHora.takeTime( "Glob. Attr." );

						lights( allLights );
						masterHora.takeTime( "Lights" );

						// Setup looks stuff
						bm.beginBlock( blockManager::blockLooks, g.data.sections.looks );
						context lookContext = bm.currentContext();
						debugMessage( L"Current Render Context is " + CValue( bm.renderContext( lookContext ) ).GetAsText () );
						node::setLookContext( bm.renderContext( lookContext ) );
						bm.switchBlock( ctxWorld );
						masterHora.takeTime( "Look Init" );

						geometry( objectList );
						masterHora.takeTime( "Geometry" );

						bm.endBlock( lookContext ); // looks
					}
					debugMessage( L"WORLD");
					bm.endBlock( ctxWorld ); // world
				}
				debugMessage( L"SCENE");
				bm.endBlock( ctxScene ); // scene


				cameraHandle tmpCam;
				theRenderer.render( tmpCam ); // cleanup

				masterHora.takeTime( "Scene" );

				if( g.data.directToRenderer && !g.jobGlobal.postFrameCommand.empty() ) {
					string cmd( parseString( g.jobGlobal.postFrameCommand ) );
					size_t splitPos = cmd.find( ' ' );
					execute( cmd.substr( 0, splitPos ), cmd.substr( splitPos )
#ifdef unix
						+ " 2>/var/tmp/affogatoout.log"
#endif
						, g.directories.base.native_file_string(), true );
				}

				masterHora.takeTime( "Post Cmd" );

				if( interactive ) {
					if( bar.IsCancelPressed() )
						break;
					bar.Increment();
				}

				bm.processJobChunk( frameCounter, chunkNo );

				masterHora.takeTime( "Jobs" );

				if( g.feedback.stopWatch )
					masterHora.printTimes();

				tidyUpCache();

				frameCounter = ( int )floor( g.getNormalizedTime() * g.animation.times.size() );
			} while( g.nextTime() );

			if( interactive )
				bar.PutVisible( false );

			masterHora.takeTime( "Scene" );

			bm.processJob( chunkNo );

			debugMessage( L"All done");

#ifdef DEBUG
//			theRenderer.unregisterRenderer( dummy->accessRenderer() );
			theRenderer.unregisterRenderer( ueberManRiRenderer::accessRenderer() );
#else
			theRenderer.unregisterRenderer( ueberManRiRenderer::accessRenderer() );
#endif
			debugMessage( L"Done" );

			bm.reset();
		}
		catch( runtime_error& err ) {
			message( stringToCString( err.what() ), messageError );
			message( L"Aborting", messageError );
		}

		debugMessage( L"Really Done" );
	}
}
