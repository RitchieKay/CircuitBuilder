#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <list>

#include <sys/time.h>

#include "CIRcomponentFactory.h"
#include "CIRbuiltInComponent.h"
#include "CIRmemory.h"
#include "CIRoscillator.h"

using namespace std;

  CIRcomponentFactory * factory = NULL;
  unsigned int input = 0;
  unsigned int output = 0;
  string outputString;
  unsigned int mainComponent = 0;
  string componentName;


int main()
{
  factory = CIRcomponentFactory::instance();

// Make the 8080 component

  bool error = false;
  vector<string> args;
  factory->clearAll();

  mainComponent = factory->makeComponent("8080", "mainComponent", args, 0, error);

  if (error || !factory->getComponent(mainComponent)->isGood()) {
    cerr << "Error making 8080" << endl;
    return 0;
  }
  factory->finishedMake();

  cout << "Successfully made 8080 chip using " << factory->getComponentCount() << " components." << endl;
 


// Obtain the oscillator period. We will drive the chip at twice this rate

  cout << "Period = " << factory->getOscillator()->getPeriod() << endl;

 const unsigned int milli = factory->getOscillator()->getPeriod();

 struct timespec time;
 time.tv_sec = 0;
 time.tv_nsec = (milli * 1000)/2;

// Trigger the load of the rom

 factory->getMemory()->loadRom();

// loop
 unsigned int counter = 0; 
 bool halt = false;


 struct timeval start;
 gettimeofday(&start, NULL);


 while(!halt) {


//   nanosleep(&time, NULL);
   factory->getComponent(mainComponent)->setInput(0, true);
   factory->getComponent(mainComponent)->endInputs();


   halt = factory->getComponent(mainComponent)->getOutput(31);

//    if ((counter % 100 == 0)) {
//      cout << "Writing ram" << endl;
//      factory->getMemory()->writeContents(); 
//    }
    counter++;

  }

 struct timeval end;
 gettimeofday(&end, NULL);

 unsigned int diff = static_cast<unsigned int>(1.0*(end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)/1000);

  factory->getMemory()->writeContents(); 
  cout << "Halt reached in " << diff << " milliseconds" << endl;

  return 1;
}

