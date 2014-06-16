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

// Default input function is _input(). Default display method is _default()
void Menu::Menu(const char *n,
		const bool loop = false,
		const void (*dis)(Menu &m) = _display,
		const char (*procin)(Menu &m) = _processInput,
		const char (*in)(Menu &m) = _input) {
	name = n;
	this->loop = loop;
	input = in(this);
	processInput = procin(this);
	display = dis(this);
	current = &this;
}

Menu *Menu::getif(Menu *t) {
	if (t) {
		last = &this;
		return t;
	} else
		return &this;
}

// Standard menu functions that may be overridden
Menu *Menu::left() {
	last = &this;
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
	do {
		current->display();

		current = current->processInput();
	} while (current);   // NULL from process input means exit menu
}

void Menu::setLCD( LiquidCrystal l  ) {
	lcd = l;
	display = &LCDdisplay;
}
void Menu::setKeypad( AnalogButtons k ) {
	keypad = k;
	processInput = &keypadProcInput;
}

const void Menu::_display(Menu &m) {
	Serial.println(m.name);
}

const char Menu::_input(Menu &m) {
	return Serial.read();
}

const Menu *Menu::_processInput(Menu &m) {
	switch (m.input()) {
	case 'u': return m.up();
	case 'd': return m.down();
	case 'l': return m.left();
	case 'r': return m.right();
	case 's': return m.right();   // Select activates child menu, just like right
	case 'b': return m.back();
	case 'q': return NULL;
	default: return m;
	}
}

void Menu::LCDdisplay(Menu &m) {
	lcd.setCursor(1, 1);
	lcd.print(m.name);
}

Menu *Menu::keypadProcInput(Menu &m) {
	Timer menuTimeout(MENU_TIMEOUT);
	do {
		Alarm.delay(1);   // Allow alarm to interrupt to do other tasks
		keypad.read();
	} while (keypad.getButtonWas() == BUTTON_NONE && !menuTimeout.timeUp());

	switch (keypad.getButtonWas()) {
	case BUTTON_UP: 	   return m.up();
	case BUTTON_DOWN: 	 return m.down();
	case BUTTON_LEFT: 	 return m.left();
	case BUTTON_RIGHT: 	 return m.right();
	case BUTTON_SELECT: return m.right();   // Select activates child menu, just like right
	// case 'b': return m.back();
	case BUTTON_NONE:   return NULL;
	default: return &this;
	}
}

// Menu construction primitives
Menu &Menu::addChild( Menu &c) {
	if (child) {
		child->addSibling(c, loop);
	} else {
		child = c;
		if (loop) {
			c.next = c.prev = c;
		}
	}
	return this;   // Return self to enable chaining
}

// Add sibling is problematic as there may be no parent. Could make protected
Menu &Menu::addSibling( Menu &c, bool _loop) {
	if (next) {
		if (_loop) {
			// Insert c between prev and this
			prev->next = c;
			c.prev = prev;
			c.next = &this;
			prev = c;
			return this;
		} else {
			// Recursively find last sibling and append
			next->addSibling(c, _loop);
		}
	} else {
		if (_loop) {
			// Shouldn't get here since next should not be null
			c.next = prev;
			c.prev = &this;
			prev = c;
			next = c;
		} else {
			next = c;
			c.prev = &this;
		}
	}
	return c;
}

// class MenuValue : Menu {
// protected:
//	int &value;
//	bool selected = false;  // When MenuValue is selected, up and down adjust value

MenuValue::MenuValue(const char *n,
		  int v,
		  const void (*dis)(Menu &m) = _display,
		  const char (*procin)(Menu &m) = _processInput,
		  const char (*in)(Menu &m) = _input) {
	name = n;
	input = in(this);
	processInput = procin(this);
	display = dis(this);
	current = &this;
	value = v;
	selected = false;
}

const void MenuValue::_display(Menu &m) {
  if (selected) {
    Serial.println("Set " + name + ": " + value);
  } else {
    Serial.println(name + " = " + value);
  }
}

Menu *MenuValue::up() {
	if (!selected)
		return getif(prev);
	else {
		value++;
		return &this;
	}
}

Menu *MenuValue::down() {
	if (!selected)
		return getif(next);
	else {
		value--;
		return &this;
	}
}

Menu *MenuValue::left() {
	if (!selected) {
		return getif(parent);
	} else {
		selected = false;
		return &this;
	}
}

Menu *MenuValue::right() {
	selected = true;
	return &this;
}

Menu *MenuValue::select() {
	return right();    // Select is the same as right, by default
}

Menu *MenuValue::back() {
	selected = false;
	return getif(last);
}

Menu *MenuValue::up() {
	if (!selected)
		return getif(prev);
	else {
		value++;
		return &this;
	}
}

Menu *MenuValue::down() {
	if (!selected)
		return getif(next);
	else {
		value--;
		return &this;
	}
}

Menu *MenuValue::left() {
	if (!selected) {
		return getif(parent);
	} else {
		selected = false;
		return &this;
	}
}

Menu *MenuValue::right() {
	selected = true;
	return &this;
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
		  float &vs,
		  const void (*dis)(Menu &m) = _display,
		  const char (*procin)(Menu &m) = _processInput,
		  const char (*in)(Menu &m) = _input) {
	name = n;
	input = in(this);
	processInput = procin(this);
	display = dis(this);
	current = &this;
	values = vs;
	index = 0;
	selected = false;
}

Menu *MenuArray::up() {
  if (selected) {
    values[index] += 0.5;
    return &this;
  } else
    return getif(prev);
}

Menu *MenuArray::down() {
  if (selected) {
    values[index] -= 0.5;
    return &this;
  } else
    return getif(next);
}

Menu *MenuArray::left() {
  if (selected) {
    index = max(0, index-1);
    return &this;
  } else
    return getif(parent);
}

Menu *MenuArray::right() {
  if (selected) {
    index = min(19, index+1);
    return &this;
  } else
    return select();
}

void MenuArray::display(Menu &m) {
     if (selected) {
       m.lcd.setCursor(1,1);
     m.lcd.setCursor(4, 1);
     m.lcd.print(index + 1, DEC);
     m.lcd.print("  ");    // Make sure previous value is overwritten
     // Make sure set point is printed in the right place
     m.lcd.setCursor(11, 1);
     m.lcd.print(values[index], DEC);
   } else {
     m.lcd.setCursor(4, 1);
     m.lcd.print(index + 1, DEC);
     m.lcd.print("  ");    // Make sure previous value is overwritten

     // Make sure set point is printed in the right place
     m.lcd.setCursor(11, 1);
     m.lcd.print(values[index], DEC);
   }
}

