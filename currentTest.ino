#include "EmonLib.h" // Include Emon Library
EnergyMonitor emon1; // Create an instance
int currPin=6;
void setup()
{
    Serial.begin(9600);
     emon1.current(currPin, 60.6);   // Current: input pin, calibration.
}

void loop()
{double Irms=emon1.calcIrms(1484)
  Serial.Println(irms*230.0)
}