#define TINY_GSM_MODEM_SIM800

#include <SoftwareSerial.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>

String voltStr;
char volt[50];

int count = 0;
SoftwareSerial SerialAT(9, 8); // RX, TX
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
    Serial.print(".");
    return false;
  }
  Serial.println("Connected to broker.");
  mqtt.subscribe(topicIn);
  return mqtt.connected();
}

void mqttCallback(char *topic, byte *payload, unsigned int len)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.write(payload, len);
  Serial.println();

  /*Only proceed if incoming message's topic matches
  if (String(topic) == topicLed) {
    ledStatus = !ledStatus;
    digitalWrite(LED_PIN, ledStatus);
    mqtt.publish(topicLedStatus, ledStatus ? "1" : "0");
  }
  */
}

void setup()
{
  Serial.begin(9600);
  SerialAT.begin(9600);
  analogReference(INTERNAL);
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
  Serial.println("Connected to telco.");
  Serial.println("Signal Quality: " + String(modem.getSignalQuality()));

  Serial.println("Connecting to GPRS network.");
  if (!modem.gprsConnect(apn, user, pass))
  {
    Serial.println("fail");
    while (true)
      ;
  }
  Serial.println("Connected to GPRS: " + String(apn));

  mqtt.setServer(broker, 1883);
  mqtt.setCallback(mqttCallback);
  Serial.println("Connecting to MQTT Broker: " + String(broker));
  while (mqttConnect() == false)
    continue;
  Serial.println();
}

void loop()
{

  if (Serial.available())
  {
    delay(10);
    String message = "";
    while (Serial.available())
      message += (char)Serial.read();
    mqttConnect();
    mqtt.publish(topicOut, volt);
  }

  if (mqtt.connected())
  {
    mqtt.loop();
  }
  //Reconnect gsm
  if (!modem.isNetworkConnected())
  {
    Serial.println("Network disconnected");
    if (!modem.waitForNetwork(10000L, true))
    {
      Serial.println(" fail");
      //delay(10000);
      count++;
      if (count > 5)
      {
        count = 0;
        Serial.println("restart");
        modem.restart(7);
      }
    }
  }

  if (!modem.isGprsConnected())
  {
    ;
    if (!modem.gprsConnect(apn, user, pass))
    {
      Serial.println(" fail");
      delay(5000);
      return;
    }
    if (modem.isGprsConnected())
    {
      Serial.println("GPRS ");
    }
  }

  //Reconnect mqtt
  if (!mqtt.connected())
  {
    Serial.println("=== MQTT NOT CONNECTED ===");
    // Reconnect every 10 seconds
    uint32_t t = millis();
    if (t - lastReconnectAttempt > 10000L)
    {
      lastReconnectAttempt = t;
      count++;
      if (count > 10)
      {
        count = 0;
        Serial.println("restart");
        modem.restart(7);
      }
      if (!mqttConnect())
      {
        lastReconnectAttempt = 0;
      }
    }
    delay(100);
    return;
  }
  // mqttConnect();
  int voltage1 = analogRead(A0) * (226 / 208);
  voltStr = String(voltage1);
  voltStr.toCharArray(volt, voltStr.length() + 1);
  //mqttConnect() ;
  mqtt.publish("voltage/1", volt);
  Serial.println(volt);
  delay(500);
  int voltage2 = analogRead(A1) * (226 / 208);
  voltStr = String(voltage2);
  voltStr.toCharArray(volt, voltStr.length() + 1);
  // mqttConnect();
  mqtt.publish("voltage/2", volt);
  Serial.println(volt);
  delay(500);
  int voltage3 = analogRead(A7) * (226 / 208);
  voltStr = String(voltage3);
  voltStr.toCharArray(volt, voltStr.length() + 1);
  //mqttConnect();
  mqtt.publish("voltage/3", volt);
  Serial.println(volt);
  delay(500);

  //delay(1000);
}