#include <IRremote.h>

#define REPORT    1 // Report to Serial

//Verbindung der Farbsensorkontakte mit dem Arduino festlegen
#define s0        8
#define s1        9
#define out       10
#define s3        11
#define s2        12
#define IR        13

IRsend irsend;

float Tolerance             = 0.10;  // Für RGB%. Zu Beginn sind 10% Sollwertabweichung erlaubt
float Tolerance_Brightness  = 0.15;

/*
 * All Smartie Colors - if you change this you have to change this in the SmartSwitch.ino
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


/*
Definition of a color in rgb% values
*/
struct Color {
  double red;
  double green;
  double blue;
  double brightness;
  enum Colors classification;
};

/*
 * Ranges for how a class of Colors is defined ( all browns, all blues, all yellows )
 * All these are %-values
 */
struct Color_Template {
  
  float red_upper;
  float red_lower;
  
  float green_upper;
  float green_lower;
  
  float blue_upper;
  float blue_lower;

  enum Colors classification;
};


// Tabelle: Mittelwerte zur Smartieerkennung. Diese Werte wurden gemessen. ( Previously know as GWxxx[] )
//    FARBE          R%     G%     B%     Helligkeit-absolut
struct Color averages[9]          =  {
  {   33.8 ,  38.8 ,  27.5 ,    201.0,    Blue      },
  {   30.8 ,  30.5 ,  38.9 ,    193.0,    Green     },
  {   16.1 ,  34.2 ,  50.5 ,    135.0,    Yellow    }, 
  {   21.5 ,  47.6 ,  31.0 ,    1247.0,   Rosa      }, 
  {   30.0 ,  44.0 ,  26.5 ,    1300.0,   Purple    }, 
  {   16.8 ,  47.1 ,  36.1 ,    1671.0,   Red       }, 
  {   17.8 ,  42.1 ,  40.1 ,    1343.0,   Orange    }, 
  {   23.0 ,  39.5 ,  37.5 ,    1630.0,   Brown     }, 
  {   17.8 ,  35.8 ,  46.7 ,    172.0,    Empty     }
};

// Tabelle: Grenzwerte zur Smartieerkennung. Diese Werte werden in loops() abhängig von der Toleranz errechnet
// Aufbau: TW_xxx[1]=unterer Grenzwert ROT, [2]= oberer Grenzwert ROT, ...
struct Color_Template colors[9] = {
  {   0,  0,   0,  0,   0,  0,  Brown    },
  {   0,  0,   0,  0,   0,  0,  Green    },
  {   0,  0,   0,  0,   0,  0,  Yellow   },
  {   0,  0,   0,  0,   0,  0,  Rosa     },
  {   0,  0,   0,  0,   0,  0,  Purple   },
  {   0,  0,   0,  0,   0,  0,  Red      },
  {   0,  0,   0,  0,   0,  0,  Blue     },
  {   0,  0,   0,  0,   0,  0,  Orange   },
  {   0,  0,   0,  0,   0,  0,  Empty    }
};


void setup() {
  pinMode( s0, OUTPUT );
  pinMode( s1, OUTPUT );
  pinMode( s2, OUTPUT );
  pinMode( s3, OUTPUT );
  pinMode( out, INPUT );
  digitalWrite( s0, HIGH );
  digitalWrite( s1, LOW );

  #ifdef REPORT
  Serial.begin(115200);
  #endif

  /* 
   *  Calibrate brigthness ( classification is useless now )
   */
  double brightness_calibration = 0;
  for ( size_t i = 1; i <= 20; i++ ) {
    struct Color c = color();
    brightness_calibration += c.brightness;
  }
  brightness_calibration = brightness_calibration / 20;
  double differences = brightness_calibration - averages[8].brightness;
  for ( size_t i = 0; i < 9; i++ ) {
    averages[i].brightness + differences;
  }

   
}

void loop() {
  for( size_t i = 0; i < 9; i++ ) {
    colors[i].red_upper   = averages[i].red * ( 1.0 + Tolerance );
    colors[i].red_lower   = averages[i].red * ( 1.0 - Tolerance );
    
    colors[i].green_upper = averages[i].green * ( 1.0 + Tolerance );
    colors[i].green_lower = averages[i].green * ( 1.0 - Tolerance );

    colors[i].blue_upper  = averages[i].blue * ( 1.0 + Tolerance );
    colors[i].blue_lower  = averages[i].blue * ( 1.0 - Tolerance );
  } 

  struct Color current_color = color();
  
  switch( current_color.classification ) {
    case Empty:
      break;
    default:
      irsend.sendSony( current_color.classification, IR);
  }
  delay(500);
}




/*
 * Return the Color + Classification
 */
struct Color color() {
  float red, green, blue;
  // Sensor für Rot-Messung einstellen
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  delay(50);
  red   = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  red   += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  red   += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  red   = (red / 3) + 0.00001;

  // Sensor für Blau-Messung einstellen
  digitalWrite(s3, HIGH); delay(50);
  blue  = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  blue  += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  blue  += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  blue  = (blue / 3) + 0.00001;

  // Sensor für Grün-Messung einstellen
  digitalWrite(s2, HIGH); delay(50);
  green   = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  green   += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  green   += pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  green   = (green / 3) + 0.00001;

  float brightness = green + blue + red;

  red      = red / brightness * 100;
  green    = green / brightness * 100;
  blue     = blue / brightness * 100;

  struct Color color;

  for ( size_t i = 0; i < 9; i++ ) {
    if(   colors[i].red_upper     > red 
      &&  colors[i].red_lower     < red
      &&  colors[i].green_upper   > green
      &&  colors[i].green_lower   < green
      &&  colors[i].blue_upper    > blue
      &&  colors[i].blue_lower    < blue
      ) {
        color.classification  = colors[i].classification;
        color.red             = red;
        color.green           = green;
        color.blue            = blue;
      }
  }
  if ( ! color.classification ) {
    color.classification = Empty;
  }
  return color;
} 
