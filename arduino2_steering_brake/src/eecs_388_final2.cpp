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


/******************************************************************************
 *   Local function prototypes
 *******************************************************************************/
static void auto_brake();
static void steering(int pos);
static void start_red_flash();
static void stop_red_flash();


/******************************************************************************
 *  Global variables
 *******************************************************************************/
volatile uint8_t red_flash_active = 0;
volatile uint8_t led_state = 0;


/******************************************************************************
 *   Function: start_red_flash() - Enable red LED flashing
 *******************************************************************************/
static void start_red_flash(){
  if (!red_flash_active){
    red_flash_active = 1;
    led_state = 0;
    gpio_write(GPIO_13, OFF);
    set_cycles(6250); //reset timer count
    enable_timer_interrupt();
  }
  return;
}


/******************************************************************************
 *   Function: stop_red_flash() - Disable red LED flashing
 *******************************************************************************/
static void stop_red_flash(){
  if (red_flash_active){
    red_flash_active = 0;
    led_state = 0;
    disable_timer_interrupt();
    gpio_write(GPIO_13, OFF);
  }
}


/******************************************************************************
 *   Function: auto_brake() - Auto Brake
 *      Pre condition: 
 *          None
 *      Post condition: 
 *          Checks the LiDAR distance and configures LEDs
 *******************************************************************************/
static void auto_brake()
{
  uint8_t dist = ser_read(); 

  if (dist == 0xA4){ // Safe distance, no braking
    stop_red_flash();
    gpio_write(GPIO_13, OFF); 
    gpio_write(GPIO_12, ON); //GREEN ON
    gpio_write(GPIO_11, OFF); 
  } else if (dist == 0xA3) { //Close, brake lightly
    stop_red_flash();
    gpio_write(GPIO_13, OFF); 
    gpio_write(GPIO_12, OFF);
    gpio_write(GPIO_11, ON); //BLUE ON
  } else if (dist == 0xA2) { // Very close, brake hard
    stop_red_flash();
    gpio_write(GPIO_13, ON); //RED ON
    gpio_write(GPIO_12, OFF); 
    gpio_write(GPIO_11, OFF); 
  } else if (dist == 0xA1){ //Too close, must stop
    gpio_write(GPIO_12, OFF); 
    gpio_write(GPIO_11, OFF); 
    start_red_flash(); //RED FLASH
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
  // example: pos = 30 => pwm = 850us
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
  if (!red_flash_active){
    gpio_write(GPIO_13, OFF);
    return;
  }

  led_state ^= 1;
  gpio_write(GPIO_13, (led_state ? ON : OFF));
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

//setup timer for 100ms - keep timer configured but disabled until needed
set_cycles(6250);
disable_timer_interrupt();
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
  // Random steering values
  int angle_values[11] = {10, 25, 75, 45, 100, 40, 125, 15, 150, 50, 170};

  //Steer & Brake
  for (int i = 0; i < 11; i++) {
    for (int j = 0; j < 50; j++) {
      steering(angle_values[i]);
      auto_brake();
    }
  }
}
