echo Building object files
g++ -o ./o/Base64Encoder.o -c Base64Encoder.cpp
g++ -o ./o/SOAPAttribute.o -c SOAPAttribute.cpp
g++ -o ./o/SOAPDispatcher.o -c SOAPDispatcher.cpp
g++ -o ./o/SOAPElement.o -c SOAPElement.cpp
g++ -o ./o/SOAPEncoder.o -c SOAPEncoder.cpp
g++ -o ./o/SOAPFault.o -c SOAPFault.cpp
g++ -o ./o/SOAPMethod.o -c SOAPMethod.cpp
g++ -o ./o/SOAPObject.o -c SOAPObject.cpp
g++ -o ./o/SOAPObjectCreator.o -c SOAPObjectCreator.cpp
g++ -o ./o/SOAPParser.o -c SOAPParser.cpp
g++ -o ./o/SOAPSender.o -c -I ../SoapNetwork SOAPSender.cpp

echo Building Library
cd o
ar -rc SimpleSOAP.lib Base64Encoder.o  SOAPDispatcher.o  SOAPEncoder.o  SOAPMethod.o  SOAPObjectCreator.o  SOAPSender.o
ar -rc SimpleSOAP.lib SOAPAttribute.o  SOAPElement.o     SOAPFault.o    SOAPObject.o  SOAPParser.o
cd ..

