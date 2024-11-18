#include "tinyCLI.h"

// Initialize the static instance pointer
tinyCLI* tinyCLI::instance = nullptr;

// Constructor
tinyCLI::tinyCLI(Stream& input, Command* cmds, int cmdCount)
  : inputStream(input), commands(cmds), commandCount(cmdCount) {
    assignHelpFunction();  // Automatically assign help function
}

// Main function to process input and execute commands
void tinyCLI::processInput() {
  if (inputStream.available()) {
    String input = inputStream.readStringUntil('\n');
    input.trim();
    String command = input.substring(0, input.indexOf(' '));
    String parameters = input.substring(input.indexOf(' ') + 1);
    executeCommand(command, parameters);
  }
}

void tinyCLI::printHelp(String params) {
    // Define the total line length
    const int lineLength = 100; // Set maximum line length to 100
    const int padding = 20;

    // Print the separator line
    Serial.println("#" + String(std::string(lineLength - 2, '-').c_str()) + "#");
    
    // Print the command list header
    String heading = "# Command" + String(std::string(padding - strlen("Command") - 1, ' ').c_str());
    heading.concat("- Description");
    heading.concat(String(std::string(lineLength - heading.length() - 1, ' ').c_str()));
    heading.concat("#");

    Serial.println(heading);
    
    // Print the separator line
    Serial.println("#" + String(std::string(lineLength - 2, '-').c_str()) + "#");

    // Print each command with manual formatting
    for (int i = 0; i < instance->commandCount; i++) {
        
        String command = "# ";
        command.concat(instance->commands[i].name);
        command.concat(std::string(padding - command.length(), ' ').c_str());
        command.concat(" - ");
        command.concat(instance->commands[i].description); 
        command.concat(String(std::string(lineLength - command.length() - 1, ' ').c_str()));
        command.concat("#");

        Serial.println(command);

    }

    // Print the bottom border
    Serial.println("#" + String(std::string(lineLength - 2, '-').c_str()) + "#");
}


// Function to assign the help function to the help command
void tinyCLI::assignHelpFunction() {
  instance = this;  // Save the current instance
  for (int i = 0; i < commandCount; i++) {
    if (String(commands[i].name).equals("help")) {
      commands[i].function = tinyCLI::printHelp;
    }
  }
}

// Function to execute a command based on input
void tinyCLI::executeCommand(String input, String parameters) {
  for (int i = 0; i < commandCount; i++) {
    if (input.equals(commands[i].name)) {
      commands[i].function(parameters);  // Pass parameters to the function
      return;
    }
  }
  inputStream.println("Unknown command. Type 'help' for a list of commands.");
}
