#include <SoftwareSerial.h>
#include "EmonLib.h" // Include Emon Library
EnergyMonitor emon1; // Create an instance
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ArduinoJson.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TINY_GSM_MODEM_SIM800
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
float genv;
bool internetMode;
bool chm;
byte nepaInput = A0;
byte inverterInput = A1;
byte genInput = A7;
byte counter0;
byte counter1;
byte counter2;
int eepromvalue;
byte toggler = 8;
int mqttSetMode;
//byte modeBtn = 2;
int bttn = 0;
byte buzzerState;
byte setOperatingMode;
byte SetMode;
float bat;
boolean oledState;
int AcVoltOut0;
int AcVoltOut1;
int AcVoltOut2;
double Irms;
float batpercent;
float genpercent;
byte operatingMode;

int wattage0;
int wattage1;
int wattage2;
byte wattagecounter;

byte phonemode;
byte phoneout;
byte phoneshtdown;

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
float r1 = 10000.0;
float r2 = 965;
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
const char apn[] = "inernet.ng.airtel.com";
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
        Serial.print(".");
        return false;
    }
    Serial.println(F("Connected to broker."));
    mqtt.subscribe(topicOut);
    return mqtt.connected();
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{

    char str[length + 1];
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    int i = 0;
    for (i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
        str[i] = (char)payload[i];
    }
    str[i] = 0; // Null termination
    Serial.println();

    StaticJsonDocument<256> doc;
    deserializeJson(doc, payload);

    // deserializeJson(doc,str); can use string instead of payload
    phonemode = doc["phmode"]; //send 1 for auto and 0 for manual mode
    phoneout = doc["phout"];   // sends 1 for main,2 for inv, 3 for gen and 4 for off
    // phoneshtdown = doc["pshtdwn"]; // send 0 to shutdown change over and 1 for on
    //{"phmode":"value","phout":"value", "pshtdwn":"value", }

    if (phonemode == 1)
    {
        operatingMode = 1;
    }
    else if (phonemode == 0)
    {
        operatingMode = 2;
        Mode = phoneout;
    }
}

void sendEsp(String message)
{ // write line to log
    // Serial.print("sending ");

    if (message == "genOff")
    {
        // Serial.println("0   ");
        Serial2.println("0");
    }
    if (message == "genOn")
    {
        // Serial.print("1    ");
        Serial2.println("1");
    }
}

void manualMode() //Funtion runs if the manual mode switch is pressed
{

    if (digitalRead(toggler) == LOW && digitalRead(3) == HIGH && digitalRead(2) == HIGH)
    {
        Serial.print("in   ");
        startTime = millis(); //initial start time
        while (digitalRead(toggler) == LOW && digitalRead(2) == HIGH && digitalRead(3) == HIGH)
        {
            currentTime = millis();
            if (currentTime - startTime >= waitTime) //test whether the period has elapsed
            {

                display.clearDisplay();
                switch (Mode)
                {

                case 1: //If the "main" menu is selected
                    display.clearDisplay();
                    display.setCursor(5, 25);
                    display.setTextSize(2);
                    display.print("OUT");
                    display.print(":");

                    display.setCursor(50, 25);
                    display.setTextSize(2);
                    display.print("INV.");
                    display.display();
                    Mode++;
                    break;

                case 2: //If the "Inverter" menu is selected
                    display.clearDisplay();
                    display.setCursor(5, 25);
                    display.setTextSize(2);
                    display.print("OUT");
                    display.print(":");

                    display.setCursor(50, 25);
                    display.setTextSize(2);
                    display.print("GEN");
                    display.display();
                    Mode++;
                    break;

                case 3: //If the "Gen" menu is selected
                    display.clearDisplay();
                    display.setCursor(5, 25);
                    display.setTextSize(2);
                    display.print("OUT");
                    display.print(":");

                    display.setCursor(50, 25);
                    display.setTextSize(2);
                    display.print("OFF");
                    display.display();
                    Mode++;
                    break;

                case 4: //If the "off" menu is selected
                    display.clearDisplay();
                    display.setCursor(5, 25);
                    display.setTextSize(2);
                    display.print("OUT");
                    display.print(":");

                    display.setCursor(50, 25);
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

        sendEsp("genOff");
        break;

    case 2: //Switch to Inv

        sendEsp("genOff");
        break;

    case 3: //Switch to Gen

        sendEsp("genOn");
        break;

    case 4: //Switch off

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

            Mode = 1;
            sendEsp("genOff");
        }
    }
    if (Mainout == 1 && Invout == 1 && Genout == 0)
    {
        NepaFaultCheck();
        if (counter0 == 0)
        {
            Mode = 1;
        }
    }

    if (Mainout == 1 && Invout == 0 && Genout == 0)
    {
        NepaFaultCheck();
        if (counter0 == 0)
        {
            Mode = 1;
        }
    }

    if (Mainout == 1 && Invout == 0 && Genout == 1)
    {
        NepaFaultCheck();
        if (counter0 == 0)
        {
            Mode = 1;
            sendEsp("genOff");
        }
    }
    if (Mainout == 0 && Invout == 0 && Genout == 1)
    {

        GenFaultCheck();
        if (counter2 == 0)
        {
            Mode = 3;
        }
    }
    if (Mainout == 1 && Invout == 0 && Genout == 1)
    {
        NepaFaultCheck();
        if (counter0 == 0)
        {
            Mode = 1;

            sendEsp("genOff");
        }
    }
    if (Mainout == 0 && Invout == 1 && Genout == 0)
    {
        InvFaultCheck();
        if (counter1 == 0)
        {
            Mode = 2;
        }
    }
    if (Mainout == 0 && Invout == 1 && Genout == 1)
    {
        InvFaultCheck();
        if (counter1 == 0)
        {
            Mode = 2;
            sendEsp("genOff");
        }
    }
}

void refreshDisplay()
{

    if (oledState == false)
    {
        display.clearDisplay();
        display.display();
    }
    else
    {
        display.clearDisplay();

        // Serial.println(Mode);
        AcVoltOut0 = (analogRead(nepaInput) * (4.9 / 1023)) * 93.67; // Convert the AC divison into volt
        if (AcVoltOut0 >= 50)                                        //Check *if there is power on the "NEPA" input
        {
            if (Mode == 1) //check if the output is "NEPA" and highlight text
            {
                display.setTextColor(BLACK, WHITE); // 'inverted' text
                display.setCursor(10, 0);
                display.setTextSize(1);
                display.println("MAIN");

                display.setCursor(10, 8);
                display.print(AcVoltOut0);
                display.print("v");
                display.setTextColor(WHITE, BLACK); // 'inverted' text
            }
            else
            {

                display.setCursor(10, 0);
                display.setTextSize(1);
                display.println("MAIN");

                display.setCursor(10, 8);
                display.print(AcVoltOut0);
                display.print("v");
            }
        }
        else

        {
            display.setCursor(10, 0);
            display.setTextSize(1);
            display.println("NEPA");

            display.setCursor(10, 8);
            display.print(" -- ");
            display.print("   ");

        }                                                                //.............................................................
        AcVoltOut1 = (analogRead(inverterInput) * (4.9 / 1023)) * 93.67; // Convert the AC divison into volt
        if (AcVoltOut1 >= 50)                                            //Check if there is power on the "inverter" input
        {
            if (Mode == 2) //check if the output is "inverter" and highlight text
            {
                display.setTextColor(BLACK, WHITE); // 'inverted' text
                display.setCursor(57, 0);
                display.setTextSize(1);
                display.println("INV");

                display.setCursor(55, 8);
                display.print(AcVoltOut1);
                display.print("v");
                display.setTextColor(WHITE, BLACK); // 'inverted' text
            }
            else
            {
                display.setCursor(57, 0);
                display.setTextSize(1);
                display.println("INV");

                display.setCursor(55, 8);
                display.print(AcVoltOut1);
                display.print("v");
            }
        }

        else
        {
            display.setCursor(57, 0);
            display.setTextSize(1);
            display.println("INV");

            display.setCursor(55, 8);
            display.print(" -- ");
            display.print("   ");
        }

        //..............................................................
        AcVoltOut2 = (analogRead(genInput) * (4.9 / 1023)) * 570.67;
        if (AcVoltOut2 >= 50) //Check *if there is power on the "Generator" input
        {
            if (Mode == 3) //check if the output is "Generator" and highlight text
            {

                display.setTextColor(BLACK, WHITE);
                display.setCursor(97, 0);
                display.setTextSize(1);
                display.println("GEN");

                display.setCursor(95, 8);
                display.print(AcVoltOut2);
                display.print("v");
                display.setTextColor(WHITE, BLACK); // 'inverted' text
            }
            else
            {

                display.setCursor(97, 0);
                display.setTextSize(1);
                display.println("GEN");

                display.setCursor(95, 8);
                display.print(AcVoltOut2);
                display.print("v");
            }
        }
        else

        {
            display.setCursor(97, 0);
            display.setTextSize(1);
            display.println("GEN");

            display.setCursor(95, 8);
            display.print(" -- ");
            display.print("   ");
        }

        ////////////////////////////////////////////////////////////////////////////////
        switch (Mode)
        {

        case 1:
            if (counter0 == 1)
            {
                display.setCursor(5, 25);
                display.setTextSize(2);
                display.print("MAIN");
                display.print(":");

                display.setCursor(50, 25);
                display.setTextSize(2);
                display.print("FAULT");

                break;
            }

            else if (counter0 == 0)
            {

                display.setCursor(5, 25);
                display.setTextSize(2);
                display.print("OUT");
                display.print(":");

                display.setCursor(50, 25);
                display.setTextSize(2);
                display.print(wattage0);
                display.print("W   ");

                break;
            }
        case 2:

            if (counter1 == 1)
            {

                display.setCursor(5, 25);
                display.setTextSize(2);
                display.print("INV");
                display.print(":");

                display.setCursor(50, 25);
                display.setTextSize(2);
                display.print("FAULT");

                break;
            }

            else if (counter1 == 0)
            {

                display.setCursor(5, 25);
                display.setTextSize(2);
                display.print("OUT");
                display.print(":");

                display.setCursor(50, 25);
                display.setTextSize(2);
                display.print(wattage1);
                display.print("W   ");

                break;
            }

        case 3:
            if (counter2 == 1)
            {

                display.setCursor(5, 25);
                display.setTextSize(2);
                display.print("GEN");
                display.print(":");

                display.setCursor(50, 25);
                display.setTextSize(2);
                display.print("FAULT");

                break;
            }

            else if (counter2 == 0)
            {
                display.setCursor(5, 25);
                display.setTextSize(2);
                display.print("OUT");
                display.print(":");

                display.setCursor(50, 25);
                display.setTextSize(2);
                display.print(wattage2);
                display.print("W   ");
                break;
            }

        case 4:
            if (wattagecounter == 1)
            {

                display.setCursor(5, 25);
                display.setTextSize(2);
                display.print("OVERLOAD!!");
                display.print(":");
                break;
            }

        case 5:

            display.setCursor(50, 25);
            display.setTextSize(2);
            display.print("OFF");

            break;

            ////////////////////////////////////////////////////////////////////////////////
        }
        display.setTextSize(1);
        display.setCursor(5, 45);
        display.println("MODE");

        display.setCursor(45, 45);
        display.println("BAT:");

        display.setCursor(45, 55);
        display.println("GEN:");

        if (operatingMode == 2) //manual mode
        {
            display.setTextSize(1);
            display.setCursor(0, 55);
            display.println("MANUAL");
        }

        else if (operatingMode == 1) //auto mode
        {
            display.setTextSize(1);
            display.setCursor(0, 55);
            display.println("AUTO.");
        }

        batpercent = batlvl();
        // Serial.println(batpercent);
        batpercent = (batpercent - 9.0) / (12.0 - 9.0) * 100;
        if (batpercent < 0)
        {
            batpercent = 0;
        }
        if (batpercent > 100)
        {
            batpercent = 100;
        }

        display.setTextSize(1);
        display.setCursor(70, 45);
        display.print(batpercent, 0);
        display.print("%");

        genpercent = genv;
        genpercent = (genpercent - 2.8) / (3.28 - 2.8) * 100;
        if (genpercent < 0)
        {
            genpercent = 0;
        }
        if (genpercent > 100)
        {
            genpercent = 100;
        }

        display.setCursor(70, 55);
        display.print(genpercent, 0);
        display.print("%");
    }

    /////////////////////////////////////////////////////////////////////////////
    if (modem.isNetworkConnected())
    {

        display.setCursor(95, 55);
        display.print(modem.getSignalQuality());
        display.print("db");
    }

    if (internetMode == 1)
    {
        if (!mqtt.connected() && modem.isGprsConnected())
        {
            display.setTextColor(BLACK, WHITE);
            display.setCursor(100, 45);
            display.print(" G ");
            display.setTextColor(WHITE, BLACK);
        }
        if (mqtt.connected() && modem.isGprsConnected())
        {
            display.setTextColor(BLACK, WHITE);
            display.setCursor(100, 45);
            display.print(" G+ ");
            display.setTextColor(WHITE, BLACK);
        }
    }
    else
    {
        display.setCursor(100, 45);
        display.print(" NC");
    }

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
    Mode = EEPROM.read(0);
    operatingMode = EEPROM.read(2);
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
    // Serial.println(input_voltage);
    return input_voltage;
}

void batLow()
{
    bat = batlvl();
    // Serial.println(bat);
    if (bat < 11.5)
    {

        ////////////////////////////
        if (operatingMode > 10) //for first time use (Empty Eeprom)
        {
            operatingMode = 2; //Set operating mode==2
        }
        // Serial.println(bat);
        // operatingMode

        if (operatingMode != setOperatingMode) //check if mode(auto or manual) has changed
        {
            Serial.println("save");
            EEPROM.write(2, operatingMode);
            setOperatingMode = operatingMode; //make the var set operating mode equal to the current mode
        }

        /////////////////////////

        if (Mode != SetMode) // Check if out (mains,inv or gen) has changed
        {
            EEPROM.write(0, Mode);
            SetMode = Mode; //make the var set operating mode equal to the current mode
        }
    }
}

void output(byte Outsignal)
{
    if (Outsignal == 4)
    {
        digitalWrite(invMosfet, LOW);
        digitalWrite(genMosfet, LOW);
        digitalWrite(nepaMosfet, LOW);
        delay(200);
    }

    else if (Outsignal == 1)
    {

        digitalWrite(invMosfet, LOW);
        digitalWrite(genMosfet, LOW);
        delay(200);
        digitalWrite(nepaMosfet, HIGH);
    }

    else if (Outsignal == 2)
    {
        digitalWrite(genMosfet, LOW);
        digitalWrite(nepaMosfet, LOW);
        delay(200);
        digitalWrite(invMosfet, HIGH);
    }

    else if (Outsignal == 3)
    {
        digitalWrite(invMosfet, LOW);
        digitalWrite(nepaMosfet, LOW);
        delay(200);
        digitalWrite(genMosfet, HIGH);
    }
}

//Setup function Start
void setup()
{
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(2), PRESS, FALLING);
    attachInterrupt(digitalPinToInterrupt(3), PRESSES, FALLING);
    pinMode(nepaInput, INPUT);
    pinMode(inverterInput, INPUT);
    pinMode(genInput, INPUT);
    //pinMode(modeBtn, INPUT_PULLUP);
    pinMode(toggler, INPUT_PULLUP);

    pinMode(invMosfet, OUTPUT);
    pinMode(nepaMosfet, OUTPUT);
    pinMode(genMosfet, OUTPUT);
    //disable all relays
    digitalWrite(invMosfet, LOW);
    digitalWrite(nepaMosfet, LOW);
    digitalWrite(genMosfet, LOW);

    Serial.begin(115200);
    Serial2.begin(9600);
    SerialAT.begin(9600);
    //analogReference(INTERNAL);

    //////////////////////////Initialize Oled screen
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    display.setTextColor(SSD1306_WHITE);
    display.display();
    oledState = true;
    /////////////////////////////////////////

    modem.restart();

    Serial.println(("Signal Quality: " + String(modem.getSignalQuality())));
    mqtt.setServer(broker, 1883);
    mqtt.setCallback(mqttCallback);

    emon1.current(6, 48.0);

    lastStateCheck();
}

//Loop Function Start Here
void loop()
{
    ch();
    //sendEsp();
    batLow();
    refreshDisplay();
    overloadcheck();
    //    opMode();
    if (Serial2.available())
    {
        delay(10);
        String message = "";
        while (Serial2.available())
            message += (char)Serial2.read();

        if (message.indexOf("Bytes received=") >= 0)
        {
            int x = message.indexOf('=');
            int y = message.indexOf(",");
            String Read = message.substring(x + 1, y);

            genv = Read.toFloat();
            Serial.println(genv);
        }
    }
    ch();
    if (mqtt.connected() && internetMode == 1)
    {
        mqtt.loop();
        StaticJsonDocument<256> doc;
        doc["Main"] = (AcVoltOut0);
        doc["Inv"] = (AcVoltOut1);
        doc["Gen"] = (AcVoltOut2);
        doc["Curr"] = (Irms);
        doc["Mode"] = (Mode);
        doc["OpMode"] = (operatingMode);
        doc["BatV"] = (batpercent);
        doc["GenV"] = (genpercent);

        //{"main":"value","Inv":"value","Gen":"value","Curr":"value", "Mode":"value", "OpMode":"value","BatV":"value","GenV":"value"}
        char out[128];
        int b = serializeJson(doc, out);
        //Serial.print("publishing bytes = ");
        //Serial.println(b, DEC);
        boolean rc = mqtt.publish(topicIn, out);
        Serial.println(internetMode);
    }
    ch();
    //Reconnect gsm
    if (!modem.isNetworkConnected() && internetMode == 1)
    {
        Serial.println(("Network disconnected"));
        if (!modem.waitForNetwork(1000L, true))
        {
            count++;
            if (count > 15)
            {
                count = 0;
                Serial.println("rst");
                modem.restart(7);
            }
        }
    }
    ch();
    if (internetMode == 1 && modem.isNetworkConnected() && modem.getSignalQuality() >= 16 && !modem.isGprsConnected())
    {
        ch();
        Serial.println("nc");
        display.clearDisplay();

        display.setTextSize(1);
        display.setCursor(35, 15);
        display.print("CONNECTING");
        display.setCursor(45, 25);
        display.print("TO THE ");
        display.setCursor(40, 35);
        display.print("INTERNET");

        display.display();
        if (!modem.gprsConnect(apn, user, pass))
        {
        }
    }
    ch();

    //Reconnect mqtt
    if (internetMode == 1 && modem.isGprsConnected() && modem.getSignalQuality() >= 16 && !mqtt.connected())
    {
        ch();
        Serial.println("!mq");
        display.clearDisplay();
        display.setCursor(15, 25);
        display.setTextSize(1);
        display.print("CONTACTING SERVER");

        display.display();
        count++;
        Serial.println(count);
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
    ch();
    // Serial.println(Mode);
    //Serial.println(operatingMode);
    Irms = emon1.calcIrms(1480);
    wattage0 = Irms * AcVoltOut0;
    wattage1 = Irms * AcVoltOut1;
    wattage2 = Irms * AcVoltOut2;

    //Serial.println("w done");

    oledState = true;
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
    AcVoltOut2 = (analogRead(genInput) * (4.9 / 1023)) * 570.67; // Convert the AC divison into volt

    if (AcVoltOut2 >= 140)
    {
        Genout = 1;
    }
    else
    {
        Genout = 0;
    }
    ch();
    Serial.println(AcVoltOut2);
    if (operatingMode == 2) //manual mode
    {
        refreshDisplay();
        manualMode();
    }

    else if (operatingMode == 1) //auto mode
    {
        refreshDisplay();
        autoMode();
    }

    if (phoneout != mqttSetMode)
    {
        Mode = phoneout;
        mqttSetMode = phoneout;
    }
    if (Mode != SetMode)
    {
        switch (Mode)
        {

        case 1: //Switch to Nepa
            NepaFaultCheck();
            if (counter0 == 0)
            {
                output(1);

                break;
            }
        case 2: //Switch to Inv
            InvFaultCheck();
            if (counter1 == 0)
            {

                output(2);

                break;
            }
        case 3: //Switch to Gen
            GenFaultCheck();
            sendEsp("genOn");

            if (counter2 == 0)
            {
                output(3);
                break;
            }
        case 4: //Switch off
            output(4);
            break;
        }
        SetMode = Mode;
    }

    ch();
}

void PRESSES()
{
    static unsigned long lastIntTime1 = 0;
    unsigned long intTime1 = millis();

    if (intTime1 - lastIntTime1 > 200 && digitalRead(8) == LOW)
    {
        Serial.println("ggf");
        chm = 1;
        switch (internetMode)
        {

        case 0:

            internetMode = 1;
            break;
        case 1:
            internetMode = 0;
            break;
        }
    }
    lastIntTime1 = intTime1;
}

void PRESS()
{
    static unsigned long lastIntTime = 0;
    unsigned long intTime = millis();
    if (intTime - lastIntTime > 200 && digitalRead(8) == LOW)
    {
        switch (operatingMode)
        {

        case 1:

            operatingMode = 2;

            break;
        case 2:
            operatingMode = 1;

            break;
        }
        Serial.println("ggf");
    }
    lastIntTime = intTime;
}
void ch()
{
    if (chm == 1)
    {
        if (internetMode == 1)
        {
            display.clearDisplay();
            display.setCursor(5, 25);
            display.setTextSize(2);
            display.print("ONLINE");

            display.display();
        }
        else if (internetMode == 0)
        {
            display.clearDisplay();
            display.setCursor(5, 25);
            display.setTextSize(2);
            display.print("OFFLINE");

            display.display();
        }
        delay(3000);
        chm = 0;
    }
}