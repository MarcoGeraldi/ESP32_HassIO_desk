#ifndef TINYCLI_H
#define TINYCLI_H

#include <Arduino.h>

class tinyCLI {
  public:
    typedef void (*CommandFunction)(String);  // Function pointer type with parameter

    // Struct to define each command
    struct Command {
      const char* name;
      const char* description;
      CommandFunction function;
    };

    // Constructor
    tinyCLI(Stream& input, Command* cmds, int cmdCount);

    // Main function to process input and execute commands
    void processInput();

    // Help function to list all available commands
    static void printHelp(String params);

  private:
    Stream& inputStream;    // Stream for input (e.g., Serial)
    Command* commands;      // Array of commands
    int commandCount;       // Number of commands

    // Function to store the instance of the class (needed for static functions)
    static tinyCLI* instance;

    // Assign help function to the command if it exists
    void assignHelpFunction();

    // Function to execute a command based on input
    void executeCommand(String input, String parameters);
};

#endif
