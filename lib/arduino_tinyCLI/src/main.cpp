#include <Arduino.h>
#include "tinyCLI.h"

// Example command functions with parameters
void controlLED(String params) {
  params.trim();
  if (params == "-on") {
    Serial.println("LED is now ON!");
    // Add code to turn on the LED here
  } else if (params == "-off") {
    Serial.println("LED is now OFF!");
    // Add code to turn off the LED here
  } else {
    Serial.println("Unknown parameter. Use '-on' or '-off'.");
  }
}

void sayHello(String params) {
  Serial.println("Hello, World! No parameters needed.");
}

// Create the list of commands
tinyCLI::Command commands[] = {
  {"hello", "Prints Hello, World", sayHello},
  {"led", "Controls the LED (use -on or -off)", controlLED},
  {"help", "Shows this help message", nullptr},  // Placeholder for help function
};

// Initialize the command line interface
tinyCLI commandLine(Serial, commands, sizeof(commands) / sizeof(commands[0]));

void setup() {
  Serial.begin(115200);
}

void loop() {
  commandLine.processInput();
}
