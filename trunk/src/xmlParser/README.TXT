XMLParser v1.09
===============

The library is composed by two files: xmlParser.cpp and xmlParser.h.
These are the only 2 files that you need when using the library inside your own projects.

To use the unicode windows version, you need to define the "_UNICODE" preprocessor
definition inside your project definition file.

Some small test examples are also given: see files xmlTest.cpp, xmlTestUnicode.cpp,
PMMLModel.xml, PMMLModeUnicode.xml. The examples are fully described inside the 
html file xmlParser.html.

To build the examples:
- unix: type "make"
- windows: 
  * Visual Studio 6.0 : double-click on xmlParser.dsw
  * Visual Studio .NET: double-click on xmlParser.sln

In order to build the examples you need some project files:
- unix: makefile
- windows: 
  * Visual Studio 6.0 : xmlParser.dsp,    xmlParserUnicode.dsp,    xmlParser.dsw
  * Visual Studio .NET: xmlParser.vcproj, xmlParserUnicode.vcproj, xmlParser.sln

Change Log
----------

* V1.00: February 20, 2002: initial version.
* V1.01: February 13, 2005: first bug-free release.
* V1.02: March 6, 2005: 2 minor changes:
   o "parseString" function declaration changed to allow easy parsing from memory buffer
   o Minor changes to allow easy compilation under old GCC under QNX
* V1.03: April 2,2005: 3 minors changes:
   o When parsing from a user-supplied memory buffer, the library was previously modifying the content of the memory buffer. This is not the case anymore.
   o Non-unicode Windows version: You can now work with unicode XML files: They are converted to ANSI charset before being processed.
   o Added Visual Studio 6.0 project files
* V1.04: May 16, 2005: 3 minors changes, 1 bug fix:
   o FIX: When creating an xml string with no formatting, the formatting did not work always (due to an un-initialized variable).
   o Improved parsing speed (try increasing the constant "memoryIncrease" if you need more speed)
   o Minor changes to allow easy compilation under MSYS/MINGW under Windows
   o Added more escape sequences.
* V1.05: May 31, 2005: 2 minors changes:
   o Changed some "char *" to "const char *"
   o Improved robustness against badly formed xml strings
* V1.06: July 11, 2005: 1 change, 1 FIX:
   o FIX: Some escape sequences were not previously correctly processed.
   o Major speed improvement. The library is now at least 10 times faster. (Try increasing the constant "memoryIncrease" if you need more speed)
   o moved the log file out of the XML file
* V1.07: July 25, 2005: 1 change
   o Added a pre-compiler directive named "APPROXIMATE_PARSING". See header of xmlParser.cpp for more info.
* V1.08: September 8,2005: 1 bug fix: 
   o FIX: on special cases, non-matching quotes were causing malfunction
* V1.09: November 22, 2005:
   o Added some new functions to be able to easily create a XML structure in memory


