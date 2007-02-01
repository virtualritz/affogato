# * 
# ****************************************************************************
# * <P> XML.c - implementation file for basic XML parser written in ANSI C++ 
# * for portability.
# * It works by using recursion and a node tree for breaking down the elements
# * of an XML document.  </P>
# *
# * @version     V1.01
# *
# * @author      Frank Vanden Berghen
# * based on original implementation by Martyn C Brown
# *
# * This library is free software; you can redistribute it and/or
# * modify it under the terms of the GNU Lesser General Public
# * License version 2.1 as published by the Free Software Foundation
# * 
# * This library is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# * Lesser General Public License for more details.
# * 
# * You should have received a copy of the GNU Lesser General Public
# * License along with this library; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# * 
# ****************************************************************************
#
#makefile for the xmlParser library
#
all : xmlTest

xmlTest : xmlParser.cpp xmlParser.h xmlTest.cpp
	g++ -g -Wall -o xmlTest xmlParser.cpp xmlTest.cpp

clean:
	rm xmlTest
	rm test.xml
	rm *~
