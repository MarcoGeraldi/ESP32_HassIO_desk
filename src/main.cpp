#include "main.h"

// Create the list of commands
tinyCLI::Command commands[] = {
    {CLI_HELP, "Shows this help message", nullptr}, // Placeholder for help function
    {CLI_RESET, "Reset the device", cli_reset},
    {CLI_MQTT_INFO, "Prints stored mqtt settings", cli_mqtt_info},
    {CLI_MQTT_SERVER, "Set MQTT broker IP address", cli_mqtt_server},
    {CLI_MQTT_PORT, "Set MQTT broker port", cli_mqtt_port},
    {CLI_MQTT_USER, "Set MQTT username", cli_mqtt_user},
    {CLI_MQTT_PASSWORD, "Set MQTT password", cli_mqtt_password},
    {CLI_WIFI_CONFIG_PORTAL, "Enable Config Portal", cli_config_portal},
    {CLI_WIFI_SSID, "Set WiFi SSID", nullptr},
    {CLI_WIFI_PSWD, "Set WiFi Password", nullptr},
};

// Initialize the command line interface
tinyCLI commandLine(Serial, commands, sizeof(commands) / sizeof(commands[0]));

void deviceUpdate()
{

  /* -------------------------- Check Buttons Status -------------------------- */
  bool anyAction = false;
  for (int i = 0; i < numButtons; i++)
  {
    if(singlePresses[i])        btn_action->setStatus(String(i+1) + "_single");
    else if (doublePresses[i])  btn_action->setStatus(String(i+1) + "_double");
    else if (longPresses[i])    btn_action->setStatus(String(i+1) + "_hold");
    else if (longReleases[i])   btn_action->setStatus(String(i+1) + "_release");
    
    /* --------------- Verify that at least one button was pressed -------------- */
    if(singlePresses[i] || doublePresses[i] || longPresses[i] || longReleases[i])  anyAction = true;

    /* ----------------- if nothing happened update empty status ---------------- */
    if (anyAction == false && i == numButtons-1) {btn_action->setStatus("");}
  }
  
  /* ------------------------------- Reset flags ------------------------------ */
  for (int i = 0; i < numButtons; i++)
  {
    singlePresses[i] = false;
    doublePresses[i] = false;
    longPresses[i] = false;
    longReleases[i] = false;
  }

  // myAction->setStatus("led_on");

  /* -------------------------- update device status -------------------------- */
  if (eth_mqttClient.connected())
  {
    myIoTdevice.update(eth_mqttClient);
  }
  else if (wifi_mqttClient.connected())
  {
    myIoTdevice.update(wifi_mqttClient);
  }
  else
  {
    // Serial.println("Client disconnected");
  }
}

void setup()
{
  /* --------------------- Initialize Serial Communication -------------------- */
  Serial.begin(115200);

  while (!Serial)
    ;

  Serial.print("\nStarting MQTTClient_Basic on " + String(ARDUINO_BOARD));
#ifdef ETHERNET_ENABLE
  Serial.println(" with " + String(SHIELD_TYPE));
  Serial.println(WEBSERVER_WT32_ETH01_VERSION);


  /* --------------------------- initialize Ethernet -------------------------- */
  // To be called before ETH.begin()
  WT32_ETH01_onEvent();

  ETH.begin(ETH_PHY_ADDR, ETH_PHY_POWER);

  WT32_ETH01_waitForConnect();
#endif

  /* ------------------------------ Initialize HW ----------------------------- */
  for (int i = 0; i < numButtons; i++)
  {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  /* -------------------------- Initialize Parameters ------------------------- */
  preferences.begin("my-app", false);

  /* ------------------------ Initiliaze WiFi Settings ------------------------ */
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  /* --------------------- Initialize WiFi Manager Object --------------------- */
  wm_init(false);

  /* ----------------------- Initialize MQTT Parameters ----------------------- */
  MQTT_init();

  /* -------------------------- Initialize IoT Device ------------------------- */
  IoT_device_init();

  // Allow the hardware to sort itself out
  delay(1500);
}

unsigned long reconnectTimestamp = 0;         // Last reconnect attempt time
unsigned long sensorUpdateTimestamp = 0;      // Last sensor update time
const unsigned long reconnectInterval = 5000; // Reconnect every 5000 ms
const unsigned long sensorInterval = 10000;    // Update sensor every 1000 ms

void loop()
{

  unsigned long currentMillis = millis();

  /* ----------------------- Process Command line inputs ---------------------- */
  commandLine.processInput();

  /* -------------------------- WiFi manager Process -------------------------- */
  wm.process();

  /* -------------------------- Verify Buttons State -------------------------- */
  for (int i = 0; i < numButtons; i++)
  {
    handleButtonState(i, millis());
  }

  /* ------------------------- Connect to MQTT Broker ------------------------- */
#ifdef ETHERNET_ENABLE
  if (WT32_ETH01_isConnected())
  {
    if (!eth_mqttClient.connected())
    {
      if (currentMillis - reconnectTimestamp >= reconnectInterval)
      {
        reconnectTimestamp = currentMillis;
        Serial.println("Trying to reconnect via Ethernet..");
        MQTT_reconnect(eth_mqttClient);

        if (eth_mqttClient.connected())
          wifi_mqttClient.disconnect();
      }
    }
    else
    {
      // Nothing to do
    }
  }
  else if (WL_CONNECTED == WiFi.status())
#endif
  //{
    /* --------------------- Try to reconnect to MQTT Broker -------------------- */
    if ( WL_CONNECTED == WiFi.status() && !wifi_mqttClient.connected())
    {
      if (currentMillis - reconnectTimestamp >= reconnectInterval)
      {
        reconnectTimestamp = currentMillis;
        Serial.println("Trying to reconnect via WiFi..");
        MQTT_reconnect(wifi_mqttClient);

        if (wifi_mqttClient.connected())
          eth_mqttClient.disconnect();
      }
    }
    else
    {
      // Nothing to do
    }
  //}

  
  /* -------------------------- Update MQTT Entities -------------------------- */
  for (int i = 0; i < numButtons; i++)
  {
    bool forceUpdate = false;
    /* ------------------------- check for any new event ------------------------ */
    if (singlePresses[i] || doublePresses[i] || longPresses[i] || longReleases[i] )
    {
      forceUpdate = true;
    }

    if (forceUpdate) {
        deviceUpdate(); 
        delay(500);
        deviceUpdate(); // clear status
    }
      
  }

  if (currentMillis - sensorUpdateTimestamp >= sensorInterval)
  {
    sensorUpdateTimestamp = currentMillis;
    deviceUpdate();
  }

  /* ------------------------ update timestamp each ms ------------------------ */
  if (timestamp != millis())
    timestamp = millis();

  /* ------------------------------ Loop Clients ------------------------------ */
  eth_mqttClient.loop();
  wifi_mqttClient.loop();
}

void IoT_device_init()
{

  /* ---------------- add all entities to the iot device object --------------- */
  myIoTdevice.addEntity(btn_action);

  /* -------------------------- Configure IoT Device -------------------------- */
  if (eth_mqttClient.connected())
    myIoTdevice.configure(eth_mqttClient); // Update device configuration
  else if (wifi_mqttClient.connected())
    myIoTdevice.configure(wifi_mqttClient); // Update device configuration
  else
    Serial.println("Failed to send Device Configuration via MQTT. Client disconnected.");
}



void MQTT_callback(char *topic, byte *message, unsigned int length)
{

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }

  Serial.println();

  /* -------------------------- update device status -------------------------- */
  deviceUpdate();
}

void handleButtonState(int buttonIndex, unsigned long currentTime)
{
  bool currentButtonState = !digitalRead(buttonPins[buttonIndex]); // Active LOW button
  //bool stateChanged = (currentButtonState != lastButtonState[buttonIndex]);
  bool stateChanged = (buttonStates[buttonIndex] != sm_lastBtnState[buttonIndex]);

  switch (buttonStates[buttonIndex])
  {
  case IDLE:
    
    //if (stateChanged) Serial.println ("Button " + String(buttonIndex) + " IDLE");
    
    /* ------------------------- Reset number of presses ------------------------ */
    buttonPresses[buttonIndex] = 0;

    if (currentButtonState)
    { // Button pressed
      //Serial.println ("Button " + String(buttonIndex) + " Transition to DEBOUNCE");
      buttonStates[buttonIndex] = DEBOUNCE;
      stateStartTime[buttonIndex] = currentTime;
    }
    break;

  case DEBOUNCE:

    //if (stateChanged) Serial.println ("Button " + String(buttonIndex) + " DEBOUNCE");
  
    if ((currentTime - stateStartTime[buttonIndex]) > debounceDelay)
    {
      if (currentButtonState)
      { // Confirmed press
        //Serial.println ("Button " + String(buttonIndex) + " Transition to PRESSED");
        buttonStates[buttonIndex] = PRESSED;
        pressStartTime[buttonIndex] = currentTime;
      }
      else
      { // False press
        //Serial.println ("Button " + String(buttonIndex) + " Transition to IDLE");
        buttonStates[buttonIndex] = IDLE;
      }
    }
    break;

  case PRESSED:
    //if (stateChanged) Serial.println ("Button " + String(buttonIndex) + " PRESSED");

    if (!currentButtonState)
    { // Button released
      //Serial.println ("Button " + String(buttonIndex) + " Short Press_" + String(buttonPresses[buttonIndex])) ;
      //Serial.println ("Button " + String(buttonIndex) + " Transition to SHORT_PRESS");
      buttonPresses[buttonIndex]++; 
      buttonStates[buttonIndex] = SHORT_PRESS;
      stateStartTime[buttonIndex] = currentTime;
    }
    else if ((currentTime - pressStartTime[buttonIndex]) > longPressDuration)
    { // Long press detected
      Serial.println ("Button " + String(buttonIndex + 1) + " Long Press");      
      //Serial.println ("Button " + String(buttonIndex) + " Transition to LONG_PRESS");
      longPresses[buttonIndex] = true;
      buttonStates[buttonIndex] = LONG_PRESS;
    }
    break;
case SHORT_PRESS:
    if (currentButtonState && (currentTime - stateStartTime[buttonIndex] > debounceDelay)) {
        // Second press detected after debounce
        //Serial.println("Button " + String(buttonIndex) + " Transition to DEBOUNCE");
        buttonStates[buttonIndex] = DEBOUNCE;
        stateStartTime[buttonIndex] = currentTime; // Reset debounce timer
    } else if ((currentTime - stateStartTime[buttonIndex]) > doublePressThreshold) {
        // Timeout for second press, register as single press
        if (buttonPresses[buttonIndex] == 1) {            
            //Serial.println("Button " + String(buttonIndex) + " Transition to SINGLE_PRESS");
            buttonStates[buttonIndex] = SINGLE_PRESS;
        } else if (buttonPresses[buttonIndex] > 1) {        
            //Serial.println("Button " + String(buttonIndex) + " Transition to DOUBLE_PRESS");
            buttonStates[buttonIndex] = DOUBLE_PRESS;
        }
    }
    break;

  case LONG_PRESS:
    //if (stateChanged) Serial.println ("Button " + String(buttonIndex) + " LONG_PRESS");
    
    if (!currentButtonState)
    { // Button released after long press
      //Serial.println ("Button " + String(buttonIndex) + " Transition to LONG_RELEASE");
      buttonStates[buttonIndex] = LONG_RELEASE;
    }
    break;

  case LONG_RELEASE:
    //if (stateChanged) Serial.println ("Button " + String(buttonIndex) + " LONG_RELEASE");
    Serial.println ("Button " + String(buttonIndex + 1) + " Long Release");
    longReleases[buttonIndex] = true;
    
    //Serial.println ("Button " + String(buttonIndex) + " Transition to IDLE");
    buttonStates[buttonIndex] = IDLE;
    break;

  case SINGLE_PRESS:
    //if (stateChanged) Serial.println ("Button " + String(buttonIndex) + " SINGLE_PRESS");
    Serial.println("Button " + String(buttonIndex + 1) + " Single Press");
    singlePresses[buttonIndex] = true;
    
    //Serial.println ("Button " + String(buttonIndex) + " Transition to IDLE");
    buttonStates[buttonIndex] = IDLE;
    break;

  case DOUBLE_PRESS:
    //if (stateChanged) Serial.println ("Button " + String(buttonIndex) + " DOUBLE_PRESS");
    Serial.println("Button " + String(buttonIndex + 1) + " Double Press");
    doublePresses[buttonIndex] = true;
    
    //Serial.println ("Button " + String(buttonIndex) + " Transition to IDLE");
    buttonStates[buttonIndex] = IDLE;
    break;
  }
  sm_lastBtnState[buttonIndex] = buttonStates[buttonIndex];
  lastButtonState[buttonIndex] = currentButtonState;
}

//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

void saveConfigCallback()
{
  /* --------------------------- Store MQTT settings -------------------------- */
  preferences.putString(PREF_MQTT_SERVER, custom_mqtt_server.getValue());
  preferences.putString(PREF_MQTT_PORT, custom_mqtt_port.getValue());
  preferences.putString(PREF_MQTT_USER, custom_mqtt_user.getValue());
  preferences.putString(PREF_MQTT_PASSWORD, custom_mqtt_password.getValue());
}

void MQTT_init()
{

  /* ----------------------- Initialize MQTT Parameters ----------------------- */
  String _mqtt_port = preferences.getString(PREF_MQTT_PORT, mqtt_port);
  String _mqtt_server = preferences.getString(PREF_MQTT_SERVER, mqtt_server);
  String _mqtt_user = preferences.getString(PREF_MQTT_USER, mqtt_user);
  String _mqtt_password = preferences.getString(PREF_MQTT_PASSWORD, mqtt_password);

  /* ---------------------- Remove whitespaces at the end --------------------- */
  _mqtt_user.trim();
  _mqtt_password.trim();
  _mqtt_port.trim();
  _mqtt_server.trim();

  custom_mqtt_server.setValue(_mqtt_server.c_str(), 128);
  custom_mqtt_port.setValue(_mqtt_port.c_str(), 128);
  custom_mqtt_user.setValue(_mqtt_user.c_str(), 128);
  custom_mqtt_password.setValue(_mqtt_password.c_str(), 128);

  wifi_mqttClient.setServer(custom_mqtt_server.getValue(), 1883);
  wifi_mqttClient.setCallback(MQTT_callback);
  eth_mqttClient.setServer(custom_mqtt_server.getValue(), 1883);
  eth_mqttClient.setCallback(MQTT_callback);

  /* ------------------------- Connect to MQTT Broker ------------------------- */
  wifi_mqttClient.connect("ESP8266Client99", _mqtt_user.c_str(), _mqtt_password.c_str());
  eth_mqttClient.connect("ESP8266Client99", _mqtt_user.c_str(), _mqtt_password.c_str());
}

void wm_init(bool _reset)
{

  if (_reset)
    wm.resetSettings(); // reset settings - wipe credentials for testing

  wm.addParameter(&custom_mqtt_server);
  wm.addParameter(&custom_mqtt_port);
  wm.addParameter(&custom_mqtt_user);
  wm.addParameter(&custom_mqtt_password);

  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(60);

  // automatically connect using saved credentials if they exist
  // If connection fails it starts an access point with the specified name
  if (wm.autoConnect("AutoConnectAP"))
  {
    Serial.println("connected...yeey :)");
  }
  else
  {
    Serial.println("Failed to connect");
  }

  // set config save notify callback
  wm.setSaveParamsCallback(saveConfigCallback);
}

void MQTT_reconnect(PubSubClient &_client)
{

  /* ------------------------ Retrieve data from memory ----------------------- */
  String _mqtt_user = preferences.getString(PREF_MQTT_USER, mqtt_user);
  String _mqtt_password = preferences.getString(PREF_MQTT_PASSWORD, mqtt_password);
  String _mqtt_port = preferences.getString(PREF_MQTT_PORT, mqtt_port);
  String _mqtt_server = preferences.getString(PREF_MQTT_SERVER, mqtt_server);

  /* ---------------------- Remove whitespaces at the end --------------------- */
  _mqtt_user.trim();
  _mqtt_password.trim();
  _mqtt_port.trim();
  _mqtt_server.trim();

  if (!_client.connected())
  {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    _client.setServer(_mqtt_server.c_str(), _mqtt_port.toInt());

    if (_client.connect("ESP8266Client99", _mqtt_user.c_str(), _mqtt_password.c_str()))
    {
      Serial.println("connected");

      /* --------------------- Update IoT Device configuration -------------------- */
      myIoTdevice.configure(_client); // Update device configuration
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(_client.state());
    }
  }
}

int randomInt()
{
  return rand();
}

bool randomBool()
{
  return rand() % 2;
}
