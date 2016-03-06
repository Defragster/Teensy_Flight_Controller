#include <ADC.h>

ADC *adc = new ADC(); // adc object;

const uint8_t ledPin1 = 27; // GR
const uint8_t ledPin2 = 28; // BL
const uint8_t ledPin3 = 29; // RD
const uint8_t VbatPin = A1;

uint16_t VBAT;
uint16_t arrVBAT[8];
uint16_t iiVBAT = 0;
uint16_t arrVBAT2[100]; // Store values for recall across USB drop
uint16_t iiVBAT2 = 0;
uint8_t ledRGB[3] = { ledPin1, ledPin2, ledPin3};

enum ColorND { white, purple, orange, red, teal, blue, green, black };
void showColor( int16_t sCol )
{ // sCol == enum ColorND
  digitalWrite(ledRGB[ 0 ], (sCol & 1));  // Given sCol these three lines make enumerated color for function
  digitalWrite(ledRGB[ 1 ], (sCol & 2));
  digitalWrite(ledRGB[ 2 ], (sCol & 4));
}

void setup()
{
  Serial.begin(9600);
  for ( int jj = 0; jj < 3; jj++ ) pinMode(ledRGB[ jj ], OUTPUT);
  pinMode(VbatPin, INPUT); //pin 15 (A1) single ended
  showColor( red ); delay(500);  showColor( white );  delay(500);  showColor( blue );  delay(500);
  showColor( red ); delay(500);  showColor( white );  delay(500);  showColor( blue );  delay(500);
  ///// ADC0 ////
  // reference can be ADC_REF_3V3, ADC_REF_1V2 (not for Teensy LC) or ADC_REF_EXT.
  adc->setReference(ADC_REF_3V3, ADC_0); // change all 3.3 to 1.2 if you change the reference to 1V2

  adc->setAveraging(1); // set number of averages
  adc->setResolution(12); // set bits of resolution

  // it can be ADC_VERY_LOW_SPEED, ADC_LOW_SPEED, ADC_MED_SPEED, ADC_HIGH_SPEED_16BITS, ADC_HIGH_SPEED or ADC_VERY_HIGH_SPEED
  // see the documentation for more information
  adc->setConversionSpeed(ADC_LOW_SPEED); // change the conversion speed
  // it can be ADC_VERY_LOW_SPEED, ADC_LOW_SPEED, ADC_MED_SPEED, ADC_HIGH_SPEED or ADC_VERY_HIGH_SPEED
  adc->setSamplingSpeed(ADC_LOW_SPEED); // change the sampling speed
}

#define S_TIME  200
#define L_TIME  1000

elapsedMillis busyTime;
elapsedMillis TimeADC;
uint32_t busyWait = L_TIME;
int ii = 0;
void loop()
{
  if ( busyTime > busyWait ) {
    if ( ii >= 8 ) ii = 0;
    busyTime = 0;
    showColor(ii++);
    if ( 8 == ii ) {
      if ( L_TIME == busyWait ) busyWait = S_TIME; else busyWait = L_TIME;
    }
  }

  if ( TimeADC > 120 ) {
    TimeADC = 0;
    arrVBAT[ iiVBAT ] = adc->analogRead(VbatPin); // read a new value, will return ADC_ERROR_VALUE if the comparison is false.
    if ( 0 != arrVBAT[ iiVBAT ] ) iiVBAT++;
    if (iiVBAT >= 8) {
      VBAT = 0;
      for ( int jj = 0; jj < 8; jj++ ) VBAT += arrVBAT[jj];
      VBAT /= 4;  // 8 Samples - remove 2.* factor below
      Serial.print("VBAT = "); Serial.print(VBAT * 3.3 / adc->getMaxValue(ADC_0), 3); Serial.println(" V");
      iiVBAT = 0;
      if ( iiVBAT2 < 100 ) arrVBAT2[iiVBAT2++] = VBAT;
    }
  }
}


int incomingByte;
void serialEvent() { //
  while (Serial.available()) {
    incomingByte = Serial.read();
    while ( iiVBAT2 > 0 ) Serial.println( arrVBAT2[--iiVBAT2] * 3.3 / adc->getMaxValue(ADC_0), 3);
    Serial.write( (char)incomingByte );
  }
}

