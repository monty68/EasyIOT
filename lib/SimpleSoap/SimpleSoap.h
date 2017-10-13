/*! \mainpage Simple SOAP 
  \image html SOAPBook.jpg


 Code by Scott Seely & Jasen Plietz 
 modifications by Gerard J. Cerchio

 Scott Seely published his book "SOAP Cross Platform Web Services Development using XML" in 2002.  The majority of the book is dedicated to describing a Simple SOAP implementation that Scott uses to illustrate the use of SOAP in the C++ programming environment. The book is accompanied by a CD-ROM that contains the project's source code. In 2003 I was casting around for a very light weight SOAP library that could be used as a front end to the ZORAN MaestroLink project, a media server with SOAP controls and XML media descriptions.  My goal was to find a simple set of routines that could perform all the front end work of the SOAP RPC including transport, envelope cracking, parameter parsing and preliminary XML validation.  Full schema validation was to occur later in the process after the XML entities were extracted from the SOAP RPC call.

All of the packages I found were "be all end all and by the way here is the kitchen sink too", type packages.  Once I ran across Scott's book I found the perfect solution to snapping a SOAP RPC front end to the project. The code is reasonably OS indifferent, very object oriented and is very self contained. Having the code accompanied by Scott's fine book creates a perfect tutorial package for those wishing to learn SOAP from the wire up.  The code contains HTTP transport, XML Parsing, basic data type conversion, SOAP method dispatching and a complete easy to use fault reporting system. 

There were a few parsing code errors for XML tokens but these were easily found and quickly cleaned up. The project is in a mature working stage ready for use by any adopter.  There have been no adjustments to the Simple SOAP code for the last few months except for preparing it for publishing on Source Forge. I recommend the user obtain a copy of Scott's book but it is not necessary to use the library package.

My modest addition to the package is essentially a parser bypass.  Scott's design provided for XML data types to be parsed and interpreted within the SOAP framework.  I provided a catch all SOAP parameter data type that treats a single XML element as a data type unto itself. This allows the SOAP front end to confirm that the XML of the RPC is well formed, but to ignore the complicated data types being passed as parameters of the XML RPC.

I also created a more flexible SOAP testing client that does not require re-coding to send unique SOAP RPC's.  The client uses a file that lists the order of a set of XML files which are the RPC calls. The test client packages the calls into SOAP envelopes and ships them to the designated SOAP server.  The return from the server is then printed out.  This technique allows for the creation of complete regressive validation suites that are run from a shell script calling the test client which then pipes the result to a diff tool to ensure that any code changes did not upset previous working code.

Added Features in 2.1 are comment parsing in the source XML, and a working Base64 transcoder.

The features of the library are:

   1. C++ OS neutral

   2. small footprint
   
   3. SOAP RPC HTTP transport
   
   4. SOAP XML validation
   
   5. SOAP RPC envelope parsing
   
   6. SOAP RPC dispatching
   
   7. SOAP RPC extendable data types without coding
   
   8. SOAP RPC fault generation
   
   9. easily extensible
  
  10. includes extendable test suite
  
  11. production ready code,tested in MSVC .NET and gcc 3.x
  
  12. fully documented

*/