#include <IRremote.h>

#define IR 13


IRrecv receiver(IR);
decode_results results;

#define COLOR Blue // Change this with every switch


/*
 * All Smartie Colors - if you change this, you have to change this in Stopper.ino as well
 */
enum Colors {
  Brown     = 63136449, // IR Codes (8 decimal chars)
  Green     = 62089471,
  Yellow    = 59576503,
  Rosa      = 58110861,
  Purple    = 56435773,
  Red       = 54132723,
  Blue      = 50573721,
  Orange    = 94109019,
  Empty     = 0 // Empty shouldn't be signaled 
};

bool is_switch_open = false;

void setup() {
  receiver.enableIRIn();
  
}

void loop() {
  if( receiver.decode( &results ) ) {
    if( results.value == COLOR ) {
      is_switch_open = openSwitch();
    } else {
      is_switch_open = closeSwitch();
    }
  }
}

/*
 * open Switch, if not already open.
 * 
 */
bool openSwitch() {
  return is_switch_open ? false : true;
}
/*
 * Opposite of the openSwitch()
 */
bool closeSwitch() {
  return false;
}
