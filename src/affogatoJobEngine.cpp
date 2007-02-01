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
#include <sstream>

// Affogato headers
#include "affogatoHelpers.hpp"
#include "affogatoJobEngine.hpp"


namespace affogato {

	using namespace std;

	// task class
	task::task( const task& copyTask ) {
		className = copyTask.className;
		title = copyTask.title;
		existFrames = copyTask.existFrames;
		frames = copyTask.frames;
		parameters = copyTask.parameters;
	}

	task::task( const string& theClass, const string &theTitle ) {
		className = theClass;
		title = theTitle;
		//existFrames.insert( frameNumber );
		//frames.push_back( frameNumber );
		//parameters = params;
	}

	void task::addParameter( const string& param, const string& paramType ) {
		parameters.push_back( parameter( param, paramType ) );
	}

	void task::addFrame( const int frameNumber ) {
		if( existFrames.end() == existFrames.find( frameNumber ) ) {
			//message( L"Now adding frame " + CValue( (long)frameNumber ).GetAsText(), messageError );
			existFrames.insert( frameNumber );
			frames.push_back( frameNumber );
			//message( L"Done", messageError );
		} /*else {
			message( L"Frame already exists: " + CValue( (long)frameNumber ).GetAsText(), messageError );
		}*/
	}

	void task::setTitle( const string& aTitle ) {
		title = aTitle;
	}

	void task::setClass( const string& aClass) {
		className = aClass;
	}

	bool task::empty() const {
		return className.empty() || parameters.empty();
	}

	void task::clear() {
		className.clear();
		title.clear();
		existFrames.clear();
		frames.clear();
		parameters.clear();
	}

	string condenseFrameSequence( const vector< int >& frames ) {
		// Thanks to Buddy Junio for providing the
		// template of this function as Python source! :)

		if( frames.empty() )
			return string();

		size_t count = frames.size();

		int start = frames[ 0 ];
		string result = toString( start );
		int currentstep = 0;
		int stepcount = 1;

		for( unsigned index = 1; index < count; index++ ) {
			// get the step
			int difference = frames[ index ] - frames[ index - 1 ];
			if( !currentstep ) {
				currentstep = difference;
				stepcount = 2;
			// check if we need a new step
			} else if( difference != currentstep ) {
				// there are at least 3 entries in this range
				if( 2 < stepcount ) {
					// at least 3 entries have the same step
					// register the previous number as the end of the previous range
					result += "-" + toString( frames[ index - 1 ] );
					// only record steps that are more than 1
					if( 1 < abs( currentstep ) ) {
						result += "@" + toString( abs( currentstep ) );
					}
					// begin a new range
					start = frames[ index ];
					// register the current number as the start of the new range
					result += "," + toString( frames[ index ] );
					currentstep = 0;
					stepcount = 1;
				} else {
					// there are 2 entries in this range
					// just list the start candidate of the range as a single entry
					// and the end candidate the new range start candidate
					result += "," + toString( frames[ index - 1 ] );
					start = frames[ index - 1 ];
					currentstep = difference;
					stepcount = 2;
				}
			} else {
				// this value conforms with the step, increment with the step count
				stepcount++;
			}
		}

		// terminate the list
		int last = frames[ count - 1 ];
		if( start != last ) {
			if( 2 < stepcount ) {
				result += "-" + toString( last );
				if( 1 < currentstep ) {
					result += "@" + toString( currentstep );
				}
			} else {
				result += "," + toString( last );
			}
		}

		return result;
	}

	string task::getJobEngineXML( indentHelper indent ) const {
		stringstream ss;

		if( !empty() ) {

			ss << indent++ << "<task class=\"" << className;
			if( !title.empty() )
				ss << "\" name=\"" << title;
			if( !frames.empty() )
				ss << "\" frames=\"" << condenseFrameSequence( frames );

			ss << "\">" << endl;

			for( list< parameter >::const_iterator it = parameters.begin(); it != parameters.end(); it++ ) {
				ss << indent << "<param";
				if( !it->type.empty() )
					ss << " type=\"" << it->type << "\"";
				ss << ">" << it->param << "</param>" << endl;
			}

			ss << --indent << "</task>" << endl;
		}

		return ss.str();
	}




	// taskList class
	taskList::taskList() {
		type = typeTaskList;
	}


	taskList::taskList( const string &theTitle, const taskListType theType ) {
		title = theTitle;
		type = theType;
	}

	taskList::taskList( const taskList& copy ) {
		title = copy.title;
		type = copy.type;

		//message( L"Copying task list", messageError );

		// Copy the list of orderedTasks and create iterators in the tasks map
		orderedTasks.clear();
		tasks.clear();
		for( orderedTaskList::const_iterator it( copy.orderedTasks.begin() ); it != copy.orderedTasks.end(); it++ ) {
			orderedTasks.push_back( *it );
			tasks[ it->title ] = --orderedTasks.end();
		}
		taskLists = copy.taskLists;
		frames = copy.frames;
		existFrames = copy.existFrames;

		//message( stringToCString( getJobEngineXML( indentHelper() ) ), messageError );


	}

	taskList& taskList::operator=( const taskList& copy ) {
		this->title = copy.title;
		this->type = copy.type;

		// Copy the list of orderedTasks and create iterators in the tasks map
		this->orderedTasks.clear();
		this->tasks.clear();
		for( taskMap::const_iterator it( copy.tasks.begin() ); it != copy.tasks.end(); it++ ) {
			this->orderedTasks.push_back( *( it->second ) );
			this->tasks[ it->first ] = --orderedTasks.end();
		}
		this->taskLists = copy.taskLists;
		this->frames = copy.frames;
		this->existFrames = copy.existFrames;

		return *this;
	}

	void taskList::clear() {
		title.clear();
		orderedTasks.clear();
		tasks.clear();
		taskLists.clear();
		frames.clear();
		existFrames.clear();
	}

	void taskList::setTitle( const string &theTitle ) {
		title = theTitle;
	}

	void taskList::setType( const taskListType theType ) {
		type = theType;
	}

	void taskList::addFrame( const int frameNumber ) {
		if( existFrames.end() == existFrames.find( frameNumber ) ) {
			//message( L"Now adding frame " + CValue( (long)frameNumber ).GetAsText(), messageError );
			existFrames.insert( frameNumber );
			frames.push_back( frameNumber );
			//message( L"Done", messageError );
		} /*else {
			message( L"Frame already exists: " + CValue( (long)frameNumber ).GetAsText(), messageError );
		}*/
	}

	void taskList::addTask( const task& aTask ) {

		if( !aTask.empty() ) {
			//message( L"Adding task", messageError );
			string hash( aTask.title + aTask.className );
			taskMap::const_iterator tasker( tasks.find( hash ) );
			if( tasks.end() == tasker ) {
				//message( L"Really adding task", messageError );
				orderedTasks.push_back( aTask );
				tasks[ hash ] = --orderedTasks.end();
				tasker = tasks.find( hash );
			}

			if( tasks.end() != tasker ) {
				for( vector< int >::const_iterator it = aTask.frames.begin(); it < aTask.frames.end(); it++ ) {
					//message( L"Adding Frame " + CValue( (long) *it ).GetAsText(), messageError );
					( tasker->second )->addFrame( *it );
					//( L"Added Frame", messageError );
				}
			}
		}
/*
		for( vector< int >::const_iterator it = aTask.frames.begin(); it < aTask.frames.end(); it++ )
			tasks[ hash ].addFrame( *it );*/
	}

	void taskList::addTaskList( const taskList& aTaskList ) {
		taskLists.push_back( aTaskList );
	}

	string taskList::getJobEngineXML( indentHelper indent ) const {
		stringstream ss;

		//message( L"Printing job file", messageError );

		if( !tasks.empty() || !taskLists.empty() ) {
			ss << indent++;

			switch( type ) {
				case typeTaskList:
					ss << "<tasklist ";
					break;
				case typeSuperTask:
					ss << "<supertask ";
			}

			ss << "class=\"render";
			if( !title.empty() )
				ss << "\" name=\"" << title;
			ss << "\">" << endl;


			for( orderedTaskList::const_iterator it = orderedTasks.begin(); it != orderedTasks.end(); it++ ) {
				ss << it->getJobEngineXML( indent );
			}

			for( taskListList::const_iterator it = taskLists.begin(); it != taskLists.end(); it++ ) {
				ss << it->getJobEngineXML( indent );
			}

			ss << --indent;

			switch( type ) {
				case typeTaskList:
					ss << "</tasklist>";
					break;
				case typeSuperTask:
					ss << "</supertask>";
			}

			ss << endl;
		}

		return ss.str();
	}

	string taskList::getJobScriptXML( indentHelper indent ) const {
		stringstream ss;

		if( subTaskList || !tasks.empty() ) {
			ss << indent++ << "<subtasks>" << endl;

			ss << --indent << "</subtasks>" << endl;
		}

		return ss.str();
	}

	string taskList::write( const string& destination, jobFormatType jobType ) const {

		string name( destination );

		switch( jobType ) {
			case jobTypeNone: { // Do nothing
				break;
			}
			case jobTypeJobEngineXML: { // new jobEngine format

				name += ".xje";

				ofstream outXML( ( name ).c_str() );
				indentHelper indent;

				outXML << "<?xml version=\"1.0\"?>" << endl;
				outXML << indent++ << "<jobEngine version=\"0.1\">" << endl;

				outXML << getJobEngineXML( indent );

				outXML << --indent << "</jobEngine>" << endl;

				break;
			}
			case jobTypeXML: { // Liquid XML job format

				name += ".xml";

				ofstream outXML( ( name ).c_str() );
				indentHelper indent;

				outXML << "<?xml version=\"1.0\"?>" << endl;
				outXML << indent++ << "<jobScript version=\"0.1\">" << endl;

				//outXML << writeJobScriptXML( indent );

				outXML << --indent << "</jobScript>" << endl;
			}
			case jobTypeAlfred: { // Pixar Alfred format -- todo
				break;
			}
		}

		return name;
	}

	void taskList::merge( const taskList& merge ) {

	}

	string writeJobEngineXML( const string& destination, const taskList& aList ) {

		string name( destination );

		name += ".xje";

		ofstream outXML( ( name ).c_str() );
		indentHelper indent;

		outXML << "<?xml version=\"1.0\"?>" << endl;

		outXML << indent++ << "<jobEngine version=\"0.1\">" << endl;
		outXML << aList.getJobEngineXML( indent );

		outXML << --indent << "</jobEngine>" << endl;

		return name;
	}

	taskManager::taskManager() {
	}

	/** \brief  Builds a hierarchy of tasks.
	 *
	 *  \param  it  a taskListPriorityMap iterator
	 *  \return A taskList containing a full hierearchy of priority-sorted tasks found in taskListMap
	 */
	taskList taskManager::buildTaskHierarchy( taskListPriorityMap::iterator it ) {
		taskList t;
		if( taskListMap.end() != it ) {
			t = it->second;
			++it;
			t.addTaskList( buildTaskHierarchy( it ) );
			//message( L"Back!", messageError );
		}
		//message( L"Bang!", messageError );
		//message( stringToCString( t.getJobEngineXML( indentHelper() ) ), messageError );
		return t;
	}

	/** \brief  Calls buildTaskHierarchy() to build a hierarchy of tasks.
	 *
	 *  \return A taskList containing a full hierearchy of priority-sorted tasks found in taskListMap
	 */
	taskList taskManager::getTaskHierarchy() {
		taskListPriorityMap::iterator it( taskListMap.begin() );
		return buildTaskHierarchy( it );
	}

	void taskManager::addTask( const priority p, const task& aTask ) {
		//message( L"Adding task to taskManager", messageError );
		taskListMap[ p ].addTask( aTask );
	}

	void taskManager::addTaskList( const priority p, const taskList& aTaskList ) {
		//message( L"Adding taskList to taskManager", messageError );
		message( stringToCString( aTaskList.getJobEngineXML( indentHelper() ) ), messageError );
		//message( L"Now adding", messageError );
		taskListMap[ p ].addTaskList( aTaskList );
		message( stringToCString( taskListMap[ p ].getJobEngineXML( indentHelper() ) ), messageError );
	}

	void taskManager::clear() {
		taskListMap.clear();
	}

}
