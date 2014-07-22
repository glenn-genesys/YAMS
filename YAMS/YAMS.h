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
#include <AnalogButtons.h>
#include <LiquidCrystal.h>

#define MENU_TIMEOUT 30000

/*#ifdef __cplusplus
extern "C" {
#endif */
// extern doesn't like templates

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
	// Standard output methods
	// Use one of these, create a new one (for a new type of display)
	// Or supply a replacement function to 'display' function pointer
	virtual void serialDisplay();
	virtual void LCDdisplay();

	// Standard menu control
	// Use one of these, create a new one (for a new type of input)
	// Or supply a replacement function to 'processInput' function pointer
	virtual Menu *serialProcessInput();
	virtual Menu *keypadProcInput();

	// Standard serial input method
	// Use one of these, create a new one (for a new type of character input)
	// Or supply a replacement function to 'getInput' function pointer
	virtual const char  serialInput();

  /*
   * General purpose constructor.
   * Optionally supply an alternative display function, menu processing function
   * or
   */
	Menu(const char *n,
		 bool loop = true,
		 void (*dis)(Menu &m) = NULL,
		 Menu* (*procin)(Menu &m) = NULL,
		 const char (*in)(Menu &m) = NULL);

  /*
   * Constructor to set input and output modes (eg. LCDOUT and KEYIN)
   * Must call setLCD and setKeypad to initialise, if used
   */
    Menu(const char *n,
       const bool loop,
       const OutputType out,
       const InputType  in);

  /*
   * All-in one constructor to use LCD for output and Keypad for input
   */
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

  // onExit is called when leaving this menu, up, down, sideways or quitting	
	virtual void onExit() {;}

	void setLCD( LiquidCrystal &l  );
	void setKeypad( AnalogButtons &k );

		// Menu construction
	Menu &addChild( Menu &c);
	Menu &addSibling( Menu &c, bool loop);   // Adds siblings in a loop if loop is true (ignores parent.loop)

  void showStructure(bool full);    // Displays the structure of the menu
};

template <class N>
class MenuValue : public Menu {
protected:
	N value, minv, maxv, inc;
	bool selected;  // When MenuValue is selected, up and down adjust value

public:
	MenuValue(const char *n,
			  N val,
			  N lowerbound,
			  N upperbound,
			  N inc,
				 void (*dis)(Menu &m) = NULL,
				 Menu* (*procin)(Menu &m) = NULL,
				 const char (*in)(Menu &m) = NULL);

	MenuValue();
	virtual ~MenuValue();

	N getValue();
	void setValue(N v);

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
  int length;
  bool  selected;
  int index;
  
public:
  MenuArray(const char *n,
		  float vs[],
		  int len,
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

/* class MenuValue : public Menu {
protected:
	N value, minv, maxv, inc;
	bool selected;  // When MenuValue is selected, up and down adjust value */
template <class N>
MenuValue<N>::MenuValue(const char *n,
		  N v,
		  N lb,
		  N ub,
		  N incr,
		  void (*dis)(Menu &m),
		  Menu* (*procin)(Menu &m),
		  const char (*in)(Menu &m)) {
	name = n;
	getInput = in;
	processInput = procin;
	display = dis;
	current = this;
	value = v;
	minv = lb;
	maxv = ub;
	inc = incr;
	selected = false;
}

template <class N>
MenuValue<N>::MenuValue() {
	name = NULL;
	getInput = NULL;
	processInput = NULL;
	display = NULL;
	current = this;
	value = minv = maxv = inc = 0;
	selected = false;
}

template <class N>
MenuValue<N>::~MenuValue() {;}

template <class N>
void MenuValue<N>::LCDdisplay() {
	lcd->clear();
	if (selected) {
		lcd->print(F("Set ") );
		lcd->print(name);
		lcd->print(": ");
		lcd->setCursor(0, 1);
		lcd->print(value);
	} else {
		lcd->print(name);
		lcd->print(" = ");
		lcd->setCursor(0, 1);
		lcd->print(value);
	}
}

template <class N>
Menu *MenuValue<N>::up() {
	if (!selected)
		return getif(prev);
	else {
		value = min(maxv, value+inc);
		return this;
	}
}

template <class N>
Menu *MenuValue<N>::down() {
	if (!selected)
		return getif(next);
	else {
		value = max(minv, value-inc);
		return this;
	}
}

template <class N>
Menu *MenuValue<N>::left() {
	if (!selected) {
		return getif(parent);
	} else {
		selected = false;
		return this;
	}
}

template <class N>
Menu *MenuValue<N>::right() {
	selected = true;
	return this;
}

template <class N>
Menu *MenuValue<N>::back() {
	selected = false;
	return getif(last);
}

template <class N>
N MenuValue<N>::getValue() {
	return value;
}
template <class N>
void MenuValue<N>::setValue(N v) {
	value = v;
}

class MenuList : public Menu {
protected:
	const char ** values;
	int length;
	int selectedIndex;
	bool selected;  // When MenuList is selected, up and down cycle through values

public:
	 MenuList (const char *n,
    		   const char * vs[],
    		   const int len,
    			  int index,
				  void (*dis)(Menu &m) = NULL,
				  Menu* (*procin)(Menu &m) = NULL,
				  const char (*in)(Menu &m) = NULL);

	 MenuList ();
	virtual ~ MenuList ();

	const char *getValue();

	virtual void setIndex(int index);
	int getIndex();

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

/*
#ifdef __cplusplus
} // extern "C"
#endif */

#endif /* YAMS_H_ */
