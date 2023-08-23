#include <ESP8266WiFi.h>

#include <ESP8266WiFi.h>
#include <espnow.h>

uint8_t newMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x65};

String message = "";
//String message = "";
uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x66};

unsigned long initTime;  //some global variables available anywhere in the program
unsigned long inits = 0; //some global variables available anywhere in the program
unsigned long timeNow;
const unsigned long sTime = 3500; //the value is a number of milliseconds

// Define variables to store incoming readings
int gen = 0;
float bat = 0;
// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message
{
  int genStatus; //// send 1 0r zero
  float batlvl;  ///0-100
} struct_message;

// Create a struct_message called DHTReadings to hold sensor readings
struct_message valuesToSend;

// Create a struct_message to hold incoming sensor readings
struct_message incomingReadings;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus)
{
  // Serial.print(("Last Packet Send Status: "));
  if (sendStatus == 0)
  {
    //Serial.println(("Delivery success"));
  }
  else
  {
    //Serial.println(("Delivery fail"));
  }
}

// Callback when data is received
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));

  //  Serial.println(F(len));
  gen = incomingReadings.genStatus;
  bat = incomingReadings.batlvl;
  Serial.print(("Bytes received="));
  Serial.print(bat);
  Serial.println(",");
}

void setup()
{
  Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  wifi_set_macaddr(STATION_IF, &newMACAddress[0]);

  // Init ESP-NOW
  if (esp_now_init() != 0)
  {
    //Serial.println(F("Error initializing ESP-NOW"));
    return;
  }

  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);

  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}

void loop()
{
  timeNow = millis(); //get the current "time" (actually the number of milliseconds since the program started)

  if (Serial.available())
  {
    delay(10);

    while (Serial.available())
    {
      message += (char)Serial.read();
    }

    if (message.indexOf("1") != -1)
    {
      valuesToSend.genStatus = 1;
      valuesToSend.batlvl = 1;

      //Serial.println("ok");
    }
    else
    {
      valuesToSend.genStatus = 0;
      valuesToSend.batlvl = 0;
    }
  }

  if (timeNow - inits >= sTime) //test whether the period has elapsed

  {

    esp_now_send(broadcastAddress, (uint8_t *)&valuesToSend, sizeof(valuesToSend));
    inits = timeNow;
  }

  // Serial.flush();
}