#ifndef affogatoExecute_H
#define affogatoExecute_H

#include <string>

namespace affogato {
	using namespace std;
	bool execute( const string &command, const string &arguments, const string &path, const bool wait = true );
}
#endif
