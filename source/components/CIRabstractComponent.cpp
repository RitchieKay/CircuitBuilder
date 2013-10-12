#include "CIRabstractComponent.h"
#include "CIRcomponentFactory.h"

#include <iostream>
#include <fstream>

using namespace std;
// Constructors/Destructors
//  
//===========================================================================
// Name     : CIRabstractComponent
// Arguments: None
// Purpose  : Default Constructor
// Notes    : CIRabstractComponent is the absract base class from which all
//            other CIR components should be derived.
//===========================================================================
CIRabstractComponent::CIRabstractComponent () :
m_good(false),
m_firstCall(true),
m_inputChanged(false),
m_finished(false),
m_okToInput(false),
m_inputPinsArray(NULL),
m_outputPinsArray(NULL),
m_connectedComponents(NULL),
m_registeredOutputsArray(NULL)
{
  m_factory = CIRcomponentFactory::instance();
  m_inputPinArray = new CIRpinArray;
  m_outputPinArray = new CIRpinArray;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : ~CIRabstractComponent
// Arguments: None
// Purpose  : Destructor
// Notes    : None
//            
//===========================================================================
CIRabstractComponent::~CIRabstractComponent ( ) 
{ 
  if (m_inputPinArray) delete m_inputPinArray;
  if (m_outputPinArray) delete m_outputPinArray;

  unsigned int size = m_outputPinsArray[0];

  for (unsigned int i = 0; i < size ; size++) {

    if (m_registeredOutputsArray[i]) delete [] m_registeredOutputsArray[i];

  }
  if (m_registeredOutputsArray) delete [] m_registeredOutputsArray;
  if (m_connectedComponents) delete [] m_connectedComponents;
  if (m_inputPinsArray) delete [] m_inputPinsArray;
  if (m_outputPinsArray) delete [] m_outputPinsArray;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : inputsChanged
// Arguments: None
// Purpose  : Public method to determine if the inputs to the components have changed.
// Notes    : "Changed" means changed since the last time that the component
//            was asked to "rememberInputs".
//
//===========================================================================

const bool CIRabstractComponent::inputsChanged() 
{
  if (m_firstCall) {
    m_firstCall = false;
    return true;
  } else { 
    return m_inputChanged;
  }
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : getOutput
// Arguments: const unsigned int outputNumber
// Purpose  : Returns the state of a particular output pin.
// Notes    : None
//
//===========================================================================
const bool CIRabstractComponent::getOutput(const unsigned int outputNumber)
{
  return  m_outputPinArray->getSignal(outputNumber);
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : computeOutputs
// Arguments: None
// Purpose  : In the abstract class this does nothing.
// Notes    : In child classes this will cause the outputs to be computed.
//
//===========================================================================
//
bool CIRabstractComponent::computeOutputs()
{
  return true;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : endInputs
// Arguments: None
// Purpose  : Public method to inform component that inputs have ended.
// Notes    : Normally an attached component will set the input pins that
//            it is attached to and then call "endInputs". This is the trigger
//            for the component to recalculate its outputs.
//
//===========================================================================
bool CIRabstractComponent::endInputs()
{ 
  computeOutputs(); 
  return true;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : setInput
// Arguments: const unsigned int inputNumber, const bool input
// Purpose  : Public method to set the state of an input pin.
// Notes    : Modified the member m_inputChanged if the new input
//            is different to the old input on the pin. This is because
//            ultimately the output is only recalculated when the inputs
//            have changed.
//
//===========================================================================
bool CIRabstractComponent::setInput(const unsigned int inputNumber, const bool input) 
{
  bool current = m_inputPinArray->getSignal(inputNumber);

  if (current != input) {
    m_inputChanged = true;
  }

  m_inputPinArray->applySignal(inputNumber, input);

  return true;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : registerWithOutput
// Arguments: const unsigned int outputNumber, const unsigned int componentIndex,
//            const unsigned int inputNumber
// Purpose  : Public method for another component to register interest in
//            the state of an output pin.
// Notes    : The "outputNumber" pin of this component is connected to the
//            "inputNumber" pin of the component given by "componentIndex".
//
//===========================================================================

bool CIRabstractComponent::registerWithOutput(const unsigned int outputNumber, 
                                              const unsigned int componentIndex,
                                              const unsigned int inputNumber)
{
  if (m_outputPinsMap.find(outputNumber) != m_outputPinsMap.end()) {
    pair<unsigned int, unsigned int> details(componentIndex, inputNumber);
    pair<unsigned int, pair<unsigned int, unsigned int> > entry(outputNumber, details);
    // m_registeredOutputs holds for each output pin the components registered to it
    // including which input pin.	
    m_registeredOutputs.insert(entry);
    return true;
  }
  return false;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : notifyOutputs
// Arguments: None
// Purpose  : Public method to instruct the component to tell the components
//            connected to its outputs of the state of each input pin.
// Notes    : i.e. call setInputs on the connected components and once
//            we are done, call endInputs.
//            This method can only be called once the runtime arrays are set-up.
//            If this is not the case we return false and do nothing.
//
//===========================================================================
bool CIRabstractComponent::notifyOutputs()
{
  if (!m_finished) return false;

  unsigned int numberComponents = m_connectedComponents[0];

  unsigned int numberPins = m_outputPinsArray[0];
  // Loop over output pins
  for (unsigned int i = 0 ; i < numberPins ; i++) {
    unsigned int numRegComponents = m_registeredOutputsArray[i][0];

    for (unsigned int j = 0 ; j < numRegComponents ; j++) {

      unsigned int compIndex = m_registeredOutputsArray[i][2*j + 1];  
      unsigned int inputNumber = m_registeredOutputsArray[i][2*j + 2];  
			
      m_factory->getComponent(compIndex)->setInput(inputNumber, getOutput(m_outputPinsArray[i+1]));
    }
  }
  // Tell all components inputs stopped
  for (unsigned int i = 0 ; i < numberComponents ; i++) {
   m_factory->getComponent(m_connectedComponents[i+1])->endInputs();
  }
  return true;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : getInputPins
// Arguments: None
// Purpose  : This is a public method to return the list of input pin numbers
// Notes    : i.e. 0, 8, 9, 10
//
//===========================================================================

const list<unsigned int> CIRabstractComponent::getInputPins() 
{
  list<unsigned int> pinsList;

  map<unsigned int, unsigned int>::iterator it = m_inputPinsMap.begin();

  while (it != m_inputPinsMap.end()) {
    pinsList.push_back((*it).first);
    it++;
  }
  return pinsList;

}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : getInputPins
// Arguments: None
// Purpose  : This is a public method to return the list of output pin numbers
// Notes    : i.e. 0, 8, 9, 10
//
//===========================================================================
const list<unsigned int> CIRabstractComponent::getOutputPins()
{
  list<unsigned int> pinsList;

  map<unsigned int, unsigned int>::iterator it = m_outputPinsMap.begin();

  while (it != m_outputPinsMap.end()) {
    pinsList.push_back((*it).first);
    it++;
  }
  return pinsList;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : inputsToInt
// Arguments: None
// Purpose  : Returns an unsigned int representing the value of the input pins
// Notes    : Can only be used if there are < 32 inputs pins! Returns 0 if the
//            component make is not finished.
//
//===========================================================================
const unsigned int CIRabstractComponent::inputsToInt()
{
  if (!m_finished) return 0;

  unsigned int numberPins = m_inputPinsArray[0];

  unsigned int pinsAsInt = 0;

  for (unsigned int i = 0; i < numberPins ; i++) {
    unsigned int pinNo = m_inputPinsArray[i+1];
    if (m_inputPinArray->getSignal(pinNo)) {
      pinsAsInt += (1 << pinNo);
    }
  }
  return pinsAsInt;

}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : outputsToPins
// Arguments: const unsigned int output1, const unsigned int output2
//            (second argument is optional, defaulting to 0)
// Purpose  : Sets the state of the output pins to that indicated by the bits 
//            of the integer arguments
// Notes    : Simply returns if the make has not been finshed.
//
//===========================================================================
void CIRabstractComponent::outputsToPins(const unsigned int output1, const unsigned int output2)
{
  if (!m_finished) return;

  unsigned int numberPins = m_outputPinsArray[0];
  for (unsigned int i = 0; i < numberPins ; i++) {
 
    unsigned int pinNo = m_outputPinsArray[i+1];

    unsigned int output = output1;
    unsigned int pin = pinNo;

    if (pinNo > 31) {
      pin -= 32;
      output = output2;
    }

    unsigned int mask = (1 << pin);
    bool signal = (mask & output) == mask;
    m_outputPinArray->applySignal(pinNo, signal);
  }
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : validInputPin
// Arguments: const unsigned int no
// Purpose  : Does the component have an input pin with this number?
// Notes    : None.
//
//===========================================================================
const bool CIRabstractComponent::validInputPin(const unsigned int no) 
{
  return (m_inputPinsMap.find(no) != m_inputPinsMap.end());
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : validOutputPin
// Arguments: const unsigned int no
// Purpose  : Does the component have an output pin with this number?
// Notes    : None.
//
//===========================================================================
const bool CIRabstractComponent::validOutputPin(const unsigned int no) 
{

  return (m_outputPinsMap.find(no) != m_outputPinsMap.end());
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : finishedMake
// Arguments: None.
// Purpose  : Called to finish the make of a component.
// Notes    : This should be called on each component once the process of
//            building the top-level component is complete. The object is
//            to create a set of arrays rather than STL containers in order
//            to improve the speed of the finished component. Unless finishedMake
//            has been called the component can' be used at "runtime" since the
//            arrays have not been constructed.
//            All the arrays have as zeroth element the number of entries.
//
//            Note that the STL containers that are used in the make are not
//            currently cleared which is a bit of a waste of memory.
//===========================================================================

bool CIRabstractComponent::finishedMake()
{
  // This constructs an array containing the numbers of all the input pins
  // of the component

  m_inputPinsArray = new unsigned int [m_inputPinsMap.size() + 1];
  m_inputPinsArray[0] = m_inputPinsMap.size();

  map<unsigned int, unsigned int>::iterator mapIt = m_inputPinsMap.begin();

  for (unsigned int index = 0; index < m_inputPinsMap.size() ; index++, mapIt++) {
    m_inputPinsArray[index+1] = (*mapIt).first;
  }

  // This does the same thing for the output pins
 
  m_outputPinsArray = new unsigned int [m_outputPinsMap.size() + 1];
  m_outputPinsArray[0] = m_outputPinsMap.size();

  mapIt = m_outputPinsMap.begin();

  for (unsigned int index = 0; index < m_outputPinsMap.size() ; index++, mapIt++) {
    m_outputPinsArray[index+1] = (*mapIt).first;
  } 

  // This constructs an array to list the components registered to each output pin.

  // Be careful with this. The m_registeredOutputsArray is an array of pointer to pointers.
  // There is an entry for each of the component output pins for each pin we have a pointer to
  // a set of integers. The first says how many elements in the set. This is followed by two
  // integers for each element. One for the component index and one for the input number.

   unsigned int numberKeys = m_outputPinsArray[0];

   m_registeredOutputsArray = new unsigned int * [numberKeys];

   // As we go through we create list of all components connected to the outputs
   // of this component
   list<unsigned int> connected_components;

   for (unsigned int i = 0; i < numberKeys ; i++) {

     unsigned int key = m_outputPinsArray[i+1];
     unsigned int numberItems = m_registeredOutputs.count(key);

     m_registeredOutputsArray[i] = new unsigned int [2*numberItems + 1];
     m_registeredOutputsArray[i][0] = numberItems; 

     pair < multimap<unsigned int, pair<unsigned int, unsigned int> >::iterator,
      multimap<unsigned int, pair<unsigned int, unsigned int> >::iterator> ii =
         m_registeredOutputs.equal_range(key);

       unsigned int index = 1;
       multimap<unsigned int, pair<unsigned int, unsigned int> >::iterator it = ii.first;
       while (it != ii.second) {
         pair<unsigned int, unsigned int> entry = (*it).second;

         unsigned int compIndex = ((*it).second).first;
         unsigned int inputNumber = ((*it).second).second;

         m_registeredOutputsArray[i][index] = compIndex;
         m_registeredOutputsArray[i][index+1] = inputNumber;

         connected_components.push_back(compIndex);
  
         it++; index++; index++;
       }

   }
   // Ensure that the list of connected components has only one entry per component.
   connected_components.sort(); connected_components.unique();

   // iterate through the list of connected components to create an array of connected
   // components.

   list<unsigned int>::iterator it = connected_components.begin();

   m_connectedComponents = new unsigned int [connected_components.size() +1];
   m_connectedComponents[0] = connected_components.size();
   unsigned int index = 1;
   while (it != connected_components.end()){
     m_connectedComponents[index] = (*it);
     it++; index++;
   }

   // That is it! Set the finished flag. Without this the component can't be used.

  m_finished = true;

  return true;
}
