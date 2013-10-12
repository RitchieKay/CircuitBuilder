
#ifndef CIRBUILTINCOMPONENT_H
#define CIRBUILTINCOMPONENT_H
#include "CIRabstractComponent.h"

#include <string>
#include <map>
#include <vector>
/**
  * class CIRbuiltInComponent
  */

class CIRbuiltInComponent : public CIRabstractComponent
{
public:
	

  // Constructors/Destructors
  //  
  CIRbuiltInComponent (const unsigned int inputPins, const unsigned int outputpins);
  virtual ~CIRbuiltInComponent ( );

  virtual bool addPins() ;
  virtual void setRule(const unsigned int, const unsigned int output );
  virtual void setRule(const vector<unsigned int> & values, bool value );
  virtual bool computeOutputs();

protected:
  map<unsigned int, unsigned int> m_ruleMap;
  unsigned int m_inputPins;
  unsigned int m_outputPins;

  unsigned int * m_ruleArray;
  unsigned int  m_ruleArraySize;
  bool m_defaultValue;

private:

  CIRbuiltInComponent ( );
  CIRbuiltInComponent (const CIRbuiltInComponent& rhs );
  CIRbuiltInComponent& operator=(const CIRbuiltInComponent& rhs );
};

#endif // CIRBUILTINCOMPONENT_H
