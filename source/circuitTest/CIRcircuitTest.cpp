#include <iostream>
#include <fstream>
#include <sstream>

#include <string>
#include <vector>
#include <list>

#include "CIRcomponentFactory.h"
#include "CIRbuiltInComponent.h"

using namespace std;

bool test8bitLatch();
bool test8bitAdder();
bool test81selector();
bool test38decoder();

bool compute(const unsigned int component, const unsigned int value, unsigned int & outValue);

vector<string> args;

CIRcomponentFactory * factory = NULL;


int main()
{
	
  factory = CIRcomponentFactory::instance();

  bool error = false;

  error = test8bitLatch();
  error |= test8bitAdder();
  error |= test81selector();
  error |= test38decoder();

  if (error) {
    cerr << "Error: one or more tests failed" << endl;
  }

  return 1;
}

bool test8bitLatch()
{
  bool error = false;
  unsigned int component = factory->makeComponent("8BIT_LATCH", "8BIT_LATCH",args, 0, error);

  if (error) return false;

  cout << "Testing 8BIT_LATCH...   " ;

  unsigned int outputValue = 0;
  compute(component, 0x4301, outputValue);
  if (outputValue != 0x43) error = true;

  compute(component, 0, outputValue);
  if (outputValue != 0x43) error = true;

  compute(component, 0x0001, outputValue);
  if (outputValue != 0x0) error = true;

  compute(component, 0x01, outputValue);
  if (outputValue != 0x0) error = true;

  compute(component, 0xFF01, outputValue);
  if (outputValue != 0xFF) error = true;


  factory->clearAll();

  if (!error) {
    cout << "SUCCESS" << endl;
  } else {
    cout << "FAILED" << endl;
  }

  return error;

}

bool test8bitAdder()
{
  bool error = false;
  unsigned int component = factory->makeComponent("8BIT_ADDER", "8BIT_ADDER",args, 0, error);

  if (error) return false;

  cout << "Testing 8BIT_ADDER...   " ;

  unsigned int outputValue = 0;
  compute(component, 0x0, outputValue);
  if (outputValue != 0x0) error = true;

  compute(component, 0x0101, outputValue);
  if (outputValue != 0x02) error = true;

  compute(component, 0xffff, outputValue);
  if (outputValue != 0x1fe) error = true;

  compute(component, 0x1029, outputValue);
  if (outputValue != 0x39) error = true;

  compute(component, 0xaabb, outputValue);
  if (outputValue != 0x165) error = true;


  factory->clearAll();

  if (!error) {
    cout << "SUCCESS" << endl;
  } else {
    cout << "FAILED" << endl;
  }

  return error;

}

bool test81selector()
{
  bool error = false;
  unsigned int component = factory->makeComponent("8-1SELECT", "8-1SELECT",args, 0, error);

  if (error) return false;

  cout << "Testing 8-1-SELECT...   " ;

  unsigned int outputValue = 0;
  compute(component, 0x01, outputValue);
  if (outputValue != 0x1) error = true;

  compute(component, 0x780, outputValue);
  if (outputValue != 0x1) error = true;

  compute(component, 0x680, outputValue);
  if (outputValue != 0x00) error = true;

  compute(component, 0x308, outputValue);
  if (outputValue != 0x1) error = true;

  compute(component, 0x4ff, outputValue);
  if (outputValue != 0x1) error = true;


  factory->clearAll();

  if (!error) {
    cout << "SUCCESS" << endl;
  } else {
    cout << "FAILED" << endl;
  }

  return error;

}

bool test38decoder()
{
  bool error = false;
  unsigned int component = factory->makeComponent("3-8DECODER", "3-8DECODER",args, 0, error);

  if (error) return false;

  cout << "Testing 3-8-DECODER...   " ;

  unsigned int outputValue = 0;
  compute(component, 0x0e, outputValue);
  if (outputValue != 0x0) error = true;

  compute(component, 0x1, outputValue);
  if (outputValue != 0x1) error = true;

  compute(component, 0xf, outputValue);
  if (outputValue != 0x80) error = true;

  compute(component, 0x3, outputValue);
  if (outputValue != 0x2) error = true;

  compute(component, 0xd, outputValue);
  if (outputValue != 0x40) error = true;


  factory->clearAll();

  if (!error) {
    cout << "SUCCESS" << endl;
  } else {
    cout << "FAILED" << endl;
  }

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
