
#ifndef CIRABSTRACTCOMPONENT_H
#define CIRABSTRACTCOMPONENT_H

#include <string>
#include <list>
#include <map>
#include "CIRpinArray.h"

class CIRcomponentFactory;

using namespace std;

class CIRabstractComponent
{
public:

  // Constructors/Destructors
  //  
  CIRabstractComponent ();
  virtual ~CIRabstractComponent ( ) = 0;

  // Public attributes
  //  
  virtual bool finishedMake();
  const bool isGood() {return m_good;} 
  const unsigned int getId() {return m_componentId;}
  void setId(const unsigned int id) { m_componentId = id;}

  bool setInput(const unsigned int inputNumber, const bool input) ;
  bool endInputs();

  const bool validInputPin(const unsigned int no);
  const bool validOutputPin(const unsigned int no);

  virtual const bool getOutput(const unsigned int outputNumber );
  virtual bool registerWithOutput(const unsigned int outputNumber, 
                                 const unsigned int componentIndex,
                                 const unsigned int inputNumber);
  virtual bool notifyOutputs();
  virtual void rememberInputs() {m_inputChanged = false;};
  virtual const bool inputsChanged() ;
  const unsigned int getNumberInputs() { return m_inputPinsMap.size();}
  const unsigned int getNumberOutputs() { return m_outputPinsMap.size();}
  const list<unsigned int> getInputPins() ;
  const list<unsigned int> getOutputPins();
  const unsigned int inputsToInt();
  void outputsToPins(const unsigned int output1, const unsigned int output0 = 0);

protected:

  virtual bool computeOutputs() = 0;

  // To be initialized in initialization list
  unsigned int m_componentId;
  bool         m_good;
  bool         m_firstCall;
  bool         m_inputChanged;
  bool         m_finished;
  bool         m_okToInput;

  unsigned int * m_inputPinsArray;
  unsigned int * m_outputPinsArray;
  unsigned int * m_connectedComponents;
  unsigned int ** m_registeredOutputsArray;

  // To be created in constructor
  CIRpinArray*  m_inputPinArray;
  CIRpinArray*  m_outputPinArray;

  CIRcomponentFactory * m_factory;

  // Neither
  map<unsigned int, unsigned int> m_inputPinsMap;
  map<unsigned int, unsigned int> m_outputPinsMap;
  multimap<unsigned int, pair<unsigned int, unsigned int> > m_registeredOutputs;

private:

  CIRabstractComponent& operator=(const CIRabstractComponent& rhs);
  CIRabstractComponent(const CIRabstractComponent& rhs);
};

#endif // CIRABSTRACTCOMPONENT_H
