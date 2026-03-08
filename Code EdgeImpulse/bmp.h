#include <Adafruit_BMP280.h>
#include <Wire.h>

Adafruit_BMP280 bmp;
#define BMP280_ADDRESS 0x76
float hysteresis = 1; // Hysteresis value in inHg

//Varibles imported from main to detect interrupt
extern bool act, finish;

void bmp_setup(){
  //Initializing Sensor Communication
  if (!bmp.begin(BMP280_ADDRESS)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }

  //Setting Sampling Mode
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. FORCED OR NORMAL */
                  Adafruit_BMP280::SAMPLING_NONE,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X4,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X4,         /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500);   /*Stand by Time*/     
}


void regulatePressure(float setPoint, int pinPump, int pinValve){
  float pressure;
  
  //Taking Forced Measurement 
  if(bmp.takeForcedMeasurement()){
    pressure= (bmp.readPressure()/3386);
  }
  
  //Calculate Pressure Difference
  float pressureDiff = pressure -  setPoint;

  //Turn on Pump if Pressure is higher than required  
  if (pressureDiff > hysteresis) {
    while (pressure > setPoint) {
        digitalWrite(pinPump, HIGH);
        digitalWrite(pinValve, LOW);
        if(bmp.takeForcedMeasurement()){
          pressure= (bmp.readPressure()/3386);
        }
        pressureDiff = pressure -  setPoint;
        Serial.print("Pump, ");
        Serial.print(setPoint);
        Serial.print(", ");
        Serial.println(pressure);
        delay(10);
        if (act || finish){
          break;
        }
    }
    digitalWrite(pinPump, LOW);
    digitalWrite(pinValve, LOW);
  }
  //Turn on Valve if Pressure is lower than required 
  else if (pressureDiff < -hysteresis) {
    while (pressure < setPoint) {
        digitalWrite(pinPump, LOW);
        digitalWrite(pinValve, HIGH);
        if(bmp.takeForcedMeasurement()){
          pressure= (bmp.readPressure()/3386);
        }
        pressureDiff = pressure -  setPoint;
        Serial.print("Valve, ");
        Serial.print(setPoint);
        Serial.print(", ");
        Serial.println(pressure);
        delay(10);
        if (act || finish){
          break;
        }
    }
    digitalWrite(pinPump, LOW);
    digitalWrite(pinValve, LOW);
  }
  //Keep it as it is if no regulation required 
  else {
      digitalWrite(pinPump, LOW);
      digitalWrite(pinValve, LOW);
  }
}