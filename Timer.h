// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

// An asychronous alarm timer, based on an existing clock. May be reset and reused.

#ifndef _Timer_H_
#define _Timer_H_
#include "Arduino.h"
//add your includes for the project Timer here

#include "Time.h"

//end of add your includes here
#ifdef __cplusplus
extern "C" {
#endif

class Timer {
private:
  time_t alarm;
  int    alarm_ms;
  
public:
  // Constructor to set an alarm in a given number of ms from current time
  Timer(long ms);
  
  // Construct an alarm for a specified time
  // Timer(time_t t);

  char* toString() const;
  
  // Extend the alarm by the given time interval in ms
  void extend(long ms);
  
  // Shorten the alarm time by the given time interval in ms
  void shorten(long ms);

  // Set a new alarm time to the given time
  void setTimer(time_t t, int ms = 0);
  
  // Returns true if the alarm has gone off and clears the alarm if clear is true
  bool timeUp(bool clear = true);
};

#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project Timer here


//Do not add code below this line
#endif /* _Timer_H_ */
