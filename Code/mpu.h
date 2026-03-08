#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

Adafruit_MPU6050 mpu;
Adafruit_Sensor *mpu_accel, *mpu_gyro;
sensors_event_t a, g;

void mpu_setup(){
  //Initializing Sensor Communication
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  //mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);


  mpu_accel = mpu.getAccelerometerSensor();
  //mpu_accel->printSensorDetails();

  mpu_gyro = mpu.getGyroSensor();
  //mpu_gyro->printSensorDetails();
}

void mpu_reading(){
  //setting reading cycle
  mpu.setCycleRate(MPU6050_CYCLE_20_HZ);
  /*//ensuring sleep mode is not active
  mpu.enableSleep(false);
  //enabling cycle mode to get reading
  mpu.enableCycle(true);*/

  delay(100);
  mpu_accel->getEvent(&a);
  mpu_gyro->getEvent(&g);

  //Putting to Sleep
  /*mpu.enableCycle(false);
  mpu.enableSleep(true);*/

  //Value Display
  Serial.print(a.acceleration.x);
  Serial.print(","); Serial.print(a.acceleration.y);
  Serial.print(","); Serial.print(a.acceleration.z);
  Serial.print(","); Serial.print(g.gyro.x);
  Serial.print(","); Serial.print(g.gyro.y);
  Serial.print(","); Serial.print(g.gyro.z);
  Serial.println();
  delay(10);
  
}