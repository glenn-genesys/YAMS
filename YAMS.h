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

#ifdef __cplusplus
extern "C" {
#endif

class withKeypad {
protected:
  AnalogButtons keypad;
  
public:
  withKeypad( AnalogButtons k );

  Menu *processInput(); // Keypad input interpreter
}

class withLCD {
protected:
  LiquidCrystal lcd;

public:
  withLCD( LiquidCrystal lcd );

  void display();
}

class withSerial {
  void display();
  Menu *processInput();  // Serial character input interpreter
}

class Menu {
protected:
	char *name;
	// Variables that hold the data structure of menu connections
	Menu *parent;
	Menu *child;  // the first of the linked list of children
	Menu *next;   // next sibling
	Menu *prev;   // previous sibling
	bool loop;    // whether this menu's children should loop or not

	static Menu *last;        // Single entry for back button
	static Menu *current;     // Current menu location

	char (*input)(Menu &m);   // Function pointer to blocking or non-blocking input function
	char (*processInput)(Menu &m);  // Function pointer to input interpreter
	void (*display)(Menu &m); // Function pointer to display method

	void _display(Menu &m);   // Default display method
	char _input(Menu &m);     // Default non-blocking function to get input
	Menu *_processInput(Menu &m); // Default input interpreter

	static Menu *getif(Menu *t);

public:
	Menu(const char *n,
		 const bool loop,
		 const void (*display)(Menu &m),
		 const Menu (*processInput)(Menu &m),
		 const char (*input)(Menu &m));

	virtual ~Menu();

	// Standard menu operation functions that may be overridden
	virtual Menu *left();
	virtual Menu *right();
	virtual Menu *up();
	virtual Menu *down();
	virtual Menu *select();
	virtual Menu *back();

	virtual void activate();

	// Menu construction
	Menu &addChild( Menu &c);
	Menu &addSibling( Menu &c, bool loop);   // Adds siblings in a loop if loop is true (ignores parent)

};

class MenuValue : Menu {
protected:
	int &value;
	bool selected = false;  // When MenuValue is selected, up and down adjust value

public:
	MenuValue(const char *n,
			  int &v,
			  const void (*display)(Menu &m),
			  const char (*processInput)(Menu &m),
			  const char (*input)(Menu &m));
};

class SerialMenu : Menu : withSerial;
class LCDMenu : Menu : withLCD;
class KeypadMenu : Menu : withKeypad;
class KeypadLCDMenu : LCDMenu : withKeypad;

class SerialMenuValue : MenuValue : withSerial;
class LCDMenuValue : MenuValue : withLCD;
class KeypadMenuValue : MenuValue : withKeypad;
class KeypadLCDMenuValue : LCDMenuValue : withKeypad;

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* YAMS_H_ */
