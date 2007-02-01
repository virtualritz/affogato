#ifndef affogatoIndentHelper_H
#define affogatoIndentHelper_H

#include <fstream>
#include <iostream>
#include <string>


namespace affogato {

	using namespace std;

	class indentHelper {
		public:
			indentHelper() {
				atomStr = "\t";
				counter = 0;
			}

			indentHelper( unsigned size ) {
				for( unsigned i = 0; i < size; i++ )
					atomStr += " ";
				counter = 0;
			}

			indentHelper( const indentHelper& cpy ) {
				counter = cpy.counter;
				fillStr = cpy.fillStr;
				atomStr = cpy.atomStr;
			}

			indentHelper& operator++() {
				++counter;
				fillStr += atomStr;
				return *this;
			}

			indentHelper operator++( int ) {
				indentHelper tmp( *this );
				++( *this );
				return tmp;
			}

			indentHelper& operator--() {
				if( 0 < counter )
					--counter;
				fillStr = fillStr.substr( 0, fillStr.length() - atomStr.length() );
				return *this;
			}

			indentHelper operator--( int ) {
				indentHelper tmp = *this;
				--( *this );
				return tmp;
			}

			indentHelper operator+( int add ) {
				indentHelper added = *this;
				if( 0 < add ) {
					added.counter += add;
					for( int i = 0; i < add; i++ )
						added.fillStr += atomStr;
				}
				return added;
			}

			friend ostream& operator <<( ostream &outs, const indentHelper &output ) {
				return outs << output.fillStr;
			}

			string get() {
				return fillStr;
			}

			unsigned getDepth() {
				return counter;
			}

		private:
			string atomStr, fillStr;
			unsigned counter;
	};
}

#endif
