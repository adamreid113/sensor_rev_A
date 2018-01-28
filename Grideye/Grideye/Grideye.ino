#include <Wire.h>

//0x69 - grideye
//0x0E - grideye initialize
#define pirpin 26
#define pir_activation 500
#define MAXTIMINGS  85
#define DHTPIN    25
#define ledcheck 32

bool pir = 0;
int tempsum = 0;
float correctedtmp;
float h;
float c;
float tempprev;
float prob_correction = 460;
float prob_base = 280;
int detectioncnt;


int dht11_dat[5] = { 0, 0, 0, 0, 0 };

void temp102(int tmpaddress){
byte first, second;
int tempbytes;
float convertedtmp;
float correctedtmp;

Wire.beginTransmission(tmpaddress); //Say hi to the sensor.
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(tmpaddress, 2);
  Wire.endTransmission();

first = (Wire.read());
second = (Wire.read());

tempbytes = ((first) << 4);  
tempbytes |= (second >> 4); 

convertedtmp = tempbytes * 0.0625;
correctedtmp = convertedtmp - 5;
};


void i2ccomms(int address,int start){
Wire.beginTransmission(address);
Wire.write(start);  // address low byte
Wire.endTransmission();
Wire.requestFrom(0x69, 1);
}

void read_dht11_dat()
{
  uint8_t laststate = HIGH;
  uint8_t counter   = 0;
  uint8_t j   = 0, i;
  float f; 
 
  dht11_dat[0] = dht11_dat[1] = dht11_dat[2] = dht11_dat[3] = dht11_dat[4] = 0;
 
  pinMode( DHTPIN, OUTPUT );
  digitalWrite( DHTPIN, LOW );
  delay( 18 );
  digitalWrite( DHTPIN, HIGH );
  delayMicroseconds( 40 );
  pinMode( DHTPIN, INPUT );
 
  for ( i = 0; i < MAXTIMINGS; i++ )
  {
    counter = 0;
    while ( digitalRead( DHTPIN ) == laststate )
    {
      counter++;
      delayMicroseconds( 1 );
      if ( counter == 255 )
      {
        break;
      }
    }
    laststate = digitalRead( DHTPIN );
 
    if ( counter == 255 )
      break;
 
    if ( (i >= 4) && (i % 2 == 0) )
    {
      dht11_dat[j / 8] <<= 1;
      if ( counter > 16 )
        dht11_dat[j / 8] |= 1;
      j++;
    }
  }
 
  if ( (j >= 40) &&
       (dht11_dat[4] == ( (dht11_dat[0] + dht11_dat[1] + dht11_dat[2] + dht11_dat[3]) & 0xFF) ) )
  {
    

  }

}

float pixels[8][8] ={
  {11, 12, 13, 14, 15, 16, 17, 18},
  {21, 22, 23, 24, 25, 26, 27, 28},
  {31, 32, 33, 34, 35, 36, 37, 38},
  {41, 42, 43, 44, 45, 46, 47, 48},
  {51, 52, 53, 54, 55, 56, 57, 58},
  {61, 62, 63, 64, 65, 66, 67, 68},
  {71, 72, 73, 74, 75, 76, 77, 78},
  {81, 82, 83, 84, 85, 86, 87, 88}

};

float prob_matrix[8][8] ={
  {11, 12, 13, 14, 15, 16, 17, 18},
  {21, 22, 23, 24, 25, 26, 27, 28},
  {31, 32, 33, 34, 35, 36, 37, 38},
  {41, 42, 43, 44, 45, 46, 47, 48},
  {51, 52, 53, 54, 55, 56, 57, 58},
  {61, 62, 63, 64, 65, 66, 67, 68},
  {71, 72, 73, 74, 75, 76, 77, 78},
  {81, 82, 83, 84, 85, 86, 87, 88}

};
int pir_inst=0;

void setup() {

  pinMode (ledcheck, OUTPUT);
  pinMode(pirpin, INPUT);  
  delay(20);
  i2ccomms(0x69, 0x0e);
  delay(20);
  Wire.begin(14,27);
  Serial.begin(115200);

}

void loop() {

  Serial.println();
/*********THERMAL ARRAY**************/
tempsum = 0;
  for (int i=0x00; i<=0x07; i++){
    for (int j=0x00; j<=0x07; j++){
        int pixel = j*2 + i * 0x08 + 0x80;
  i2ccomms(0x69, pixel);
  pixels[i][j] = Wire.read()*.25;

  tempsum = tempsum + pixels[i][j];
 
       }
     }//writes value for each thermal array pixel

/***************TMP******************/
temp102(0x00);


/***************PIR****************/
int pir_inst=analogRead(pirpin);
if (pir_inst > 1000){
  pir = 1;
  }
  else{
    pir = 0;
    }



/*************Hum******************/
read_dht11_dat();
 


/*******TA PROBABILITY FUNCTION*******/
for (int i=0x00; i<=0x07; i++){
    for (int j=0x00; j<=0x07; j++){
      
      float exponent = pixels[i][j]/tempprev;
      
prob_matrix[i][j] = pow(prob_base, exponent) - prob_correction;
 if (prob_matrix[i][j] > 0){
  detectioncnt = detectioncnt + 1;
 }
       }
     }

/*************Serial Print************/

for(int k = 0x00; k < 0x07; k++){
 
  for(int l = 0x00; l < 0x07; l++){
    
  Serial.print(pixels[k][l]);
  Serial.print('\t');
    }

  int l=0;
  Serial.println(pixels[k][l]);
  Serial.println(' ');
    
} 

Serial.println(' ');
for(int k = 0x00; k < 0x07; k++){

  for(int l = 0x00; l < 0x07; l++){
    
  Serial.print(prob_matrix[k][l]);
  Serial.print('\t');
    }

  int l=0;
  Serial.println(prob_matrix[k][l]);
  Serial.println(' ');
    
} 




  Serial.println(tempsum);
 
  Serial.print("pir: ");
  Serial.print(pir);
  Serial.println(' ');
 
  if (dht11_dat[2]>2){
    h = dht11_dat[0]; 
    c = dht11_dat[2];
    tempprev = c;
    /*Serial.print("temp:");
    Serial.println(c);
    Serial.print("hum: ");
    Serial.println(h);*/
  }
  Serial.print("othertemp: ");
  Serial.println(correctedtmp);

  Serial.print("detection number: ");
  Serial.println(detectioncnt);
  if (detectioncnt > 0 ){
    digitalWrite(ledcheck, HIGH);
  }
  else {
    digitalWrite(ledcheck, LOW);
    }
  detectioncnt = 0;
}

