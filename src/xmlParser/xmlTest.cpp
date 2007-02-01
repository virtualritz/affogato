/**
 ****************************************************************************
 * <P> XML.c - XML parser test example for cross-plateform version </P>
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
  XMLNode xMainNode=XMLNode::openFileHelper("PMMLModel.xml","PMML");

  // this prints "RANK For <you>":
  XMLNode xNode=xMainNode.getChildNode("Header");  
  printf("Application Name is: '%s'\n", xNode.getChildNode("Application").getAttribute("name"));

  // this prints "Hello World!"
  printf("Text inside Header tag is :'%s'\n", xNode.getText());
  
  // this gets the number of "NumericPredictor" tags:  
  xNode=xMainNode.getChildNode("RegressionModel").getChildNode("RegressionTable"); 
  int n=xNode.nChildNode("NumericPredictor");

  // this prints the "coefficient" value for all the "NumericPredictor" tags:
  int i,iterator=0;  
  for (i=0; i<n; i++)
    printf("coeff %i=%s\n",i+1,xNode.getChildNode("NumericPredictor",&iterator).getAttribute("coefficient"));

  // this create a file named "test.xml" based on the content of the first "Extension" tag of the XML file:  
  char *t=xMainNode.getChildNode(_T("Extension")).createXMLString(true,&i); 
  FILE *f=fopen("test.xml","w");
  fprintf(f,"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n");
  fwrite(t,sizeof(char),i,f);
  printf("%s",t);
  fclose(f);
  free(t);

    // compare these 4 lines ...
  t=stringDup(xMainNode.getAttribute("version"));       // get version number
  xMainNode=XMLNode::emptyXMLNode;                      // free from memory the top of the xml Tree
  printf("PMML Version :%s\n",t);                       // print version number
  free(t);                                              // free version number

  // ... with the following 3 lines (currently commented, because of error):
  //  t=xMainNode.getAttribute("version");      // get version number (note that there is no 'stringDup')
  //  xMainNode=XMLNode::emptyXMLNode;          // free from memory the top of the xml Tree AND the version number inside 't' var
  //  printf("PMML Version :%s\n",t);           // since the version number in 't' has been free'd, this will not work

  // We create in memory from scratch the following XML structure:
  //  <?xml version="1.0"?>
  //      <body> Hello universe. </body>
  // ... and we transform it into a standard C string that is printed on screen.
  xNode=XMLNode::createXMLTopNode();
  XMLNode xn=xNode.addChild(stringDup("xml"),TRUE); xn.addAttribute(stringDup("version"),stringDup("1.0"));
  xn=xn.addChild(stringDup("body"));  xn.addText(stringDup("Hello \"universe\"!"));
  t=xNode.createXMLString(true);
  printf("XMLString created from scratch:\n%s",t);
  free(t);

  return 0;
}
