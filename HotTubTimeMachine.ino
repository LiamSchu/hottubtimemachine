// include the library code:
#include <LiquidCrystal.h>

// Tub water thermistor stats
#define BCOEFFICIENT 3650 // The beta coefficient of the thermistor (usually 3000-4000)
#define TEMPERATURENOMINAL 20
#define THERMISTOR_NOMINAL_WATER_TEMP 11945      
#define SERIES_RESISTOR_WATER_TEMP 11364  // the value of the fixed resistor used to create the voltage divider
// determine the amount of hysteresis (degrees F overshoot/undershoot before shutoff or before reactivating)
#define HYSTERESIS 0.25

// how many samples to take and average, more takes longer but is more 'smooth'
#define NUMSAMPLES 50
// Set the desired hot tub temperature
float desiredTemp = 101;


// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Define which Arduino pins to use
const uint8_t RELAY_HEATER_PIN = 8, // the number of the Relay 1 Digital I/O pin
//              RELAY_2_PIN = 5,
//              RELAY_3_PIN = 6,
//              RELAY_4_PIN = 7,
              WATER_TEMP_PIN = 14;     // on arduino uno, pin 14 is A0 analog pin
//              HEATER_TEMP_PIN = 15;  // pin A1
           
// Delay between checking or changing states of heater/pump/etc
const long heaterCheckDelay = 10000;  // 5 minutes: interval at which to potentially change whether the heater is on or off (milliseconds)

// Start with the relays open (heater off)
int relayHeaterState = LOW;
//    relay2State = LOW,
//    relay3State = LOW,
//    relay4State = LOW;

// Generally, you should use "unsigned long" for variables that hold time as the value will quickly become too large for an int to store
unsigned long t0HeaterLoop = 0;  // will store last time the heater check was run
float waterTemp = 50;

const int buttonTempUp = 9;  // button pin to increase temperature
const int buttonTempDn = 10;  // button pin to decrease temperature
bool buttonStateInc = 0;  // Is increase Button Pressed
bool buttonStateDec = 0;  // Is decrease Button Pressed
float ChangeTempByDegrees = 0.5; // Press button to change temperature by this degrees F
const float MaxSetPointTemp = 104;
const float MinSetPointTemp = 95;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup(void) 
{
  Serial.begin(9600);
  analogReference(EXTERNAL);

  // set the digital pins assigned to the relays as outputs:
  pinMode(RELAY_HEATER_PIN, OUTPUT);
//  pinMode(RELAY2PIN, OUTPUT);
//  pinMode(RELAY3PIN, OUTPUT);
//  pinMode(RELAY4PIN, OUTPUT);

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Water Temp: ");
  lcd.setCursor(0, 1);
  lcd.print("Heater is: ");
  pinMode(buttonTempUp, INPUT);
  pinMode(buttonTempDn, INPUT);
}

void loop(void) 
{
  waterTemp = RunHeaterCheck();
  Inc_Dec_Temp_Setpoint();
//  randomTempCheck();
//  delay(1000);
}

void randomTempCheck()
{
  waterTemp = ReadThermistor(WATER_TEMP_PIN, SERIES_RESISTOR_WATER_TEMP, THERMISTOR_NOMINAL_WATER_TEMP);

  Serial.print("Temperature "); 
  Serial.print(waterTemp);
  Serial.println(" *F");
  Serial.print("Heater is: ");
  if (relayHeaterState)
  {
    Serial.println("on");
  } else {
    Serial.println("off");
  }
}

float RunHeaterCheck()
{
  if (Is_It_Time(t0HeaterLoop, heaterCheckDelay) == true){
    waterTemp = ReadThermistor(WATER_TEMP_PIN, SERIES_RESISTOR_WATER_TEMP, THERMISTOR_NOMINAL_WATER_TEMP);

    // check the temperature and close the relay if it is too cold, open it if it's warm enough
    if (relayHeaterState==LOW)  // if heater is off, check if the water is cold
    {
      if (waterTemp < desiredTemp - HYSTERESIS) 
      {
        // Open or close the relay (turn on/off the heater)
        relayHeaterState = HIGH;
        digitalWrite(RELAY_HEATER_PIN, relayHeaterState);

        // inform the display
        Serial.print("Heater Came on at water temperature: ");
        Serial.print(waterTemp);
        Serial.println(" *F");

        lcd.setCursor(0, 1);
        lcd.print("Heater is: On   ");
      }
    } else // Heater must be on, check if too warm
    {
      if (waterTemp > desiredTemp + HYSTERESIS) 
      {
        // Open or close the relay (turn on/off the heater)
        relayHeaterState = LOW;
        digitalWrite(RELAY_HEATER_PIN, relayHeaterState);

        // inform the display
        Serial.print("Heater turned off at water temperature: ");
        Serial.print(waterTemp);
        Serial.println(" *F");

        lcd.setCursor(0, 1);
        lcd.print("Heater is: Off  ");
      }
    }

  }
}

bool Is_It_Time(unsigned long timeLastReset, long interval)
{
  // check to see if it's time to retest the heat (or other device); that is, if the difference
  // between the current time and last time the heater was adjusted is bigger than the defined interval
  unsigned long currentTimeMilliSec = millis();
  const long elapsedTime = currentTimeMilliSec - timeLastReset;
  if (elapsedTime >= interval) 
  {
    // set the reset time to now because it is time to run the analysis
    t0HeaterLoop = currentTimeMilliSec;
    return true;
  } else {
    return false;
  }
}

float ReadThermistor(int pinNum, float seriesResistor, float thermistorNominal)
{
  uint8_t i;
  float average;
  average = 0;
  float thermistorImpedance;

  // take N samples in a row and average them
  for (i=0; i< NUMSAMPLES; i++) 
  {
    average += analogRead(pinNum);
  }
  average /= NUMSAMPLES;

  // convert the value to resistance
  thermistorImpedance = seriesResistor / (1023 / average - 1);

  float temperatureF;
  temperatureF = 1 / ((log(thermistorImpedance / thermistorNominal)/BCOEFFICIENT) + (1.0 / (TEMPERATURENOMINAL + 273.15))); // Invert (1/B * ln(R/Ro) + (1/To))
  temperatureF = (temperatureF-273.15) * 9/5 + 32;                                                                 // convert temp C to temp F
  
  lcd.setCursor(11, 0);
  lcd.print(temperatureF);

  return temperatureF;
}

void Inc_Dec_Temp_Setpoint()
{
  if (buttonStateInc != digitalRead(buttonTempUp)) // Read I/O to see if button press action has changed (just pressed or just released)
  {
    buttonStateInc = 1 - buttonStateInc; // True becomes False, False becomes True
    if((desiredTemp < MaxSetPointTemp) && (buttonStateInc == true)) // Temp is below maximum and button has just been pressed
    {
        desiredTemp += ChangeTempByDegrees; // Increase the setpoint temperature by defined amount
        print_Temp_SP(); // show on LCD and send to serial (computer)
    }
  }
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonStateDec != digitalRead(buttonTempDn)) // Read I/O to see if button press action has changed (just pressed or just released)
  {
    buttonStateDec = 1 - buttonStateDec; // True becomes False, False becomes True
    if((desiredTemp > MinSetPointTemp) && (buttonStateDec == true)) // Temp is above minimum and button has just been pressed
    {
      desiredTemp -= ChangeTempByDegrees; // Increase the setpoint temperature by defined amount
      print_Temp_SP(); // show on LCD and send to serial (computer)
    }
  }
}

void print_Temp_SP()
{
  Serial.print("Temperature Set to: ");
  Serial.println(desiredTemp);
  lcd.setCursor(0, 1);
  lcd.print("New Setpt:");
  lcd.setCursor(11, 1);
  lcd.println(desiredTemp);
//  delay(50);
}