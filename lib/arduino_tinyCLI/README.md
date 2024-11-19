# tinyCLI for Arduino

tinyCLI is a lightweight command-line interface (CLI) library for Arduino that allows you to define and handle custom commands through the serial interface. It is ideal for controlling devices, debugging, and interacting with your Arduino in real-time through the Serial Monitor.

## Features
- Create custom commands with parameters.
- Automatically handles a help command to display all available commands.
- Easily extensible for new commands and functionalities.
- Small memory footprint, making it ideal for resource-constrained environments like microcontrollers.

## Table of Contents
- [Features](#features)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Example Commands](#example-commands)
- [Command List](#command-list)
- [File Structure](#file-structure)
- [License](#license)

## Getting Started

### Prerequisites
To use this project, you'll need:
- **Arduino IDE** installed on your system.
- An **Arduino board** (e.g., Uno, Mega, or any compatible board).
- Basic understanding of **Arduino programming**.

### Installing
1. Clone the repository to your local machine:
   ```bash
   git clone https://github.com/MarcoGeraldi/arduino_tinyCLI.git
2. Open the project folder in Arduino IDE
3. Ensure that the following files are added to your project:
- `main.cpp`: Contains the main sketch that initializes `tinyCLI` and sets up the commands.
- `tinyCLI.h`: The header file defining the `tinyCLI` class and command structure.
- `tinyCLI.cpp`: The implementation file for the `tinyCLI` class.

## Usage

### Setting Up
1. Open the Arduino IDE and load the `main.cpp` file.
2. Connect your Arduino board to your computer.
3. Upload the sketch to your Arduino board.
4. Open the Serial Monitor (or any terminal application) and set the baud rate to `115200` (or as defined in the `Serial.begin()` call).
5. Type in any of the defined commands and press Enter.

### Example Commands
- `hello` - Prints "Hello, World!" to the Serial Monitor.
- `led -on` - Turns on the LED (you will need to define what "LED ON" does in your setup).
- `led -off` - Turns off the LED.
- `help` - Prints the list of available commands.

### Example Output

Open the serial monitor and send the string "help". The output will be something similar to this: 
![image](https://github.com/user-attachments/assets/0ca1dfc7-ecca-48d4-aff7-ba842b16724d)

### Command List
You can define commands using the following structure:
```cpp
    tinyCLI::Command commands[] = {
      {"command_name", "Command description", commandFunction},
      // Add more commands here
    };
```

For example, a command to turn an LED on or off:

```cpp
void controlLED(String params) {
  params.trim();
  if (params == "-on") {
    // Code to turn on the LED
  } else if (params == "-off") {
    // Code to turn off the LED
  } else {
    Serial.println("Unknown parameter. Use '-on' or '-off'.");
  }
}
```


You can extend the commands array to add more commands as needed.

## File Structure
- `main.cpp`: Contains the main Arduino sketch with the `setup()` and `loop()` functions, along with the custom commands you define.
- `tinyCLI.cpp`: Implements the `tinyCLI` class, including input processing, command execution, and the help function.
- `tinyCLI.h`: The header file where the `tinyCLI` class is declared, along with the command structure and function signatures.

## License
This project is licensed under the MIT License.
