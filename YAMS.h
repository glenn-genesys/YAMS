/*
 * YAMS.h
 *
 *  Created on: Jun 7, 2014
 *      Author: glenn burgess
 *
 * Yet Another Menu System
 *
 * Design principles:
 * . Support hierarchical menus
 * . Class based
 * . Default display routine
 * . Default menu actions for up/down/left/right/select
 * . Override default actions in subclasses for specific menus to encapsulate routines
 * . Common menu items types for value setting etc.
 *
 * Configurable: whether menu lists loop back or not
 *
 * Example usage:
 * Menu mm("Main Menu");
 *   //beneath is list of menu items needed to build the menu
 *   Menu runningtime    = Menu("Get/Set Runtime", false);
 *
 *     MenuValue runDays      = MenuValue("Days", 0);
 *     MenuValue runHours     = MenuValue("Hours", 0);
 *
 * mm.addChild(runingTime);
 * runningTime.addChild(runDays, runHours);
 *
 */

#ifndef YAMS_H_
#define YAMS_H_

#if defined(ARDUINO) && ARDUINO >= 100
  #include <Arduino.h>
#else
  #include <WProgram.h>
#endif

#include "TimeAlarms.h"
#include "AnalogButtons.h"
#include <LiquidCrystal.h>

#define MENU_TIMEOUT 30000

#ifdef __cplusplus
extern "C" {
#endif

/* Define a common interface for displays
 * But this sucks because it means we can't use the capabilities of each display
 * such as a 2-line LCD, or serial output, in the best way for ecah display.
class MenuDisplay {
public:
  virtual print(char *message);
  virtual println(char *message);
  virtual clear();
} */

class Menu {
public:
	enum OutputType {SEROUT, LCDOUT};
	enum InputType  {SERIN, KEYIN};

protected:
	const char *name;
	// Variables that hold the data structure of menu connections
	Menu *parent;
	Menu *child;  // the first of the linked list of children
	Menu *next;   // next sibling
	Menu *prev;   // previous sibling
	bool loop;    // whether this menu's children should loop or not

	static Menu *last;        // Single entry for back button
	static Menu *current;     // Current menu location

  // Function pointers
	void  (*display)(Menu &m); // Function pointer to display method
	Menu* (*processInput)(Menu &m);  // Function pointer to input interpreter
	char const (*getInput)(Menu &m);   // Function pointer to blocking or non-blocking input function

	static OutputType output;
	static InputType input;

	static LiquidCrystal *lcd;
	static AnalogButtons *keypad;

	Menu *getif(Menu *t);

public:
	// Standard input and output methods
	virtual void serialDisplay();
	virtual void LCDdisplay();

	virtual Menu *serialProcessInput();
	virtual Menu *keypadProcInput();

	virtual const char  serialInput();

	Menu(const char *n,
		 bool loop,
		 void (*dis)(Menu &m) = NULL,
		 Menu* (*procin)(Menu &m) = NULL,
		 const char (*in)(Menu &m) = NULL);

    Menu(const char *n,
       const bool loop,
       const OutputType out,
       const InputType  in);

    Menu(const char *n,
		const bool loop,
		LiquidCrystal &l,
		AnalogButtons &k);

	Menu();

	virtual ~Menu();
	
	// Standard menu operation functions that may be overridden
	virtual Menu *left();
	virtual Menu *right();
	virtual Menu *up();
	virtual Menu *down();
	virtual Menu *select();
	virtual Menu *back();

	virtual void activate();

	void setLCD( LiquidCrystal &l  );
	void setKeypad( AnalogButtons &k );

		// Menu construction
	Menu &addChild( Menu &c);
	Menu &addSibling( Menu &c, bool loop);   // Adds siblings in a loop if loop is true (ignores parent.loop)

  void showStructure(bool full);    // Displays the structure of the menu
};

class MenuValue : public Menu {
protected:
	int value, minv, maxv;
	bool selected;  // When MenuValue is selected, up and down adjust value

public:
	MenuValue(const char *n,
			  int val,
			  int lowerbound,
			  int upperbound,
				 void (*dis)(Menu &m) = NULL,
				 Menu* (*procin)(Menu &m) = NULL,
				 const char (*in)(Menu &m) = NULL);

	MenuValue();
	virtual ~MenuValue();

	int getValue();
	void setValue(int v);

	// void serialDisplay();
	void LCDdisplay();

	// Menu *serialProcessInput();
	// Menu *keypadProcInput();

	// const char  serialInput();

	Menu *left();
	Menu *right();
	Menu *up();
	Menu *down();
	// Menu *select();
	Menu *back();
};

class MenuArray : public Menu {
protected:
  float *values;
  bool  selected;
  int index;
  
public:
  MenuArray(const char *n,
		  float vs[],
			 void (*dis)(Menu &m) = NULL,
			 Menu* (*procin)(Menu &m) = NULL,
			 const char (*in)(Menu &m) = NULL);

  MenuArray();
  virtual ~MenuArray();

  // void serialDisplay();
	void LCDdisplay();

	// Menu *serialProcessInput();
	// Menu *keypadProcInput();

	// const char  serialInput();

	Menu *left();
	Menu *right();
	Menu *up();
	Menu *down();
	Menu *select();
	// Menu *back();
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* YAMS_H_ */
