#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RCSwitch.h>
#include "util.h"

RCSwitch mySwitch = RCSwitch();

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


unsigned long previousMillis = 0;
const long interval = 100;
unsigned long counter = 0;

void setup() {
  Serial.begin(9600);
  /*
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
*/
  DEBUG_PRINT_LN("Hi!");
  mySwitch.enableReceive(RC_PIN);
  setupPins();

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    DEBUG_PRINT_LN(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  display.clearDisplay();

}

// Variables to handle changes in Request commands
int currentRequestCommand = REQUEST_COMMAND_UNKNOWN;
int previousRequestCommand = currentRequestCommand;

void loop() {
  currentMillis = millis();

  handleStopsSignals();
  handleLookUnlockCommands();
  
  checkRequestCommand();
  handleRequestCommandChanges();
















  
  handleDoorCommands();
  updateDisplay();

  previousMillis = currentMillis;
  
  previousRequestCommand = currentRequestCommand;
}


void handleRequestCommandChanges() {
  if (previousRequestCommand != currentRequestCommand) {

    DEBUG_PRINT("Identified a new request command: ");
    Serial.println(getRequestCommandName(currentRequestCommand));

    if (currentRequestCommand == REQUEST_COMMAND_TO_OPEN) {
      if (canOpenLeftDoor()) {
        unlockLeftDoorStartMillis = currentMillis; // Start unlock process for left door
        openLeftDoorStartMillis = currentMillis + TIME_TO_LOCK_UNLOCK_DOORS; // Start open left door proccess
      }
      if (canOpenRightDoor()) {
        unlockRightDoorStartMillis = currentMillis; // Start unlock process for right door
        openRightDoorStartMillis = currentMillis + TIME_TO_LOCK_UNLOCK_DOORS + TIME_TO_WAIT_DOOR_EACH_OTHER; // Start open right door proccess
      }
    } else if (currentRequestCommand == REQUEST_COMMAND_TO_CLOSE) {
      if (canCloseLeftDoor()) {
        unlockLeftDoorStartMillis = currentMillis; // Start unlock process for left door
        closeLeftDoorStartMillis = currentMillis + TIME_TO_LOCK_UNLOCK_DOORS + TIME_TO_WAIT_DOOR_EACH_OTHER; // Start open left door proccess
      }
      if (canCloseRightDoor()) {
        unlockRightDoorStartMillis = currentMillis; // Start unlock process for right door
        closeRightDoorStartMillis = currentMillis + TIME_TO_LOCK_UNLOCK_DOORS; // Start open right door proccess
      }
    } else if (currentRequestCommand == REQUEST_COMMAND_TO_OPEN_LEFT_DOOR) {
      if (canOpenLeftDoor()) {
        unlockLeftDoorStartMillis = currentMillis; // Start unlock process for left door
        openLeftDoorStartMillis = currentMillis + TIME_TO_LOCK_UNLOCK_DOORS; // Start open left door proccess
      }
    }
  }
}

void setupPins() {
  pinMode(MOTOR_ACTUATOR_RELAY_PIN, OUTPUT);
  digitalWrite(MOTOR_ACTUATOR_RELAY_PIN, LOW);
  pinMode(RELAY_12_36_PIN, OUTPUT);
  digitalWrite(RELAY_12_36_PIN, LOW);
  
  pinMode(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, OUTPUT);
  digitalWrite(OPEN_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
  pinMode(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, OUTPUT);
  digitalWrite(CLOSE_LEFT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
  pinMode(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, OUTPUT);
  digitalWrite(OPEN_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);
  pinMode(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, OUTPUT);
  digitalWrite(CLOSE_RIGHT_MOTOR_ACTUATOR_RELAY_PIN, LOW);

  pinMode(INTERIOR_LIGHT_RELAY_PIN, OUTPUT);
  digitalWrite(INTERIOR_LIGHT_RELAY_PIN, LOW);

  pinMode(OPEN_CLOSE_BUTTON_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(SIGNAL_LAMP_PIN, OUTPUT);
  digitalWrite(SIGNAL_LAMP_PIN, LOW);

  pinMode(OBSTACLE_LAMP_PIN, OUTPUT);
  digitalWrite(OBSTACLE_LAMP_PIN, LOW);

  pinMode(LEFT_DOOR_OPEN_PIN, INPUT);
  pinMode(LEFT_DOOR_CLOSE_PIN, INPUT);
  pinMode(RIGHT_DOOR_OPEN_PIN, INPUT);
  pinMode(RIGHT_DOOR_CLOSE_PIN, INPUT);

}


void checkRequestCommand() {

  unsigned long receivedValue = 0;

  if (mySwitch.available()) {
    receivedValue = mySwitch.getReceivedValue();
    DEBUG_PRINT("Received Value: ");
    Serial.println(receivedValue);
    if (receivedValue == KEY_1_CODE || receivedValue == KEY_2_CODE || receivedValue == KEY_3_CODE) {
      DEBUG_PRINT_LN("Received Close Command");
      currentRequestCommand = REQUEST_COMMAND_TO_CLOSE;
    }

    if (receivedValue == KEY_1_CODE + 1 || receivedValue == KEY_2_CODE + 1 || receivedValue == KEY_3_CODE + 1) {
      DEBUG_PRINT_LN("Received Open Command");
      currentRequestCommand = REQUEST_COMMAND_TO_OPEN;
    }
    if (receivedValue == KEY_1_CODE + 3 || receivedValue == KEY_2_CODE + 3 || receivedValue == KEY_3_CODE + 3) {
      DEBUG_PRINT_LN("Received Open Left Door Command");
      currentRequestCommand = REQUEST_COMMAND_TO_OPEN_LEFT_DOOR;
    }

    mySwitch.resetAvailable();
  }

}

void updateDisplay() {
  counter = counter + 1;
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  //display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.println(counter);
  display.print("Request State: ");  display.println(getRequestCommandName(currentRequestCommand));

  display.display();
}