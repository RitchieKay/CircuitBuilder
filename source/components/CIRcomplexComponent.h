
#ifndef CIRCOMPLEXCOMPONENT_H
#define CIRCOMPLEXCOMPONENT_H
#include <sys/time.h>
#include <string>
#include <vector>
#include <map>
#include "CIRabstractComponent.h"


class CIRcomplexComponent : public CIRabstractComponent
{
public:

  // Constructors/Destructors
  //  
  CIRcomplexComponent ();
  virtual bool construct(const string & name); 
  virtual ~CIRcomplexComponent ( );

  virtual bool computeOutputs();

protected:

  virtual bool finishedMake();
  bool loadFromFile();
  void setComponentInputs();
  bool computeComponentOutputs();
  void setOutputBits();
  bool calculationDue();
  const string getFullComponentName(const string & fullSectionName, const string & name);



  // To be initialized in initialization list
  unsigned int * m_componentsArray;
  unsigned int * m_componentsRegisteredToOutputsArray;
  unsigned int * m_componentsRegisteredToInputsArray;
  unsigned int**  m_componentsRegisteredToOutputPinArray;
  unsigned int**  m_componentsRegisteredToInputPinArray;
  unsigned int**  m_componentsRegisteredToInvertPinArray;

  // Don't require initialization

  map<string, unsigned int> m_componentsMap;
  list<unsigned int> m_componentsRegisteredToInputsList;
  list<unsigned int> m_componentsRegisteredToOutputsList;

  multimap<unsigned int, pair<unsigned int, unsigned int> > m_componentsRegisteredToInputPinMultiMap;
  multimap<unsigned int, pair<unsigned int, unsigned int> > m_componentsRegisteredToInvertPinMultiMap;
  multimap<unsigned int, pair<unsigned int, unsigned int> > m_componentsRegisteredToOutputPinMultiMap;

private:
  CIRcomplexComponent(const CIRcomplexComponent & rhs);
  CIRcomplexComponent & operator=(const CIRcomplexComponent & rhs);
};

#endif // CIRCOMPLEXCOMPONENT_H
