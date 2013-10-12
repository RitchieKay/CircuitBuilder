
#ifndef CIRCOMPONENTFACTORY_H
#define CIRCOMPONENTFACTORY_H

#include <string>
#include <vector>
#include <map>

using namespace std;
// Forward declarations

class CIRabstractComponent;
class CIRcomplexComponent;
class CIRbuiltInComponent;
class CIRmemory;
class CIRoscillator;

class CIRcomponentFactory
{
public:

  // Constructors/Destructors
  //  
  CIRcomponentFactory ( );
  virtual ~CIRcomponentFactory ( );

  virtual void clearAll();
  virtual void finishedMake();

  static CIRcomponentFactory* instance ( );

  virtual unsigned int makeComponent(const string & componentName, const string & id, 
                                     const vector<string> & arg_list, 
                                     const unsigned int makerId, bool & error);
  virtual CIRbuiltInComponent* makeBuiltIn(const string & componentName, const vector<string> & arg_list);
  virtual CIRcomplexComponent* makeComplex();

  virtual bool connectFromTo(const unsigned int fromComp,
                             const unsigned int outputNumber,
                             const unsigned int toComp,
                             const unsigned int inputNumber);

  unsigned int addComponent (const string & id, const unsigned int makerId,  CIRabstractComponent* component, bool & error );

  unsigned int getComponentCount() { return m_componentCounter;}
 
  // Not sure if this makes it much faster
  inline CIRabstractComponent* getComponent (const unsigned int index )
  {
    CIRabstractComponent* retVal = NULL;
    if (!m_finished) {
      retVal = m_componentMap[index];
    } else {
      retVal = m_componentPtr[index];
    }
    return retVal;	
  };

  CIRmemory * getMemory() {return m_memoryPtr;}
  CIRoscillator * getOscillator() {return m_oscillatorPtr;}

  // dumps
  void dumpComponents(); 
  void dumpPins(); 

protected:

  // To be initialized in initialization list
  unsigned int m_componentCounter;
  bool m_finished;
  CIRabstractComponent** m_componentPtr;
  CIRmemory * m_memoryPtr;
  CIRoscillator * m_oscillatorPtr;

  // No need to initialize

  map<unsigned int, CIRabstractComponent*> m_componentMap;
  map<unsigned int, string> m_componentIdMap;

private:
  CIRcomponentFactory(const CIRcomponentFactory & rhs);
  CIRcomponentFactory& operator=(const CIRcomponentFactory & rhs);

};

#endif // CIRCOMPONENTFACTORY_H
