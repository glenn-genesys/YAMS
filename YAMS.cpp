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

// Memory allocation for static variables

Menu *Menu::last;        // Single entry for back button
Menu *Menu::current;     // Current menu location
LiquidCrystal *Menu::lcd;
AnalogButtons *Menu::keypad;
Menu::InputType Menu::input;
Menu::OutputType Menu::output;

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

// Default input function is _input(). Default display method is _default()
Menu::Menu(const char *n,
		bool loop,
		void (*dis)(Menu &m),
		Menu* (*procin)(Menu &m),
		char const (*in)(Menu &m)) {
	name = n;
	this->loop = loop;
	getInput = in;
	processInput = procin;
	display = dis;
	Menu::current = this;
	parent = child = next = prev = NULL;
}

Menu::Menu() {
	name = NULL;
	this->loop = false;
	getInput = NULL;
	processInput = NULL;
	display = NULL;
	parent = child = next = prev = NULL;
}

Menu::Menu(const char *n,
       const bool loop,
       const OutputType out,
       const InputType  in) {
	name = n;
	this->loop = loop;
	getInput = NULL;
	processInput = NULL;
	display = NULL;
	input = in;
	output = out;
	Menu::current = this;
	parent = child = next = prev = NULL;
}

Menu::Menu(const char *n,
       const bool loop,
       LiquidCrystal &l,
       AnalogButtons &k) {
	name = n;
	this->loop = loop;
	getInput = NULL;
	processInput = NULL;
	display = NULL;
	Menu::current = this;
	parent = child = next = prev = NULL;
	setLCD(l);
	setKeypad(k);
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
	// Serial.println(freeRam());
	// showStructure(true);
	if (output == LCDOUT) {
		lcd->clear();
	}
	
	Menu *previous;

	if (!current) current = (last ? last : this);

	do {
		if (output == SEROUT && previous != current) {
			current->serialDisplay();
		} else if (output == LCDOUT) {
			current->LCDdisplay();
		}
		previous = current;

		switch (input) {
		case SERIN: { current = current->serialProcessInput(); break; }
		case KEYIN: { current = current->keypadProcInput(); break; }
		}
	} while (current);   // NULL from process input means exit menu

	current = previous;  // Remember where we were
}

void Menu::setLCD( LiquidCrystal &l  ) {
	lcd = &l;
	output = LCDOUT;
}
void Menu::setKeypad( AnalogButtons &k ) {
	keypad = &k;
	input = KEYIN;
}

void Menu::serialDisplay() {
	Serial.println(name);
}

const char Menu::serialInput() {
	return Serial.read();
}

Menu *Menu::serialProcessInput() {
	// switch (getInput(*this)) {
	switch (serialInput()) {
	case 'u': return up();
	case 'd': return down();
	case 'l': return left();
	case 'r': return right();
	case 's': return select();
	case 'b': return back();
	case 'q': return NULL;
	default: return this;
	}
}

void Menu::LCDdisplay() {
	// Serial.print(F("LCDdisplay: "));
	// Serial.println(name);

	char text[17];
	sprintf(text, "%-16s", name);   // Right pad name with spaces to force overwriting on LCD

	lcd->setCursor(0, 0);
	lcd->print(text);
}

Menu *Menu::keypadProcInput() {
	Timer menuTimeout(MENU_TIMEOUT);
	while (keypad->getButtonJustPressed()) keypad->read();
	do {
		// Alarm.delay(0);   // Allow alarm to interrupt to do other tasks
		keypad->read();
	} while (keypad->getButtonWas() == BUTTON_NONE && !menuTimeout.timeUp());

	switch (keypad->getButtonWas()) {
	case BUTTON_UP: 	return up();
	case BUTTON_DOWN: 	return down();
	case BUTTON_LEFT: 	return left();
	case BUTTON_RIGHT: 	return right();
	case BUTTON_SELECT: return select();
	// case 'b': return m.back();
	case BUTTON_NONE:   return NULL;
	default: return this;
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
		  int lb,
		  int ub,
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
	selected = false;
}

MenuValue::MenuValue() {
	name = NULL;
	getInput = NULL;
	processInput = NULL;
	display = NULL;
	current = this;
	value = minv = maxv = 0;
	selected = false;
}

MenuValue::~MenuValue() {;}

void MenuValue::LCDdisplay() {
	lcd->setCursor(0, 0);
	lcd->print(F("                "));     // Erase previous text
	lcd->setCursor(0, 0);
	if (selected) {
		lcd->print(F("Set ") );
		lcd->print(name);
		lcd->print(": ");
		lcd->print(value, 1);
	} else {
		lcd->print(name);
		lcd->print(" = ");
		lcd->print(value, 1);
	}
	lcd->print("   ");
}

Menu *MenuValue::up() {
	if (!selected)
		return getif(prev);
	else {
		value = min(maxv, value+1);
		return this;
	}
}

Menu *MenuValue::down() {
	if (!selected)
		return getif(next);
	else {
		value = max(minv, value-1);
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
	getInput = in;
	processInput = procin;
	display = dis;
	current = this;
	values = vs;
	index = 0;
	selected = false;
}

MenuArray::MenuArray() {
	name = NULL;
	getInput = NULL;
	processInput = NULL;
	display = NULL;
	current = NULL;
	values = NULL;
	index = 0;
	selected = false;
}

MenuArray::~MenuArray() {;}

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
	  if (index == 0) {
		  selected = false;
	  } else {
		index--;
	  }
	  return this;
  } else
    return getif(parent);
}

Menu *MenuArray::right() {
  if (selected) {
    index = min(19, index+1);
    return this;
  } else {
	selected = true;
	return this;
   }
}

Menu *MenuArray::select() {
	selected = !selected;
	return this;
}

void MenuArray::LCDdisplay() {
	lcd->home();
	lcd->print(F("                "));     // Erase previous text
	lcd->home();
	if (selected) {
		lcd->print(F("Day "));
		lcd->print(index + 1, DEC);
		lcd->print(F(" : "));
		lcd->print(values[index], 1);
		lcd->print(F("    "));
	} else {
		lcd->print(name);
	}
}


