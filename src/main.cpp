/*
* A modified design of a junk soldering station
* https://github.com/sergey12malyshev/Solder_Station
* Origin: https://www.allaboutcircuits.com/projects/do-it-yourself-soldering-station-with-an-atmega8/
* Version: 1.1.0
*/
#include <Arduino.h>
#include <PID_v1.h>

#define COMMON_CATHODE     false // Set to true for a common cathode indicator

enum Modes {WORK, NO_SOLDER};
Modes mode = WORK;

static int16_t digit_common_pins[] = {A3, A4, A5}; // General conclusions for the triple 7-segment LED indicator
static int16_t max_digits = 3;
static int16_t current_digit = max_digits - 1;

static uint32_t updaterate = 500; // Changes how often the indicator is updated. Not less than 500
static uint32_t lastupdate;

static int16_t temperature = 0;

/* Defines aggressive and conservative settings of the PID controller */ 
double aggKp = 4, aggKi = 0.2, aggKd = 1;
double consKp = 1, consKi = 0.05, consKd = 0.25;

double Setpoint, Input, Output;

PID myPID(&Input, &Output, &Setpoint, consKp, consKi, consKd, DIRECT); // Set links and initial settings

static void show(int16_t value, Modes workMode); 

static void convertThermocoupleData(void)
{
  const uint16_t measured = 303; // The measured value during calibration (Already adjusted to its design)
  Input = analogRead(0);  // Read the temperature
  Input = map(Input, 0, measured, 25, 350); // Convert a 10-bit number to degrees Celsius
}

static double convertReostatData(void)
{  
  double newSetpoint = analogRead(1); // Read the set value by the potentiometer
  newSetpoint = map(newSetpoint, 0, 1023, 150, 350); // and convert it to degrees Celsius (minimum 150, maximum 350)
  return newSetpoint;
}

static inline void heratbeatLed(void)
{
  PORTB ^=(1UL << PINB5);
}

static inline void disablePWM(void)
{
  analogWrite(11, 0);
}

static void regulator(void)
{
  double gap = abs(Setpoint - Input); // Distance from the set value

  if (gap < 10.0)
  { // we are close to the set value, using conservative settings
    myPID.SetTunings(consKp, consKi, consKd);
  }
  else
  {
    // we are far from the set value, we use aggressive settings
    myPID.SetTunings(aggKp, aggKi, aggKd);
  }
  myPID.Compute();
  analogWrite(11, Output); // Install PWM
}

int main(void)
{
  init();
 //initVariant();

  DDRD = B11111111;       // Set the ATMEGA pins of port D from 0 to 7 as outputs
  DDRB |= (1UL << PINB5); // Let's set pb5 to the output

  for (int16_t y = 0; y < max_digits; y++)
  {
    pinMode(digit_common_pins[y], OUTPUT);
  }

  myPID.SetOutputLimits(0, 220); // Setting the power limit to a maximum of 85% (220/255)
  myPID.SetMode(AUTOMATIC);
  lastupdate = millis();
  Setpoint = 0;

  while(true) 
  {
    convertThermocoupleData();
     
    if (millis() - lastupdate > updaterate)  /* Update the sting temperature */
    {
      lastupdate = millis();
      temperature = Input; 
      heratbeatLed();
    }

    double newSetpoint = convertReostatData();
    double change  = abs(newSetpoint - Setpoint);

    if (change > 3)   /* Display the set value if there was a change */
    {
      Setpoint = newSetpoint;
      temperature = newSetpoint;
      lastupdate = millis();
    }

    /* State machine */
    if (temperature <= 380)
    {
      regulator();
      show(temperature, WORK);
    }
    else
    {
      show(temperature, NO_SOLDER); // We will display an accident (overheating or lack of soldering iron)
      disablePWM();
    }

    if (serialEventRun) serialEventRun(); // is loop
  }
}

static void show(int16_t value, Modes workMode) 
{
  uint8_t const digits[] = {
    B00111111, B00000110, B01011011, B01001111, B01100110, B01101101, B01111101, B00000111, B01111111, B01101111,\
    B01000000 /*- no soldering-iron symbol*/
  };
  const uint8_t noSolderingIronSymbol = 10u; // ---
  int16_t digits_array[] = {0};
  bool empty_most_significant = true;

  for (int16_t z = max_digits - 1; z >= 0; z--) // Cycle through all digits
  {
    if (workMode == WORK)
    {
      digits_array[z] = value / pow(10, z); // Now we take each digit from the number
    }
    else if(workMode == NO_SOLDER)
    {
     digits_array[z] = noSolderingIronSymbol; 
    }
    
    if (digits_array[z] != 0 ) 
        empty_most_significant = false; // Do not display the leading zeros
    
    value = value - digits_array[z] * pow(10, z);
    if (z == current_digit)
    {
      if (!empty_most_significant || z == 0) // Check that this is not a leading zero, and display the current digit
      { 
#if (!COMMON_CATHODE)
        PORTD = ~digits[digits_array[z]];
#endif
      }
      else
      {
        PORTD = B11111111;
      }
#if COMMON_CATHODE
     digitalWrite(digit_common_pins[z], LOW);
#else
     digitalWrite(digit_common_pins[z], HIGH);
#endif
    } 
    else 
    {
#if COMMON_CATHODE
      digitalWrite(digit_common_pins[z], HIGH);
#else
      digitalWrite(digit_common_pins[z], LOW);
#endif
    }

  }
  current_digit--;
  if (current_digit < 0)
  {
    current_digit = max_digits;
  }
}