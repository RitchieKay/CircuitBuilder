#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "CIRmemory.h"
#include "CIRcomponentFactory.h"

using namespace std;
// Constructors/Destructors
//  
//===========================================================================
// Name     : CIRmemory
// Arguments: string pins, string romFile
// Purpose  : Default Constructor
// Notes    : This is a specialist class for handling a memory component.
//            Memory can be constructed as a complex component but large
//            amounts of memory require lots of components - hence processing
//            time. 
//            Constructor takes number of address pins and a file defining "rom"
//            to enable the initialisation of the memory. This is another good
//            reason for wanting a specialist class!
//===========================================================================
CIRmemory::CIRmemory (string pins, string romFile) :
m_oldmemwrite(false),
m_romFile(romFile)

{
  istringstream iS(pins);
  iS >> m_addressPins;
  m_maxAddress = static_cast<unsigned int>(pow(2, m_addressPins));
  m_memoryMap = new unsigned char [m_maxAddress];
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : ~CIRmemory
// Arguments: None
// Purpose  : Destructor
// Notes    : None
//
//===========================================================================
CIRmemory::~CIRmemory ( ) 
{
  if (m_memoryMap) delete [] m_memoryMap; 
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : addPins
// Arguments: None
// Purpose  : Add pins to the chip. 
// Notes    : This is a historical artefact. Should be in constructor
//
//===========================================================================
bool CIRmemory::addPins()
{
  bool error = false;
  // 8 input and output pins
  // also initialise memory
  for (int i = 0 ; i < 8 ; i++) {
    m_inputPinsMap[i] = 0; 
    m_outputPinsMap[i] = 0;
    m_memoryMap[i] = 0xff;
  }
  // initialise memory
  for (unsigned int i = 0 ; i < m_maxAddress ; i++) {
    m_memoryMap[i] = 0xff;
  }

  // One data write pin
  m_inputPinsMap[8] = 0;//pinId;

  // One load data pin
  m_inputPinsMap[9] = 0;//pinId;

  // One write data pin
  m_inputPinsMap[10] = 0;//pinId;
  //  address pins

  for (unsigned int i = 16 ; i < 16+m_addressPins ; i++) {
    m_inputPinsMap[i] = 0;//pinId;
  }
  return error;
}

//--------------------------------------------------------------------------
//===========================================================================
// Name     : computeOutputs
// Arguments: None
// Purpose  : compute the outputs of the memory.
// Notes    : Also support reading the rom file or writing a ram file if 
//            particular inputs set.
//
//===========================================================================

bool CIRmemory::computeOutputs()
{
  if (!m_romFile.empty() && m_inputPinArray->getSignal(9) )  { 
   loadRom();
  }
  unsigned int address = 0;
  for (unsigned int i = 16 ; i < m_addressPins+16 ; i++) {
    address += (m_inputPinArray->getSignal(i)) << (i-16);
  }

  bool memwrite = m_inputPinArray->getSignal(8);
  if ( memwrite    && !m_oldmemwrite) {

    unsigned int value = 0;
    for (int i = 0 ; i < 8 ; i++) {
      value += (m_inputPinArray->getSignal(i)) << (i);
    }
cout << "write " << address << " " << value << endl;
    m_memoryMap[address] = value;
  }
  m_oldmemwrite = memwrite;
  outputsToPins(m_memoryMap[address]);

  if (m_inputPinArray->getSignal(10)) {
    writeContents();
  }
  return true;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : loadRom
// Arguments: None
// Purpose  : Initialise memory contents from rom file
// Notes    : Format of rom file is
//            00000000: 06 aa 0e 01 16 bb 1e 02 7a 80 32 00 01 79 8b 32
//            00000010: 01 01 06 05 0e aa 16 03 1e bb 79 93 32 00 02 78
//            Can be as many bytes as desired following address.
//
//===========================================================================

void CIRmemory::loadRom()
{
   ifstream inputStream;

   string filename = "rom/" + m_romFile + ".rom";

   inputStream.open(filename.c_str());

   if (!inputStream) {
     cerr << "Error: Can't open rom file " << filename << endl;
     return;
   }

   string line;

   while (getline(inputStream, line)) {
    if (( line[0] == '#') || line.empty()) continue;
 
      unsigned int colonPos = line.find_first_of(":");
      if (colonPos == string::npos) continue;
      string saddress = line.substr(0, colonPos);
      istringstream iS(saddress);
      unsigned int address;
      iS >> hex >> address;

      if (address > m_maxAddress) continue;

      string data;
      string number="0123456789ABCSDEFabcedf";
      // strip out anything but numbers
      for (unsigned int i = colonPos + 1; i < line.size() ; i++) {
        if (line[i] == '!') break;
        if (number.find(line[i]) != string::npos) {
          data = data + line[i];
        }
      }

      unsigned int pos = 0;
      while (pos < (data.size()/2)) {

        istringstream sByte(data.substr(2*pos, 2));
        unsigned int byte;
        sByte >> hex >> byte;

        if ((address + pos) <= m_maxAddress) {
          m_memoryMap[address + pos] = byte;
        }
        pos += 1; 
      }

    }
    inputStream.close();
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : writeContents
// Arguments: None
// Purpose  : Writes the memory contents to a file in the ram directory
// Notes    : Format of ram file is
//            00000000: 06 aa 0e 01 16 bb 1e 02 7a 80 32 00 01 79 8b 32
//            00000010: 01 01 06 05 0e aa 16 03 1e bb 79 93 32 00 02 78
//
//===========================================================================

void CIRmemory::writeContents()
{
  string filename = "ram/" + m_romFile + ".ram";
  ofstream oS;
  oS.open(filename.c_str(), ios::out);
  for (unsigned int i = 0 ; i < m_maxAddress ; i++) {
    if ((i % 16) == 0) {
      oS << setfill('0') << setw(8) <<  i << ":    ";
    }
    oS << setfill('0') << setw(2) << hex << (unsigned int) m_memoryMap[i] << " " ;
    if (((i+1) % 16) == 0) {
      oS << endl;
    }
  }
}
