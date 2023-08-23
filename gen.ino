#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Servo.h>

Servo servo;

byte batSense = A0;
byte servoPort = 2;
byte genRelay2 = 4;
byte genRelay1 = 5;
byte acSense = 12;
byte manualButton = 13;
byte powerSwitchRelay = 14;
bool stat = false;
byte retry = 0;
const int numReadings = 20;
int flag = 0;
unsigned long initTime; //some global variables available anywhere in the program
unsigned long timeNow;
const unsigned long setTime = 10000; //the value is a number of milliseconds

unsigned long inits; //some global variables available anywhere in the program
unsigned long now;
const unsigned long sTime = 3500; //the value is a number of milliseconds
unsigned long startTime; //some global variables available anywhere in the program
unsigned long currentTime;
const unsigned long waitTime = 2000; //the value is a number of milliseconds

int readings[numReadings]; // the readings from the analog input
int readIndex = 0;         // the index of the current reading
int total = 0;             // the running total
int average = 0;           // the average
int inputPin = A0;
float input_voltage = 0.0;
float temp = 0.0;
float r1 = 10000.0;
float r2 = 1060.0;

uint8_t newMACAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x66};

uint8_t broadcastAddress[] = {0x32, 0xAE, 0xA4, 0x07, 0x0D, 0x65};
// Define variables to store DHT readings to be sent
float temperature;
float bat;

// Define variables to store incoming readings
int genStat = 0;
float batLvl;
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
  //Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0)
  {
    Serial.println("Delivery success");
  }
  else
  {
    Serial.println("Delivery fail");
  }
}

// Callback when data is received
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len)
{
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");

  genStat = incomingReadings.genStatus;
  Serial.println(incomingReadings.genStatus);
  //batLvl = incomingReadings.batlvl;
}

float batlvl()
{

  total = total - readings[readIndex];
  readings[readIndex] = analogRead(batSense);
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= numReadings)
  {
    readIndex = 0;
  }
  average = total / numReadings;
  // temp = ( average * 5.4567) / 1023.0;
  temp = (analogRead(A0) * 1) / 1023.0;
  input_voltage = temp / (r2 / (r1 + r2));
  return input_voltage;
}
void batLow()
{


  batlvl();
  if (batlvl() < 3.2)
  {
    digitalWrite(powerSwitchRelay, HIGH);
  }
  else
  {
    digitalWrite(powerSwitchRelay, LOW);
  }
}

bool GenStart( int retry)
{ flag = 1;
  if (retry == 0) {
    servo.write(120); ////OPEN CHOKE
    digitalWrite(genRelay1, HIGH);
    delay(200);
    digitalWrite(genRelay2, HIGH);
    delay(2000);
    digitalWrite(genRelay2, LOW);
    servo.write(0); ////close CHOKE
    delay(3000);
    if (digitalRead(acSense) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else if (retry == 1) {

    servo.write(0); ////OPEN CHOKE
    digitalWrite(genRelay1, HIGH);
    delay(200);
    digitalWrite(genRelay2, HIGH);
    delay(2000);
    digitalWrite(genRelay2, LOW);
    servo.write(0); ////close CHOKE
    delay(3000);
    if (digitalRead(acSense) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else if (retry == 2) {

    servo.write(120); ////OPEN CHOKE
    digitalWrite(genRelay1, HIGH);
    delay(200);
    digitalWrite(genRelay2, HIGH);
    delay(3000);
    digitalWrite(genRelay2, LOW);
    servo.write(0); ////close CHOKE
    delay(3000);
    if (digitalRead(acSense) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }


  }
  else if (retry == 3) {

    servo.write(0); ////OPEN CHOKE
    digitalWrite(genRelay1, HIGH);
    delay(200);
    digitalWrite(genRelay2, HIGH);
    delay(3000);
    digitalWrite(genRelay2, LOW);
    servo.write(0); ////close CHOKE
    delay(3000);
    if (digitalRead(acSense) == 0)
    {
      return true;
    }
    else
    {
      return false;
    }


  }




}

bool GenStop()
{
  
  flag = 0;
  digitalWrite(genRelay1, LOW);

  delay(3000);
  if (digitalRead(acSense) == HIGH)
  {

    return true;
  }
  else
  {
    return false;
  }
}

void setup()
{
  Serial.begin(9600);
  //        esp.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  wifi_set_macaddr(STATION_IF, &newMACAddress[0]);

  // Init ESP-NOW
  if (esp_now_init() != 0)
  {
    Serial.println("Error initializing ESP-NOW");
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

  pinMode(manualButton, INPUT_PULLUP);
  pinMode(acSense, INPUT);
  pinMode(genRelay2, OUTPUT);
  pinMode(genRelay1, OUTPUT);
  pinMode(batSense, INPUT);
  pinMode(powerSwitchRelay, OUTPUT);
  digitalWrite(powerSwitchRelay, HIGH);

  servo.attach(servoPort); //D4

  servo.write(0);
  //   servo.detach(servoPort);
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
  {
    readings[thisReading] = 0;
  }
}

void loop()
{
  timeNow = millis(); //get the current "time" (actually the number of milliseconds since the program started)
  now = millis();     //get the current "time" (actually the number of milliseconds since the program started)



  if (digitalRead(manualButton) == LOW)
  {
    //Serial.print("in");
    startTime = millis(); //initial start time
    while (digitalRead(manualButton) == LOW)
    {
      currentTime = millis();
      delay(1);
      if (currentTime - startTime >= waitTime) //test whether the period has elapsed
      {
        if (flag == 0 && digitalRead(acSense) == HIGH ) //if gen is off
        {
          GenStart(0);

        }
        else if (flag == 1 && digitalRead(acSense) == LOW) //if gen is on
        {
          GenStop();
          flag = 0;
        }
        else if (flag == 1 && digitalRead(acSense) == HIGH ) //if gen is off
        {
          GenStart(0);

        }
        else if (flag == 0 && digitalRead(acSense) == LOW ) //if gen is off
        {
          GenStop();

        }


        startTime = millis();
      }
    }
  }












  //wait for elapsed time
  if (timeNow - initTime >= setTime) //test whether the period has elapsed

  {
    if (genStat == 1 && retry < 4 && stat == 0) // if genstat variable is ==1 and retry hasnt timed out
    {
      if (digitalRead(acSense) == HIGH)
      {
        if (GenStart(retry) == true)
        {
          retry = 0;
          stat = 1;
        }
        else
        {
          stat = 0;
          retry++;
        }
      }
      else {
        retry = 0;
        stat = 1;
      }
    }

    initTime = timeNow;
  }

  if (genStat == 0.0) // if genstat variable is ==0
  {
    if (digitalRead(acSense) == LOW) // if gen is on
    {
      GenStop();
    }


  }

  if (digitalRead(acSense) == HIGH) // if gen is off set the stat vatiable to 0
  {
    stat = 0;
  }
  else
  {
    stat = 1;
  }

  //Set values to send
  valuesToSend.genStatus = stat;
  valuesToSend.batlvl = batlvl();
  Serial.println(digitalRead(acSense));


  if (now - inits >= sTime) //test whether the period has elapsed

  {

    esp_now_send(broadcastAddress, (uint8_t *)&valuesToSend, sizeof(valuesToSend));
    inits = now;
  }
}
