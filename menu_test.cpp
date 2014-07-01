#include "YAMS.h"
#include <LiquidCrystal.h>
#include "AnalogButtons.h"

#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input
#define LCD_BACKLIGHT_PIN         3  // D3 controls LCD backlight

  /*--------------------------------------------------------------------------------------
    Init the LCD library with the LCD pins to be used
  --------------------------------------------------------------------------------------*/
  LiquidCrystal tlcd( 8, 9, 4, 5, 6, 7 );   //Pins for the freetronics 16x2 LCD shield. LCD: ( RS, E, LCD-D4, LCD-D5, LCD-D6, LCD-D7 )

  /*--------------------------------------------------------------------------------------
   * Init the analog button shield using pin A0
   */
  AnalogButtons tkeypad(BUTTON_ADC_PIN, 0, 98, 252, 407, 637);        // Other one


void localMenu() {
  Menu topMenu("Top", true);
  Menu fileMenu("File", false);
  Menu editMenu("Edit", false);
  Menu optionMenu("Options", false);
  
  topMenu.addChild(fileMenu).addChild(editMenu).addChild(optionMenu);
  
  topMenu.activate();
}

void lcdMenu() {
  Menu topMenu("Top", true, &Menu::LCDdisplay);
  Menu fileMenu("File", false, &Menu::LCDdisplay);
  Menu editMenu("Edit", false, &Menu::LCDdisplay);
  Menu optionMenu("Options", false, &Menu::LCDdisplay);
  
  topMenu.addChild(fileMenu).addChild(editMenu).addChild(optionMenu);
  
  topMenu.setLCD(tlcd);
  
  topMenu.activate();
}

void keypadMenu() {
  Menu topMenu("Top", true, &Menu::serialDisplay, &Menu::keypadProcInput);
  Menu fileMenu("File", false, &Menu::serialDisplay, &Menu::keypadProcInput);
  Menu editMenu("Edit", false, &Menu::serialDisplay, &Menu::keypadProcInput);
  Menu optionMenu("Options", false, &Menu::serialDisplay, &Menu::keypadProcInput);
  
  topMenu.addChild(fileMenu).addChild(editMenu).addChild(optionMenu);
  
  topMenu.setKeypad(tkeypad);
  
  topMenu.activate();
}

  float t_schedule[20]      = {19, 19, 19.5, 20, 20, 20.5, 21, 21.5, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23};

  MenuValue *t_runDays      = new MenuValue("Days", 0);
  MenuValue *t_runHours     = new MenuValue("Hours", 0);
  
  // Menu runningTime       = Menu(runStr, true, &Menu::LCDdisplay, &Menu::keypadProcInput).addChild( runDays ).addChild( runHours );
  // Menu *runningTime       = new Menu(runStr, true, &Menu::LCDdisplay, &Menu::keypadProcInput);
  Menu *t_runningTime       = new Menu("Get/Set Run Time", false);
  
  MenuArray *t_scheduleMenu = new MenuArray("Schedule", t_schedule);
  
  // Menu mm = Menu(mainStr, true, &Menu::LCDdisplay, &Menu::keypadProcInput).addChild( runningTime ).addChild( scheduleMenu );
  // Menu *mm = new Menu(mainStr, false, &Menu::LCDdisplay, &Menu::keypadProcInput);
  Menu *gm = new Menu("Main Menu", false);

void globalMenu() {
  gm->addChild(*t_runningTime);
  t_runningTime->addSibling(*t_scheduleMenu, false);
  t_runningTime->addChild(*t_runDays).addChild(*t_runHours);
  
  gm->activate();
}

void m_setup() {
	  Serial.begin(19200);
	  delay(5000);

	   //button adc input
	   pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
	   digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0
	   //lcd backlight control
	   pinMode( LCD_BACKLIGHT_PIN, OUTPUT );     //D3 is an output
	   digitalWrite( LCD_BACKLIGHT_PIN, HIGH );

	   // Serial.println(F("Setup global menu"));
	   tlcd.setCursor(0, 0);
	   tlcd.print(F("Testing"));
	   globalMenu();
}

void m_loop() {
	// Serial.println(F("Local menu"));
	// localMenu();
	Serial.println(F("Global menu"));
	gm->activate();
  
	Serial.println(F("LCD menu"));
	lcdMenu();
	Serial.println(F("Keypad menu"));
	keypadMenu();
}
