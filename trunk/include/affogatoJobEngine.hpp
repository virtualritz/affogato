#ifndef affogatoJobEngine_H
#define affogatoJobEngine_H
/** RSP JobEngine XML job dependency format interface class.
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
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>

// Boost headers
#include <boost/shared_ptr.hpp>

// Affogato headers
#include "affogatoIndentHelper.hpp"


namespace affogato {

	using namespace std;

	string condenseFrameSequence( const vector< int >& frames );

	class parameter {
		friend class task;
		friend class superTask;
		friend class taskList;

		public:
			parameter( const string& aParam, const string& aType = string() ) {
				type	= aType;
				param	= aParam;
			}
			parameter( const parameter& cpy ) {
				type	= cpy.type;
				param	= cpy.param;
			}

			typedef list< parameter > array;
		private:
			string type;
			string param;
	};


	class task {
		friend class taskList;

		public:
								task() {};
								task( const task& cpyTask );
			task&				operator=( const task& cpyTask );
								task( const string& theClass, const string &theTitle );
			void				addParameter( const string& parameter, const string& paramType = string() );
			void				addFrame( const int frameNumber );
			void				setTitle( const string& aTitle );
			void				setClass( const string& aClass);
			string				getJobEngineXML( indentHelper indent ) const;
			bool				empty() const;
			void				clear();

		private:
			string				title;
			string				className;
			vector< int >		frames;
			set< int >			existFrames; // We just maintain this to quickly find if a frame exists
			parameter::array	parameters;
	};

	class taskList {
		friend class taskManager;

		public:

			typedef enum {
				jobTypeNone,
				jobTypeJobEngineXML,
				jobTypeXML,
				jobTypeAlfred
			}					jobFormatType;

			typedef enum {
				typeTaskList,
				typeSuperTask
			}					taskListType;

								taskList();
								taskList( const string &theTitle, const taskListType theType = typeTaskList );
								taskList( const taskList& cpy );
			taskList&			operator=( const taskList& cpy );
			void				clear();
			void				setTitle( const string &theTitle );
			void				setType( const taskListType theType );
			void				addFrame( const int frameNumber );
			void				addTask( const task& aTask );
			void				addTaskList( const taskList& aTask );
			//void				deleteTask( const string &theTitle );
			string				write( const string& destination, jobFormatType jobType ) const;
			void				merge( const taskList& mergeList );


			friend string		writeJobEngineXML( const string& destination, const taskList& aList );//, const ioManager& io );

		private:
			taskListType		type;
			string				getJobEngineXML( indentHelper indent ) const;
			string				getJobScriptXML( indentHelper indent ) const;
			typedef				boost::shared_ptr< taskList > taskListPtr;
			taskListPtr			subTaskList;
			string				title;
			typedef list< task > orderedTaskList; // we use a list to make sure iterators are not invalidated when changing the list and the order of tasks is kept strictly
			orderedTaskList		orderedTasks;
			typedef map< string, orderedTaskList::iterator > taskMap;
			taskMap				tasks;
			//typedef map< string, taskList > taskListMap;
			typedef list< taskList > taskListList;
			taskListList		taskLists;
			vector< int >		frames;
			set< int >			existFrames; // We just maintain this to quickly find if a frame exists
	};

	string writeJobEngineXML( const string& destination, const taskList& aList );

	/** Manages sorting tasks and taskLists into a hierarchy depending on a priority.
	 *
	 */
	class taskManager {
		public:
			typedef unsigned priority;
								taskManager();
								taskManager( const taskManager& cpy );
			taskManager&		operator=( const taskManager& cpy );
			void				clear();
			void				addTask( const priority self, const task& aTask ); //, const priority waitFor
			void				addTaskList( const priority self, const taskList& aTaskList ); // , const priority waitFor
			taskList			getTaskHierarchy();

		private:
			typedef map< priority, taskList > taskListPriorityMap;
			typedef map< priority, taskListPriorityMap::iterator> taskListWaitForPriorityMap;
			taskListPriorityMap taskListMap;
			taskListWaitForPriorityMap taskListWaitForMap;
			taskList			buildTaskHierarchy( taskListPriorityMap::iterator it );
	};

}

#endif
