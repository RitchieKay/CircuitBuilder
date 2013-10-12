#include "CIRcomponentFactory.h"
#include "CIRabstractComponent.h"
#include "CIRbuiltInComponent.h"
#include "CIRcomplexComponent.h"
#include "CIRoscillator.h"
#include "CIRmemory.h"

#include <iostream>
#include <fstream>
#include <sstream>
namespace {
	CIRcomponentFactory * ptr_CIRcomponentDir = NULL;
}

// Constructors/Destructors
//  
//===========================================================================
// Name     : instance
// Arguments: None
// Purpose  : instance method of singleton class
// Notes    : 
//           
//===========================================================================

CIRcomponentFactory * CIRcomponentFactory::instance()
{
  if (!ptr_CIRcomponentDir) {
    ptr_CIRcomponentDir = new CIRcomponentFactory;
  }
  return ptr_CIRcomponentDir;
}

//===========================================================================
// Name     : CIRcomponentFactory
// Arguments: None
// Purpose  : Default Constructor
// Notes    : CIRcomponentFactory is responsible for making the components.
//            Depending on the component name it will make a built-in, specialised
//            or complex class.
//            It maintains a centralised list of all components in use.
//===========================================================================

CIRcomponentFactory::CIRcomponentFactory ( ) :
m_componentCounter(0),
m_finished(false),
m_componentPtr(NULL),
m_memoryPtr(NULL),
m_oscillatorPtr(NULL)
{
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : ~CIRcomponentfactory
// Arguments: None
// Purpose  : Destructor
// Notes    : None
//
//===========================================================================

CIRcomponentFactory::~CIRcomponentFactory ( ) {
  clearAll();

}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : clearAll
// Arguments: None
// Purpose  : Public method to destroy everything created.
// Notes    : None
//
//===========================================================================

void CIRcomponentFactory::clearAll()
{
  m_finished = false;
  unsigned int count = 0;
  map<unsigned int, CIRabstractComponent*>::iterator it = m_componentMap.begin();
  while (it != m_componentMap.end()) {

    if ((*it).second) delete (*it).second;

    count++;
    it++;
  }

  m_componentCounter = 0;

  m_componentMap.clear();
  m_componentIdMap.clear();

  if (m_componentPtr) {
    delete [] m_componentPtr;
    m_componentPtr = NULL;
  }

}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : finishedMake
// Arguments: None
// Purpose  : Once all components have been built finishedMake must be called
//            before they can be used as explained in component classes.
// Notes    : At this level converts the map of components to an array for
//            faster look-up.
//
//===========================================================================
void CIRcomponentFactory::finishedMake()
{
  m_componentPtr = new CIRabstractComponent * [m_componentCounter];

  map<unsigned int, CIRabstractComponent*>::iterator it = m_componentMap.begin();
  while (it != m_componentMap.end()) {

    m_componentPtr[(*it).first] = (*it).second;    

    // Called finished make on components
    m_componentPtr[(*it).first]->finishedMake();

    it++;
  }

  m_finished = true;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : addComponent
// Arguments: const string & id, const unsigned int makerId, CIRabstractComponent* component, 
//            bool & error
// Purpose  : Protected method to add a component to the map
// Notes    : 
//
//===========================================================================

unsigned int CIRcomponentFactory::addComponent (const string & id, const unsigned int makerId, CIRabstractComponent* component, bool & error )
{
  error = false;

  // Check that the supplied id exists
  if (m_componentCounter) {

    if (m_componentIdMap.find(makerId) == m_componentIdMap.end()) {
      cerr << "Error: component maker has specified a false identity" << endl;
      error = true;
      return 0;
    }

  }

  string fullId = m_componentIdMap[makerId] + "." + id; 

  component->setId(m_componentCounter);

  unsigned int retVal = m_componentCounter;
  m_componentMap[m_componentCounter] = const_cast<CIRabstractComponent*>(component);
  m_componentIdMap[m_componentCounter] = fullId;
  m_componentCounter++;
  return retVal;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : dumpComponents
// Arguments: None
// Purpose  : Public method to dump all components to the screen.
// Notes    :
//
//===========================================================================

void CIRcomponentFactory::dumpComponents()
{
  cout << "+++++++++++++++++Start component dump+++++++++++++++" << endl;
  map<unsigned int, string >::iterator it = m_componentIdMap.begin();
  while (it != m_componentIdMap.end()) {
    cout << (*it).first  << "  " << (*it).second << endl;
    it++;
  }
  cout << "++++++++++++++++++End component dump++++++++++++++++" << endl;

}

//--------------------------------------------------------------------------
//===========================================================================
// Name     : makeComponent
// Arguments: const string & componentName, const string & id
//            const vector<string> & arg_list, const unsigned int makerId, bool & error
// Purpose  : This is the main method where components are made.
// Notes    : This method recognises whether the component is a built-in:
//            AND_GATE, OR_GATE, NOR_GATE, NAND_GATE, XOR_GATE, BUFFER, INVERTOR
//            whether it is one of the specialist components - OSCILLATOR, MEMORY
//            otherwise assumes it is a complexComponent defined in a component
//            definition file.
//
//===========================================================================
unsigned int CIRcomponentFactory::makeComponent(const string & componentName, const string & id,
                                                const vector<string> & arg_list,
                                                  const unsigned int makerId, bool & error)
{
        // First check if is a built in and makeComponent
        CIRabstractComponent * comp = NULL;
        unsigned int componentId;

        // Build an oscillator. In this case the argument is the periodicity.
        if (componentName == "OSCILLATOR") {
           string arg = "200";
           if (arg_list.size() > 0) arg = arg_list[0];
           comp = new CIRoscillator(arg);
           if (m_oscillatorPtr) {
             cout << "Warning: more than one oscillator defined." << endl;
           }
           m_oscillatorPtr = dynamic_cast<CIRoscillator*>(comp);
           componentId = addComponent(id, makerId, comp, error);
           (dynamic_cast<CIRoscillator*>(comp))->addPins();
        }
        // Build memory. In this case the arguments are the number of address pins on the
        // memory chip and a file definining the rom contents (optional).
        else if (componentName == "MEMORY") {
           string arg = "10";
           string file ="";
           if (arg_list.size() > 0) arg = arg_list[0];
           if (arg_list.size() > 1) file = arg_list[1];

           comp = new CIRmemory(arg, file);
           if (m_memoryPtr) {
             cout << "Warning: more than one memory defined." << endl;
           }
           m_memoryPtr = dynamic_cast<CIRmemory*>(comp);
           componentId = addComponent(id, makerId, comp, error);
           (dynamic_cast<CIRmemory*>(comp))->addPins();
        // Tries to make a build-in component AND_GATE, OR_GATE etc
        } else if ((comp = makeBuiltIn(componentName, arg_list))) {
          componentId = addComponent(id, makerId, comp, error);
           (dynamic_cast<CIRbuiltInComponent*>(comp))->addPins();
        // If not successful must be a complex
        } else if ((comp = makeComplex())) {
          componentId = addComponent(id, makerId, comp, error);

          if (!error) {
          (  dynamic_cast<CIRcomplexComponent*>(comp))->construct(componentName);
          } else {
            delete comp;
            cerr << "Error: complex component could not be constructed" << endl;
            error = true;
          }
        // Or it is undefined.....
        } else {

          error = true;
          cerr << "Error: Unidentified component. Not making." << endl;
          return 0;
        }
        return componentId;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : makeBuiltIn
// Arguments: const string & componentName,  const vector<string> & arg_list
// Purpose  : This tries to make a built-in component.
// Notes    : A built-in component is a component with N inputs and 1 output
//            whose output can be computed using a simple formula.
//            It is this method which knows about AND_GATES and OR_GATES
//            and sets the rule of the built-in component accordingly.
//            The CIRbuiltInComponent object is fairly dumb.
//
//            Rules can be defined in two ways. See CIRbuiltInComponent for details.
//===========================================================================

CIRbuiltInComponent * CIRcomponentFactory ::makeBuiltIn(const string & componentName, 
                                                        const vector<string> & arg_list)
{
  bool hasArg = false;
  unsigned int arg; 
  if (arg_list.size() > 0) {
    string sarg = arg_list[0];
    istringstream(sarg) >> arg;
    hasArg = true;
  }
  vector<unsigned int> values;

  CIRbuiltInComponent * comp = NULL;
  if (componentName == "AND_GATE") { // Recognise an AND_GATE
    unsigned int inputs = 2;
    if (hasArg && (arg > 2)) inputs = arg; // Can have N input pins given by optional
    values.push_back((1 << inputs) - 1);   // argument. Default = 2.
     comp = new CIRbuiltInComponent(inputs, 1);
     comp->setRule(values, true);
  }
  else if (componentName == "OR_GATE") {
    unsigned int inputs = 2;
    if (hasArg && (arg > 2)) inputs = arg;
    values.push_back(0);
    comp = new CIRbuiltInComponent(inputs, 1);
    comp->setRule(values, false);
  }
  else if (componentName == "NAND_GATE") {
    unsigned int inputs = 2;
    if (hasArg && (arg > 2)) inputs = arg;
    values.push_back((1 << inputs) - 1);


    comp = new CIRbuiltInComponent(inputs, 1);
    comp->setRule(values, false);
  }
  else if (componentName == "XOR_GATE") { // No arguments. Always has 2 pins!

    comp = new CIRbuiltInComponent(2, 1);
    comp->setRule(0, 0);
    comp->setRule(1, 1);
    comp->setRule(2, 1);
    comp->setRule(3, 0);
  }
  else if (componentName == "NOR_GATE") {
    unsigned int inputs = 2;
    if (hasArg && (arg > 2)) inputs = arg;
    values.push_back(0);
    comp = new CIRbuiltInComponent(inputs, 1);
    comp->setRule(values, true);
  }
  else if (componentName == "INVERTOR") { // No arguments. Always has 1 pin

    comp = new CIRbuiltInComponent( 1, 1);
    comp->setRule(0, 1);
    comp->setRule(1, 0);
  }
  else if (componentName == "BUFFER") { // No arguments. Always has 1 pin
    comp = new CIRbuiltInComponent(1, 1);
    comp->setRule(0, 0);
    comp->setRule(1, 1);
  }
  return comp;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : makeComplex
// Arguments: None
// Purpose  : Creates a complex component but doesn't construct it.
// Notes    : This is slightly perverse but is because the top-level
//            component of a circuit will always be a complex component
//            and it is conceptually nicer to have this with component id = 0
//            If we construct first before adding to the list all the sub-components
//            would appear before it. 
//===========================================================================

CIRcomplexComponent * CIRcomponentFactory::makeComplex()
{
  CIRcomplexComponent * comp = new CIRcomplexComponent();
  return comp;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : connectFromTo
// Arguments: const unsigned int fromComp, const unsigned int outputNumber
//            const   unsigned int toComp, const unsigned int inputNumber
// Purpose  : This is the wiring part of the factory. Connects output pin
//            of one component to input pin of another.
// Notes    : 
//===========================================================================

// Method for connecting components
bool CIRcomponentFactory::connectFromTo(const unsigned int fromComp,
                                        const unsigned int outputNumber,
                                        const   unsigned int toComp, 
                                        const unsigned int inputNumber)
{
  // check both componets exist
  if (!(CIRcomponentFactory::instance()->getComponent(fromComp) &&
    CIRcomponentFactory::instance()->getComponent(toComp))) return false;

  CIRabstractComponent * comp =
                               CIRcomponentFactory::instance()->getComponent(fromComp);

  return comp->registerWithOutput(outputNumber, toComp, inputNumber);

}
//--------------------------------------------------------------------------

