
#ifndef CIRpinArrayArray_H
#define CIRpinArrayArray_H

#include <string>
#include <list>
#include <map>

#include <bitset>

using namespace std;
const unsigned int CIRpinArrayNum = 2;

class CIRpinArray
{
public:

  // Constructors/Destructors
  //  
  CIRpinArray();
  virtual ~CIRpinArray ( );

  // Public methods

  void applySignal(const unsigned int pinNumber, const bool input) ;
  const bool getSignal(const unsigned int pinNumber);

protected:

  // Protected attributes
  //
  unsigned int * m_pins;
private:

};

#endif // CIRpinArrayArray_H
