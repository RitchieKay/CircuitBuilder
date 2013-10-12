
#ifndef CIROSCILLATOR_H
#define CIROSCILLATOR_H
#include "CIRabstractComponent.h"
#include <sys/time.h>

#include <string>
#include <map>

class CIRoscillator : public CIRabstractComponent
	
{
public:
	

  // Constructors/Destructors
  //  
  CIRoscillator (const string period);
  virtual ~CIRoscillator ( );

  virtual bool addPins() ;
  virtual bool computeOutputs();

  virtual unsigned int getPeriod() {return m_period;}

protected:
  //  
  unsigned int m_period; // micro-secs
  bool m_state;
  struct timeval m_lastTime;

private:

CIRoscillator ( );

};

#endif // CIROSCILLATOR_H
