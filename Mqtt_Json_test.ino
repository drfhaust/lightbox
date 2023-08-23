#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 61);
const char* server = "192.168.1.41";

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);
///
//receive data
///
  void callback(char* topic, byte* payload, unsigned int length) {

   char str[length+1];
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  int i=0;
  for (i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    str[i]=(char)payload[i];

  }
 str[i] = 0; // Null termination
Serial.println();
  //practise string
  //char json[] = "{\"sensor\":\"gps\",\"time\":1351824120,\"data\":[48.756080,2.302038]}";
  
  StaticJsonDocument <256> doc;
  deserializeJson(doc,payload);


// deserializeJson(doc,str); can use string instead of payload
const char* sensor = doc["sensor"];
long time          = doc["time"];
float latitude    = doc["data"][0];
float longitude   = doc["data"][1];


Serial.print("time =");
Serial.println(time,2);
Serial.print("sensor =");
Serial.println(sensor,2);
Serial.print("latitude =");
Serial.println(latitude,2);
Serial.print("longitude =");
Serial.println(longitude,2);
Serial.println(sensor);

}

void setup()
{
    // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
  mqttClient.setServer(server, 1883);
  mqttClient.setCallback(callback);



  if (mqttClient.connect("arduino-1")) {
    // connection succeeded
    Serial.println("Connected ");
    boolean r= mqttClient.subscribe("arduino-test");
    Serial.println("subscribe ");
    Serial.println(r);
  } 
  else {
    // connection failed
    // mqttClient.state() will provide more information
    // on why it failed.
    Serial.println("Connection failed ");
  }

}

  void loop()
  {
    //




StaticJsonDocument<256> doc;
  doc["sensor"] = "gps";
  doc["time"] = 1351824120;

  // Add an array.
  //
 JsonArray data = doc.createNestedArray("data");
  data.add(48.756080);
  data.add(2.302038);
//doc["data"]=data;
  // Generate the minified JSON and send it to the Serial port.
  //
  char out[128];
int b =serializeJson(doc, out);
  Serial.print("publishing bytes = ");
Serial.println(b,DEC);
  boolean rc = mqttClient.publish("arduino-test", out);
  // The above line prints:
  // {"sensor":"gps","time":1351824120,"data":[48.756080,2.302038]}
delay(5000);
    mqttClient.loop();
  }