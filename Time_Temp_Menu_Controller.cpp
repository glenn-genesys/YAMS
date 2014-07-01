/*
New requirements
. Display current days/hours running time, current temp and set point
19.375 C / 20.0 C
12 Days 15:23:59s

After 5 s returns to normal display and normal running. During menu access don't check temp or change output
Button press returns to menu view -- wherever menu was, unless menu was last activated over 60 seconds ago

Menu:
Running time
-> Days  +/-
-> Hours +/-
Schedule
-> Day 0 +/-
etc
Review
-> Daily review  (replay min/max/set each day, cycle every 3 sec, disable time-out)
-> Hourly review (replay min/max each hour, cycle every 2 sec, disable time-out)
Appliance  Heater/Cooler
Mode       Schedule/Fixed

While running, record history of temperatures, min and max per hour


. Menu to set mode: heating/cooling
. Alarm if cooling when needs to heat, or vice versa
. Alarm if unable to reach set point
. Check for crazy set points 0 < T < 30
. Record history of temperatures, per hour
. Menu option to show temperature history
*/

/*--------------------------------------------------------------------------------------
  Includes
--------------------------------------------------------------------------------------*/
#include "Time_Temp_Menu_Controller.h"
#include <OneWire.h>
#include <Wire.h>
#include <LiquidCrystal.h>   // include LCD library
#include <Time.h>
#include "Timer.h"
#include "YAMS.h"
#include "AnalogButtons.h"

/*--------------------------------------------------------------------------------------
  Defines
--------------------------------------------------------------------------------------*/
// Pins in use
#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input
#define LCD_BACKLIGHT_PIN         3  // D3 controls LCD backlight
#define CONTROL_PIN               0  // D0 turns on power relay
#define BUZZER_PIN                11 // Piezo buzzer

#define AVERAGE_WEIGHT            0.1 // For use when calculating moving average
#define RESWITCH_TIME             10  // 120s Minimum time between power cycling (10 for testing)
// Error codes
#define NO_SENSOR                 1    // No or unexpected 1-wire temperature sensor connected
#define SETPOINT_TOO_HIGH_LOW     2    // Can't reach setpoint. Too high (heating) or low (cooling).
#define WRONG_MODE                3    // Mode is set to heating (cooling) but should be cooling (heating)
#define WRONG_APPLIANCE           4    // Mode is set to heating (cooling) but attached appliance cools (heats)

#define NO_ALARMS                 false   // To turn off all warning alarms
#define MENU_TIMEOUT              5000    // ms of inactivity before menu mode will be replaced by normal running
#define REDRAW_PERIOD             950    // How often to read temp and display values

//some example macros with friendly labels for LCD backlight/pin control, tested and can be swapped into the example code as you like
#define LCD_BACKLIGHT_OFF()     digitalWrite( LCD_BACKLIGHT_PIN, LOW )
#define LCD_BACKLIGHT_ON()      digitalWrite( LCD_BACKLIGHT_PIN, HIGH )
#define LCD_BACKLIGHT(state)    { if( state ){digitalWrite( LCD_BACKLIGHT_PIN, HIGH );}else{digitalWrite( LCD_BACKLIGHT_PIN, LOW );} }

// DS1822 22 4A 34 1C 0 0 0 9E
/*--------------------------------------------------------------------------------------
  Global Variables
--------------------------------------------------------------------------------------*/
float temperature;
float smoothedTemp      = 20;
float setPoint          = 21.5;            // set point temp in degrees C
boolean recentlySwitched= false;
boolean coolingMode     = true;
boolean applianceOn     = false;

boolean showSmoothed    = true;
float schedule[20]      = {19, 19, 19.5, 20, 20, 20.5, 21, 21.5, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23};

float hourlyMax[24]     = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float hourlyMin[24]     = {50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50};
float dailyMax[24]      = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
float dailyMin[24]      = {50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50};

/*--------------------------------------------------------------------------------------
 * Init the analog button shield using pin A0
 */
// AnalogButtons keypad(BUTTON_ADC_PIN, 0, 145, 333, 505, 741);    // Freetronics
AnalogButtons keypad(BUTTON_ADC_PIN, 0, 98, 252, 407, 637);        // Other one

/*--------------------------------------------------------------------------------------
  Init the LCD library with the LCD pins to be used
--------------------------------------------------------------------------------------*/
LiquidCrystal lcd( 8, 9, 4, 5, 6, 7 );   //Pins for the freetronics 16x2 LCD shield. LCD: ( RS, E, LCD-D4, LCD-D5, LCD-D6, LCD-D7 )

/*--------------------------------------------------------------------------------------
  Init the 1-wire library with the 1-wire pin to be used
--------------------------------------------------------------------------------------*/
OneWire  ds(2);  // on pin 2 (a 4.7K resistor is necessary)

//beneath is list of menu items needed to build the menu
char *daySt = "Days";
char *hrStr = "Hours";
char *runStr = "Get/Set Runtime";
char *schedStr = "Schedule";
char *mainStr = "Main Menu";

// By default, value enters a submenu to increment/decrement the value, on select
MenuValue *runDays      = new MenuValue(daySt, 0);
MenuValue *runHours     = new MenuValue(hrStr, 0);

// Menu runningTime       = Menu(runStr, true, &Menu::LCDdisplay, &Menu::keypadProcInput).addChild( runDays ).addChild( runHours );
Menu *runningTime       = new Menu(runStr, true, &Menu::LCDdisplay, &Menu::keypadProcInput);

MenuArray *scheduleMenu = new MenuArray(schedStr, schedule);

// Menu mm = Menu(mainStr, true, &Menu::LCDdisplay, &Menu::keypadProcInput).addChild( runningTime ).addChild( scheduleMenu );
Menu *mm = new Menu(mainStr, false, &Menu::LCDdisplay, &Menu::keypadProcInput);

/*    MenuItem review         = MenuItem(menu, "Review", 2);
      MenuItem dailyReview  = MenuItem(menu, "DailyReview", 3);
      MenuItem hourlyReview = MenuItem(menu, "HourlyReview", 3);

    review.addRight(MenuList({dailyReview, hourlyReview});

    menu.getRoot().addRight( MenuList({runningTime, schedule, review}) );
*/

// Forward declarations
void normalDisplay();
void updateTemp();

/*--------------------------------------------------------------------------------------
  setup()
  Called by the Arduino framework once, before the main loop begins
--------------------------------------------------------------------------------------*/
void setup(void) {
  Serial.begin(19200);

   //button adc input
   pinMode( BUTTON_ADC_PIN, INPUT );         //ensure A0 is an input
   digitalWrite( BUTTON_ADC_PIN, LOW );      //ensure pullup is off on A0
   //lcd backlight control
   pinMode( LCD_BACKLIGHT_PIN, OUTPUT );     //D3 is an output
   LCD_BACKLIGHT_ON();
   // relay control
   pinMode(CONTROL_PIN, OUTPUT);     

   // Set time to 8:29:00am 14 May 2014
   // setTime(8,29,0,14,5,14);
   setTime(9,50,0,2,7,14);

   // Initialise temperature sensor
   updateTemp();
   Alarm.timerRepeat(1, updateTemp);            // read temperature every second
   // Alarm.timerRepeat(3600, updateTempHistory);  // record hourly and daily maxima and minima

   //set up the LCD number of columns and rows: 
   lcd.begin( 16, 2 );

   // Menu setup
    /*
    Menu:
    Running time
    -> Days  +/-
    -> Hours +/-
    Schedule
    -> Day 0 +/-
    etc
    Review
    -> Daily review  (replay min/max/set each day, cycle every 3 sec, disable time-out)
    -> Hourly review (replay min/max each hour, cycle every 2 sec, disable time-out)
    Appliance  Heater/Cooler
    Mode       Schedule/Fixed
    */

   Alarm.delay(2000);

   runningTime->addChild( *runDays ).addChild( *runHours );
   mm->addChild( *runningTime ).addChild( *scheduleMenu );

   mm->setLCD(lcd);
   mm->setKeypad(keypad);

   lcd.setCursor(0, 0);
   if (coolingMode) {
	   lcd.print(F("Cooling"));
	   Serial.print(F("Cooling"));
   } else {
	   lcd.print(F("Heating"));
	   Serial.print(F("Heating"));
   }

   Alarm.delay(2000);

   normalDisplay();
}

/*--------------------------------------------------------------------------------------
  loop()
  Arduino main loop
--------------------------------------------------------------------------------------*/
/* void loop(void) {
  float reading;

  runTime = Time(millis());

  // Current set point depends on the schedule
  setPoint = schedule[runTime.days];

  // Read temperature every second, but don't slow down button reading loop
  if (runTime.secs != lastUpdate) {
    reading = readTemp();

    smoothedTemp = smoothedTemp * (1 - AVERAGE_WEIGHT) + reading * AVERAGE_WEIGHT;

    // Select button should swap between schedule and operational view

    if (showSmoothed) {
      lcd.setCursor(0, 0);
      lcd.print("Smoothed : ");
      lcd.print(smoothedTemp, DEC);
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Current  : ");
      
      lcd.print(reading, DEC);
    }

    // Make sure we don't thrash -- ensure at least two minutes have passed
    if (coolingOn != smoothedTemp > setPoint && lastSwitch + RESWITCH_TIME < millis()/1000) {
      // Switch on or off
      Serial.println("Cooling " + coolingOn);
      Serial.print("Last switch ");
      Serial.println(lastSwitch, DEC);

      coolingOn = smoothedTemp > setPoint;

      lastSwitch = millis()/1000;
    }

    if (coolingOn) {
        digitalWrite( CONTROL_PIN, HIGH );
        digitalWrite( LCD_BACKLIGHT_PIN, LOW );
        delay( 10 );
        digitalWrite( LCD_BACKLIGHT_PIN, HIGH );   //leave the backlight on at exit
    } else {
      digitalWrite( CONTROL_PIN, LOW );
    }
    
    Serial.println(runTime.secs, DEC);
    
    lastUpdate = runTime.secs;
  }
  
   button.read();

   if (button.buttonJustPressed) {
     //show text label for the button pressed
     switch( button.buttonWas )
     {
        case BUTTON_NONE:
        {
           break;
        }
        case BUTTON_UP:
        {
           schedule[menuDay] += 0.5;
           break;
        }
        case BUTTON_DOWN:
        {
           schedule[menuDay] -= 0.5;
           break;
        }
        case BUTTON_LEFT:
        {
           menuDay = max(menuDay - 1, 0);
          break;
        }
        case BUTTON_RIGHT:
        {
           menuDay = min(menuDay + 1, 19);
           break;
        }
       case BUTTON_SELECT:
       {
         showSmoothed = !showSmoothed;
         break;
        }
        default:
       {
          break;
       }
     }
     lcd.setCursor(4, 1);
     lcd.print(menuDay + 1, DEC);
     lcd.print("  ");    // Make sure previous value is overwritten

     // Make sure set point is printed in the right place
     lcd.setCursor(11, 1);
     lcd.print(schedule[menuDay], DEC);
     
     delay(300);
   }

} */

void alarm(int alarmType) {
  if (NO_ALARMS) return;

  switch (alarmType)
  {
    case NO_SENSOR:
    {
      tone(BUZZER_PIN, 2000, 500);
      Alarm.delay(500);
      break;
    }
    case SETPOINT_TOO_HIGH_LOW:
    {
      tone(BUZZER_PIN, 500, 500);
      Alarm.delay(0);
      tone(BUZZER_PIN, 500, 500);
      Alarm.delay(0);
      tone(BUZZER_PIN, 5000, 500);
      Alarm.delay(0);
      tone(BUZZER_PIN, 5000, 500);
      Alarm.delay(0);
      break;
    }
    case WRONG_MODE:
    {
      for (int i=100; i<5000; i=i+100 ) {
        tone(BUZZER_PIN, i, 50);
        Alarm.delay(0);
      }
      break;
    }
    case WRONG_APPLIANCE:
    {
      for (int i=5000; i>100; i=i-100) {
        tone(BUZZER_PIN, i, 50);
        Alarm.delay(0);
      }
      break;
    }
  }
}

/*
   Display current days/hours running time, current temp and set point
     19.375 C / 20.0C
     12 Days 15:23:59
*/
void normalDisplay() {

   // Serial.println("Normal display");
   lcd.setCursor( 0, 0 );   //top left

   lcd.print(smoothedTemp, 2);
   // Overwrite from fourth decimal place
   // lcd.setCursor( 6, 0 );
   lcd.print("C /  ");
   lcd.print(setPoint, 2);
   lcd.print("C");
   
   lcd.setCursor( 0, 1 );   //bottom left

   char tempStr[16];
   sprintf(tempStr, "Day %02d %02d:%02d:%02d", day(), hour(), minute(), second());

   lcd.print(tempStr);
}

void unlockAppliance() {
  Serial.println(F("Unlock appliance"));

    recentlySwitched = false;
}

/*
 * Running mode
 * Display normal display
 * Any button press will go to menu mode
 */
void runningMode(float setPoint) {

  Serial.println(F("Running mode"));

  Timer redrawTime = Timer(REDRAW_PERIOD);
  if (redrawTime.timeUp(false)) Serial.print("Timer fail");
  // redraw = Alarm.timerRepeat(REDRAW_PERIOD, normalDisplay);

  do {
	Serial.print(".");
	Alarm.delay(100);    // Allow system to check for other events

    keypad.read();
    
    if (redrawTime.timeUp() || true) {
    
      normalDisplay();
      redrawTime.extend(REDRAW_PERIOD);
      
      // Apply temperature control logic
      if (!recentlySwitched) {
      	Serial.print("*");
        boolean prevState = applianceOn;
        applianceOn = coolingMode == (smoothedTemp > setPoint);
        
        if (prevState != applianceOn) {
          Serial.print(F("Turn appliance "));
          Serial.println(applianceOn ? "on" : "off");

          recentlySwitched = true;
          Alarm.timerOnce(RESWITCH_TIME, unlockAppliance);      // Allow switching again after reswitch period
        }
      }
      if (applianceOn) {
          digitalWrite( CONTROL_PIN, HIGH );
          digitalWrite( LCD_BACKLIGHT_PIN, LOW );
          delay( 20 );
          digitalWrite( LCD_BACKLIGHT_PIN, HIGH );   //leave the backlight on at exit
      } else {
        digitalWrite( CONTROL_PIN, LOW );
      }

      float rateOfChange		= temperature - smoothedTemp;
      // Check for alarm states
      // if (applianceOn && (coolingMode == (rateOfChange > 0.0))) alarm(WRONG_APPLIANCE);
      // if (!applianceOn && ((coolingMode && smoothedTemp < setPoint - 0.5) || (!coolingMode && smoothedTemp > setPoint + 0.5))) alarm(WRONG_MODE);

    }
  } while (!keypad.getButtonJustPressed()); 
}

/*--------------------------------------------------------------------------------------
  loop()
  Arduino main loop
--------------------------------------------------------------------------------------*/
void loop(void) {
  runningMode(setPoint);
  
  Serial.println(keypad.getButtonJustPressed(), DEC);

  mm->activate();
}

float readTemp() {
  byte addr[8] = {0x22, 0x4A, 0x34, 0x1C, 0x0, 0x0, 0x0, 0x9E};
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  
  /* Serial.print("ROM =");
  for( i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addr[i], HEX);
  } */

  // delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad
  
  int dataSum = 0;
  
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
    dataSum += data[i];
    
    // Serial.print(data[i], HEX);
  }
  // Serial.println();

  if (dataSum == 0) alarm(NO_SENSOR);

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }

  // delay(10);

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 0);        // start next conversion, with parasite power off at the end
  
  Serial.print((float)raw / 16.0, DEC);
  Serial.println(" C");
  
  return (float)raw / 16.0;
}    

// Update global temperature variables. Method, to be called by timer
void updateTemp() {
   temperature = readTemp();
   
   if (temperature != 0.0 && temperature < 85.0) {
	   smoothedTemp = smoothedTemp * (1 - AVERAGE_WEIGHT) + temperature * AVERAGE_WEIGHT;
   }

   setPoint = schedule[day() - 1];

   hourlyMax[runHours->getValue()] = max(hourlyMax[runHours->getValue()], smoothedTemp);
   hourlyMin[runHours->getValue()] = min(hourlyMin[runHours->getValue()], smoothedTemp);
   dailyMax[runDays->getValue()]   = max(dailyMax[runDays->getValue()], smoothedTemp);
   dailyMin[runDays->getValue()]   = min(dailyMin[runDays->getValue()], smoothedTemp);
}

void updateRuntime() {
   
}
