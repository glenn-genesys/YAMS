#include "Timer.h"

/* class Timer {
private:
  time_t alarm;
  int    alarm_ms;
*/
  
  // Constructor to set an alarm in a given number of ms from current time
  Timer::Timer(long ms) {
    alarm    = now() + ms / 1000;
    alarm_ms = (ms % 1000 + millis() % 1000);     // ms will be compared to millis() so add millis now
    
    alarm = now() + ms / 1000 + (alarm_ms >= 1000 ? 1 : 0);
    alarm_ms = alarm_ms % 1000;
  }
  
  // Construct an alarm for a specified time, according to the given clock
  // Signature clash with Timer(long ms)
  /* Timer::Timer(time_t t) {
    alarm = t;
    alarm_ms = 0;
  } */

  // Not memory safe -- do not use
  /* char* Timer::toString() const {
	  if (alarm == -1)   return "Not set";
	  if (alarm >= 0 && (now() > alarm || (now() == alarm && millis()%1000 > alarm_ms))) return "Beep";
	  if (alarm - now() < 60) {
		  char *result = new char[18];

		  sprintf(result, "%02d.%03d", alarm, alarm_ms);

		  return result;
	  } else {
		  char *result = new char[18];

		  sprintf(result, "%02d:%02d:%02d %02d/%02d/%02d", hour(alarm), minute(alarm), second(alarm),
				  day(alarm), month(alarm), year(alarm));

		  return result;
	  }
  } */

  void Timer::print() const {
	  if (alarm == -1) {
		  Serial.println(F("Not set"));
		  return;
	  }
	  if (alarm >= 0 && (now() > alarm || (now() == alarm && millis()%1000 > alarm_ms))) {
		  Serial.println(F("Beep"));
		  return;
	  }
	  if (alarm - now() < 60) {
		  char result[18];
		  // sprintf(result, "%02d.%03d", alarm, alarm_ms);   // Displays ms as 000 for some reason?
		  // Serial.println(result);
		  Serial.print(alarm, DEC);
		  Serial.print(".");
		  // Serial.println(alarm_ms, DEC);
		  sprintf(result, "%03d", alarm_ms);
		  Serial.println(result);
		  return;
	  } else {
		  char result[18];

		  sprintf(result, "%02d:%02d:%02d %02d/%02d/%02d", hour(alarm), minute(alarm), second(alarm),
				  day(alarm), month(alarm), year(alarm));

		  Serial.println(result);
	  }
  }
  
  // Ensure the ms of this alarm are >=0 and <1000
  /* void Timer::trunc() {
    alarm += ms/1000 + (alarm_ms >= 1000 ? 1 : 0);
    alarm_ms = alarm_ms % 1000;
  } */
  
  // Extend the alarm by the given number of milliseconds
  void Timer::extend(long ms) {
    alarm_ms += ms % 1000;
    alarm += ms/1000 + (alarm_ms >= 1000 ? 1 : 0);
    alarm_ms = alarm_ms % 1000;
  }
  
  // Shorten the alarm time by the given time interval
  void Timer::shorten(long ms) {
    alarm_ms -= ms % 1000;
    alarm -= trunc(ms/1000);
    if (alarm_ms < 0) {
      alarm--;
      alarm_ms = alarm_ms + 1000;
    }
  }

  // Set a new alarm time to the given time
  void Timer::setTimer(time_t t, int ms) {
    alarm = t;
    alarm_ms = ms;
  }
  
  // Returns true if the alarm has gone off and clears the alarm if clear is true
  bool Timer::timeUp(bool clear) {
    if (alarm >= 0 && (now() > alarm || (now() == alarm && millis()%1000 > alarm_ms))) {
        if (clear) alarm = -1;
        return true;
      } else
      return false;
  }
