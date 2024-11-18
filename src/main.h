#ifndef _MAIN_H
#define _MAIN_H

/* -------------------------------------------------------------------------- */
/*                                Include Files                               */
/* -------------------------------------------------------------------------- */
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <Preferences.h>
#include <PubSubClient.h>
#include "definitions.h"
#include <iot_cli.h>
#include <IoT_device.h>
#include <WebServer_WT32_ETH01.h>
/* -------------------------------------------------------------------------- */
/*                                   Macros                                   */
/* -------------------------------------------------------------------------- */
#define DEBUG_ETHERNET_WEBSERVER_PORT Serial
#define _ETHERNET_WEBSERVER_LOGLEVEL_ 3

#define BTN1_PIN 5
#define BTN2_PIN 6
#define BTN3_PIN 7
#define BTN4_PIN 10
#define BTN5_PIN 20

/* -------------------------------------------------------------------------- */
/*                                Enumerations                                */
/* -------------------------------------------------------------------------- */

/* ------------------------------ Button states ----------------------------- */
enum ButtonState
{
    IDLE,
    DEBOUNCE,
    PRESSED,
    SHORT_PRESS,
    SINGLE_PRESS,
    DOUBLE_PRESS,
    LONG_PRESS,
    LONG_RELEASE
};

/* -------------------------------------------------------------------------- */
/*                                  Variables                                 */
/* -------------------------------------------------------------------------- */
unsigned long timestamp = 0;

/* ----------------------- MQTT client default values ----------------------- */
char mqtt_server[40];
char mqtt_port[6] = "";
char mqtt_user[34] = "";
char mqtt_password[34] = "";

/* -----------------------  WiFiManager MQTT Definition ---------------------- */
WiFiManager wm;
WiFiManagerParameter custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
WiFiManagerParameter custom_mqtt_user("mqtt_user", "MQTT Username", mqtt_user, 32);
WiFiManagerParameter custom_mqtt_password("mqtt_password", "MQTT Password", mqtt_password, 32);

/* -------------- Preference Object to store data in the memory ------------- */
Preferences preferences;

/* ------------------------------- MQTT Client ------------------------------ */
WiFiClient espClient;
PubSubClient wifi_mqttClient(espClient);
WiFiClient ethClient;
PubSubClient eth_mqttClient(ethClient);

/* ------------------------------- IoT Device ------------------------------- */
Device myIoTdevice;
auto btn_action = std::make_shared<Sensor>("Action");

// Constants
const int numButtons = 5;             // Number of buttons
const int debounceDelay = 50;         // Debounce delay in milliseconds
const int longPressDuration = 300;    // Duration for long press in milliseconds
const int doublePressThreshold = 300; // Maximum delay between presses for a double press

// Button pins (update according to your setup)
int buttonPins[numButtons] = {BTN1_PIN, BTN2_PIN, BTN3_PIN, BTN4_PIN, BTN5_PIN};
int buttonPresses[numButtons] = {0};
bool singlePresses[numButtons] = {0};
bool doublePresses[numButtons] = {0};
bool longPresses[numButtons] = {0};
bool longReleases[numButtons] = {0};

// Variables for button state tracking
ButtonState buttonStates[numButtons] = {IDLE};
unsigned long stateStartTime[numButtons] = {0};
unsigned long pressStartTime[numButtons] = {0};
bool lastButtonState[numButtons] = {false};
bool sm_lastBtnState[numButtons] = {false};
/* -------------------------------------------------------------------------- */
/*                               Data Structures                              */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                  Typedefs                                  */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                                  Functions                                 */
/* -------------------------------------------------------------------------- */

void wm_init(bool _reset);
void saveConfigCallback();

void MQTT_init();
void MQTT_callback(char *topic, byte *message, unsigned int length);
void MQTT_reconnect(PubSubClient &_client);

void IoT_device_init();
void handleButtonState(int buttonIndex, unsigned long currentTime);

int randomInt();
bool randomBool();

#endif