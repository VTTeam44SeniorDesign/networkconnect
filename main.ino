/* 
  Sketch generated by the Arduino IoT Cloud Thing "Untitled"
  https://create.arduino.cc/cloud/things/a0778f3b-94e9-402c-81b0-b9429841d0b9 

  Arduino IoT Cloud Variables description

  The following variables are automatically generated and updated when changes are made to the Thing

  bool autonomous_mode;
  bool test_switch;
  bool turn_left;
  bool turn_right;

  Variables which are marked as READ/WRITE in the Cloud Thing will also have functions
  which are called when their values are changed from the Dashboard.
  These functions are generated with the Thing and added at the end of this sketch.
*/

#include "thingProperties.h"

// Required initializations for TimerInterrupt Library
 #if !( ( defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_PORTENTA_H7_M4) || defined(ARDUINO_GIGA) ) && defined(ARDUINO_ARCH_MBED) )
   #error This code is designed to run on Portenta_H7 platform! Please check your Tools->Board setting.
 #endif
 // These define's must be placed at the beginning before #include "Portenta_H7_TimerInterrupt.h"
 // _TIMERINTERRUPT_LOGLEVEL_ from 0 to 4
 // Don't define _TIMERINTERRUPT_LOGLEVEL_ > 0. Only for special ISR debugging only. Can hang the system.
 #define _TIMERINTERRUPT_LOGLEVEL_     4

// Include Header Files
 // Can be included as many times as necessary, without `Multiple Definitions` Linker Error
 #include "Portenta_H7_TimerInterrupt.h"
 // To be included only in main(), .ino with setup() to avoid `Multiple Definitions` Linker Error
 #include "Portenta_H7_ISR_Timer.h"

// Define Constants
 // Input Pin assignment definitions
 // Pins 24 & 25, 30 & 31, 32 & 33, 34 & 35, 36 & 37, 38 & 39 did not work with interrupts (we Do Not know why)
 // ***Motor 1: Front Left***
 //    Motor CCW == Wheel Spins Backward
 //    Motor CW == Wheel Spins Forward
 #define M1_A_PIN 22
 #define M1_B_PIN 23
 // ***Motor 2: Front Right***
 //    Motor CCW == Wheel Spins Forward
 //    Motor CW == Wheel Spins Backward
 #define M2_A_PIN 26
 #define M2_B_PIN 27
 // ***Motor 3: Back Left***
 //    Motor CCW == Wheel Spins Backward
 //    Motor CW == Wheel Spins Forward
 #define M3_A_PIN 28
 #define M3_B_PIN 29
 // ***Motor 4: Back Right***
 //    Motor CCW == Wheel Spins Backward
 //    Motor CW == Wheel Spins Forward
 #define M4_A_PIN 40
 #define M4_B_PIN 41
 // Output Pin assignment definitions
 // GPIO Pins 2 through 13 can be used as PWM channels
 // Our motor drivers don't have an EN pin to switch polarity, so each motor needs 2 PWM channels (1 for CCW & 1 for CW)
 #define PWM_M1_CCW 2
 #define PWM_M1_CW 3
 #define PWM_M2_CCW 4
 #define PWM_M2_CW 5
 #define PWM_M3_CCW 6
 #define PWM_M3_CW 7
 #define PWM_M4_CCW 8
 #define PWM_M4_CW 9
 // Timer Properties
 // TIMER_INTERVAL_MS = (1 / Timer_Frequency_Hz) * 1000
 #define FS_HZ                     200    
 //#define COUNTS_PER_REVOLUTION     3415.92    // !!!! This value is only compatible with the 118 RPM motors
 #define COUNTS_PER_REVOLUTION     4776.384   // !!!! This value is only compatible with the 84 rpm motors
 #define PI                        3.14159
 #define WHEEL_DIAMETER_FT         0.5    // 0.5 ft = 6 in
 #define FEET_PER_GRID_SQUARE      10    // UNITS: feet

// ** Declare Global Variables **
 // Timer
 const double TIMER_INTERVAL_MS  = (1.0/FS_HZ) * 1000;
 // Position Control
 const double COUNTS_PER_GRID_SQUARE = (FEET_PER_GRID_SQUARE / ( PI*(WHEEL_DIAMETER_FT) ) ) * COUNTS_PER_REVOLUTION;
 const double COUNTS_PER_90_DEG_TURN = 0.75 * COUNTS_PER_REVOLUTION;
 // Encoder Counts
 static bool M1_A_val;
 static bool M1_B_val;
 static bool M2_A_val;
 static bool M2_B_val;
 static bool M3_A_val;
 static bool M3_B_val;
 static bool M4_A_val;
 static bool M4_B_val;
 static long M1_count;
 static long M2_count;
 static long M3_count;
 static long M4_count;
 // Velocity Calculation
 static long M1_count_km1;
 static long M2_count_km1;
 static long M3_count_km1;
 static long M4_count_km1;
 static int Encoder_velocity;
 static bool TimerInterrupt_flag;
 static double M1_rpm;
 static double M2_rpm;
 static double M3_rpm;
 static double M4_rpm;
 // Controller
 static bool start_enable, steady_enable, left_enable, right_enable;
 static float m, dt;  // setpoint = m * dt
 static float t_endctrl;
 static int eps;  // tollerance for position control (in encoder counts)
 static float Kp_v, Ki_v, Kd_v;
 static float Kp_p, Ki_p, Kd_p;
 static double setpoint, error, M1_error_km1, M2_error_km1, M3_error_km1, M4_error_km1, edot;
 static double error_integral_M1, error_integral_M2, error_integral_M3, error_integral_M4;
 static int PWM_control_signal1;
 static int PWM_control_signal2;
 static int PWM_control_signal3;
 static int PWM_control_signal4;
 
void setup() {
  // Initialize serial and wait for port to open:
  Serial.begin(9600);
  // This delay gives the chance to wait for a Serial Monitor without blocking if none is found
  delay(1500); 

  // Defined in thingProperties.h
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  
  /*
     The following function allows you to obtain more information
     related to the state of network and IoT Cloud connection and errors
     the higher number the more granular information you’ll get.
     The default is 0 (only errors).
     Maximum is 4
 */
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
  
  // Initialize Variables
  // Encoder Counts
  M1_A_val = digitalRead(M1_A_PIN);
  M1_B_val = digitalRead(M1_B_PIN);
  M1_count = 0;

  M2_A_val = digitalRead(M2_A_PIN);
  M2_B_val = digitalRead(M2_B_PIN);
  M2_count = 0;

  M3_A_val = digitalRead(M3_A_PIN);
  M3_B_val = digitalRead(M3_A_PIN);
  M3_count = 0;

  M4_A_val = digitalRead(M4_A_PIN);
  M4_B_val = digitalRead(M4_A_PIN);
  M4_count = 0;
  // Velocity Calculation
  M1_count_km1 = 0;
  M1_rpm = 0;

  M2_count_km1 = 0;
  M2_rpm = 0;

  M3_count_km1 = 0;
  M3_rpm = 0;

  M4_count_km1 = 0;
  M4_rpm = 0;

  Encoder_velocity = 0;
  TimerInterrupt_flag = false;

  // Controller
  start_enable = false;
  steady_enable = false;
  left_enable = false;
  right_enable = false;
  m = 55; // rpm/s
  // dt is defined in loop()
  t_endctrl = 1.5; // length of time it takes to start/stop (seconds)
  eps = 10;   // tollerance for position control (in encoder counts)
  // define proportional, integral, and derivative gains for position control (Kx_p) and velocity control (Kx_v)
  Kp_p = 3;
  Ki_p = 0;
  Kd_p = 2;
  Kp_v = 5;
  Ki_v = 0;
  Kd_v = 0;
  setpoint = 0;
  error = 0;
  M1_error_km1 = 0;
  M2_error_km1 = 0;
  M3_error_km1 = 0;
  M4_error_km1 = 0;
  error_integral_M1 = 0;
  error_integral_M2 = 0;
  error_integral_M3 = 0;
  error_integral_M4 = 0;
  edot = 0;
  PWM_control_signal1 = 0;
  PWM_control_signal2 = 0;
  PWM_control_signal3 = 0;
  PWM_control_signal4 = 0;
// Initialize Pins GPIO as Input with Interrupt for reading Encoder channels A & B
  pinMode(M1_A_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(M1_A_PIN), M1_A_ISR, CHANGE);
  pinMode(M1_B_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(M1_B_PIN), M1_B_ISR, CHANGE);

  pinMode(M2_A_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(M2_A_PIN), M2_A_ISR, CHANGE);
  pinMode(M2_B_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(M2_B_PIN), M2_B_ISR, CHANGE);

  pinMode(M3_A_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(M3_A_PIN), M3_A_ISR, CHANGE);
  pinMode(M3_B_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(M3_B_PIN), M3_B_ISR, CHANGE);

  pinMode(M4_A_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(M4_A_PIN), M4_A_ISR, CHANGE);
  pinMode(M4_B_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(M4_B_PIN), M4_B_ISR, CHANGE);
  
// Initialize PWM Pins as output (and then use analogWrite() to write PWM %)
  pinMode(PWM_M1_CCW, OUTPUT);
  pinMode(PWM_M1_CW, OUTPUT);

  pinMode(PWM_M2_CCW, OUTPUT);
  pinMode(PWM_M2_CW, OUTPUT);

  pinMode(PWM_M3_CCW, OUTPUT);
  pinMode(PWM_M3_CW, OUTPUT);

  pinMode(PWM_M4_CCW, OUTPUT);
  pinMode(PWM_M4_CW, OUTPUT);

// **** Configure Timer using TimerInterrupt Library ****
  // Interval in microsecs
  if (ITimer.attachInterruptInterval(TIMER_INTERVAL_MS * 1000, TimerHandler))
  {
    Serial.print(F("Starting ITimer OK, millis() = ")); Serial.println(millis());
  }
  else
  {
    Serial.println(F("Can't set ITimer. Select another freq. or timer"));
  }
  delay(1000);
}

void loop() {
  ArduinoCloud.update();
  // Handling Timer Flag (see TimerHandler())
  if (TimerInterrupt_flag == true)
  {
    // The 'Forward' command is brocken up into two parts: first is start_enable is for acceleration and 
    // implements velocity control, and second is steady_enable which is for top speed/decceleration and
    // implements position control
    if (start_enable == true)
    {
      // Velocity Control Loop
      error = setpoint - M1_rpm;
      error_integral_M1 = error_integral_M1 + (error * (TIMER_INTERVAL_MS/1000)); // = sum( e*dt )
      edot = (error - M1_error_km1) / (TIMER_INTERVAL_MS/1000);  // = de/dt
      M1_error_km1 = error;
      PWM_control_signal1 = Kp_v*error + Ki_v*error_integral_M1 + Kd_v*edot;

      error = setpoint - M2_rpm;
      error_integral_M2 = error_integral_M2 + (error * (TIMER_INTERVAL_MS/1000)); // = sum( e*dt )
      edot = (error - M2_error_km1) / (TIMER_INTERVAL_MS/1000);  // = de/dt
      M2_error_km1 = error;
      PWM_control_signal2 = Kp_v*error + Ki_v*error_integral_M2 + Kd_v*edot;

      error = setpoint - M3_rpm;
      error_integral_M3 = error_integral_M3 + (error * (TIMER_INTERVAL_MS/1000)); // = sum( e*dt )
      edot = (error - M3_error_km1) / (TIMER_INTERVAL_MS/1000);  // = de/dt
      M3_error_km1 = error;
      PWM_control_signal3 = Kp_v*error + Ki_v*error_integral_M3 + Kd_v*edot;

      error = setpoint - M4_rpm;
      error_integral_M4 = error_integral_M4 + (error * (TIMER_INTERVAL_MS/1000)); // = sum( e*dt )
      edot = (error - M4_error_km1) / (TIMER_INTERVAL_MS/1000);  // = de/dt
      M4_error_km1 = error;
      PWM_control_signal4 = Kp_v*error + Ki_v*error_integral_M4 + Kd_v*edot;
      
      // Check start/stop condition
      if (dt > t_endctrl)
      {
        // we have finished the start sequence, continue at constant speed and initiate position control
        start_enable = false;
        steady_enable = true;
        Serial.println("S");
        delay(100);
        setpoint = COUNTS_PER_GRID_SQUARE;
      }
    }
    else if (steady_enable == true)
    {
      // Position Control Loop for driving !!FORWARD!!
      // setpoint is set above in: if(dt > t_endctrl)
      // setpoint = COUNTS_PER_GRID_SQUARE
      if ((setpoint - abs(M1_count)) > eps)
      {
        error = setpoint - M1_count;
        error_integral_M1 = error_integral_M1 + (error * (TIMER_INTERVAL_MS/1000)); // = sum( e*dt )
        edot = (error - M1_error_km1) / (TIMER_INTERVAL_MS/1000);  // = de/dt
        M1_error_km1 = error;
        PWM_control_signal1 = Kp_p*error + Ki_p*error_integral_M1 + Kd_p*edot;
      }
      else
      {
        PWM_control_signal1 = 0;
      }
      
      if ((setpoint - abs(M2_count)) > eps)
      {
        error = setpoint - M2_count;
        error_integral_M2 = error_integral_M2 + (error * (TIMER_INTERVAL_MS/1000)); // = sum( e*dt )
        edot = (error - M2_error_km1) / (TIMER_INTERVAL_MS/1000);  // = de/dt
        M2_error_km1 = error;
        PWM_control_signal2 = Kp_p*error + Ki_p*error_integral_M2 + Kd_p*edot;
      }
      else
      {
        PWM_control_signal2 = 0;
      }

      if ((setpoint - abs(M3_count)) > eps)
      {
        error = setpoint - M3_count;
        error_integral_M3 = error_integral_M3 + (error * (TIMER_INTERVAL_MS/1000)); // = sum( e*dt )
        edot = (error - M3_error_km1) / (TIMER_INTERVAL_MS/1000);  // = de/dt
        M3_error_km1 = error;
        PWM_control_signal3 = Kp_p*error + Ki_p*error_integral_M3 + Kd_p*edot;
      }
      else
      {
        PWM_control_signal3 = 0;
      }

      if ((setpoint - abs(M4_count)) > eps)
      {
        error = setpoint - M4_count;
        error_integral_M4 = error_integral_M4 + (error * (TIMER_INTERVAL_MS/1000)); // = sum( e*dt )
        edot = (error - M4_error_km1) / (TIMER_INTERVAL_MS/1000);  // = de/dt
        M4_error_km1 = error;
        PWM_control_signal4 = Kp_p*error + Ki_p*error_integral_M4 + Kd_p*edot;
      }
      else
      {
        PWM_control_signal4 = 0;
      }

      if ( ((setpoint - abs(M1_count)) < eps) && ((setpoint - abs(M3_count)) < eps)
              && ((setpoint - abs(M3_count)) < eps) && ((setpoint - abs(M4_count)) < eps) )
        // all motors have reached their desired position
        // this is inside the steady_enable (Forward) loop so all wheels were spinning forward
      {
        PWM_control_signal1 = 0;
        PWM_control_signal2 = 0;
        PWM_control_signal3 = 0;
        PWM_control_signal4 = 0;
        analogWrite(PWM_M1_CW, 0);
        analogWrite(PWM_M2_CCW, 0);
        analogWrite(PWM_M3_CW, 0);
        analogWrite(PWM_M4_CW, 0);
        steady_enable = false;
      }
    }
    // Right and Left can use the same control loop, we just 
    // change the polarity when applying the signal later on
    else if (left_enable == true || right_enable == true)
    {
      // Position Control Loop for Turning 
      // setpoint is set in the command handling loop: if(inChar == 'l')
      // setpoint = COUNTS_PER_90_DEG_TURN
      if ((setpoint - abs(M1_count)) > eps)
      {
        error = setpoint - abs(M1_count);
        error_integral_M1 = error_integral_M1 + (error * (TIMER_INTERVAL_MS/1000)); // = sum( e*dt )
        edot = (error - M1_error_km1) / (TIMER_INTERVAL_MS/1000);  // = de/dt
        M1_error_km1 = error;
        PWM_control_signal1 = Kp_p*error + Ki_p*error_integral_M1 + Kd_p*edot;
      }
      else
      {
        PWM_control_signal1 = 0;
      }
      
      if ((setpoint - abs(M2_count)) > eps)
      {
        error = setpoint - abs(M2_count);
        error_integral_M2 = error_integral_M2 + (error * (TIMER_INTERVAL_MS/1000)); // = sum( e*dt )
        edot = (error - M2_error_km1) / (TIMER_INTERVAL_MS/1000);  // = de/dt
        M2_error_km1 = error;
        PWM_control_signal2 = Kp_p*error + Ki_p*error_integral_M2 + Kd_p*edot;
      }
      else
      {
        PWM_control_signal2 = 0;
      }

      if ((setpoint - abs(M3_count)) > eps)
      {
        error = setpoint - abs(M3_count);
        error_integral_M3 = error_integral_M3 + (error * (TIMER_INTERVAL_MS/1000)); // = sum( e*dt )
        edot = (error - M3_error_km1) / (TIMER_INTERVAL_MS/1000);  // = de/dt
        M3_error_km1 = error;
        PWM_control_signal3 = Kp_p*error + Ki_p*error_integral_M3 + Kd_p*edot;
      }
      else
      {
        PWM_control_signal3 = 0;
      }

      if ((setpoint - abs(M4_count)) > eps)
      {
        error = setpoint - abs(M4_count);
        error_integral_M4 = error_integral_M4 + (error * (TIMER_INTERVAL_MS/1000)); // = sum( e*dt )
        edot = (error - M4_error_km1) / (TIMER_INTERVAL_MS/1000);  // = de/dt
        M4_error_km1 = error;
        PWM_control_signal4 = Kp_p*error + Ki_p*error_integral_M4 + Kd_p*edot;
      }
      else
      {
        PWM_control_signal4 = 0;
      }

      if ( ((setpoint - abs(M1_count)) < eps) && ((setpoint - abs(M3_count)) < eps)
              && ((setpoint - abs(M3_count)) < eps) && ((setpoint - abs(M4_count)) < eps) )
        // all motors have reached their desired position
        // this is inside the steady_enable (Forward) loop so all wheels were spinning forward
      {
        PWM_control_signal1 = 0;
        PWM_control_signal2 = 0;
        PWM_control_signal3 = 0;
        PWM_control_signal4 = 0;
        analogWrite(PWM_M1_CW, 0);
        analogWrite(PWM_M2_CCW, 0);
        analogWrite(PWM_M3_CW, 0);
        analogWrite(PWM_M4_CW, 0);
        if (left_enable == true)
        // Turning Left: M1 & M3 = Backward
        //               M2 & M4 = Forward
        {
          analogWrite(PWM_M1_CCW, 0);
          analogWrite(PWM_M2_CCW, 0);
          analogWrite(PWM_M3_CCW, 0);
          analogWrite(PWM_M4_CW, 0);
          left_enable = false;
        }
        else if (right_enable == true)
        // Turning Left: M1 & M3 = Forward
        //               M2 & M4 = Backward
        {
          analogWrite(PWM_M1_CW, 0);
          analogWrite(PWM_M2_CW, 0);
          analogWrite(PWM_M3_CW, 0);
          analogWrite(PWM_M4_CCW, 0);
          right_enable = false;
        }
      }
      
    }
    // Chop Control Signal (PWM must be between 0 - 255)
    // Motor 1:
    if (PWM_control_signal1 > 255)
    {
      // PWM can't be greater than 255
      PWM_control_signal1 = 255;
    }
    else if (PWM_control_signal1 < 0)
    {
      // for now we don't want PWM_control_signal to be negative
      PWM_control_signal1 = 0;
    }
    // Motor 2:
    if (PWM_control_signal2 > 255)
    {
      // PWM can't be greater than 255
      PWM_control_signal2 = 255;
    }
    else if (PWM_control_signal2 < 0)
    {
      // for now we don't want PWM_control_signal to be negative
      PWM_control_signal2 = 0;
    }
    // Motor 3:
    if (PWM_control_signal3 > 255)
    {
      // PWM can't be greater than 255
      PWM_control_signal3 = 255;
    }
    else if (PWM_control_signal3 < 0)
    {
      // for now we don't want PWM_control_signal to be negative
      PWM_control_signal3 = 0;
    }
    // Motor 4:
    if (PWM_control_signal4 > 255)
    {
      // PWM can't be greater than 255
      PWM_control_signal4 = 255;
    }
    else if (PWM_control_signal4 < 0)
    {
      // for now we don't want PWM_control_signal to be negative
      PWM_control_signal4 = 0;
    }
    // Apply Control Signal
    // For 'Forward' all wheels turn forward:
    //    Motors 2 (Front Right) & 4 (Back Right) = Forward
    //    Motors 1 (Front Left) & 3 (Back Left) = Forward
    if (start_enable == true || steady_enable == true)
    // we are executing 'Forward'
    {
      analogWrite(PWM_M1_CCW, PWM_control_signal1);
      analogWrite(PWM_M2_CW, PWM_control_signal2);
      analogWrite(PWM_M3_CW, PWM_control_signal3);
      analogWrite(PWM_M4_CW, PWM_control_signal4);
    }
    // For 'Left Turn' right wheels turn forward, left wheels turn backward:
    //    Motors 2 (Front Right) & 4 (Back Right) = Forward
    //    Motors 1 (Front Left) & 3 (Back Left) = Backward 
    if (left_enable == true)
    // we are executing 'Left Turn'
    {
      // since we are turning we want to do it more slowly (for now)
      analogWrite(PWM_M1_CCW, 0.5 * PWM_control_signal1);
      analogWrite(PWM_M2_CCW, 0.5 * PWM_control_signal2);
      analogWrite(PWM_M3_CW, 0.5 * PWM_control_signal3);
      analogWrite(PWM_M4_CCW, 0.5 * PWM_control_signal4);
    }
    // For 'Right Turn' right wheels turn backward, left wheels turn forward:
    //    Motors 2 (Front Right) & 4 (Back Right) = Backward
    //    Motors 1 (Front Left) & 3 (Back Left) = Foroward 
    if (right_enable == true)
    {
      // since we are turning we want to do it more slowly (for now)
      analogWrite(PWM_M1_CW, 0.5 * PWM_control_signal1);
      analogWrite(PWM_M2_CW, 0.5 * PWM_control_signal2);
      analogWrite(PWM_M3_CCW, 0.5 * PWM_control_signal3);
      analogWrite(PWM_M4_CW, 0.5 * PWM_control_signal4);
    }

    TimerInterrupt_flag = false;  // we are handling the flag, so set it back to false  
    Serial.print(PWM_control_signal1);
    Serial.print("\t");
    Serial.println(PWM_control_signal2);
  }
// Handling Incoming Commands (Keystrokes)
  if (Serial.available() > 0)
  {
    char inChar = Serial.read();
    if (inChar == 'd')
    // begin forward
    {
      // initiate "motor start" control loop
      start_enable = true;
      steady_enable = false;
      left_enable = false;
      right_enable = false;
      M1_count = 0;
      M2_count = 0;
      M3_count = 0;
      M4_count = 0;
      setpoint = 0;
      dt = 0;
      Serial.print("d\n");
    }
    if (inChar == 'l')
    // left turn
    {
      start_enable = false;
      steady_enable = false;
      left_enable = true;
      right_enable = false;
      M1_count = 0;
      M2_count = 0;
      M3_count = 0;
      M4_count = 0;
      setpoint = COUNTS_PER_90_DEG_TURN;
      Serial.print("l\n");
    }
    if (inChar == 'r')
    // right turn
    {
      start_enable = false;
      steady_enable = false;
      left_enable = false;
      right_enable = true;
      M1_count = 0;
      M2_count = 0;
      M3_count = 0;
      M4_count = 0;
      setpoint = COUNTS_PER_90_DEG_TURN;
      Serial.print("r\n");
    }
    else if (inChar == 'e')
    // emergency stop
    {
      start_enable = false;
      steady_enable = false;
      left_enable = false;
      right_enable = false;
      PWM_control_signal1 = 0;
      analogWrite(PWM_M1_CCW, 0); 
      analogWrite(PWM_M1_CW, 0);  // turn motor  completly off
      PWM_control_signal2 = 0;
      analogWrite(PWM_M2_CCW, 0); 
      analogWrite(PWM_M2_CW, 0); // turn motor completly off
      PWM_control_signal3 = 0;
      analogWrite(PWM_M3_CCW, 0);
      analogWrite(PWM_M3_CCW, 0); // turn motor completly off
      PWM_control_signal4 = 0;
      analogWrite(PWM_M4_CCW, 0); 
      analogWrite(PWM_M4_CW, 0); // turn motor completly off
      Serial.print("e\n");
    }
  }
  
  
}



/*
  Since TestSwitch is READ_WRITE variable, onTestSwitchChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onTestSwitchChange()  {
  // Add your code here to act upon TestSwitch change
  if (test_switch == 1){
    digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
    Serial.println("LED on");
    analogWrite(PWM_M1_CCW, 255);
    analogWrite(PWM_M2_CCW, 255);
    analogWrite(PWM_M3_CCW, 255);
    analogWrite(PWM_M4_CCW, 255);
    Serial.print("forward\n");
  }
  else
  {
    digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
    Serial.println("LED off");
    analogWrite(PWM_M1_CCW, 0);
    analogWrite(PWM_M2_CCW, 0);
    analogWrite(PWM_M3_CCW, 0);
    analogWrite(PWM_M4_CCW, 0);
    Serial.print("stop\n");
  }
  
  if (turn_left == 1) {
    start_enable = false;
      steady_enable = false;
      left_enable = true;
      right_enable = false;
      M1_count = 0;
      M2_count = 0;
      M3_count = 0;
      M4_count = 0;
      setpoint = COUNTS_PER_90_DEG_TURN;
      Serial.print("l\n");
  }
  
  if (turn_right == 1) {
    start_enable = false;
    steady_enable = false;
    left_enable = false;
    right_enable = true;
    M1_count = 0;
    M2_count = 0;
    M3_count = 0;
    M4_count = 0;
    setpoint = COUNTS_PER_90_DEG_TURN;
    Serial.print("r\n");
  }
}


/*
  Since AutonomousMode is READ_WRITE variable, onAutonomousModeChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onAutonomousModeChange()  {
  // Add your code here to act upon AutonomousMode change
}



/*
  Since MoveForward is READ_WRITE variable, onMoveForwardChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onMoveForwardChange()  {
  // Add your code here to act upon MoveForward change
  
}

/*
  Since MoveBackward is READ_WRITE variable, onMoveBackwardChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onMoveBackwardChange()  {
  // Add your code here to act upon MoveBackward change
}

/*
  Since TurnLeft is READ_WRITE variable, onTurnLeftChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onTurnLeftChange()  {
  // Add your code here to act upon TurnLeft change
}


/*
  Since TurnRight is READ_WRITE variable, onTurnRightChange() is
  executed every time a new value is received from IoT Cloud.
*/
void onTurnRightChange()  {
  // Add your code here to act upon TurnRight change
}

