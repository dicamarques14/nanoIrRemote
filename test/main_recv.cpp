/*
 * SimpleSender.cpp
 *
 *  Demonstrates sending IR codes in standard format with address and command
 *  An extended example for sending can be found as SendDemo.
 *
 *  Copyright (C) 2020-2022  Armin Joachimsmeyer
 *  armin.joachimsmeyer@gmail.com
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 *  MIT License
 */
#include <Arduino.h>

#define DISABLE_CODE_FOR_RECEIVER // Disables restarting receiver after each send. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not used.
// #define SEND_PWM_BY_TIMER         // Disable carrier PWM generation in software and use (restricted) hardware PWM.
// #define USE_NO_SEND_PWM           // Use no carrier PWM, just simulate an active low receiver signal. Overrides SEND_PWM_BY_TIMER definition
#define IR_RECEIVE_PIN 2 // To be compatible with interrupt example, pin 2 is chosen here.
#define IR_SEND_PIN 3
#define TONE_PIN 4
#define APPLICATION_PIN 5
#define ALTERNATIVE_IR_FEEDBACK_LED_PIN 6 // E.g. used for examples which use LED_BUILDIN for example output.
#define _IR_TIMING_TEST_PIN 7
/*
 * Helper macro for getting a macro definition as string
 */
#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif
/*
 * This include defines the actual pin number for pins like IR_RECEIVE_PIN, IR_SEND_PIN for many different boards and architectures
 */
#include "TinyIRReceiver.hpp"

volatile struct TinyIRReceiverCallbackDataStruct sCallbackData;

void setup()
{
  Serial.begin(115200);
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_TINYIR));

  // Enables the interrupt generation on change of IR input signal
  if (!initPCIInterruptForTinyReceiver())
  {
    Serial.println(F("No interrupt available for pin " STR(IR_RECEIVE_PIN))); // optimized out by the compiler, if not required :-)
  }
  Serial.println(F("Ready to receive NEC IR signals at pin " STR(IR_RECEIVE_PIN)));
}

void loop()
{
  if (sCallbackData.justWritten)
  {
    sCallbackData.justWritten = false;
    Serial.print(F("Address=0x"));
    Serial.print(sCallbackData.Address, HEX);
    Serial.print(F(" Command=0x"));
    Serial.print(sCallbackData.Command, HEX);
    if (sCallbackData.Flags == IRDATA_FLAGS_IS_REPEAT)
    {
      Serial.print(F(" Repeat"));
    }
    if (sCallbackData.Flags == IRDATA_FLAGS_PARITY_FAILED)
    {
      Serial.print(F(" Parity failed"));
    }
    Serial.println();
  }
  /*
   * Put your code here
   */
}

/*
 * This is the function, which is called if a complete command was received
 * It runs in an ISR context with interrupts enabled, so functions like delay() etc. should work here
 */
#if defined(ESP8266) || defined(ESP32)
IRAM_ATTR
#endif
void handleReceivedTinyIRData(uint8_t aAddress, uint8_t aCommand, uint8_t aFlags)
{
  /*
   * Printing is not allowed in ISR context for any kind of RTOS
   * For Mbed we get a kernel panic and "Error Message: Semaphore: 0x0, Not allowed in ISR context" for Serial.print()
   * for ESP32 we get a "Guru Meditation Error: Core  1 panic'ed" (we also have an RTOS running!)
   */
  // Print only very short output, since we are in an interrupt context and do not want to miss the next interrupts of the repeats coming soon
  printTinyReceiverResultMinimal(&Serial, aAddress, aCommand, aFlags);
}