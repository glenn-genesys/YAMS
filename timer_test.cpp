#import "Timer.h"

/*
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
*/

int u_freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void showFreeRam() {
	Serial.print(u_freeRam(), DEC);
	Serial.println(F(" free RAM"));
}

Timer *t1;
Timer *t2;
Timer *t3;
Timer *t4;

void resetTimers() {
	if (t1) delete t1;
	if (t2) delete t2;
	if (t3) delete t3;
	if (t4) delete t4;
	t1 = new Timer(10);
	t2 = new Timer(1390);
	t3 = new Timer(1400);
	t4 = new Timer(1990);
}

void creationTests() {
	Timer t = Timer(0);
	t.print();
	t.setTimer(10, 123);
	t.print();

	t.extend(900);
	t.print();

	t.shorten(900);
	t.print();

	t.extend(2807);
	t.print();

	t.shorten(2807);
	t.print();
}

void t_setup() {
	Serial.begin(19200);

	// Set time to 8:29:00am 14 May 2014
	// setTime(8,29,0,14,5,14);
	// Fails when time set -- need to investigate

	delay(5000);
	creationTests();

	showFreeRam();
	resetTimers();
	Serial.print("t1: ");
	t1->print();
	Serial.print("t2: ");
	t2->print();
	Serial.print("t3: ");
	t3->print();
	Serial.print("t4: ");
	t4->print();

	setTime(8,29,0,16,6,14);
}



void testTimer( Timer *t ) {
  if (t->timeUp(false)) Serial.print("expired");
}

void t_loop() {
	showFreeRam();
	Serial.print("now=");
	Serial.print(now(), DEC);
	Serial.print(" millis=");
	Serial.println(millis(), DEC);
	Serial.print("t1: ");
	// testTimer(t1);
	t1->print();
	Serial.print("t2: ");
	t2->print();
	Serial.print("t3: ");
	t3->print();
	Serial.print("t4: ");
	t4->print();
	delay(500);
	if (t4->timeUp(false)) resetTimers();
}
