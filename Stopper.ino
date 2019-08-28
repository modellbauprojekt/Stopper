#include <Wire.h>

#define REPORT    1 // Report to Serial

//Verbindung der Farbsensorkontakte mit dem Arduino festlegen
#define s1        9
#define s0        8
#define s2        12
#define s3        11
#define out       10

float Tolerance             = 0.10;  // Für RGB%. Zu Beginn sind 10% Sollwertabweichung erlaubt
float Tolerance_Brightness  = 0.15;

/*
 * All Smartie Colors
 */
enum Colors {
  Brown     = 0,
  Green     = 1,
  Yellow    = 2,
  Rosa      = 3,
  Purple    = 4,
  Red       = 5,
  Blue      = 6,
  Orange    = 7,
  Empty     = 8
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


// Tabelle: Mittelwerte zur Smartieerkennung. Diese Werte wurden gemessen.
//    FARBE          R%     G%     B%     Helligkeit-absolut
struct Color averages[9]          =  {
  { 33.8 , 38.8 , 27.5 ,  201.0  },
  { 30.8 , 30.5 , 38.9 ,  193.0  },
  { 16.1 , 34.2 , 50.5 ,  135.0  }, 
  { 21.5 , 47.6 , 31.0 ,  1247.0 }, 
  { 30.0 , 44.0 , 26.5 ,  1300.0 }, 
  { 16.8 , 47.1 , 36.1 ,  1671.0 }, 
  { 17.8 , 42.1 , 40.1 ,  1343.0 }, 
  { 23.0 , 39.5 , 37.5 ,  1630.0 }, 
  { 17.8 , 35.8 , 46.7 ,  172.0  }
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

  for ( size_t i = 1; i < 21; i++ ) {
    struct Color c = color();
    
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
}

// Hier werden die Werte vom Farbsensor ausgelesen und unter den
// entsprechenden Variablen gespeichert
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
