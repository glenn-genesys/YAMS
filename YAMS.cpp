/*
 * YAMS.cpp
 *
 *  Created on: Jun 7, 2014
 *      Author: glenn burgess
 */

#include "YAMS.h"
#include "AnalogButtons.h"
#include "Timer.h"

/* class Menu {
private:
	char *name;
	// Variables that hold the data structure of menu connections
	Menu * parent;
	Menu * child;  // the first of the linked list of children
	Menu * next;   // next sibling
	Menu * prev;   // previous sibling
	bool loop;     // whether this menu's children should loop or not

	static menu *last;
*/

Menu *Menu::last;        // Single entry for back button
Menu *Menu::current;     // Current menu location
LiquidCrystal *Menu::lcd;
AnalogButtons *Menu::keypad;

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

// Default input function is _input(). Default display method is _default()
Menu::Menu(const char *n,
		bool loop = false,
		void (*dis)(Menu &m),
		Menu* (*procin)(Menu &m),
		char const (*in)(Menu &m)) {
	name = n;
	this->loop = loop;
	input = in;
	processInput = procin;
	display = dis;
	Menu::current = this;
	parent = child = next = prev = NULL;
}

Menu::Menu() {
	name = NULL;
	this->loop = loop;
	input = NULL;
	processInput = NULL;
	display = NULL;
	parent = child = next = prev = NULL;
}

Menu::~Menu() {;}

Menu *Menu::getif(Menu *t) {
	if (t) {
		last = this;
		return t;
	} else
		return this;
}

// Standard menu functions that may be overridden
Menu *Menu::left() {
	last = this;
	return parent;   // If parent is null then exit menu
}

Menu *Menu::right() {
	return getif(child);
}

Menu *Menu::up() {
	return getif(prev);
}

Menu *Menu::down() {
	return getif(next);
}

Menu *Menu::select() {
	return right();    // Select is the same as right, by default
}

Menu *Menu::back() {
	return getif(last);
}

void Menu::activate() {
	lcd->setCursor(0,0);
	lcd->print("Boo");
	  Serial.println(freeRam());
	 showStructure(true);
	//  Serial.println(freeRam());
	
	if (current)
	do {
    	Menu::LCDdisplay(*current);

		current = Menu::keypadProcInput(*current);
	} while (current);   // NULL from process input means exit menu
}

void Menu::setLCD( LiquidCrystal l  ) {
	lcd = &l;
	display = &LCDdisplay;
}
void Menu::setKeypad( AnalogButtons k ) {
	keypad = &k;
	processInput = &keypadProcInput;
}

void Menu::serialDisplay(Menu &m) {
	Serial.println(m.name);
}

const char Menu::serialInput(Menu &m) {
	return Serial.read();
}

Menu *Menu::serialProcessInput(Menu &m) {
	switch (m.input(m)) {
	case 'u': return m.up();
	case 'd': return m.down();
	case 'l': return m.left();
	case 'r': return m.right();
	case 's': return m.right();   // Select activates child menu, just like right
	case 'b': return m.back();
	case 'q': return NULL;
	default: return &m;
	}
}

void Menu::LCDdisplay(Menu &m) {
	Serial.print(F("LCDdisplay: "));
	Serial.println(m.name);
	lcd->setCursor(0, 0);
	lcd->print(m.name);
}

Menu *Menu::keypadProcInput(Menu &m) {
  Serial.println(F("keypadProcInput"));
	Timer menuTimeout(MENU_TIMEOUT);
	do {
		Alarm.delay(0);   // Allow alarm to interrupt to do other tasks
		keypad->read();
	} while (keypad->getButtonWas() == BUTTON_NONE && !menuTimeout.timeUp());

  Serial.print(F("Button was "));
  Serial.println(keypad->getButtonWas(), DEC);

	switch (keypad->getButtonWas()) {
	case BUTTON_UP: 	   return m.up();
	case BUTTON_DOWN: 	 return m.down();
	case BUTTON_LEFT: 	 return m.left();
	case BUTTON_RIGHT: 	 return m.right();
	case BUTTON_SELECT: return m.right();   // Select activates child menu, just like right
	// case 'b': return m.back();
	case BUTTON_NONE:   return NULL;
	default: return &m;
	}
}

// Menu construction primitives
Menu &Menu::addChild( Menu &c) {
	if (child) {
		child->addSibling(c, loop);
	} else {
		child = &c;
		c.parent = this;
		if (loop) {
			c.next = c.prev = &c;
		}
	}
	return *this;   // Return self to enable chaining
}

// Add sibling is problematic as there may be no parent. Could make protected
Menu &Menu::addSibling( Menu &c, bool _loop) {
	if (next) {
		if (_loop) {
			// Insert c between prev and this
			prev->next = &c;
			c.prev = prev;
			c.next = this;
			prev = &c;
			c.parent = parent;
			return *this;
		} else {
			// Recursively find last sibling and append
			next->addSibling(c, _loop);
		}
	} else {
		if (_loop) {
			// Shouldn't get here since next should not be null
			c.next = prev;
			c.prev = this;
			prev = &c;
			next = &c;
			c.parent = parent;
		} else {
			next = &c;
			c.prev = this;
			c.parent = parent;
		}
	}
	return c;
}

// Recursively display the structure of the menu
void Menu::showStructure(bool full) {
	Serial.println(name);
  
  // Show parent name, prev and next names
  if (parent && full) {
	  Serial.print(F("Parent "));
	  Serial.println(parent->name);
  }
  
  if (prev && full) {
    Serial.print(F("Prev "));
    Serial.println(prev->name);
  }
  if (next && full) {
   Serial.print(F("Next "));
   Serial.println(next->name);
  }
  
  // Recursively list children
  if (child) {
    Serial.println(F("Children:"));
  
    Menu *m = child;
    do {
      m->showStructure(full);
      m = m->next;
    } while (m && m != child);

    Serial.print(F("End children of "));
    Serial.println(name);
  }
  
}    

// class MenuValue : Menu {
// protected:
//	int &value;
//	bool selected = false;  // When MenuValue is selected, up and down adjust value

MenuValue::MenuValue(const char *n,
		  int v,
		  void (*dis)(Menu &m),
		  Menu* (*procin)(Menu &m),
		  const char (*in)(Menu &m)) {
	name = n;
	input = in;
	processInput = procin;
	display = dis;
	current = this;
	value = v;
	selected = false;
}

void MenuValue::v_display(Menu &mm) {
	MenuValue &m = static_cast<MenuValue &>(mm);
  if (m.selected) {
	    Serial.print(F("Set ") );
	    Serial.print(m.name);
	    Serial.print(": ");
	    Serial.println(m.value, DEC);
  } else {
	    Serial.print(m.name);
	    Serial.print(" = ");
	    Serial.println(m.value, DEC);
  }
}

Menu *MenuValue::up() {
	if (!selected)
		return getif(prev);
	else {
		value++;
		return this;
	}
}

Menu *MenuValue::down() {
	if (!selected)
		return getif(next);
	else {
		value--;
		return this;
	}
}

Menu *MenuValue::left() {
	if (!selected) {
		return getif(parent);
	} else {
		selected = false;
		return this;
	}
}

Menu *MenuValue::right() {
	selected = true;
	return this;
}

Menu *MenuValue::select() {
	return right();    // Select is the same as right, by default
}

Menu *MenuValue::back() {
	selected = false;
	return getif(last);
}

int MenuValue::getValue() {
	return value;
}
void MenuValue::setValue(int v) {
	value = v;
}

/* class MenuArray : Menu {
protected:
  int[] values;
  bool  selected = false;
  */

MenuArray::MenuArray(const char *n,
		  float vs[],
		  void (*dis)(Menu &m),
		  Menu* (*procin)(Menu &m),
		  const char (*in)(Menu &m)) {
	name = n;
	input = in;
	processInput = procin;
	display = dis;
	current = this;
	values = vs;
	index = 0;
	selected = false;
}

Menu *MenuArray::up() {
  if (selected) {
    values[index] += 0.5;
    return this;
  } else
    return getif(prev);
}

Menu *MenuArray::down() {
  if (selected) {
    values[index] -= 0.5;
    return this;
  } else
    return getif(next);
}

Menu *MenuArray::left() {
  if (selected) {
    index = max(0, index-1);
    return this;
  } else
    return getif(parent);
}

Menu *MenuArray::right() {
  if (selected) {
    index = min(19, index+1);
    return this;
  } else
    return select();
}

void MenuArray::a_display(Menu &mm) {
	MenuArray &m = static_cast<MenuArray &>(mm);
     if (m.selected) {
       m.lcd->setCursor(1,1);
     m.lcd->setCursor(4, 1);
     m.lcd->print(m.index + 1, DEC);
     m.lcd->print("  ");    // Make sure previous value is overwritten
     // Make sure set point is printed in the right place
     m.lcd->setCursor(11, 1);
     m.lcd->print(m.values[m.index], DEC);
   } else {
     m.lcd->setCursor(4, 1);
     m.lcd->print(m.index + 1, DEC);
     m.lcd->print("  ");    // Make sure previous value is overwritten

     // Make sure set point is printed in the right place
     m.lcd->setCursor(11, 1);
     m.lcd->print(m.values[m.index], DEC);
   }
}

