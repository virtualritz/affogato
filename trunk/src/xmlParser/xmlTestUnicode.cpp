/**
 ****************************************************************************
 * <P> XML.c - XML parser test example for unicode windows version </P>
 * @version     V1.09
 *
 * @author      Frank Vanden Berghen
 * 
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 ****************************************************************************
 */

#include <stdio.h>
#include "xmlParser.h"

int main(int argc, char **argv)
{
  // this open and parse the XML file:  
  XMLNode xMainNode=XMLNode::openFileHelper("PMMLModelUnicode.xml",_T("PMML"));

  // this prints "RANK For <you>":
  XMLNode xNode=xMainNode.getChildNode(_T("Header"));  
  printf("Application Name is: '%S'\n", xNode.getChildNode(_T("Application")).getAttribute(_T("name")));

  // this prints "Hello World!"
  printf("Text inside Header tag is :'%S'\n", xNode.getText());
  
  // this gets the number of "NumericPredictor" tags:  
  xNode=xMainNode.getChildNode(_T("RegressionModel")).getChildNode(_T("RegressionTable")); 
  int n=xNode.nChildNode(_T("NumericPredictor"));

  // this prints the "coefficient" value for all the "NumericPredictor" tags:
  int iterator=0;
  for (int i=0; i<n; i++)
    printf("coeff %i=%S\n",i+1,xNode.getChildNode(_T("NumericPredictor"),&iterator).getAttribute(_T("coefficient")));

  // this create a file named "testUnicode.xml" based on the content of the first "Extension" tag of the XML file:  
  TCHAR *t=xMainNode.getChildNode(_T("Extension")).createXMLString(true,&i); 
  FILE *f=fopen("testUnicode.xml","wb");
  _fputts(_T("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"),f);
  fwrite(t,sizeof(TCHAR),i,f);
  printf("%S",t);
  fclose(f);
  free(t);

  // compare these 4 lines ...
  t=stringDup(xMainNode.getAttribute(_T("version")));       // get version number
  xMainNode=XMLNode::emptyXMLNode;                          // free from memory the top of the xml Tree
  printf("PMML Version :%S\n",t);                           // print version number
  free(t);                                                  // free version number

  // ... with the following 3 lines (currently commented, because of error):
  //  t=xMainNode.getAttribute(_T("version"));      // get version number (note that there is no 'stringDup')
  //  xMainNode=XMLNode::emptyXMLNode;              // free from memory the top of the xml Tree AND the version number inside 't' var
  //  printf("PMML Version :%S\n",t);               // since the version number in 't' has been free'd this will not work

  // We create in memory from scratch the following XML structure:
  //  <?xml version="1.0"?>
  //      <body> Hello "universe". </body>
  // ... and we transform it into a standard C string that is printed on screen.
  xNode=XMLNode::createXMLTopNode();
  XMLNode xn=xNode.addChild(stringDup(_T("xml")),TRUE); xn.addAttribute(stringDup(_T("version")),stringDup(_T("1.0")));
  xn=xn.addChild(stringDup(_T("body")));  xn.addText(stringDup(_T("Hello \"universe\"!")));
  t=xNode.createXMLString(true);
  printf("XMLString created from scratch:\n%S",t);
  free(t);

  return 0;
}
