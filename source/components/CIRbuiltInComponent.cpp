#include <iostream>
#include <fstream>
#include "CIRbuiltInComponent.h"
#include "CIRcomponentFactory.h"

using namespace std;
// Constructors/Destructors
//  
//===========================================================================
// Name     : CIRbuiltInComponent
// Arguments: None
// Purpose  : Default Constructor
// Notes    : CIRbuiltInComponents are things like AND_GATES which can 
//            be defined by a simple rule.
//            This class simply provides the mechanisms. It does not
//            know what an AND_GATE is!!!
//===========================================================================

CIRbuiltInComponent::CIRbuiltInComponent (const unsigned int inputPins, const unsigned int outputPins) :
m_inputPins(inputPins),
m_outputPins(outputPins),
m_ruleArray(NULL)
{
  // Unlike the parent class, built-in components are always good
  m_good = true;
}
//--------------------------------------------------------------------------
CIRbuiltInComponent::CIRbuiltInComponent ()
{
}
//===========================================================================
// Name     : ~CIRbuiltInComponent
// Arguments: None
// Purpose  : Destructor
// Notes    : None
//
//===========================================================================

CIRbuiltInComponent::~CIRbuiltInComponent ( ) 
{ 
  if (m_ruleArray) delete [] m_ruleArray;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : addPins
// Arguments: None
// Purpose  : Public method to add pins to component
// Notes    : This is a historical artefact. There is no reason now
//            why this shouldn't be done when the object is created.
//
//===========================================================================

bool CIRbuiltInComponent::addPins()
{
  for (unsigned int i = 0 ; i < m_inputPins ; i++) {
    m_inputPinsMap[i] = 0; // Also an artefact. Should be a vector!
  }
  for (unsigned int i = 0 ; i < m_outputPins ; i++) {
    m_outputPinsMap[i] = 0;
  }
  return true;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : setRule
// Arguments: const vector<unsigned int> & values, bool value
// Purpose  : Sets the rule by which the output is calculated.
// Notes    : This is the first type of rule. A vector describes the
//            inputs on the input pins that produce the given value on the
//            output pin. All other values produce the inverse.
//            e.g. an AND_GATE produce value true when all inpus are one.
//            Therefore vector has one element with value (2^input pins)-1
//
//===========================================================================
void CIRbuiltInComponent::setRule(const vector<unsigned int> & values, bool value)
{

  m_ruleArraySize = values.size();
  m_ruleArray = new unsigned int [m_ruleArraySize];
  m_defaultValue = value;

  for (unsigned int i = 0; i < m_ruleArraySize ; i++) {
    m_ruleArray[i] = values[i];
  }
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : setRule
// Arguments: const unsigned int input, const unsigned int output
// Purpose  : Sets the rule by which the output is calculated.
// Notes    : This is the second type of rule. A given value on the input pins
//            produces a given output. Normally would be called multiple times
//            for each possibility.
//
//===========================================================================

void CIRbuiltInComponent::setRule(const unsigned int input, const unsigned int output)
{
  m_ruleMap[input] = output;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : computeOutputs
// Arguments: None
// Purpose  : Computes output according to the rule.
// Notes    : 
//
//===========================================================================

bool CIRbuiltInComponent::computeOutputs()
{
  if (m_ruleArray) {
    // This is assuming that a built in component has no more than 32 pins!
    unsigned int inputValue = inputsToInt();
    bool value = !m_defaultValue;
    for (unsigned int i = 0; i < m_ruleArraySize ; i++) {
      if (m_ruleArray[i] == inputValue) {
       value = m_defaultValue;
       break;
      }
    }
    outputsToPins(value);
    return true; 

  } else {
    map<unsigned int, unsigned int>::iterator it = m_ruleMap.find(inputsToInt());
    if (it != m_ruleMap.end()) {
      unsigned int output = (*it).second;;
      outputsToPins(output);
      return true;
    }
    return false;
  }
}
//--------------------------------------------------------------------------

