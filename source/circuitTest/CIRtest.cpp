#include <iostream>
#include <fstream>
#include <sstream>

#include <string>
#include <vector>
#include <list>

#include "CIRcomponentFactory.h"
#include "CIRbuiltInComponent.h"

using namespace std;

bool testComponent(string);

bool compute(const unsigned int component, const unsigned int value, unsigned int & outValue);

CIRcomponentFactory * factory = NULL;


int main()
{
	
  factory = CIRcomponentFactory::instance();

  bool error = false;

  string comp;
  cout << "Enter component to test : " ;
  cin >> comp;
  testComponent(comp); 

  if (error) {
    cerr << "Error: one or more tests failed" << endl;
  }

  return 1;
}

bool testComponent(string comp)
{
  bool error = false;
  vector<string> args;
  unsigned int component = factory->makeComponent(comp, comp,args, 0, error);

  factory->finishedMake();

  if (error || !factory->getComponent(component)->isGood()) return false;

  cout << "Testing " << comp << " ...   " ;

  unsigned int outputValue = 0;

  for (;;) {

    cout << "Enter input value (hex) : "; 
    unsigned int inputValue;
    cin >> hex >> inputValue;

    compute(component, inputValue, outputValue);

    cout << "Output value = " << hex << outputValue << endl;

  }

  factory->clearAll();

  return error;

}

bool compute(const unsigned int component, const unsigned int inValue, unsigned int & outValue)
{

  list<unsigned int> inputs = factory->getComponent(component)->getInputPins();
  list<unsigned int>::iterator it = inputs.begin();

  while (it != inputs.end()) {

    int mask = 1 << (*it);

    factory->getComponent(component)->setInput((*it), (inValue & mask)/mask);
    it++;
  }

  factory->getComponent(component)->endInputs();

  list<unsigned int> outputs = factory->getComponent(component)->getOutputPins();
  list<unsigned int>::iterator outIt = outputs.begin();

  outValue = 0;

  while (outIt != outputs.end()) {
    unsigned int pinOutput = factory->getComponent(component)->getOutput(*outIt);
    outValue += (pinOutput << (*outIt)) ;
    outIt++;
  }
  return true;
}
