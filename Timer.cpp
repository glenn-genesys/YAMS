#include <Timer.h>

/* class Timer {
private:
  time_t alarm;
  int    alarm_ms;
*/
  
  // Constructor to set an alarm in a given number of ms from current time
  Timer::Timer(long ms) {
    alarm    = ms / 1000;
    alarm_ms = ms % 1000;
  }
  
  // Construct an alarm for a specified time, according to the given clock
  /* Timer::Timer(time_t t) {
    alarm = t;
    alarm_ms = 0;
  } */

  char* Timer::toString() const {
	  if (alarm == -1)   return "Not set";
	  if (alarm < now()) return "Beep";
	  if (alarm - now() < 60) return "" + alarm + "." + alarm_ms;

	  char result[18];

	  sprintf(result, "%02d:%02d:%02d %02d/%02d/%02d", hour(alarm), minute(alarm), second(alarm),
			  day(alarm), month(alarm), year(alarm));

	  return result;
  }
  
  // Extend the alarm by the given number of milliseconds
  void Timer::extend(long ms) {
    alarm_ms += ms % 1000;
    alarm += trunc(ms/1000) + (alarm_ms >= 1000 ? 1 : 0);
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
  bool Timer::timeUp(bool clear = true) {
    if (alarm >= 0 && now() >= alarm) {
        if (clear) alarm = -1;
        return true;
      } else
      return false;
  }
