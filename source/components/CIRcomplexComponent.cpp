#include "CIRcomplexComponent.h"
#include "CIRcomponentFactory.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

// Note. The way to tidy this up is to have a class that reads the component file,
// constructs the full names of all the components and the connections.
// Once that is done, we can just loop through the components, create them
// and wire them up.

// The finished make duplicates a lot of code. This can just be tidied up by creating functions

using namespace std;

namespace {
//===========================================================================
// Function to remove whitespace from strings.
//===========================================================================
  std::string strip(const string& s)
  {
  string result;
  stringstream iS(s); 
  iS >> result;
  return result;
  }
  // This constant is a safeguard against badly wired run-away components.
  // No real component should need anywhere need this many iterations until
  // it reaches stability.
  const unsigned int MAX_STABILITY_ITERATIONS = 100;
}

// Constructors/Destructors
//  
//===========================================================================
// Name     : CIRcomplexComponent
// Arguments: None
// Purpose  : Default Constructor
// Notes    : CIRcomplexComponents are the meat and drink of CIR. A complex
//            component consists of one/many individual components which are
//            inter-connected. Some of the internal components take their
//            inputs from the input pins of the component and some provide
//            their output to the output pins.
//            The components are defined in files with the name
//            <COMPONENT_NAME>.cmp located in a components directory under the
//            working directory of the process (at the moment).
//            The file format is described below...
//            Like all other components a CIRcomplexComponent is a child of
//            CIRabstractComponent and all of its consituent components are
//            likewise children of CIRabstractComponent.
//===========================================================================

CIRcomplexComponent::CIRcomplexComponent () :
m_componentsArray(NULL),
m_componentsRegisteredToOutputsArray(NULL),
m_componentsRegisteredToInputsArray(NULL),
m_componentsRegisteredToOutputPinArray(NULL),
m_componentsRegisteredToInputPinArray(NULL),
m_componentsRegisteredToInvertPinArray(NULL)
{
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : ~CIRcomplexComponent
// Arguments: None
// Purpose  : Destructor
// Notes    : Deletes all the memory associated to the runtime arrays.
//
//===========================================================================
CIRcomplexComponent::~CIRcomplexComponent ( ) 
{
  unsigned int size = m_outputPinsArray[0];

  for (unsigned int i = 0; i < size ; size++) {
    if (m_componentsRegisteredToOutputPinArray[i]) delete [] m_componentsRegisteredToOutputPinArray[i];
  }
  if (m_componentsRegisteredToOutputPinArray) delete [] m_componentsRegisteredToOutputPinArray;

  size = m_inputPinsArray[0];
  for (unsigned int i = 0; i < size ; size++) {
    if (m_componentsRegisteredToInputPinArray[i]) delete [] m_componentsRegisteredToInputPinArray[i];
  }
  if (m_componentsRegisteredToInputPinArray) delete [] m_componentsRegisteredToInputPinArray;
  for (unsigned int i = 0; i < size ; size++) {
    if (m_componentsRegisteredToInvertPinArray[i]) delete [] m_componentsRegisteredToInvertPinArray[i];
  }
  if (m_componentsRegisteredToInvertPinArray) delete [] m_componentsRegisteredToInvertPinArray;
 
  if (m_componentsRegisteredToOutputsArray) delete [] m_componentsRegisteredToOutputsArray;
  if (m_componentsRegisteredToInputsArray) delete [] m_componentsRegisteredToInputsArray;
  if (m_componentsArray) delete [] m_componentsArray;

}
//===========================================================================
// Name     : construct
// Arguments: const string & componentName
// Purpose  : Creates the component from the definition file.
// Notes    : Creating a component means creating each of the components from
//            which it is constructed and "wiring" then together.
//            The sub-components may themselves be complex components and so the
//            build process is iterative although that does not concern this
//            method. Normally a circuit is conceived as one mammoth complex
//            component. This component is constructed and in the process of
//            that all the individual components will also have been constructed.
//            This is called the component make.
//            Each component is defined in a single file with the extension .cmp
//            The files are located in the components directory.
//
//            The file contains the definition of the subcomponents and how they
//            are wired together.
//
//            There are essentially two kinds of entry:
//            1)   component definition
//            2)   wiring
//
//            Comment lines start with a '#'. Currently comments must be on individual lines
//            not following component definitions or wiring.
//
//            A component definition line looks like: AND_GATE=and
//
//            Some built-in components take arguments. For instance, to define an OR_GATE
//            with 5 inputs, the following syntax is used:
//              OR_GATE=or(5)
//            where the text before the = is the type of component (either a built-in
//            or one defined in a component definition file. The name following the 
//            equals is a unique identifier for the particular component built. (See
//            following comment about sections)
//
//            A wiring definition looks like: comp1:0 = comp2:1
//
//            This connects the 1st output of component 'comp2' with the first input of
//            component 'comp1'. The input side is before the =, the output after it.
//            Note it is the presence of a colon before the =
//            that distinguishes a wiring definition from a component definition.
//
//            It is very important that the input pin of each component can only be fed
//            from the one output pin of one other component. Doing otherwise can result
//            in a short-circuit! Translated this means that each input pin for each 
//            sub-component can only appear once on the left of an equals sign!
//            
//            Component and wiring definitions can be scattered through the file in any
//            order but it is an error if a component is used in a wiring definition that
//            has not been defined.
//
//            In addition to connected sub-components to each other it is possible to connect
//            them to the input or output pins of the component itself.
//
//            To connect a pin of a sub-component with a pin of the main component the syntax
//            is: 
//               comp1:3 = INPUT:1
//            This connects the 3rd input pin of 'comp1' to the 1st input pin of the main
//            component. This looks like it breaks the rule of "inputs on the left" but it
//            doesn't because conceptually we are connecting the sub-component to the output
//            of the input pin.
//
//            It is also possible to connect a component to the logically inverted input. This
//            saves the use of additional INVERTOR components to achieve this:
//              comp1:3 = INVERT:1
//
//            It is also possible to connect a component to the voltage rail or to ground
//            directly in the following way:
//              comp1:4 = HIGH or comp1:4 = LOW
//
//            To connect a sub-component to the output of the main component:
//              OUTPUT:3 = comp2:1
//            i.e the input to the 3rd output pin is connected to the 1st output of comp2.
//
//            SECTIONS
//            
//            The file can be divided into sections using the following syntax.
//            SECTION=<section_name>
//            .....
//            .....
//            END_SECTION.
//
//            Sections can be located within other sections. A component defined within a
//            section can only be used in a wiring definition within the same section (but
//            can be used in sub-sections of that section).
//            The uniqueness of component names is therefore only required within sections.
//            Components can be given identical names if they are defined in different
//            sections. This can be useful in long files to avoid having to give each 
//            of the many AND_GATEs meaningful unique names!
//
//            Internally the name given to a component in a component definition is pre-prended
//            with the name of the section(s) where it is defined starting with . for the
//            main section - i.e. .section1.section1-a.comp1
//
//            When a component name is used in a wiring definition the software looks first
//            for a component name defined in the same section, then for one in the parent
//            section until the first match is found.
//
//            Error checking:
//            The error checking within the method is now quite robust. Every error that I
//            could think of is detected and flagged to cerr. The construction is then stopped
//            and false returned.
//
//            This is a very long method which should be split up eventually!
//===========================================================================

bool CIRcomplexComponent::construct(const string & componentName)
{
   // Checks that a name is supplied
   if (componentName.empty()) {
     cerr << "Error: No name for component. Can't build." << endl;
     return false;
   }

   // Checks that the correspondng definition file can be opened.
   ifstream inputStream;
   string filename = "components/" + componentName + ".cmp";
   inputStream.open(filename.c_str());

   if (!inputStream) {
     cerr << "Error: Can't open file " << filename << " for component " << componentName << "." << m_componentId << endl;
     return false;
   }

  // Now we can get on with the make

   bool wiring     = false; // start off assuming the line is not a wiring definition
   int  sectionDepth  = 0;  // we start with section depth 0 - i.e. at top of section tree.
   string fullSectionName = ""; // with section name ""

   // These variables are used in the error detection
   unsigned int lineNumber = 0;
   bool error = false;
   string errorText = "";

   // This map is used to ensure that there are not multiple connections to inputs
   map<string, int> uniqueInputMap;

   string line;
   // Iterate through the file reading one line at a time.
   while (getline(inputStream, line) && !error) {
    lineNumber++;
    // Any line with a '#' is a comment!
    if (( line.find('#') != string::npos) || line.empty()) continue;

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Recogise section definitions, calculate the section depth and the
    // full section name to prepend to component definitions.
    if ((line.find("END_SECTION", 0) != string::npos)){

      unsigned int dotPos = fullSectionName.find_last_of(".");
      fullSectionName = fullSectionName.substr(0, dotPos);
      sectionDepth--;

      if (sectionDepth < 0) {
        error = true;
        errorText = "Too many SECTION_ENDs";
      }

      continue;
    } else if ((line.find("SECTION", 0) != string::npos)) {
      unsigned int equalPos = line.find("=");
      if (equalPos == string::npos) {
        error = true;
        errorText = "Section name invalid";
        continue;
      }
      string sectionName = strip(line.substr(equalPos+1));
      fullSectionName = fullSectionName + "." + sectionName;
      sectionDepth++;
      continue;
    } 
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // By this stage lines should either be component or wiring definitions.
    // They can be distinguised because wiring definitions always have a ':'
    // before the '='
 
    unsigned int equalPos = line.find("=");
    unsigned int colonPos = line.find(":");

    if ((equalPos == string::npos) && (colonPos == string::npos)) {
        error = true;
        errorText = "Line is neither component nor wiring definition.";
        continue;
    }

    // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // This should be a component definition
    if  (equalPos < colonPos) { 
      // find the equals

      vector<string> arg_list; // This will be a list of arguments used when making the component
      unsigned int equalPos = line.find("=");

      if (equalPos == string::npos) {
       error = true;
       errorText = "Line is neither component nor wiring definition.";
       continue;
      }
      string componentName = strip(line.substr(0, equalPos));
      string componentId   = line.substr(equalPos + 1);

      unsigned int oBracketPos = componentId.find("("); // some components take arguments
      unsigned int cBracketPos = componentId.find(")"); // in brackes following the name

      if ((oBracketPos != string::npos) && (cBracketPos != string::npos) && ( oBracketPos < cBracketPos)) {
        // Arguments are separated by commas
        string args = componentId.substr(oBracketPos+1, cBracketPos-oBracketPos-1);
        unsigned int commaPos = 0;

        while ( (commaPos = args.find(",")) != string::npos) {

          string part = strip(args.substr(0, commaPos));
          args = args.substr(commaPos + 1);
          arg_list.push_back(part); 
        }
        arg_list.push_back(strip(args));

      }
      componentId = strip(componentId.substr(0, oBracketPos)); //remove arguments from id

      // prepend the section to the component name
      if (!fullSectionName.empty())
        componentId = fullSectionName + "." + strip(componentId.substr(0, oBracketPos));

      // Check that component does not already exist
      if (m_componentsMap.find(componentId) != m_componentsMap.end()) {
        error = true;
        errorText = "Component name \"" + componentId + "\" already exists";
        continue;
      }
      // Go ahead and make the component.
      unsigned int index = m_factory->makeComponent(componentName, componentId, arg_list, getId(), error);

      if (!error) m_componentsMap[componentId] = index;

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // End of component definition.
    // This must now be a wiring definition.
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // connect them up
    } else  {  // We have a wiring definition
       wiring = true;
       // find the equals
       unsigned int equalPos = line.find("=");

       if (equalPos == string::npos) {
         error = true;
         errorText = "Invalid wiring definition";
         continue;
       }

       // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

       string inputSide = strip(line.substr(0, equalPos));
       string outputSide   = strip(line.substr(equalPos + 1)); 

       // deal with the input side - i.e. left side
       bool connectToOutput = false; // Could connect to an OUTPUT pin
       unsigned int inputNumber = 0;
       string inputName="";

       unsigned int pos;
       if ((pos = inputSide.find("OUTPUT")) != string::npos) {
         connectToOutput = true;
         // look for _
         istringstream iS(inputSide.substr(7));
         iS >> inputNumber;

       } else {
         // look for a dot
         if ((pos = inputSide.find(':')) == string::npos) {

           error = true;
           errorText = "Unable to find input pin number";
           continue;
         }
         inputName = strip(inputSide.substr(0, pos));
         istringstream iS(inputSide.substr(pos+1));
         iS >> inputNumber;
         
       } 
       // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       // deal with the output side - i.e. right side

       bool connectToInput = false; // Could connect to INPUT pin
       bool invertInput = false;    // Could connect to inverse of INPUT pin
       bool hasFixedValue = false;  // Could connect to HIGH/LOW
       bool fixedValue = false;
       unsigned int outputNumber = 0;
       string outputName="";

       if ((pos = outputSide.find("INPUT")) != string::npos) {
         connectToInput = true;
         // look for _
         istringstream iS(outputSide.substr(6));
         iS >> outputNumber;

       } else if ((pos = outputSide.find("INVERT")) != string::npos) {
         connectToInput = true;
         // look for _
         istringstream iS(outputSide.substr(7));
         iS >> outputNumber;
         invertInput = true;
       } else if ((pos = outputSide.find("HIGH")) != string::npos) {
         hasFixedValue = true;
        fixedValue = true;
       } else if ((pos = outputSide.find("LOW")) != string::npos) {
         hasFixedValue = true;
         fixedValue = false;
       } else { // This is the normal case. Connect to a pin of another component
         // look for a colon
         if ((pos = outputSide.find(':')) == string::npos) {
           error = true;
           errorText = "Unable to find output pin number";
           continue;
         }
         outputName = strip(outputSide.substr(0, pos));

         istringstream iS(outputSide.substr(pos+1));
         iS >> outputNumber;

       }
       // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++

       // Now find the full name of each component
       if (outputName != "") {

         outputName = getFullComponentName(fullSectionName, outputName);

         if (!(m_componentsMap.find(outputName) != m_componentsMap.end())) {
           error = true;
           errorText = "Could not find match for output component " + outputName;
           continue;
         }
       }

       if (inputName != "") {
         inputName = getFullComponentName(fullSectionName, inputName);
         if (!(m_componentsMap.find(inputName) != m_componentsMap.end())) {
           error = true;
           errorText = "Could not find match for input component " + inputName;
           continue;
         }
       }
       // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       // Input names must be unique

       ostringstream oS;
       oS << inputName << ":" << inputNumber;

       if (uniqueInputMap.find(oS.str()) != uniqueInputMap.end()) {

         error = true;
         errorText = "Multiple inputs for " + oS.str()  ;
         continue;
       }
       uniqueInputMap[oS.str()] = lineNumber;
       //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 

       // If we have a connection to an input
       if (connectToInput) {

         if (m_componentsMap.find(inputName) != m_componentsMap.end()) {
           if (!(m_factory->getComponent(m_componentsMap[inputName]))->validInputPin(inputNumber)) {
             error = true;
             errorText = "Invalid input pin";
             continue;
           }

           pair<unsigned int, unsigned int> record(m_componentsMap[inputName],  inputNumber);
           pair<unsigned int, pair<unsigned int, unsigned int> > entry(outputNumber, record);

           if (!invertInput) {
             m_componentsRegisteredToInputPinMultiMap.insert(entry);
           } else {
             m_componentsRegisteredToInvertPinMultiMap.insert(entry);
           }
           m_componentsRegisteredToInputsList.push_back(m_componentsMap[inputName]);
           
           unsigned int pinId = 0;// m_factory->makePin(true, inputNumber, getId(), error);
           // looks wrong but it is the output of the input pin!
           m_inputPinsMap[outputNumber] = pinId; 
         } else {
            error = true;
            errorText =  "Invalid component \"" +  inputName  + "\" in wiring definition";
            continue;

         }
       // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       } else if (connectToOutput) {

         if (m_componentsMap.find(outputName) != m_componentsMap.end()) {
           if (!(m_factory->getComponent(m_componentsMap[outputName]))->validOutputPin(outputNumber)) {

             error = true;
             errorText = "Invalid output pin";
             continue;
           }

           pair<unsigned int, unsigned int> record(m_componentsMap[outputName],  outputNumber);
           pair<unsigned int, pair<unsigned int, unsigned int> > entry(inputNumber, record);
           m_componentsRegisteredToOutputPinMultiMap.insert(entry);
           m_componentsRegisteredToOutputsList.push_back(m_componentsMap[outputName]);
           
           unsigned int pinId = 0;//m_factory->makePin(false, outputNumber, getId(), error);
           // looks wrong but it is the input of the output pin!
           m_outputPinsMap[inputNumber] = pinId;

          } else {
            error = true;
            errorText =  "Invalid component \"" +  outputName  + "\" in wiring definition";
            continue;
         }
       // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       } else if (hasFixedValue) {
         map<string, unsigned int>::iterator it = m_componentsMap.find(inputName);

         if (it != m_componentsMap.end()) {
         int componentIndex = (*it).second;
         if (!(m_factory->getComponent(componentIndex))->validInputPin(inputNumber)) {

             error = true;
             errorText = "Invalid fixed input pin";
             continue;
           }

         m_factory->getComponent(componentIndex)->setInput(inputNumber, fixedValue);

// Set pin
         } else {
            error = true;
            errorText =  "Invalid component \"" +  inputName  + "\" in wiring definition";
            continue;
         }

       // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       } else {
         if ((m_componentsMap.find(outputName) != m_componentsMap.end()) ||
             (m_componentsMap.find(inputName) != m_componentsMap.end())) {
           // Check that the pins exist
           if (!(m_factory->getComponent(m_componentsMap[outputName]))->validOutputPin(outputNumber)) {
             error = true;
             errorText = "Invalid output pin";
             continue;
           }
           if (!(m_factory->getComponent(m_componentsMap[inputName]))->validInputPin(inputNumber)) {
             error = true;
             errorText = "Invalid input pin";
             continue;
           }
           // Then connect them up. This line is what the method is all about!!!
           if (!m_factory->connectFromTo(m_componentsMap[outputName], outputNumber,
                                                          m_componentsMap[inputName],  inputNumber)) {
            error = true;
            errorText =  "Unable to establish connection";
            continue;
           } 
         } else {
            error = true;
            errorText =  "Invalid component \"" +  inputName + "\" or \"" + outputName  + "\" in wiring definition";
            continue;
         }
       }
     }
   }

   inputStream.close();

   if (error) {
     cerr << "Error constructing component: " << componentName << " at line: " << lineNumber << ". Error text = " << errorText << endl; 

   }
   m_componentsRegisteredToInputsList.sort();
   m_componentsRegisteredToInputsList.unique();
   m_componentsRegisteredToOutputsList.sort();
   m_componentsRegisteredToOutputsList.unique();

   m_good = (m_componentsMap.size() > 0) && wiring && (!error);

   return m_good;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : getFullComponentName
// Arguments: const string & fullSectionName, const string & name
// Purpose  : Creates the full component name by searching for the
//            name in the next section upwards and upwards until found.
// Notes    : This is the one concession to keeping the above method short!
//
//===========================================================================

const string CIRcomplexComponent::getFullComponentName(const string & fullSectionName,
                                                       const string & name)
{
  string fullname = name;
  unsigned int dotPos;
  string tmp_str = fullSectionName + ".";
  while ( (dotPos = tmp_str.find_last_of(".")) != string::npos) {

    string tmpName = tmp_str.substr(0, dotPos+1) + name;

    if (m_componentsMap.find(tmpName) != m_componentsMap.end()) {
      fullname = tmpName;
      break;
    }
    tmp_str = tmp_str.substr(0, dotPos);
  }
  return fullname;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : finishedMake
// Arguments: None.
// Purpose  : This performs the same job as the same method in the parent class.
//            Once the build of the components has been finished all the information
//            stored in STL containers can be converted into arrays for a faster runtime.
//           
// Notes    : Unless this step is performed the component is not usable.
//
//===========================================================================

bool CIRcomplexComponent::finishedMake()
{
  
  // Call the parent method which completes the make
  CIRabstractComponent::finishedMake();

  // LISTS OF COMPONENTS REGISTERED TO INPUT AND OUTPUT

  // This is an array of sub-components registered to the input pins
  m_componentsRegisteredToInputsArray = new unsigned int [m_componentsRegisteredToInputsList.size()+1];

  list<unsigned int>::iterator compIt = m_componentsRegisteredToInputsList.begin();
  m_componentsRegisteredToInputsArray[0] = m_componentsRegisteredToInputsList.size();

  unsigned int index = 1;
  while (compIt != m_componentsRegisteredToInputsList.end()) {
  
    unsigned int componentIndex = *compIt;
    m_componentsRegisteredToInputsArray[index] = componentIndex;
    compIt++; index++;
  }

  // This is an array of sub-components registered to the output pins
  m_componentsRegisteredToOutputsArray = new unsigned int [m_componentsRegisteredToOutputsList.size()+1];

  m_componentsRegisteredToOutputsArray[0] = m_componentsRegisteredToOutputsList.size();
  compIt = m_componentsRegisteredToOutputsList.begin();
  index = 1;

  while (compIt != m_componentsRegisteredToOutputsList.end()) {

    unsigned int componentIndex = *compIt;
    m_componentsRegisteredToOutputsArray[index] = componentIndex;
    compIt++; index++;
  }
 
  // INPUT, OUTPUT & COMPONENTS MAP

  // This is an array of all components
  m_componentsArray = new unsigned int [m_componentsMap.size() + 1];
  m_componentsArray[0] = m_componentsMap.size();

  map<string, unsigned int>::iterator compMapIt = m_componentsMap.begin();

  for (index = 0; index < m_componentsMap.size() ; index++) {
    m_componentsArray[index+1] = (*compMapIt).second;
    compMapIt++;
  }

  // This is an array of components registered to the output pins. 

  // This is a complex array of pointers to pointers.
  // The first index is the number of the output pin labelled sequentially 0-n
  // Then follows an array of components registered to that output. Two integers
  // per entry. One for the component id, one for the input number
  
   unsigned int numberKeys = m_outputPinsArray[0];

   m_componentsRegisteredToOutputPinArray = new unsigned int * [numberKeys];

   for (unsigned int i = 0; i < numberKeys ; i++) {
     unsigned int key = m_outputPinsArray[i+1];
     unsigned int numberItems = m_componentsRegisteredToOutputPinMultiMap.count(key);

     m_componentsRegisteredToOutputPinArray[i] = new unsigned int [2*numberItems + 1];
     m_componentsRegisteredToOutputPinArray[i][0] = numberItems;

     pair < multimap<unsigned int, pair<unsigned int, unsigned int> >::iterator,
      multimap<unsigned int, pair<unsigned int, unsigned int> >::iterator> ii =
         m_componentsRegisteredToOutputPinMultiMap.equal_range(key);

     unsigned int index = 1;
     multimap<unsigned int, pair<unsigned int, unsigned int> >::iterator it = ii.first;
     while (it != ii.second) {
       pair<unsigned int, unsigned int> entry = (*it).second;

       unsigned int compIndex = ((*it).second).first;
       unsigned int outputNumber = ((*it).second).second;

       m_componentsRegisteredToOutputPinArray[i][index] = compIndex;
       m_componentsRegisteredToOutputPinArray[i][index+1] = outputNumber;

       it++; index++; index++;
     }
   }

  // This is the same for the input pins.

   numberKeys = m_inputPinsArray[0];

   m_componentsRegisteredToInputPinArray = new unsigned int * [numberKeys];

   for (unsigned int i = 0; i < numberKeys ; i++) {

     unsigned int key = m_inputPinsArray[i+1];
     unsigned int numberItems = m_componentsRegisteredToInputPinMultiMap.count(key);
     m_componentsRegisteredToInputPinArray[i] = new unsigned int [2*numberItems + 1];
     m_componentsRegisteredToInputPinArray[i][0] = numberItems;

     pair < multimap<unsigned int, pair<unsigned int, unsigned int> >::iterator,
      multimap<unsigned int, pair<unsigned int, unsigned int> >::iterator> ii = m_componentsRegisteredToInputPinMultiMap.equal_range(key);

     unsigned int index = 1;
     multimap<unsigned int, pair<unsigned int, unsigned int> >::iterator it = ii.first;
     while (it != ii.second) {
       pair<unsigned int, unsigned int> entry = (*it).second;

       unsigned int compIndex = ((*it).second).first;
       unsigned int inputNumber = ((*it).second).second;

       m_componentsRegisteredToInputPinArray[i][index] = compIndex;
       m_componentsRegisteredToInputPinArray[i][index+1] = inputNumber;

         it++; index++; index++;
     }
   }

  // This is the same for sub-components registered to the inverse of the input pins

   m_componentsRegisteredToInvertPinArray = new unsigned int * [numberKeys];

   for (unsigned int i = 0; i < numberKeys ; i++) {

     unsigned int key = m_inputPinsArray[i+1];

     unsigned int numberItems = m_componentsRegisteredToInvertPinMultiMap.count(key);

     m_componentsRegisteredToInvertPinArray[i] = new unsigned int [2*numberItems + 1];
     m_componentsRegisteredToInvertPinArray[i][0] = numberItems;

     pair < multimap<unsigned int, pair<unsigned int, unsigned int> >::iterator,
      multimap<unsigned int, pair<unsigned int, unsigned int> >::iterator> ii = m_componentsRegisteredToInvertPinMultiMap.equal_range(key);

     unsigned int index = 1;
     multimap<unsigned int, pair<unsigned int, unsigned int> >::iterator it = ii.first;
     while (it != ii.second) {
       pair<unsigned int, unsigned int> entry = (*it).second;

       unsigned int compIndex = ((*it).second).first;
       unsigned int inputNumber = ((*it).second).second;

       m_componentsRegisteredToInvertPinArray[i][index] = compIndex;
       m_componentsRegisteredToInvertPinArray[i][index+1] = inputNumber;

         it++; index++; index++;
     }
   }
  return true;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : computeOutputs
// Arguments: None.
// Purpose  : This computs the outputs of a complex component.
//
// Notes    : Performs the following steps:
//            1) First we tell the subcomponents to remember their input
//            2) Then we set the input on all the components connected to the input pins
//            3) Then we iterate through components asking them if their inputs have changed.
//               If so they must notify their outputs and remember their inputs again
//               We iterate until all subcomponents are stable
//            4) Then we set the bits on the output pins
// 
//               The number of iterations is limited by the variable 
//               "MAX_STABILITY_ITERATIONS" to stop the component from exploding!!

//
//===========================================================================
bool CIRcomplexComponent::computeOutputs() 
{
  if (!m_finished) return false; // Only continue if the make has finished

  // first clear all the output bits
  outputsToPins(0, 0);

  // iterate through all components to compute their outputs

  // Step 1
  unsigned int size = m_componentsArray[0];


  for (unsigned int index = 0 ; index < size ; index++) {
    unsigned int componentIndex = m_componentsArray[index+1];
    m_factory->getComponent(componentIndex)->rememberInputs();
  }

  // Step 2
  setComponentInputs();

  // Step 3

  if (!computeComponentOutputs()) {
    cerr << "Error: Components of " <<  m_componentId << 
             " did not reach stability in reasonable time" << endl;
    return false;
  }

  // Step 4

  setOutputBits();

  return true;

}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : setComponentInputs
// Arguments: None.
// Purpose  : This supplies the inputs from the input pins to the sub-components
//            connected to them.
//
// Notes    : 

//
//===========================================================================

//
void CIRcomplexComponent::setComponentInputs()
{
  if (!m_finished) return;
  // Do this in three stages, first we tell components to expect inputs ....

  unsigned int numComponents = m_componentsRegisteredToInputsArray[0];
  unsigned int numberInputs = m_inputPinsArray[0];

  for (unsigned int i = 0; i < numberInputs ; i++) {
    unsigned int inputNumber = m_inputPinsArray[i+1];

    // INPUT

    unsigned int numItems = m_componentsRegisteredToInputPinArray[i][0];

    for (unsigned int j = 0; j < numItems ; j++) {

      unsigned int componentIndex = m_componentsRegisteredToInputPinArray[i][2*j + 1];
      unsigned int componentInputNumber = m_componentsRegisteredToInputPinArray[i][2*j + 2];

      CIRabstractComponent * comp = m_factory->getComponent(componentIndex);
      comp->setInput( componentInputNumber, m_inputPinArray->getSignal(inputNumber));

    }

    // INVERT
    numItems = m_componentsRegisteredToInvertPinArray[i][0];

    for (unsigned int j = 0; j < numItems ; j++) {

      unsigned int componentIndex = m_componentsRegisteredToInvertPinArray[i][2*j + 1];
      unsigned int componentInputNumber = m_componentsRegisteredToInvertPinArray[i][2*j + 2];

      CIRabstractComponent*  comp = m_factory->getComponent(componentIndex);
      comp-> setInput( componentInputNumber, !m_inputPinArray->getSignal(inputNumber));

    }
  }
  // Tell all sub-components that inputs have finished
  // This is a trigger for them to calculate their own outputs.
  for (unsigned int i = 0 ; i < numComponents ; i++) {
    unsigned int componentIndex = m_componentsRegisteredToInputsArray[i+1];
    m_factory->getComponent(componentIndex)->endInputs();
  }
}

//--------------------------------------------------------------------------
//===========================================================================
// Name     : computeComponentOutputs
// Arguments: None.
// Purpose  : This calculate the outputs of the main component
//
// Notes    : This is an iterative process. We loop round all the sub-components
//            If their inputs have changed then they should notify the components
//            connected to them of the outputs.
//            The notifyOutputs method sets the inputs on the connected components
//            and triggers a recalculation of the outputs. This may change the 
//            inputs of a sub-component which was previously stable so we iterate
//            around until all sub-components are stable - i.e have not changed
//            value.
//            Badly defined components may never reach stability. We duck out
//            if we have to iterate too many times.
//
//===========================================================================

bool CIRcomplexComponent::computeComponentOutputs()
{
  if (!m_finished) return false;
  bool componentsStable = false;
  unsigned int numberIterations = 0;

  while (!componentsStable) {

    if (numberIterations++ > MAX_STABILITY_ITERATIONS) return false;

    componentsStable = true; // We'll set to false when we find they aren't

    unsigned int numberComponents = m_componentsArray[0];

    for (unsigned int i = 0; i < numberComponents ; i++) {

      unsigned int componentIndex = m_componentsArray[i+1];
      CIRabstractComponent * comp = m_factory->getComponent(componentIndex);

      if (comp->inputsChanged()) {
        comp->notifyOutputs();
        comp->rememberInputs();
        componentsStable = false;
      }
    }

  }
  return true;
}
//--------------------------------------------------------------------------
//===========================================================================
// Name     : setOutputBits
// Arguments: None.
// Purpose  : This applies the signal to the output pins.
//
// Notes    : 
//
//===========================================================================

void CIRcomplexComponent::setOutputBits()
{
  if (!m_finished) return;
  unsigned int numberOutputs = m_outputPinsArray[0];

  for (unsigned int i = 0; i < numberOutputs ; i++) {  

    unsigned int pinNumber = m_outputPinsArray[1+i];
    unsigned int numItems = m_componentsRegisteredToOutputPinArray[i][0];
    for (unsigned int j = 0 ; j < numItems ; j++) {

      unsigned int componentIndex = m_componentsRegisteredToOutputPinArray[i][2*j + 1];
      unsigned int componentOutputNumber = m_componentsRegisteredToOutputPinArray[i][2*j + 2];
      bool output = m_factory->
           getComponent(componentIndex)->getOutput(componentOutputNumber);

      m_outputPinArray->applySignal(pinNumber, output);
    }
  }
}
//--------------------------------------------------------------------------
