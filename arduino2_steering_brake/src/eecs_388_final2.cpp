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

//Global variables
volatile uint8_t interrupt_count = 0;
volatile uint8_t is_red = 1;
volatile uint8_t led_state = 0;

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
  uint8_t dist = ser_read(); 

  if (dist == 0xA4){ // Safe distance, no braking
    gpio_write(GPIO_13, OFF); //RED OFF
    gpio_write(GPIO_12, ON); //GREEN ON
  } else if (dist == 0xA3) { //Close, brake lightly
    gpio_write(GPIO_13, ON); //YELLOW ON
    gpio_write(GPIO_12, ON);
  } else if (dist == 0xA2) { // Very close, brake hard
    gpio_write(GPIO_12, OFF); //GREEN OFF
    gpio_write(GPIO_13, ON); //RED ON
  } else if (dist == 0xA1){ //Too close, must stop
    gpio_write(GPIO_12, OFF); //GREEN OFF
    gpio_write(GPIO_13, ON);
    //TO DO
    //NEED TO REMOVE THIS FOR LOOP AND GENERATE INTERRUPT
    for (int i=0; i < 20; i++){ // flash for 2 seconds
      gpio_write(GPIO_13, ON); //gpio_write(GPIO_13, (red_led_state == 1 ? ON : OFF)); //RED FLASH => STOP
      delay_ms(FLASH);
      gpio_write(GPIO_13, ON);
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
static void steering(int pos) {
  // Task-4: 
  // Your code goes here (Use Lab 05 for reference)
  // Check the project document to understand the task
  int pwm = SERVO_PULSE_MIN + (pos * ((SERVO_PULSE_MAX - SERVO_PULSE_MIN) / 180)); //540-2400us
  // ex: pos = 30 => pwm = 850us
  gpio_write(GPIO_6, ON);
  delay_us(pwm);

  //Delay remaining pulse
  gpio_write(GPIO_6, OFF);
  if (pwm < 1000){
    delay_ms(19);
    delay_us(1000-pwm);
  } else if (pwm < 2000) {
    delay_ms(18);
    delay_us(2000-pwm);
  } else {
    delay_ms(17);
    delay_us(3000-pwm);
  }
  return;
}

/******************************************************************************
 *  INTERRUPT SERVICE ROUTINE. This is the function the ATmega328P jumps to when TCNT1 == OCR1A
 *******************************************************************************/
ISR(TIMER1_COMPA_vect) {
    interrupt_count++; // keep track of how many intervals have elapsed

    led_state ^= 1; // toggle LED to create blinking effect

    if (is_red) {
        gpio_write(GPIO_13, led_state ? ON : OFF);
        gpio_write(GPIO_11, OFF);
    } else {
        gpio_write(GPIO_11, led_state ? ON : OFF);
        gpio_write(GPIO_13, OFF);
    }

    if (interrupt_count >= 10) {
        interrupt_count = 0;
        is_red ^= 1;
    }
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

//setup timer for 100ms
enable_timer_interrupt();
set_cycles(6250);
enable_interrupt();

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
void loop() {
  // Task-4: 
  // Setup simulated code for the angles from the lab sheet
  int angle_values[11] = {10, 25, 75, 45, 100, 40, 125, 15, 150, 50, 170};
  //uint8_t dist = ser_read(); 
  
  while(1){
    // Task-1&2:
    // Receive data from arudino1 for the distance and setup the LEDs
    auto_brake();

    //  Task-4:
    for (int i = 0; i < 11; i++) {
      for (int j = 0; j < 50; j++) {
        steering(angle_values[i]);
      }
    }
  }
}
