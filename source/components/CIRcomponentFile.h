
#ifndef CIRCOMPONENTFILE_H
#define CIRCOMPONENTFILE_H
#include <string>
#include <vector>
#include <map>


struct CIRcomponentDefinition
{
  std::string componentName;
  std::string componentId;
  std::vector<std::string> modifiers;
};


class CIRcomponentFile
{
public:

  // Constructors/Destructors
  //  
  CIRcomponentFile (const std::string & source_file);
  virtual ~CIRcomponentFile();

  const std::vector<CIRcomponentDefinition>& componentDefinitions();

protected:

  bool construct(const std::string & source_file);
  const std::string getFullComponentName(const std::string & fullSectionName, const std::string & name);

  // To be initialized in initialization list

  // Don't require initialization

   std::vector<CIRcomponentDefinition> m_componentDefinitions;

private:
  CIRcomponentFile ();
  CIRcomponentFile(const CIRcomponentFile & rhs);
  CIRcomponentFile & operator=(const CIRcomponentFile & rhs);
};

#endif // CIRCOMPLEXCOMPONENT_H
