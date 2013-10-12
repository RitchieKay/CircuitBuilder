#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <ncurses.h>
#include <string>
#include <list>

#include <pthread.h>

#include "CIRcomponentFactory.h"
#include "CIRbuiltInComponent.h"

using namespace std;

  CIRcomponentFactory * factory = NULL;
  unsigned int input = 0;
  unsigned int output1 = 0;
  unsigned int output2 = 0;
  string outputString;
  unsigned int mainComponent = 0;
  int titleLine;
  int inputLine;
  int outputLine;
  int commandLine;
  int infoLine;
  int detailLine;
  string componentName;
  void clearLine(WINDOW * win, const int line);
  void showPrompt();
  void writeInputLine();
  void writeOutputLine();
  void writeInputDetail();
  void doLoop();
  bool loadComponent();
  void showInputOutput();

WINDOW *create_newwin(int height, int width, int starty, int startx);

WINDOW *output_win, *command_win;
void *dataLoop(void * arg)
{
// ofstream oS;
// oS.open("ComponentOutput.dat", ios::out);

 const unsigned int milli = 20;

 list<unsigned int> inputs = factory->getComponent(mainComponent)->getInputPins();

 struct timespec time;
 time.tv_sec = 0;        /* seconds */
 time.tv_nsec = milli * 100;       /* nanoseconds */


 for(;;) {


   nanosleep(&time, NULL);
   list<unsigned int>::iterator it = inputs.begin();

   while (it != inputs.end()) {

     int mask = 1 << (*it);

     factory->getComponent(mainComponent)->setInput((*it), (input & mask)/mask);
     it++;
   }

   it = inputs.begin();

   factory->getComponent(mainComponent)->endInputs();


   list<unsigned int> outputs = factory->getComponent(mainComponent)->getOutputPins();
   list<unsigned int>::iterator outIt = outputs.begin();
//   cout << "-----------------------------------------------------" << endl << endl;
//   cout << "Component " << componentName << " has " << dec << outputs.size() << " output pins. Values are :" << endl ;
    unsigned int hexOutput1 = 0;
    unsigned int hexOutput2 = 0;
    ostringstream oS;
    while (outIt != outputs.end()) {
     unsigned int pinOutput = factory->getComponent(mainComponent)->getOutput(*outIt);
      oS <<  pinOutput ;

      if ((*outIt) < 32) {

        hexOutput1 += (pinOutput << (*outIt)) ;

     } else {
        hexOutput2 += (pinOutput << ((*outIt)-32)) ;

     }
      outIt++;
    }
    output1 = hexOutput1;
    output2 = hexOutput2;
    outputString = oS.str();
  } 
//  oS.close();
  return NULL;
}

void initDisplay()
{
  initscr() ;
 keypad(stdscr, TRUE);
 nonl();
 cbreak();
 noecho();

 titleLine = 0;
 inputLine = 2;
 outputLine = 4;
 commandLine = LINES-20;
 infoLine = 2;
 detailLine = 5;

// if (has_colors()) {
//   start_color();
//   init_pair(COLOR_GREEN, COLOR_GREEN, COLOR_BLACK);
//   attron(COLOR_PAIR(COLOR_GREEN));
// }
	int startx, starty, width, height;

  starty = 0;	/* Calculating for a center placement */
  startx = 0;	/* of the window		*/
  refresh();
  output_win = create_newwin(10, COLS, starty, startx);
  command_win = create_newwin(LINES-16, COLS, starty+10, startx);

//  wattron(output_win, COLOR_PAIR(COLOR_GREEN));
//  wattron(command_win, COLOR_PAIR(COLOR_GREEN));

 mvprintw(titleLine,   COLS/2 - 10, "CIRCUIT GENERATOR v1.0 ");
 mvprintw(titleLine+1, COLS/2 - 18, "=======================================");

 mvwprintw(output_win, outputLine, 1, "Current output = ");
 mvwprintw(output_win, inputLine,  1, "Current input  = ");
 wrefresh(output_win);

 mvwprintw(command_win, infoLine,   1, "Available commands: i (change inputs byte by byte)");
 mvwprintw(command_win, infoLine+1, 1, "                  : f (flip single input bit)");
 mvwprintw(command_win, infoLine+2, 1, "                  : w (change whole input)");
 mvwprintw(command_win, infoLine+3, 1, "                  : l (load new component)");
 mvwprintw(command_win, infoLine+4, 1, "                  : x (exit)");

 wrefresh(command_win);

 showPrompt();  


}

void exitDisplay()
{
  endwin();
}
int main()
{

    factory = CIRcomponentFactory::instance();
    initDisplay();

    doLoop();

    exitDisplay();

}
void doLoop()
{
 for (;;) {

   halfdelay(1);
   
   int c = wgetch(command_win);
   char str[80];
   echo();
   if (c == 'i') {
     mvwprintw(command_win, commandLine, 1, "Enter byte to change : ");
     wgetstr(command_win, str);
     if (str[0] != 'x') {
       int byte = atoi(str);
       clearLine(command_win, commandLine);
       mvwprintw(command_win, commandLine ,1, "Enter new value (hex):  ");
       wgetstr(command_win, str);
       if (str[0] != 'x') {
         unsigned int value;
         stringstream iS(str);
         iS >> hex >> value;
         union {
           unsigned int i;
           unsigned char c[4];
         };
         i = input;
         c[byte] = value;
         input = i;
       }  
     }
   } else if (c == 'x') {
     break;
   } else if (c == 'f') {
     mvwprintw(command_win, commandLine ,1, "Enter bit to flip:  ");
     wgetstr(command_win, str);
     if (str[0] != 'x') {
       stringstream iS(str);
       int bit;
       iS >> dec >> bit;
       input ^= (1 << bit);
     }
   } else if (c == 'w') {
     mvwprintw(command_win, commandLine ,1, "Enter new input value (hex):  ");
     wgetstr(command_win, str);
     if (str[0] != 'x') {
       stringstream iS(str);
       iS >> hex >> input;
     }
   } else if (c == 'l') {
     mvwprintw(command_win, commandLine ,1, "Enter component name:  ");
     wrefresh(command_win);
     wgetstr(command_win, str);
     if (str[0] != 'x') {
       componentName = str;
       if (loadComponent()) {
         string s = "Loaded component " + componentName;
         mvwprintw(output_win, detailLine + 5, 1, s.c_str());
         writeInputDetail();
       } else {
         mvwprintw(output_win, detailLine + 5, 1, "Failed to load component");
       }
       wrefresh(output_win);
     }
   }
   clearLine(command_win, commandLine);
   noecho();
   showInputOutput(); 
   showPrompt();
 }

}

bool loadComponent()
{

  bool error = false;
  vector<string> args;
  factory->clearAll();

  mainComponent = factory->makeComponent(componentName, "mainComponent", args, 0, error);
 

  if (error || !factory->getComponent(mainComponent)->isGood()) {
    return false;
  }
  factory->finishedMake();

  pthread_t t;
  int rc = pthread_create(&t, NULL, dataLoop, NULL);

//        factory->dumpComponents();
//        factory->dumpPins();


  return true;

}

void writeInputDetail()
{
  list<unsigned int> inputs = factory->getComponent(mainComponent)->getInputPins();

  list<unsigned int>::iterator it = inputs.begin();

  clearLine(output_win, detailLine+2);
  clearLine(output_win, detailLine+3);

  ostringstream oS;
  oS << "Component " << componentName << " has " << inputs.size() << " input pins. They are :" ;

  mvwprintw(output_win, detailLine+2, 3, oS.str().c_str());
  oS.str("");

  int maxInput = 0;
  while (it != inputs.end()) {
    oS << *it << " "; 
    maxInput = *it;
    it++; 
   }
  mvwprintw(output_win, detailLine+3, 3, oS.str().c_str());
  oS.str("");


  wrefresh(output_win);
  oS << "Inputs have total of " << (maxInput/8) + 1 <<  "bytes." << endl;

//  mvprintw(detailLine+2, 3, oS.str().c_str());

/*          int byte = 0;

          if (((maxInput/8) + 1)  > 1) {
            cout << "Enter byte to modify (0 = LSB) : " ;
            cin >> byte;
          }

          unsigned int userInput = 0; 
          cout << "Enter inputs (hex) : ";
          cin >> hex >> userInput;


          userInput  <<= ( 8 * byte);
          unsigned int mask = 0xff << ( 8 * byte);

          mask = 0xFFFFFFFF - mask;

          unsigned int tmpInput = input & mask;
          input = tmpInput | userInput;

          cout << "Input = " << hex << input << endl;          

          // We send each input several times to mimic a constant flow of current


        }
   }

  return 1; */
}

void showInputOutput()
{
  static unsigned int oldInput = 0xbabababa;
  static unsigned int oldOutput1 = 0xbabababa;
  static unsigned int oldOutput2 = 0xbabababa;
  if (input != oldInput) {
    writeInputLine();
    oldInput = input;
  }
  if ((output1 != oldOutput1)||(output2 != oldOutput2)) {
    writeOutputLine();
    oldOutput1 = output1;
    oldOutput2 = output2;
  }


}

void clearLine(WINDOW* win, const int line)
{
  for (int i = 1 ; i < COLS-1 ; i++) {
    mvwaddch(win, line, i, ' ');
  }
  wrefresh(win);
}
void showPrompt()
{
  mvwprintw(command_win, commandLine, 1, ": ");
  wrefresh(command_win);
}
void writeInputLine()
{
  ostringstream oS;
  oS << hex << setfill('0') << setw(8) << input;

  mvwprintw(output_win, inputLine, 20, oS.str().c_str());
  wrefresh(output_win);
}
void writeOutputLine()
{
  ostringstream oS;
  oS << hex << setfill('0') << setw(8) << output1 << "  " << setfill('0') << setw(8)<< output2;
  string s = oS.str() + "  " + outputString;
  mvwprintw(output_win, outputLine, 20, s.c_str());
  wrefresh(output_win);
}
WINDOW *create_newwin(int height, int width, int starty, int startx)
{
  WINDOW *local_win;

  local_win = newwin(height, width, starty, startx);
  box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
  wrefresh(local_win);		/* Show that box 		*/

  return local_win;
}
