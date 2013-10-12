
#ifndef CIRMEMORY_H
#define CIRMEMORY_H
#include "CIRabstractComponent.h"
#include <sys/time.h>

#include <string>
#include <map>

class CIRmemory : public CIRabstractComponent
	
{
public:
	

  // Constructors/Destructors
  //  
  CIRmemory (string addressPins, string romFile = "");
  virtual ~CIRmemory ( );

  virtual bool addPins() ;
  virtual bool computeOutputs();

  void loadRom();
  void writeContents();

protected:

  unsigned char* m_memoryMap;
  unsigned int m_addressPins;
  unsigned int m_maxAddress;
  bool m_oldmemwrite;
  string m_romFile;
private:

  CIRmemory ();
  CIRmemory (const CIRmemory & rhs);
  CIRmemory& operator= (const CIRmemory & rhs);

};

#endif // CIRMEMORY_H
