#include "CIRpinArray.h"

#include <iostream>
#include <fstream>

using namespace std;
//===========================================================================
// Name     : CIRpinArray
// Arguments: None.
// Purpose  : Default Constructor
// Notes    : This models the input/output pins of a component. Each
//            abtstract component therefore has two of these.
//===========================================================================
CIRpinArray::CIRpinArray ()  
{
  // internally the state of the pins is contained in two integers.
  // This gives us 64 pins.
  m_pins = new unsigned int [CIRpinArrayNum ];
  for (unsigned int i = 0 ; i < CIRpinArrayNum ;i++) {
    m_pins[i] = 0;
  }
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : ~CIRpinArray
// Arguments: None
// Purpose  : Destructor
// Notes    : None
//
//===========================================================================
CIRpinArray::~CIRpinArray ( ) 
{ 
  delete [] m_pins;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : applySignal
// Arguments: None
// Purpose  : apply a signal to a specified pin
// Notes    : None
//
//===========================================================================
void CIRpinArray::applySignal(const unsigned int pinNumber, const bool input) 
{
  unsigned int index = pinNumber / (8 * sizeof(unsigned int));
  if (index >= CIRpinArrayNum) {
    cerr << "Error: bad pinNumber " << pinNumber << endl;
  }
  unsigned int inputNumber = pinNumber - (8 * index * sizeof(unsigned int));
  unsigned int mask = (1 << inputNumber);
  if (input) {
    m_pins[index] |= mask;
  } else {
    mask = ~mask;
    m_pins[index] &= mask;
  }
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : getSignal
// Arguments: None
// Purpose  : get signal from a specified pin
// Notes    : None
//
//===========================================================================

const bool CIRpinArray::getSignal(const unsigned int pinNumber)
{
  unsigned int index = pinNumber / (8 * sizeof(unsigned int));
  if (index >= CIRpinArrayNum) {
    cerr << "Error: bad pinNumber " << pinNumber << endl;
  }
  unsigned int inputNumber = pinNumber - (8 * index * sizeof(unsigned int));
  unsigned int mask = (1 << inputNumber);
  bool current = (m_pins[index] & mask)/mask;
  return current;
}
//--------------------------------------------------------------------------
