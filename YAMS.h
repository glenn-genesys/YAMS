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

class Menu {
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

  // Functions pointers
	void  (*display)(Menu &m); // Function pointer to display method
	Menu* (*processInput)(Menu &m);  // Function pointer to input interpreter
	char const (*input)(Menu &m);   // Function pointer to blocking or non-blocking input function

	static LiquidCrystal *lcd;
	static AnalogButtons *keypad;

	/* static void serialDisplay(Menu &m);   // Default display method
	static void lcdDisplay(Menu &m);
	static char serialInput(Menu &m);     // Default non-blocking function to get input
	static Menu *serialProcessInput(Menu &m); // Default input interpreter
  	static Menu *keypadProcInput(Menu &m);
	 */

	Menu *getif(Menu *t);

public:
	// Default implementations
	static void  serialDisplay(Menu &m);
	static Menu* serialProcessInput(Menu &m); // Default input interpreter
	static const char  serialInput(Menu &m);     // Default non-blocking function to get input

	static void LCDdisplay(Menu &m);
	static Menu *keypadProcInput(Menu &m);

	Menu(const char *n,
		 bool loop,
		 void (*dis)(Menu &m) = serialDisplay,
		 Menu* (*procin)(Menu &m) = serialProcessInput,
		 const char (*in)(Menu &m) = serialInput);

  /* Menu(const char *n,
       const bool loop,
       const io   parts);

	enum io;
   */

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

	void setLCD( LiquidCrystal l  );
	void setKeypad( AnalogButtons k );

		// Menu construction
	Menu &addChild( Menu &c);
	Menu &addSibling( Menu &c, bool loop);   // Adds siblings in a loop if loop is true (ignores parent.loop)

  void showStructure(bool full);    // Displays the structure of the menu
};

class MenuValue : public Menu {
protected:
	int value;
	bool selected;  // When MenuValue is selected, up and down adjust value

public:
	MenuValue(const char *n,
			  int v,
			  void (*dis)(Menu &m) = v_display,
			  Menu* (*procin)(Menu &m) = serialProcessInput,
			  const char (*in)(Menu &m) = serialInput);

	int getValue();
	void setValue(int v);

	static void v_display(Menu &mm);

	Menu *left();
	Menu *right();
	Menu *up();
	Menu *down();
	Menu *select();
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
		  void (*dis)(Menu &m) = a_display,
		  Menu* (*procin)(Menu &m) = serialProcessInput,
		  const char (*in)(Menu &m) = serialInput);

	static void a_display(Menu &mm);

	Menu *left();
	Menu *right();
	Menu *up();
	Menu *down();
	// Menu *select();
	// Menu *back();
};

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* YAMS_H_ */
