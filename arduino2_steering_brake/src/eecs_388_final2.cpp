/******************************************************************************
 *   Header Includes
 *******************************************************************************/
#include <Arduino.h>
#include <stdint.h>
#include <stdio.h>

#include "eecs_388_lib.h"

/******************************************************************************
 *   Constant definitions
 *******************************************************************************/

/*ServoMotor constants*/
#define SERVO_PULSE_MAX (2400)    /* 2400 us */
#define SERVO_PULSE_MIN (544)     /* 544 us */
#define SERVO_PERIOD    (20000)   /* 20000 us (20ms) */
#define MIN_ANGLE       (0)       /* degrees */
#define MAX_ANGLE       (180)     /* degrees */
#define FLASH           (100)     /* 100 ms */


/******************************************************************************
 *   Local function prototypes
 *******************************************************************************/
static void auto_brake();
static void steering(int pos);

/******************************************************************************
 *   Function: auto_brake() - Auto Brake
 *      Pre condition: 
 *          None
 *      Post condition: 
 *          Checks the LiDAR distance and configures LEDs
 *******************************************************************************/
static void auto_brake()
{
  // Task-2: 
  // Your code goes here (Use Lab 2 & 4 for reference)
  // Check the project document to understand the task
  uint8_t dist_rcvd_low = ser_read();
  uint8_t dist_rcvd_high = ser_read();
  uint16_t dist = dist_rcvd_low | (dist_rcvd_high >> 8);
  ser_printf("%d", dist);

  if (dist > 200){ // Safe distance, no braking
    gpio_write(GPIO_13, OFF); //RED OFF
    gpio_write(GPIO_12, ON); //GREEN ON
  } else if (dist > 100) { //Close, brake lightly
    gpio_write(GPIO_13, ON); //YELLOW ON
    gpio_write(GPIO_12, ON);
  } else if (dist > 60) { // Very close, brake hard
    gpio_write(GPIO_12, OFF); //GREEN OFF
    gpio_write(GPIO_13, ON); //RED ON
  } else { //Too close, must stop
    gpio_write(GPIO_12, OFF); //GREEN OFF
    //RED FLASH
    for (int i=0; i < 20; i++){ // 2 seconds
      gpio_write(GPIO_13, ON); //RED FLASH => STOP
      delay_ms(FLASH);
      gpio_write(GPIO_13, OFF);
    }
  }

  return;
}


/******************************************************************************
 *   Function: steering() - Steering
 *      Pre condition: 
 *          None
 *      Post condition: 
 *          Control the servomotor with GPIO_6
 *******************************************************************************/
static void steering(int pos)
{
    // Task-4: 
    // Your code goes here (Use Lab 05 for reference)
    // Check the project document to understand the task
    int pwm = SERVO_PULSE_MAX + ((pos * (SERVO_PULSE_MAX - SERVO_PULSE_MIN)) / 180);
    gpio_write(GPIO_6, ON);
    delay_us(pwm);
    gpio_write(GPIO_6, OFF);
    delay_us(SERVO_PERIOD - pwm);
}

/******************************************************************************
 *   Function: setup() - Initializes the Arduino System
 *      Pre condition: 
 *          Hardware must be properly connected (BMP180 sensors, etc.)
 *      Post condition: 
 *          Runs initialization calls one time on power up
 *          UART is initialized for ser_printf()
 *          I2C is initialized for communication with BMP180
 *          calib_data is filled with calibration data from BMP180
 *******************************************************************************/
void setup() 
{
uart_init();     // Initialize UART for serial output

//Setup Auto-break LEDs for Distance
gpio_mode(GPIO_13, GPIO_OUTPUT); //RED
gpio_mode(GPIO_12, GPIO_OUTPUT); //GREEN
gpio_mode(GPIO_11, GPIO_OUTPUT); //BLUE

//Setup GPIO_6 for PWM output
gpio_mode(GPIO_6, GPIO_OUTPUT);

ser_printf("System Initialized");
}

/******************************************************************************
 *   Function: loop() - Main execution loop
 *      Pre condition: 
 *          setup() has been executed and system is initialized
 *      Post condition: 
 *          Performs a single iteration of the system's function
 *          Repeates indefinetely unless the board is reset or powered off
 *******************************************************************************/
void loop() 
{
// Task-4: 
// Setup simulated code for the angles from the lab sheet
int angle_values[11] = {10, 25, 75, 45, 100, 40, 125, 15, 150, 50, 170};

// Task-1&2:
// Receive data from arudino1 for the distance and setup the LEDs
auto_brake();

//  Task-4:
for (int i = 0; i < 11; i++)
  {
  // Here, we set the angle to 180 if the prediction from the DNN is a positive angle
  // and 0 if the prediction is a negative angle.
  // This is so that it is easier to see the movement of the servo.
  // You are welcome to pass the angle values directly to the steering function.
  // If the servo function is written correctly, it should still work,
  // only the movements of the servo will be more subtle
  if( angle_values[i] > 0 )
    {
    steering(180);
    }
  else 
    {
    steering(0);
    }
           
  // Uncomment the line below to see the actual angles on the servo.
  // Remember to comment out the if-else statement above!
  // steering(angle);
  }
  //FOR THE PRESENTATION: MOST IMPORTANT PART IS WHAT WOULD I ADD IN THE FUTURE
}
