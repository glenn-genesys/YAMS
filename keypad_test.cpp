#include "AnalogButtons.h"

#define BUTTON_ADC_PIN           A0  // A0 is the button ADC input

/*--------------------------------------------------------------------------------------
 * Init the analog button shield using pin A0
 */
AnalogButtons button(BUTTON_ADC_PIN);
bool prevPressed = false;

void kt_setup() {}

void displayButton(int b) {
     switch( b )
     {
        case BUTTON_NONE:
        {
           break;
        }
        case BUTTON_UP:
        {
           Serial.print("u");
           break;
        }
        case BUTTON_DOWN:
        {
           Serial.print("d");
           break;
        }
        case BUTTON_LEFT:
        {
           Serial.print("l");
          break;
        }
        case BUTTON_RIGHT:
        {
           Serial.print("r");
           break;
        }
       case BUTTON_SELECT:
       {
           Serial.print("s");
         break;
        }
        default:
       {
          break;
       }
     }

}

void kt_loop() {
   button.read();

   if (button.getButtonJustPressed() && !prevPressed) {
     //show text label for the button pressed
	   // Serial.print(button.voltageWas, DEC);
      displayButton(button.getButtonWas());
      Serial.println(" pressed");
      prevPressed = true;
   }

   if (button.getButtonJustReleased() && prevPressed) {
     //show text label for the button pressed
	   // Serial.print(button.voltageWas, DEC);
      displayButton(button.getButtonWas());
      Serial.println(" released");
      prevPressed = false;
   }
}
