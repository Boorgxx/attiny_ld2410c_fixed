/*
 * ATtiny85 Digispark - HLK-LD2410C mmWave Radar (OPTIMIZED)
 * 
 * Optimalizováno pro ATtiny85: minimální RAM, efektivní kód
 * 
 * Hardware:
 * - LD2410C OUT → P2 (PB2)
 * - LED → P1 (PB1)
 * - LD2410C VCC → 5V
 * - LD2410C GND → GND
 */

#include <DigiKeyboard.h>

// Pin definitions
#define SENSOR_PIN 2
#define LED_PIN 1

// Timing constants (optimized for ATtiny85)
#define DEBOUNCE_TIME 500UL
#define SEND_INTERVAL 1000UL
#define HEARTBEAT_INTERVAL 5000UL
#define BLINK_FAST 100UL
#define FADE_IN_TIME 3000UL   // 2 seconds fade in
#define FADE_OUT_TIME 1000UL  // 3 seconds fade out
#define FADE_TOTAL_TIME (FADE_IN_TIME + FADE_OUT_TIME)  // 5s total cycle

// State machine (uint8_t saves RAM vs enum)
#define STATE_LOW 0
#define STATE_HIGH 1
#define STATE_DEBOUNCING_HIGH 2
#define STATE_DEBOUNCING_LOW 3

// Global state (minimized RAM usage)
static uint8_t state = STATE_LOW;
static uint8_t sendCount = 0;
static uint8_t ledState = 0;
static char pendingChar = 0;
static uint16_t fadeStartTime = 0;      // For fade timing
static uint16_t stateChangeTime = 0;
static uint16_t nextSendTime = 0;
static uint16_t lastHeartbeat = 0;
static uint16_t lastBlinkTime = 0;      // For fast blink only

void setup() {
  // Configure pins
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // USB initialization
  DigiKeyboard.delay(2000);
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.delay(500);
  
  // Initialize state
  state = STATE_LOW;
  sendCount = 0;
  pendingChar = 0;
  ledState = 0;
  fadeStartTime = 0;
  analogWrite(LED_PIN, 0);
}

void loop() {
  uint16_t now = (uint16_t)millis();  // 16-bit sufficient for intervals
  
  // Read sensor once per loop
  uint8_t sensorHigh = digitalRead(SENSOR_PIN);
  
  // LED control: fast blink when detected, fade when idle
  if (sensorHigh) {
    // Fast blinking during detection
    if ((uint16_t)(now - lastBlinkTime) >= BLINK_FAST) {
      lastBlinkTime = now;
      ledState ^= 1;
      digitalWrite(LED_PIN, ledState);
    }
  } else {
    // Smooth fade in/out when no detection
    uint16_t fadeElapsed = (uint16_t)(now - fadeStartTime);
    
    if (fadeElapsed >= FADE_TOTAL_TIME) {
      fadeStartTime = now;  // Restart cycle
      fadeElapsed = 0;
    }
    
    uint8_t brightness;
    if (fadeElapsed < FADE_IN_TIME) {
      // Fade in: 0 -> 255 over 2 seconds
      brightness = (uint8_t)((fadeElapsed * 255UL) / FADE_IN_TIME);
    } else {
      // Fade out: 255 -> 0 over 3 seconds
      uint16_t fadeOutElapsed = fadeElapsed - FADE_IN_TIME;
      brightness = 255 - (uint8_t)((fadeOutElapsed * 255UL) / FADE_OUT_TIME);
    }
    
    analogWrite(LED_PIN, brightness);
  }
  
  // State machine (optimized for size and speed)
  switch (state) {
    case STATE_LOW:
      if (sensorHigh) {
        state = STATE_DEBOUNCING_HIGH;
        stateChangeTime = now;
      }
      break;
      
    case STATE_HIGH:
      if (!sensorHigh) {
        state = STATE_DEBOUNCING_LOW;
        stateChangeTime = now;
      }
      break;
      
    case STATE_DEBOUNCING_HIGH:
      if (!sensorHigh) {
        state = STATE_LOW;  // False alarm
      } else if ((uint16_t)(now - stateChangeTime) >= DEBOUNCE_TIME) {
        state = STATE_HIGH;
        pendingChar = 'p';
        sendCount = 3;
        nextSendTime = now;
        lastHeartbeat = now;
      }
      break;
      
    case STATE_DEBOUNCING_LOW:
      if (sensorHigh) {
        state = STATE_HIGH;  // Still detected
      } else if ((uint16_t)(now - stateChangeTime) >= DEBOUNCE_TIME) {
        state = STATE_LOW;
        pendingChar = 'o';
        sendCount = 3;
        nextSendTime = now;
      }
      break;
  }
  
  // Send scheduled keys (3x with 1s spacing)
  if (sendCount > 0 && (uint16_t)(now - nextSendTime) < 32768U) {
    DigiKeyboard.write(pendingChar);  // More efficient than print
    sendCount--;
    if (sendCount > 0) {
      nextSendTime = now + SEND_INTERVAL;
    } else {
      pendingChar = 0;
    }
  }
  
  // Heartbeat: send 'p' every 5s while HIGH
  if (state == STATE_HIGH && sendCount == 0) {
    if ((uint16_t)(now - lastHeartbeat) >= HEARTBEAT_INTERVAL) {
      DigiKeyboard.write('p');
      lastHeartbeat = now;
    }
  }
  
  DigiKeyboard.delay(10);  // USB polling
}
