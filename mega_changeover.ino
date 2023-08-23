#include <SoftwareSerial.h>
#include "EmonLib.h" // Include Emon Library
EnergyMonitor emon1; // Create an instance
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define TINY_GSM_MODEM_SIM800
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <TinyGsmClient.h>
#include <PubSubClient.h>

void autoMode();
void refreshDisplay();
void output(byte Outsignal);
void batLow();
float batlvl();
void TurnOnffGen();
void TurnOnGen();
void GenFaultCheck();
void InvFaultCheck();
void NepaFaultCheck();
void overloadcheck();
void lastStateCheck();
void fault();
void manualMode();
void sendEsp();
boolean mqttConnect();
void mqttCallback();

byte nepaInput = A0;
byte inverterInput = A1;
byte genInput = A7;
byte counter0;
byte counter1;
byte counter2;
int eepromvalue;
byte toggler = 3;
byte buzzerState;
byte SetMode;
float bat;
boolean oledState;
int AcVoltOut0;
int AcVoltOut1;
int AcVoltOut2;
double Irms;

int wattage0;
int wattage1;
int wattage2;
byte wattagecounter;

//buzzer== A3

//auto/ manual pin
//byte modePin = 2;

//Check inputs
byte Genout;
byte Invout;
byte Mainout;

//relay mosfets
byte invMosfet = 4;
byte nepaMosfet = 5;
byte genMosfet = 6;

//esp-01 pins
//SoftwareSerial esp(10, 11); // RX, TX

byte currPin = A6;

unsigned long startTime; //some global variables available anywhere in the program
unsigned long currentTime;
const unsigned long waitTime = 2000; //the value is a number of milliseconds

unsigned long initTime; //some global variables available anywhere in the program
unsigned long timeNow;
const unsigned long SetTime = 8000; //the value is a number of milliseconds

//mode 1=nepa mode 2=inverter mode 3= gen mode 4= off all
byte Mode = 1;

float input_voltage = 0.0;
float temp = 0.0;
float r1 = 12000.0;
float r2 = 800.011;

// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C

// Define proper RST_PIN if required.
#define RST_PIN -1

SSD1306AsciiAvrI2c display;

//eeprom
#include <EEPROM.h>
int addr = 0;

// volt conv
//String voltStr;
char volt[10];
int vvolt;
/// gsm
byte count = 0;

#define SerialAT Serial1 //Connect SIM_RX to 18, SIM_TX to 19 of MEGA
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(client);

uint32_t lastReconnectAttempt = 0;
//Network details
const char apn[] = "web.gprs.mtnnigeria.net";
const char user[] = "web";
const char pass[] = "web";

// MQTT details
const char *broker = "broker.hivemq.com";
const char *topicOut = "dan/out";
const char *topicIn = "dan/in";
const char *mqtt_user = "changeover";
const char *mqtt_pass = "changeover";

boolean mqttConnect()
{

  // boolean status = ;
  if (!mqtt.connect("GsmClientName"))
  {
    //Serial.print(".");
    return false;
  }
  Serial.println(F("Connected to broker."));
  mqtt.subscribe(topicIn);
  return mqtt.connected();
}

void mqttCallback(char *topic, byte *payload, unsigned int len)
{

  Serial.print(topic);

  Serial.write(payload, len);
  Serial.println("");

  /*Only proceed if incoming message's topic matches
  if (String(topic) == topicLed) {
    ledStatus = !ledStatus;
    digitalWrite(LED_PIN, ledStatus);
    mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
  }
  */
}

void sendEsp(char message)
{ // write line to log
  Serial2.println(message);
}

void manualMode() //Funtion runs if the manual mode switch is pressed
{
  if (digitalRead(toggler) == LOW && oledState == false)
  {
    oledState = true;
  }

  if (digitalRead(toggler) == LOW && oledState == true)
  {

    startTime = millis(); //initial start time
    while (digitalRead(toggler) == LOW)
    {
      currentTime = millis();
      if (currentTime - startTime >= waitTime) //test whether the period has elapsed
      {
        display.clearDisplay();

        switch (Mode)
        {

        case 1: //If the "main" menu is selected
          //display.clearDisplay();
          display.setCursor(5, 3);
          display.setTextSize(2);
          display.print("OUT");
          display.print(":");

          display.setCursor(50, 3);
          display.setTextSize(2);
          display.print("INV.");
          display.display();

          Mode++;
          break;

        case 2: //If the "Inverter" menu is selected
          display.setCursor(5, 3);
          display.setTextSize(2);
          display.print("OUT");
          display.print(":");

          display.setCursor(50, 3);
          display.setTextSize(2);
          display.print("GEN");
          display.display();

          Mode++;
          break;

        case 3: //If the "Gen" menu is selected
          display.setCursor(5, 3);
          display.setTextSize(2);
          display.print("OUT");
          display.print(":");

          display.setCursor(50, 3);
          display.setTextSize(2);
          display.print("OFF.");
          display.display();

          Mode++;
          break;

        case 4: //If the "off" menu is selected
          display.setCursor(5, 3);
          display.setTextSize(2);
          display.print("OUT");
          display.print(":");

          display.setCursor(50, 3);
          display.setTextSize(2);
          display.print("MAINS");
          display.display();
          Mode++;
          break;
        }

        if (Mode > 4)
        {
          Mode = 1;
        }
        startTime = millis();
      }
    }
  }

  switch (Mode)
  {

  case 1: //Switch to Nepa
    NepaFaultCheck();
    if (counter0 == 0)
    {
      output(1);
      sendEsp("genOff");
      break;
    }
  case 2: //Switch to Inv
    InvFaultCheck();
    if (counter1 == 0)
    {
      sendEsp("genOff");
      output(2);

      break;
    }
  case 3: //Switch to Gen
    GenFaultCheck();
    sendEsp("genOn");

    if (counter2 == 0)
    {
      sendEsp("genOn");
      output(3);

      break;
    }
  case 4: //Switch off
    output(4);
    sendEsp("genOff");
    break;
  }
}

void autoMode()
{

  if (Mainout == 0 && Invout == 0 && Genout == 0)
  {
    sendEsp("genOn");
  }

  if (Mainout == 1 && Invout == 1 && Genout == 1)
  {
    if (counter0 == 0)
    {

      NepaFaultCheck();
      output(1);
      Mode = 1;
      sendEsp("genOff");
    }
  }

  if (Mainout == 1 && Invout == 0 && Genout == 0)
  {
    NepaFaultCheck();
    if (counter0 == 0)
    {
      Mode = 1;
      output(1);
    }
  }

  if (Mainout == 1 && Invout == 0 && Genout == 1)
  {
    NepaFaultCheck();
    if (counter0 == 0)
    {
      Mode = 1;
      sendEsp("genOff");
      output(1);
    }
  }
  if (Mainout == 0 && Invout == 0 && Genout == 1)
  {

    GenFaultCheck();
    if (counter2 == 0)
    {
      Mode = 3;
      output(3);
    }
  }
  if (Mainout == 1 && Invout == 0 && Genout == 1)
  {
    NepaFaultCheck();
    if (counter0 == 0)
    {
      Mode = 1;
      output(1);
      sendEsp("genOff");
    }
  }
  if (Mainout == 0 && Invout == 1 && Genout == 0)
  {
    InvFaultCheck();
    if (counter1 == 0)
    {
      Mode = 2;
      output(2);
    }
  }
  if (Mainout == 0 && Invout == 1 && Genout == 1)
  {
    InvFaultCheck();
    if (counter1 == 0)
    {
      Mode = 2;
      sendEsp("genOff");
      output(2);
    }
  }
}

void refreshDisplay()
{
  display.clearDisplay();

  //Serial.println(Mode);
  //Serial.println(AcVoltOut0); // Display the AC voltage on serial monitor
  //Serial.println(AcVoltOut1); // Display the AC voltage on serial monitor
  //Serial.println(AcVoltOut2); // Display the AC voltage on serial monitor

  if (AcVoltOut0 >= 50) //Check *if there is power on the "NEPA" input
  {
    if (Mode == 1) //check if the output is "NEPA" and highlight text
    {

      display.setTextColour(BLACK, WHITE);
      display.setCursor(10, 0);
      display.setTextSize(1);
      display.println("MAIN");

      display.setTextColour(BLACK, WHITE);
      display.setCursor(10, 1);
      display.print(AcVoltOut0);
      display.print("v");
      display.display();
    }
    else
    {

      display.setCursor(10, 0);
      display.setTextSize(1);
      display.println("NEPA");

      display.setCursor(10, 1);
      display.print(AcVoltOut0);
      display.print("v");
      display.display();
    }
  }
  else

  {
    display.setCursor(10, 0);
    display.setTextSize(1);
    display.println("    ");

    display.setCursor(10, 1);
    display.print("   ");
    display.print("   ");
    display.display();
  }
  //.............................................................

  if (AcVoltOut1 >= 50) //Check if there is power on the "inverter" input
  {
    if (Mode == 2) //check if the output is "inverter" and highlight text
    {
      display.setTextColour(BLACK, WHITE);
      display.setCursor(55, 0);
      display.setTextSize(1);
      display.println("INV");

      display.setTextColour(BLACK, WHITE);
      display.setCursor(55, 1);
      display.print(AcVoltOut1);
      display.print("v");
      display.display();
    }
    else
    {
      display.setCursor(55, 0);
      display.setTextSize(1);
      display.println("INV");

      display.setCursor(55, 1);
      display.print(AcVoltOut1);
      display.print("v");
      display.display();
    }
  }

  else
  {
    display.setCursor(55, 0);
    display.setTextSize(1);
    display.println("    ");

    display.setCursor(55, 1);
    display.print("   ");
    display.print("   ");
    display.display();
  }

  //..............................................................

  if (AcVoltOut2 >= 50) //Check *if there is power on the "Generator" input
  {
    if (Mode == 3) //check if the output is "Generator" and highlight text
    {

      display.setTextColor(BLACK, WHITE);
      display.setCursor(95, 0);
      display.setTextSize(1);
      display.println("GEN");

      display.setTextColor(BLACK, WHITE);
      display.setCursor(95, 1);
      display.print(AcVoltOut2);
      display.print("v");
      display.display();
    }
    else
    {

      display.setCursor(95, 0);
      display.setTextSize(1);
      display.println("GEN");

      display.setCursor(95, 1);
      display.print(AcVoltOut2);
      display.print("v");
      display.display();
    }
  }
  else

  {
    display.setCursor(95, 0);
    display.setTextSize(1);
    display.println("    ");

    display.setCursor(95, 1);
    display.print("   ");
    display.print("   ");
    display.display();
  }

  ////////////////////////////////////////////////////////////////////////////////
  switch (Mode)
  {

  case 1:
    if (counter0 == 1)
    {
      display.setCursor(5, 3);
      display.setTextSize(2);
      display.print("MAIN");
      display.print(":");

      display.setCursor(50, 3);
      display.setTextSize(2);
      display.print("FAULT");
      display.display();
      break;
    }

    else if (counter0 == 0)
    {

      display.setCursor(5, 3);
      display.setTextSize(2);
      display.print("OUT");
      display.print(":");

      display.setCursor(50, 3);
      display.setTextSize(2);
      display.print(wattage0);
      display.print("W   ");
      display.display();
      break;
    }
  case 2:

    if (counter1 == 1)
    {

      display.setCursor(5, 3);
      display.setTextSize(2);
      display.print("INV");
      display.print(":");

      display.setCursor(50, 3);
      display.setTextSize(2);
      display.print("FAULT");
      display.display();
      break;
    }

    else if (counter1 == 0)
    {

      display.setCursor(5, 3);
      display.setTextSize(2);
      display.print("OUT");
      display.print(":");

      display.setCursor(50, 3);
      display.setTextSize(2);
      display.print(wattage1);
      display.print("W   ");
      display.display();
      break;
    }

  case 3:
    if (counter2 == 1)
    {

      display.setCursor(5, 3);
      display.setTextSize(2);
      display.print("GEN");
      display.print(":");

      display.setCursor(50, 3);
      display.setTextSize(2);
      display.print("FAULT");
      display.display();
      break;
    }

    else if (counter2 == 0)
    {
      display.setCursor(5, 3);
      display.setTextSize(2);
      display.print("OUT");
      display.print(":");

      display.setCursor(50, 3);
      display.setTextSize(2);
      display.print(wattage2);
      display.print("W   ");

      display.display();
      break;
    }

  case 4:
    if (wattagecounter == 1)
    {

      display.setCursor(5, 3);
      display.setTextSize(2);
      display.print("OVERLOAD!!");
      display.print(":");
      display.display();
      break;
    }

  case 5:

    display.setCursor(45, 3);
    display.setTextSize(2);
    display.print("OFF");
    display.print(":");
    display.display();
    break;

    ////////////////////////////////////////////////////////////////////////////////
  }
  display.setTextSize(1);
  display.setCursor(5, 6);
  display.println("MODE");

  display.setCursor(55, 6);
  display.println("BATV");

  display.setCursor(95, 6);
  display.println("GENV");
  display.display();

  ////////////////////////////////////////////////////////////////////////

  if (digitalRead(2) == LOW) //manual mode
  {
    display.setTextSize(1);
    display.setCursor(0, 54);
    display.println("MANUAL");
    display.display();
  }

  else if (digitalRead(2) == HIGH) //auto mode
  {
    display.setTextSize(1);
    display.setCursor(0, 54);
    display.println("AUTO.");
    display.display();
  }

  float batpercent = batlvl();
  Serial.println(batpercent);
  batpercent = (batpercent - 9.0) / (12.8 - 9.0) * 100;
  if (batpercent < 0)
  {
    batpercent = 0;
  }
  if (batpercent > 100)
  {
    batpercent = 100;
  }

  display.setTextSize(1);
  display.setCursor(55, 54);
  display.print(batpercent, 0);
  display.print("%");

  display.setCursor(95, 54);
  display.print("100");
  display.print("%");
  display.display();
}

////////////////////////////////////////////////////////////////////////

void fault()
{
  timeNow = millis();                //get the current "time" (actually the number of milliseconds since the program started)
  if (timeNow - initTime >= SetTime) //test whether the period has elapsed

  {
    if (buzzerState == 0)
    {
      digitalWrite(A3, HIGH);
      buzzerState = 1;
    }
    else
    {
      digitalWrite(A3, LOW);
      buzzerState = 0;
    }

    initTime = timeNow; //IMPORTANT to save the start time of the current LED state.
  }

  if (wattage0 < 16001 || wattage1 < 16001 || wattage2 < 16001)
  {
    wattagecounter = 0;
  }

  if (AcVoltOut0 <= 255 || AcVoltOut0 > 130 && AcVoltOut0 > 30)
  {
    counter0 = 0;
  }
  if (AcVoltOut1 <= 255 || AcVoltOut1 > 130 && AcVoltOut1 > 30)
  {
    counter1 = 0;
  }
  if (AcVoltOut2 <= 255 || AcVoltOut2 > 130 && AcVoltOut2 > 30)
  {
    counter2 = 0;
  }
}
void lastStateCheck()
{
  //Read eeprom
  eepromvalue = EEPROM.read(0);

  if (eepromvalue == 1) //last state was Nepa
  {
    NepaFaultCheck();

    digitalWrite(genMosfet, LOW); //Turn off Gen
    digitalWrite(invMosfet, LOW); //Turn off inverter
    delay(50);
    digitalWrite(nepaMosfet, HIGH); //Switch to Nepa
  }

  if (eepromvalue == 2) //last state was Inverter
  {
    InvFaultCheck();

    digitalWrite(genMosfet, LOW);  //Turn off Gen
    digitalWrite(nepaMosfet, LOW); //Turn off Nepa
    delay(50);
    digitalWrite(invMosfet, HIGH); //Switch on inverter
  }

  if (eepromvalue == 3) //last state was Gen
  {
    GenFaultCheck();

    digitalWrite(nepaMosfet, LOW); //Switch off Nepa
    digitalWrite(invMosfet, LOW);  //Turn off inverter
    delay(50);
    digitalWrite(genMosfet, HIGH); //Switch on Gen
  }
}

void overloadcheck()

{

  if (wattage0 >= 16001 || wattage1 >= 16001 || wattage2 >= 16001)
  {
    output(4);
    wattagecounter = 1;
    fault();
  }
}

void NepaFaultCheck()
{

  if (AcVoltOut0 > 255 || AcVoltOut0 < 130 && AcVoltOut0 > 30)
  {
    Serial.println(("faulty mains"));
    counter0 = 1;
    digitalWrite(nepaMosfet, LOW);
    fault();
  }
}

void InvFaultCheck()
{

  if (AcVoltOut1 > 255 || AcVoltOut1 < 130 && AcVoltOut1 > 30)
  {
    Serial.println(("faulty mains"));
    counter1 = 1;
    digitalWrite(invMosfet, LOW);
    fault();
  }
}

void GenFaultCheck()
{
  if (AcVoltOut2 > 255 || AcVoltOut2 < 130 && AcVoltOut2 > 30)
  {
    Serial.println(("faulty mains"));
    counter2 = 1;
    digitalWrite(genMosfet, LOW);
    fault();
  }
}

void TurnOnGen()
{
  sendEsp("genOn");
}

void TurnOnffGen()
{
  sendEsp("genOff");
}

float batlvl()
{

  // temp = ( average * 5.4567) / 1023.0;
  temp = (analogRead(A2) * 4.8) / 1023.0;
  input_voltage = temp / (r2 / (r1 + r2));

  return input_voltage;
}

void batLow()
{
  bat = batlvl();
  //Serial.println(bat);
  if (bat < 10.0)
  {
    if (Mode != SetMode)
    {
      EEPROM.write(0, Mode);
      SetMode = Mode;
    }
  }
  else
  {
  }
}

void output(byte Outsignal)
{
  if (Outsignal == 4)
  {
    digitalWrite(invMosfet, LOW);
    delay(100);
    digitalWrite(genMosfet, LOW);
    delay(100);
    digitalWrite(nepaMosfet, LOW);
  }

  else if (Outsignal == 1)
  {

    digitalWrite(invMosfet, LOW);
    delay(100);
    digitalWrite(genMosfet, LOW);
    delay(100);
    digitalWrite(nepaMosfet, HIGH);
  }

  else if (Outsignal == 2)
  {

    digitalWrite(invMosfet, HIGH);
    delay(100);
    digitalWrite(genMosfet, LOW);
    delay(100);
    digitalWrite(nepaMosfet, LOW);
  }

  else if (Outsignal == 3)
  {
    digitalWrite(invMosfet, LOW);
    delay(100);
    digitalWrite(genMosfet, HIGH);
    delay(100);
    digitalWrite(nepaMosfet, LOW);
  }
}

//Setup function Start
void setup()
{
  pinMode(nepaInput, INPUT);
  pinMode(inverterInput, INPUT);
  pinMode(genInput, INPUT);
  pinMode(2, INPUT_PULLUP);
  pinMode(toggler, INPUT_PULLUP);

  pinMode(invMosfet, OUTPUT);
  pinMode(nepaMosfet, OUTPUT);
  pinMode(genMosfet, OUTPUT);
  //disable all relays
  digitalWrite(invMosfet, LOW);
  digitalWrite(nepaMosfet, LOW);
  digitalWrite(genMosfet, LOW);
  //digitalWrite(genMosfet, HIGH);

  Serial.begin(115200);
  Serial2.begin(9600);
  SerialAT.begin(9600);
  //analogReference(INTERNAL);
  Serial.println("System start.");
  modem.restart(7);
  Serial.println("Modem: " + modem.getModemInfo());
  Serial.println("Searching for telco provider.");
  if (!modem.waitForNetwork())
  {
    Serial.println("fail");
    while (true)
      ;
  }
  Serial.println(("Connected to telco."));
  Serial.println(("Signal Quality: " + String(modem.getSignalQuality())));

  Serial.println(("Connecting to GPRS network."));
  if (!modem.gprsConnect(apn, user, pass))
  {
    Serial.println(("fail"));
    while (true)
      ;
  }
  Serial.println(("Connected to GPRS: " + String(apn)));

  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
  Serial.println(("Connecting to MQTT Broker: " + String(broker)));
  while (mqttConnect() == false)
    continue;
  Serial.println("");

  //////////////////////////Initialize Oled screen
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();
  display.display();
  /////////////////////////////////////////

  emon1.current(6, 48.0);

  lastStateCheck();
}

//Loop Function Start Here
void loop()
{
  //sendEsp();
  batLow();
  refreshDisplay();
  overloadcheck();

  if (Serial2.available())
  {
    delay(10);
    String message = "";
    while (Serial2.available())
      message += (char)Serial2.read();

    Serial.println(message);
  }

  if (mqtt.connected())
  {
    mqtt.loop();
  }
  //Reconnect gsm
  if (!modem.isNetworkConnected())
  {
    //Serial.println(("Network disconnected"));
    if (!modem.waitForNetwork(10000L, true))
    {
      count++;
      if (count > 5)
      {
        count = 0;
        modem.restart(7);
      }
    }
  }

  if (!modem.isGprsConnected())
  {
    if (!modem.gprsConnect(apn, user, pass))
    {
      delay(5000);
      return;
    }
  }

  //Reconnect mqtt
  if (!mqtt.connected())
  {
    count++;
    if (count > 250)
    {
      count = 0;
      modem.restart(7);
      delay(100);
    }
    if (!mqttConnect())
    {
      // lastReconnectAttempt = 0;
    }
  }

  // mqttConnect();

  itoa(AcVoltOut0, volt, 10);
  mqtt.publish("voltage/1", volt);
  itoa(AcVoltOut1, volt, 10);
  mqtt.publish("voltage/2", volt);
  itoa(AcVoltOut2, volt, 10);
  mqtt.publish("voltage/3", volt);

  Irms = emon1.calcIrms(1480);
  wattage0 = Irms * AcVoltOut0;
  wattage1 = Irms * AcVoltOut1;
  wattage2 = Irms * AcVoltOut2;

  Serial.println(Irms);

  AcVoltOut0 = (analogRead(nepaInput) * (4.9 / 1023)) * 93.67; // Convert the AC divison into volt
  if (AcVoltOut0 >= 140)
  {
    Mainout = 1;
  }
  else
  {
    Mainout = 0;
  }
  //int faultflagnepa;

  //Read the division at analog pin A0 (0 to 1023)
  AcVoltOut1 = (analogRead(inverterInput) * (4.9 / 1023)) * 93.67; // Convert the AC divison into volt

  if (AcVoltOut1 >= 140)
  {
    Invout = 1;
  }
  else
  {
    Invout = 0;
  }

  //Read the division at analog pin A0 (0 to 1023)
  AcVoltOut2 = (analogRead(genInput) * (4.9 / 1023)) * 93.67; // Convert the AC divison into volt

  if (AcVoltOut2 >= 140)
  {
    Genout = 1;
  }
  else
  {
    Genout = 0;
  }

  if (digitalRead(2) == LOW) //manual mode
  {
    manualMode();
  }

  else if (digitalRead(2) == HIGH) //auto mode
  {
    autoMode();
  }
}
