/* Includes ---------------------------------------------------------------- */
#include <HAR_inferencing.h>
#include <Adafruit_MPU6050.h> // Use Adafruit library for MPU6050
#include <Adafruit_Sensor.h>
#include <Wire.h>

/* Constant defines -------------------------------------------------------- */
#define MAX_ACCEPTED_RANGE  50.0f

/* Private variables ------------------------------------------------------- */
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
Adafruit_MPU6050 mpu;
Adafruit_Sensor *mpu_accel, *mpu_gyro;
sensors_event_t a, g;

int err;
signal_t input_signal;
ei_impulse_result_t result = { 0 };

/**
* @brief      Arduino setup function
*/

void mpu_setup(){
  //Initializing Sensor Communication
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  
  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  //mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  
  
  mpu_accel = mpu.getAccelerometerSensor();
  //mpu_accel->printSensorDetails();

  mpu_gyro = mpu.getGyroSensor();
  //mpu_gyro->printSensorDetails();
}


void ml_setup()
{
    if (EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME != 6) { // Changed to 6 for accelerometer and gyroscope
        ei_printf("ERR: EI_CLASSIFIER_RAW_SAMPLES_PER_FRAME should be equal to 6 (the 3 accelerometer axes + 3 gyroscope axes)\n");
        return;
    }
}

/**
 * @brief Return the sign of the number
 * 
 * @param number 
 * @return int 1 if positive (or 0) -1 if negative
 */
float ei_get_sign(float number) {
    return (number >= 0.0) ? 1.0 : -1.0;
}

/**
* @brief      Get data and run inferencing
*
* @param[in]  debug  Get debug info if true
*/


void sample_mpu()
{
    //ei_printf("Sampling...\n");

    // Allocate a buffer here for the values we'll read from the IMU
    float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE] = { 0 };

    for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix += 6) { // Changed to 6 for accelerometer and gyroscope
        // Determine the next tick (and then sleep later)
        uint64_t next_tick = micros() + (EI_CLASSIFIER_INTERVAL_MS * 1000);

        mpu_accel->getEvent(&a);
        mpu_gyro->getEvent(&g);

        buffer[ix] = a.acceleration.x;
        buffer[ix + 1] = a.acceleration.y;
        buffer[ix + 2] = a.acceleration.z;
        buffer[ix + 3] = g.gyro.x;
        buffer[ix + 4] = g.gyro.y;
        buffer[ix + 5] = g.gyro.z;

        /*for (int i = 0; i < 6; i++) {
            if (fabs(buffer[ix + i]) > MAX_ACCEPTED_RANGE) {
                buffer[ix + i] = ei_get_sign(buffer[ix + i]) * MAX_ACCEPTED_RANGE;
            }
        }
        
        buffer[ix + 0] *= CONVERT_G_TO_MS2;
        buffer[ix + 1] *= CONVERT_G_TO_MS2;
        buffer[ix + 2] *= CONVERT_G_TO_MS2;*/

        /*Serial.print(millis());
        Serial.print(","); Serial.print(buffer[ix]);
        Serial.print(","); Serial.print(buffer[ix+1]);
        Serial.print(","); Serial.print(buffer[ix+2]);
        Serial.print(","); Serial.print(buffer[ix+3]);
        Serial.print(","); Serial.print(buffer[ix+4]);
        Serial.print(","); Serial.print(buffer[ix+5]);
        Serial.println();*/

        

        delayMicroseconds(next_tick - micros());
    }

    // Turn the raw buffer in a signal which we can the classify
    err = numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &input_signal);
    if (err != 0) {
        ei_printf("Failed to create signal from buffer (%d)\n", err);
        return;
    }
}


float inference(){
  enum Activity { idle = 20, walk = 15, run = 10 };
  float activity[3] = {idle, run, walk};
  // Run the classifier

    err = run_classifier(&input_signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
    }

  int predicted_activity = 0;
  for (int i = 1; i <= 4; i++) 
  {
    if (result.classification[i].value > result.classification[predicted_activity].value) 
    {
      predicted_activity = i;
    }
  }
  return activity[predicted_activity];

#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif
}


void print_prediction(){
  // print the predictions
    ei_printf("Predictions ");
    ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);
    ei_printf(": \n");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
    }
}


void mpu_reading(){
  mpu_accel->getEvent(&a);
  mpu_gyro->getEvent(&g);

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

/*#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_ACCELEROMETER
#error "Invalid model for current sensor"
#endif*/
