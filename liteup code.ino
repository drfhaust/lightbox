#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//#include <Fonts/FreeMono9pt7b.h  >

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


const int numReadings = 20;


int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
int inputPin = A0;



float input_voltage = 0.0;
float temp=0.0;
float r1=100000.0;
float r2=5120.0;


bool oledState=false;
bool outOne;
bool outTwo;
bool outThree;

String state1 = "Off";
String state2 = "Off";
String state3 = "Off";
String state4 = "Off";
int count=0;
int countz=0;

int coun=0;





// the setup function runs once when you press reset or power the board
void setup() {
// analogReference(DEFAULT);
//analogReference(INTERNAL);
  Serial.begin(9600);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(2,INPUT_PULLUP);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
   pinMode(7, OUTPUT); 
    pinMode(8, OUTPUT);
    pinMode(9, OUTPUT);
   pinMode(10, OUTPUT);
    digitalWrite(4, LOW); 
   digitalWrite(5, LOW);  
   digitalWrite(6, LOW);  
   digitalWrite(7, LOW);
    digitalWrite(8, LOW);  
   digitalWrite(9, LOW);  
   digitalWrite(10, LOW);    

   if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
   display.clearDisplay();



//display.setFont(&FreeMono9pt7b);

  display.clearDisplay();
oledState=true;
  // put your setup code here, to run once:
 for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

}

// the loop function runs over and over again forever
void loop() {

count++;
  unsigned int x=0;
float AcsValue=0.0,Samples=0.0,AvgAcs=0.0,AcsValueF=0.0;

  for (int x = 0; x < 50; x++){ //Get 150 samples
 
       delay (1); // let ADC settle before next sample 3ms
  AcsValue = analogRead(A2);     //Read current sensor values   

  Samples = Samples + AcsValue;  //Add samples together
  
 }


AvgAcs=Samples/50.0;//Taking Average of Samples

//((AvgAcs * (5.0 / 1024.0)) is converitng the read voltage in 0-5 volts
//2.5 is offset(I assumed that arduino is working on 5v so the viout at no current comes
//out to be 2.5 which is out offset. If your arduino is working on different voltage than 
//you must change the offset according to the input voltage)
//0.185v(185mV) is rise in output voltage when 1A current flows at input
AcsValueF = (2.6 - (AvgAcs * (5.2 / 1023.0)) )/0.066;

//Serial.println(abs(AcsValueF));//Print the read current on Serial monitor
//delay(50);
    float currs=abs(AcsValueF);
if (currs<0.8){
  currs=0;
}

    

    
  if(digitalRead(2)==1){ 
   digitalWrite(4, HIGH);
   
   if (coun<1 ){
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    Serial.println(input_voltage);
     oledState=true;
  }
  coun++;
  }
  
  else{
       digitalWrite(4, LOW);
       oledState=false;
       coun=0;
  }
  
  
     total = total - readings[readIndex];
  // read from the sensor:
  
  readings[readIndex] = analogRead(A0);
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits



 
        temp = ( average * 5.4567) / 1023.0; 
   // temp = ( analogRead(A0) * 1.1) / 1023.0; 
   input_voltage = temp / (r2/(r1+r2));
   
//Serial.println(input_voltage);
//float poww= currs * input_voltage;
 unsigned int y=0;
float Sample=0.0,Avgs=0.0, poww=0.0;

  for (int y = 0; x < 50; x++){ //Get 150 samples
 
       delay (2); // let ADC settle before next sample 3ms
 poww= currs * input_voltage;  

  Sample = Sample + poww;  //Add samples together
  
 // delay (3); // let ADC settle before next sample 3ms
}


Avgs=Sample/50.0;//Taking Average of Samples




if (countz>100){
 display.begin(SSD1306_SWITCHCAPVCC, 0x3C) ;
  

countz=0;
//display.setFont(&FreeMono9pt7b);

 // display.clearDisplay();
}

countz++;
//Serial.println(Avgs);//Print the read current on Serial monitor
Serial.println(coun);

  input_voltage=(input_voltage-9.0)/(12.2 - 9.0) * 100;
 if(input_voltage<0){
  input_voltage=0;
 }
 if(input_voltage>100){
  input_voltage=100;
 }
 
if(oledState==true){
 //Serial.println(voltage);
 // display.setFont(&FreeMono9pt7b);

  // Display static text
 if (count>5){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(75, 5);
   display.print(input_voltage,0);
   display.println("%");
   

   
   display.setTextSize(5);
  display.setCursor(20, 25);
  display.print(poww,0);
  display.setTextSize(3);
   display.print("w");
  count=0;}
  display.display(); 
 /*  display.setTextColor(WHITE);
  display.setCursor(0, 44);
  

  
  display.setCursor(5, 10);
   display.print("Light 1: ");
   display.println(state1);
    
   display.setCursor(5, 21);
   display.print("Light 2: ");
   display.println(state2);
    
   display.setCursor(5, 32);
   display.print("Light 3: ");
   display.println(state3);
  display.display(); 
  delay(10);
*/
}
else{
  display.clearDisplay();
   display.display();
}





    
  
                       // wait for a second


 int sensorValue = analogRead(A3);
  // Serial.println(count);
   if (sensorValue<=30){

    if (state1 == "Off"){
       digitalWrite(7, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(10, HIGH);                     // wait for a second
    state1 = "On";
    }
   
     
  else if(state1 == "On") {
    digitalWrite(7, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(10, LOW);
  state1 = "Off" ;
  }
  }
   if (sensorValue>50 && sensorValue<120){

 if (state2 == "Off"){
       digitalWrite(6, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(9, HIGH);                     // wait for a second
    state2 = "On";
    }
   
     
  else if(state2 == "On") {
    digitalWrite(6, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(9, LOW);
  state2 = "Off" ;
  }
  }
   if (sensorValue>140 && sensorValue<300){
 if (state3 == "Off"){
       digitalWrite(5, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(8, HIGH);                     // wait for a second
    state3 = "On";
    }
   
     
  else if(state3 == "On") {
    digitalWrite(5, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(8, LOW);
  state3 = "Off" ;
  }
}
}