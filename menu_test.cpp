#include "YAMS.h"
#include <LiquidCrystal.h>
#include "AnalogButtons.h"

#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input
#define LCD_BACKLIGHT_PIN         3  // D3 controls LCD backlight

// #define TEST

  /*--------------------------------------------------------------------------------------
    Init the LCD library with the LCD pins to be used
  --------------------------------------------------------------------------------------*/
  LiquidCrystal tlcd( 8, 9, 4, 5, 6, 7 );   //Pins for the freetronics 16x2 LCD shield. LCD: ( RS, E, LCD-D4, LCD-D5, LCD-D6, LCD-D7 )

  /*--------------------------------------------------------------------------------------
   * Init the analog button shield using pin A0
   */
  // AnalogButtons tkeypad(BUTTON_ADC_PIN, 0, 98, 252, 407, 637);        // Other one
  AnalogButtons tkeypad(BUTTON_ADC_PIN, AnalogButtons::OTHER);        // Other one


void localMenu() {
  Menu topMenu("Top", true, Menu::LCDOUT, Menu::KEYIN);
  Menu fileMenu("File", false);
  Menu editMenu("Edit", false);
  Menu optionMenu("Options", false);
  
  topMenu.addChild(fileMenu).addChild(editMenu).addChild(optionMenu);
  
  topMenu.activate();
}

void lcdMenu() {
  Menu topMenu("Top", true, Menu::LCDOUT, Menu::SERIN);
  Menu fileMenu("File", false);
  Menu editMenu("Edit", false);
  Menu optionMenu("Options", false);
  
  topMenu.addChild(fileMenu).addChild(editMenu).addChild(optionMenu);
  
  topMenu.setLCD(tlcd);
  
  topMenu.activate();
}

void keypadMenu() {
  Menu topMenu("Top", true, Menu::SEROUT, Menu::KEYIN);
  Menu fileMenu("File", false);
  Menu editMenu("Edit", false);
  Menu optionMenu("Options", false);
  
  topMenu.addChild(fileMenu).addChild(editMenu).addChild(optionMenu);
  
  topMenu.setKeypad(tkeypad);
  
  topMenu.activate();
}

  float t_schedule[20]      = {19, 19, 19.5, 20, 20, 20.5, 21, 21.5, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23};

  char *dyStr   = "Days";
  char *hourStr  = "Hours";
  char *runtStr  = "Get/Set Run Time";
  char *schedStr = "Schedule";
  char *mainStr  = "Main Menu";

  MenuValue<int> *t_runDays;
  MenuValue<float> *t_runHours;
  
  Menu *t_runningTime;
  
  MenuArray *t_scheduleMenu;
  
  Menu *gm;

void globalMenu() {
	t_runDays      = new MenuValue<int>(dyStr, 0, 0, 20, 1);
	t_runHours     = new MenuValue<float>(hourStr, 0, 0, 20, 1);

	t_runningTime       = new Menu(runtStr, false);

	t_scheduleMenu = new MenuArray(schedStr, t_schedule, 20);

	gm = new Menu(mainStr, false, tlcd, tkeypad);

	// t_runningTime->addSibling(*t_scheduleMenu, false);
	t_runningTime->addChild(*t_runDays).addChild(*t_runHours);
	gm->addChild(*t_runningTime).addChild(*t_scheduleMenu);

	gm->activate();
}

Menu test, test1, test2;
MenuValue<int> testInt;

void localInitMenu() {
	test = Menu("Hello", true, tlcd, tkeypad);
	test1 = Menu("Press a key");
	test2 = Menu("Any key");

	testInt = MenuValue<int>("Value", 4, 1, 10, 2);

	test1.addChild(test2);
	test.addChild(testInt).addChild(test1);

	test.activate();
}

#ifdef TEST
void setup() {
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

	   delay(2000);
	   globalMenu();
}

void loop() {
	gm->activate();
}

/* void loop() {
	// localInitMenu();
	Serial.println(F("LCD menu"));
	lcdMenu();
	Serial.println(F("Keypad menu"));
	keypadMenu();
	Serial.println(F("Global menu"));
	gm->activate();
	Serial.println(F("Local menu"));
	localMenu();
  
}
*/
#endif
