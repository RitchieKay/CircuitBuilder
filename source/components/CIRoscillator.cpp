#include <iostream>
#include <fstream>
#include <sstream>
#include "CIRoscillator.h"
#include "CIRcomponentFactory.h"

using namespace std;
// Constructors/Destructors
//  
//===========================================================================
// Name     : CIRoscillator
// Arguments: string period
// Purpose  : Default Constructor
// Notes    : This is a specialist component representing an oscillator.
//            it will change its output based on the time since it last
//            changed its output
//===========================================================================

CIRoscillator::CIRoscillator (const string period):
m_state(0)
{
  istringstream iS(period);
  iS >> m_period;
  m_good = true;
  gettimeofday(&m_lastTime, NULL);
}
CIRoscillator::CIRoscillator ( ) {
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : ~CIRpins
// Arguments: None
// Purpose  : Destructor
// Notes    : None
//
//===========================================================================

CIRoscillator::~CIRoscillator ( ) { }
//--------------------------------------------------------------------------
//===========================================================================
// Name     : addPins
// Arguments: None
// Purpose  : Add pins to the chip.
// Notes    : This is a historical artefact. Should be in constructor
//
//===========================================================================

bool CIRoscillator::addPins()
{
  bool error = false;
  unsigned int pinId = 0;
  unsigned int spareId = 0;

  m_inputPinsMap[0] = pinId;
  m_inputPinsMap[1] = spareId;
  m_inputPinsMap[0] = pinId;
  m_outputPinsMap[0] = pinId;

  return error;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : computeOutputs
// Arguments: None
// Purpose  : compute the outputs of the oscillator
// Notes    : Alternately 1 and 0.
//            Note that the oscillator does not possess a timer. It must be
//            regularly prompted to compute its outputs
//
//===========================================================================

bool CIRoscillator::computeOutputs()
{
  struct timeval t;
  gettimeofday(&t, NULL);

  unsigned int diff = static_cast<unsigned int>(1.0*(t.tv_sec - m_lastTime.tv_sec)*1000000 + (t.tv_usec - m_lastTime.tv_usec));

  if (diff > m_period) {

    // set current output to spare input
    bool input = m_outputPinArray->getSignal(0);

    m_inputPinArray->applySignal(1, !input);
    // This is fictious but conceptually time is an input to the oscillator. If the
    // oscillating period has elapsed the input has changed!! This allows things to
    // happen.
    m_inputChanged = true;

    m_lastTime = t;
  }

  // Input 1 is a fictional input. Input 0 is the on switch!
  bool signal = m_inputPinArray->getSignal(1) &
                m_inputPinArray->getSignal(0);

  outputsToPins(signal);

  return true;
}
//--------------------------------------------------------------------------
