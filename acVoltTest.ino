int nepaInput = A0; 
int inverterInput = A1; 
int genInput = A7;
void setup() {
  pinMode(nepaInput, INPUT);
  pinMode(inverterInput, INPUT);
  pinMode(genInput, INPUT);
  Serial.begin(9600);
}

void loop() 
{ 

   int vrout0 = analogRead(nepaInput); //Read the division at analog pin A0 (0 to 1023)
   int AcVoltOut0 = (vrout0 * (5.0 / 1023))*93.67; // Convert the AC divison into volt
   Serial.println(AcVoltOut0); // Display the AC voltage on serial monitor

 int vrout1 = analogRead(inverterInput); //Read the division at analog pin A0 (0 to 1023)
   int AcVoltOut1 = (vrout1 * (5.0 / 1023))*93.67; // Convert the AC divison into volt
   Serial.println(AcVoltOut1); // Display the AC voltage on serial monitor
   
    int vrout2 = analogRead(genInput); //Read the division at analog pin A0 (0 to 1023)
   int AcVoltOut2 = (vrout2 * (5.0 / 1023))*93.67; // Convert the AC divison into volt
   Serial.println(AcVoltOut2); // Display the AC voltage on serial monitor
   


   delay(500);
    }