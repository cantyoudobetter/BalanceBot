/* 
 Contact information
 -------------------
 */
//Test change
#include <Wire.h>
#include "Kalman.h" 
#include "ComPacket.h"
#include "I2C.h"
//#include <EEPROM.h>
#include "music.h"
//#include "MyEEprom.h"
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
#include "PinChangeInt.h" // for RC reciver

//#define RESTRICT_PITCH // Comment out to restrict roll to ±90deg instead - please read: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf
//#define USEEEPROM     //uncomment to use eeprom for storing PID parameters 

//#define RCWork true

//Rc receiver   //2 channels
#define UP_DOWN_IN_PIN   16
#define  LEFT_RIGHT_IN_PIN  17
#define  THROTTLE_IN_PIN  14
#define  AUX_IN_PIN 14  //is it use RC ,high level :no
bool RCWork = true;

volatile uint8_t bUpdateFlagsRC = 0;
#define UP_DOWN_FLAG   0b10
#define LEFT_RIGHT_FLAG 0b100
#define RUDDERT_FLAG 0b1000

bool isInMucis = false;
bool isInBuzzer = false;

uint32_t UpDownStart;
uint32_t LeftRightStart;
uint32_t ThrottleStart;

volatile uint16_t UpDownEnd = 0;
volatile uint16_t LeftRightEnd = 0;
volatile uint16_t ThrottleEnd = 0;

int UpDownIn;
int LeftRightIn;

boolean useThrottle = false;
 //Speaker Mode
 #define SPK_OFF  0x00
 #define SPK_ON  0x01
 #define SPK_ALARM 0x02 

Kalman kalmanX; // Create the Kalman instances
Kalman kalmanY;
ComPacket SerialPacket;

/* IMU Data */
double accX, accY, accZ;
double gyroX, gyroY, gyroZ;
int16_t tempRaw;

double gyroXangle, gyroYangle; // Angle calculate using the gyro only
double compAngleX, compAngleY; // Calculated angle using a complementary filter
double kalAngleX, kalAngleY; // Calculated angle using a Kalman filter

uint32_t timer;
uint8_t i2cData[14]; // Buffer for I2C data

#define BUZZER 13
#define LED 13
//motor  define
#define PWM_L 6  //M1
#define PWM_R 5   //M2
#define DIR_L1 8
#define DIR_L2 7
#define DIR_R1 3
#define DIR_R2 4
//encoder define
#define SPD_INT_R 11   //interrupt 
#define SPD_PUL_R 12   
#define SPD_INT_L 10   //interrupt 
#define SPD_PUL_L 9   

int Speed_L,Speed_R;
int Position_AVG;
int Position_AVG_Filter;
int Position_Add;
int pwm,pwm_l,pwm_r;
double Angle_Car;
double Gyro_Car;

double KA_P,KA_D;
double KP_P,KP_I;
double K_Base;
//struct EEpromData SavingData;
//struct EEpromData ReadingData;

int Speed_Need,Turn_Need;
int Speed_Diff,Speed_Diff_ALL;

uint32_t LEDTimer;
bool blinkState = false;

uint32_t calTimer;
uint32_t delayTimer;
uint32_t  lampTimer;
uint32_t  lcdTimer;

byte speakMode;

//Music play
int tonelength;
int tonePin=13;
int tonecnt = 0;
uint32_t  BuzzerTimer;

void setup() {
  Serial.begin(9600);
  lcd.begin();
  lcd.print("B~Bot");
  Init();  
  //mySerial.begin(9600);
  Wire.begin();
  TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz

  i2cData[0] = 7; // Set the sample rate to 1000Hz - 8kHz/(7+1) = 1000Hz
  i2cData[1] = 0x00; // Disable FSYNC and set 260 Hz Acc filtering, 256 Hz Gyro filtering, 8 KHz sampling
  i2cData[2] = 0x00; // Set Gyro Full Scale Range to ±250deg/s
  i2cData[3] = 0x00; // Set Accelerometer Full Scale Range to ±2g

  while (i2cWrite(0x19, i2cData, 4, false)); // Write to all four registers at once
  while (i2cWrite(0x6B, 0x01, true)); // PLL with X axis gyroscope reference and disable sleep mode

  while (i2cRead(0x75, i2cData, 1));
  if (i2cData[0] != 0x68) { // Read "WHO_AM_I" register
    Serial.print(F("Error reading sensor"));
    while (1);
  }

  delay(200); // Wait for sensor to stabilize

  /* Set kalman and gyro starting angle */
  while (i2cRead(0x3B, i2cData, 6));
  accX = (i2cData[0] << 8) | i2cData[1];
  accY = (i2cData[2] << 8) | i2cData[3];
  accZ = (i2cData[4] << 8) | i2cData[5];

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // It is then converted from radians to degrees
#ifdef RESTRICT_PITCH // Eq. 25 and 26
  double roll  = atan2(accY, accZ) * RAD_TO_DEG;
  double pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
#else // Eq. 28 and 29
  double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  double pitch = atan2(-accX, accZ) * RAD_TO_DEG;
#endif

  kalmanX.setAngle(roll); // Set starting angle
  kalmanY.setAngle(pitch);
  gyroXangle = roll;
  gyroYangle = pitch;
  compAngleX = roll;
  compAngleY = pitch;
 
 /* 
  if( analogRead(AUX_IN_PIN) > 1000)
  {
    RCWork = true;
  }
  else
  {
     RCWork = false;
  }
 */
 
  // using the PinChangeInt library, attach the interrupts
  // used to read the channels
  Serial.print("RCWork: "); Serial.println(RCWork);
  if(RCWork == true)
  {
    PCintPort::attachInterrupt(UP_DOWN_IN_PIN, calcUpDown,CHANGE);
    PCintPort::attachInterrupt(LEFT_RIGHT_IN_PIN, calcLeftRight,CHANGE);
    PCintPort::attachInterrupt(THROTTLE_IN_PIN, calcThrottle,CHANGE);
  }
  PCintPort::attachInterrupt(SPD_INT_L, Encoder_L,FALLING);
  PCintPort::attachInterrupt(SPD_INT_R, Encoder_R,FALLING);
  
  timer = micros();
  LEDTimer = millis();
  lampTimer = millis();
  lcdTimer = millis();
 // tonelength = sizeof(tune)/sizeof(tune[0]);
  
 
}

void loop() {

  double DataAvg[3];
  double AngleAvg = 0;

  DataAvg[0]=0; DataAvg[1]=0; DataAvg[2]=0;
       
    while(1)
    { //Serial.println("here");
      if(UpdateAttitude())
      { 
                                    
        DataAvg[2] = DataAvg[1];
        DataAvg[1] = DataAvg[0];
        DataAvg[0] = Angle_Car;
        AngleAvg = (DataAvg[0]+DataAvg[1]+DataAvg[2])/3;
        if(AngleAvg < 40 || AngleAvg > -40){  
          PWM_Calculate();
          Car_Control();
        }
      }      
      //UserComunication();
       ProcessRC(); 
       MusicPlay(); 

       if (millis() - lcdTimer > 1000) {
           lcd.setCursor(7,0);
           if(!useThrottle) 
           {
              double temperature = (double)tempRaw / 340.0 + 36.53;
              lcd.print(temperature);
              lcd.print(char(223)); lcd.print("C  ");
           } else {
              lcd.print("**TURBO**");
           }
           lcd.setCursor(0,1);
           
           char outL[3];
           String str;
           str=String(Speed_L);
           str.toCharArray(outL,3);
           char outR[3];
           str=String(Speed_R);
           str.toCharArray(outR,3);
           char finalRow[16];
           strcpy(finalRow,outL);
           strcat(finalRow,"|");
           strcat(finalRow,outR);
           strcat(finalRow,"    ");
           lcd.print(finalRow);
           lcd.setCursor(10,1);
           lcd.print(pwm);
           
           /*lcd.print("    ");
           lcd.setCursor(8,1);
           lcd.print(Speed_R);  lcd.print("    ");*/
           lcdTimer = millis();
       } 
    }
  

}

bool StopFlag = true;

void PWM_Calculate()
{  
  
  float ftmp = 0;
  ftmp = (Speed_L + Speed_R) * 0.5;
  if( ftmp > 0)
    Position_AVG = ftmp +0.5;  
   else
    Position_AVG = ftmp -0.5;
    
    Speed_Diff = Speed_L - Speed_R;
    Speed_Diff_ALL += Speed_Diff;
    
//  Position_AVG_Filter *= 0.5;		//speed filter
//  Position_AVG_Filter += Position_AVG * 0.5;
  Position_AVG_Filter = Position_AVG;
  
  Position_Add += Position_AVG_Filter;  //position
  //Serial.print(Speed_Need);  Serial.print("\t"); Serial.println(Turn_Need);
  
  Position_Add += Speed_Need;  //
  int turbo = 800;
  if (useThrottle) turbo = ThrottleEnd;
  
  Position_Add = constrain(Position_Add, -1*turbo, turbo);  //these contraints set the top speed
  // Serial.print(Position_AVG_Filter);  Serial.print("\t"); Serial.println(Position_Add);
//Serial.print((Angle_Car-2 + K_Base)* KA_P);  Serial.print("\t");
  pwm =  (Angle_Car-5 + K_Base)* KA_P   //P
      + Gyro_Car * KA_D //D
      +  Position_Add * KP_I    //I
      +  Position_AVG_Filter * KP_P; //P
   // Serial.println(pwm);  
   
    // if(Speed_Need ==0)
   //   StopFlag = true;
     
  if((Speed_Need != 0) && (Turn_Need == 0))
  {
    if(StopFlag == true)
    {
      Speed_Diff_ALL = 0;
      StopFlag = false;
    }
    pwm_r = int(pwm + Speed_Diff_ALL);
    pwm_l = int(pwm - Speed_Diff_ALL);
  
  }
  else
  {
    StopFlag = true;
     pwm_r = pwm + Turn_Need; //
     pwm_l = pwm - Turn_Need;

  }

  Speed_L = 0;
  Speed_R = 0;
}

void Car_Control()
{  
 if (pwm_l<0)
  {
    digitalWrite(DIR_L1, HIGH);
    digitalWrite(DIR_L2, LOW);
    pwm_l =- pwm_l;  //cchange to positive
  }
  else
  {
    digitalWrite(DIR_L1, LOW);
    digitalWrite(DIR_L2, HIGH);
  }
  
  if (pwm_r<0)
  {
    digitalWrite(DIR_R1, LOW);
    digitalWrite(DIR_R2, HIGH);
    pwm_r = -pwm_r;
  }
  else
  {
    digitalWrite(DIR_R1, HIGH);
    digitalWrite(DIR_R2, LOW);
  }
  if( Angle_Car > 45 || Angle_Car < -45 )
  {
    pwm_l = 0;
    pwm_r = 0;
  }
 // pwm_l = pwm_l;  //adjust Motor different
//  pwm_r = pwm_r;
  // Serial.print(pwm_l);  Serial.print("\t"); Serial.println(pwm_r);
  analogWrite(PWM_L, pwm_l>255? 255:pwm_l);
  analogWrite(PWM_R, pwm_r>255? 255:pwm_r);
  
  /*
  Serial.println("pwm:"); Serial.print(pwm);
  Serial.print("\t");
  Serial.print("pwm_L:"); Serial.print(pwm_l);
  Serial.print("pwm_R:"); Serial.print(pwm_r);
 */ 
}


void Init()
{
  pinMode(BUZZER, OUTPUT);
  pinMode(SPD_PUL_L, INPUT);//
  pinMode(SPD_PUL_R, INPUT);//
  pinMode(PWM_L, OUTPUT);//
  pinMode(PWM_R, OUTPUT);//
  pinMode(DIR_L1, OUTPUT);//
  pinMode(DIR_L2, OUTPUT);
  pinMode(DIR_R1, OUTPUT);//
  pinMode(DIR_R2, OUTPUT); 

  pinMode(AUX_IN_PIN,INPUT);
  pinMode(UP_DOWN_IN_PIN, INPUT);//
  pinMode(LEFT_RIGHT_IN_PIN, INPUT);//
  //init variables
  Speed_L = 0;
  Speed_R = 0;
  Position_AVG = 0;
  Position_AVG_Filter = 0;
  Position_Add = 0;
  pwm = 0;pwm_l = 0;pwm_r = 0;
  Speed_Diff = 0;Speed_Diff_ALL = 0;
  
  KA_P = 25.0;
  KA_D = 2; //3.5;
  KP_P = 30;
  KP_I = 0.34;
  K_Base = 6.7;
  
 /*ReadingData.KA_P = KA_P;
 ReadingData.KA_D = KA_D;
 ReadingData.KP_P = KP_P;
 ReadingData.KP_I =KP_I;
 ReadingData.K_Base = K_Base;
  */
  Speed_Need = 0;
  Turn_Need = 0;
  /*
 ReadFromEEprom(&ReadingData);
  KA_P = ReadingData.KA_P;
  KA_D = ReadingData.KA_D;
  KP_P = ReadingData.KP_P;
  KP_I = ReadingData.KP_I;
  K_Base = ReadingData.K_Base;
  Serial.print("PID Data is");Serial.print("\t");
  Serial.print(KA_P);Serial.print("\t");
  Serial.print(KA_D);Serial.print("\t");
  Serial.print(KP_P);Serial.print("\t");  
  Serial.print(KP_I);Serial.print("\t");    
  Serial.print(K_Base);Serial.println("\t");
*/
}


void Encoder_L()   //car up is positive car down  is negative
{
  if (digitalRead(SPD_PUL_L))
    Speed_L += 1;
  else
    Speed_L -= 1;
 //  Serial.print("SPEED_L:    ");
   // Serial.println(Speed_L);
}

void Encoder_R()    //car up is positive car down  is negative
{
  if (!digitalRead(SPD_PUL_R))
    Speed_R += 1;
  else
    Speed_R -= 1;
    
  // Serial.print("SPEED_R:    ");
   // Serial.println(Speed_R);
}


int UpdateAttitude()
{
 if((micros() - timer) >= 10000) 
 {    //10ms 
  /* Update all the values */
  while (i2cRead(0x3B, i2cData, 14));
  accX = ((i2cData[0] << 8) | i2cData[1]);
  accY = ((i2cData[2] << 8) | i2cData[3]);
  accZ = ((i2cData[4] << 8) | i2cData[5]);
  tempRaw = (i2cData[6] << 8) | i2cData[7];
  gyroX = (i2cData[8] << 8) | i2cData[9];
  gyroY = (i2cData[10] << 8) | i2cData[11];
  gyroZ = (i2cData[12] << 8) | i2cData[13];

  double dt = (double)(micros() - timer) / 1000000; // Calculate delta time
  timer = micros();

  // Source: http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf eq. 25 and eq. 26
  // atan2 outputs the value of -π to π (radians) - see http://en.wikipedia.org/wiki/Atan2
  // It is then converted from radians to degrees
#ifdef RESTRICT_PITCH // Eq. 25 and 26
  double roll  = atan2(accY, accZ) * RAD_TO_DEG;
  double pitch = atan(-accX / sqrt(accY * accY + accZ * accZ)) * RAD_TO_DEG;
#else // Eq. 28 and 29
  double roll  = atan(accY / sqrt(accX * accX + accZ * accZ)) * RAD_TO_DEG;
  double pitch = atan2(-accX, accZ) * RAD_TO_DEG;
#endif

  double gyroXrate = gyroX / 131.0; // Convert to deg/s
  double gyroYrate = gyroY / 131.0; // Convert to deg/s

#ifdef RESTRICT_PITCH
  // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
  if ((roll < -90 && kalAngleX > 90) || (roll > 90 && kalAngleX < -90)) {
    kalmanX.setAngle(roll);
    compAngleX = roll;
    kalAngleX = roll;
    gyroXangle = roll;
  } else
    kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculate the angle using a Kalman filter

  if (abs(kalAngleX) > 90)
    gyroYrate = -gyroYrate; // Invert rate, so it fits the restriced accelerometer reading
  kalAngleY = kalmanY.getAngle(pitch, gyroYrate, dt);
#else
  // This fixes the transition problem when the accelerometer angle jumps between -180 and 180 degrees
  if ((pitch < -90 && kalAngleY > 90) || (pitch > 90 && kalAngleY < -90)) {
    kalmanY.setAngle(pitch);
    compAngleY = pitch;
    kalAngleY = pitch;
    gyroYangle = pitch;
  } else
    kalAngleY = kalmanY.getAngle(pitch, gyroYrate, dt); // Calculate the angle using a Kalman filter

  if (abs(kalAngleY) > 90)
    gyroXrate = -gyroXrate; // Invert rate, so it fits the restriced accelerometer reading
  kalAngleX = kalmanX.getAngle(roll, gyroXrate, dt); // Calculate the angle using a Kalman filter
#endif

 // gyroXangle += gyroXrate * dt; // Calculate gyro angle without any filter
 // gyroYangle += gyroYrate * dt;
  //gyroXangle += kalmanX.getRate() * dt; // Calculate gyro angle using the unbiased rate
  //gyroYangle += kalmanY.getRate() * dt;

 // compAngleX = 0.93 * (compAngleX + gyroXrate * dt) + 0.07 * roll; // Calculate the angle using a Complimentary filter
  //compAngleY = 0.93 * (compAngleY + gyroYrate * dt) + 0.07 * pitch;

  // Reset the gyro angle when it has drifted too much
  if (gyroXangle < -180 || gyroXangle > 180)
    gyroXangle = kalAngleX;
  if (gyroYangle < -180 || gyroYangle > 180)
    gyroYangle = kalAngleY;

  /* Print Data */
#if 0 // Set to 1 to activate
  Serial.print(accX); Serial.print("\t");
  Serial.print(accY); Serial.print("\t");
  Serial.print(accZ); Serial.print("\t");

  Serial.print(gyroX); Serial.print("\t");
  Serial.print(gyroY); Serial.print("\t");
  Serial.print(gyroZ); Serial.print("\t");

  Serial.print("\t");
  Serial.println("");
#endif

#if 0
//  This is the rotation around the axle
  Serial.print(roll); Serial.print("\t");
 // Serial.print(gyroXangle); Serial.print("\t");
 // Serial.print(compAngleX); Serial.print("\t");
  Serial.print(kalAngleX); Serial.print("\t");

  Serial.print("\t");

// Pitch is the rotation not rurning left or right but yawing side to side.  I am not sure why we care about this value as it is almost always near zero.  Can we just use the roll?
  Serial.print(pitch); Serial.print("\t");
 // Serial.print(gyroYangle); Serial.print("\t");
 // Serial.print(compAngleY); Serial.print("\t");
  Serial.print(kalAngleY); Serial.println("\t");
#endif
#if 0 // Set to 1 to print the temperature
  Serial.print("\t");

  double temperature = (double)tempRaw / 340.0 + 36.53;
  Serial.print(temperature); Serial.print("\t");
#endif

  //Serial.print("\r\n");
  
  Angle_Car = kalAngleX;   //negative backward  positive forward
  Gyro_Car = gyroXrate;
 //Serial.print(Angle_Car);Serial.print("\t");Serial.println(Gyro_Car);

   return 1;
 }
  return 0;
}

void LEDBlink()
{
  if((millis() - LEDTimer) > 500)
  {
     LEDTimer = millis();
     blinkState = !blinkState;
   // digitalWrite(LED, blinkState);
   
   // Serial.print("Angle_Car\t");
   // Serial.println(Angle_Car);//Serial.print("\t");Serial.println(Gyro_Car);
  }  
}




//rc receiver interrupt routine
//------------------------------------------------------

void calcUpDown()
{
 // Serial.println("in up down here");
  if(digitalRead(UP_DOWN_IN_PIN) == HIGH)
  {
    UpDownStart = micros();
  }
  else
  {
    UpDownEnd = (uint16_t)(micros() - UpDownStart);
    bUpdateFlagsRC |= UP_DOWN_FLAG;
  }
}

void calcLeftRight()
{
  // Serial.println("in Left Right here");
  if(digitalRead(LEFT_RIGHT_IN_PIN) == HIGH)
  {
    LeftRightStart = micros();
  }
  else
  {
    LeftRightEnd = (uint16_t)(micros() - LeftRightStart);
    bUpdateFlagsRC |= LEFT_RIGHT_FLAG;
    
  }
}
void calcThrottle()
{
 // Serial.println("in up down here");
  if(digitalRead(THROTTLE_IN_PIN) == HIGH)
  {
    ThrottleStart = micros();
  }
  else
  {
    ThrottleEnd = (uint16_t)(micros() - ThrottleStart);
    //bUpdateFlagsRC |= RUDDER_FLAG;
    if (ThrottleEnd > 1300) {
      useThrottle = true;
    } else {
      useThrottle = false;
    }
    ///Serial.println(ThrottleEnd);
  }
}


#define  RC_ERROR 100
#define RC_CAR_SPEED  160
int RCTurnSpeed = 80;
void ProcessRC()
{
  //Serial.println(".");

  
  if(bUpdateFlagsRC)
  {
     noInterrupts(); // turn interrupts off quickly while we take local copies of the shared variables

    if(bUpdateFlagsRC & UP_DOWN_FLAG)
    {
      UpDownIn = UpDownEnd;
      if( abs(UpDownIn - 1500) > RC_ERROR ) 
      {
        if(UpDownIn > 1500) // car up
         Speed_Need = RC_CAR_SPEED;
         else       // car down
         Speed_Need = -RC_CAR_SPEED;
          //Serial.println("here up down");
      }
      else
      {
        Speed_Need = 0;
      }
    }
   
    if(bUpdateFlagsRC & LEFT_RIGHT_FLAG)
    {
      LeftRightIn = LeftRightEnd;
      if( abs(LeftRightIn - 1500) > RC_ERROR)
      {
      //  RCTurnSpeed = 100;//map(ThrottleIn,1000,2000,0,200);
        if(LeftRightIn > 1500)
        Turn_Need = RCTurnSpeed;  // right
        else   // left
        Turn_Need = -RCTurnSpeed;
         //Serial.println("here Left Right");
      }
      else
      {
        Turn_Need = 0;
      } 
    }
    
    bUpdateFlagsRC = 0;
   
    interrupts();
  }/*
  if( useThrottle ) 
  {
         uint32_t note = (ThrottleEnd - 1300) * 10;
         if (note >65535) note = 65535;
         if (note <31 ) note = 31;
         //Serial.print(ThrottleEnd);
         //Serial.print("\t");
         //Serial.println(note);
         if (note <= 0) note = 1;
         tone(tonePin,note,100);  
         //noTone(tonePin); 
  }
  else 
  {
     noTone(tonePin);
  }
*/
}













//vvvvvvvvvv Optional Stuff vvvvvvvvv


void MusicPlay()
{

//  if((Speed_Need !=0 ) || (Turn_Need !=0 )) //when move let's play music
  //if(useThrottle) //when move let's play music
  if (false)
  {
      if((millis() - BuzzerTimer) >= (12*marioduration[tonecnt]))
      {
         noTone(tonePin);
         tonecnt++;
         if (tonecnt >= sizeof(mario)/sizeof(int) ) tonecnt = 0;
         //if(tonecnt>=tonelength)
         //tonecnt = 0;
         BuzzerTimer = millis();
         tone(tonePin,mario[tonecnt]);  
         isInMucis  = true;
      }   
  }
  else if(isInMucis == true)
  {
    isInMucis = false;
     noTone(tonePin);
  }
 
 /*
  if( Angle_Car > 45 || Angle_Car < -45 ) //if car fall down ,let's alarm
  {
     if((millis() - LEDTimer) > 200)
    {
          LEDTimer = millis();
         blinkState = !blinkState;
         digitalWrite(LED, blinkState);
         isInBuzzer = true;
    }
  }
  else if(isInBuzzer == true)
  {
     digitalWrite(LED, 0);
     isInBuzzer = false;
  }

*/
//  if( analogRead(AUX_IN_PIN) > 1000 ) {

//Serial.print("throttle:");
//Serial.println(analogRead(AUX_IN_PIN));

/*
  if(false ) {
     if((millis() - LEDTimer) > 200)
    {
          LEDTimer = millis();
         blinkState = !blinkState;
         digitalWrite(LED, blinkState);
         isInBuzzer = true;
    }
    
  } else {
          digitalWrite(LED, 0);
          isInBuzzer = false;
    
  }
*/
  
}



/*
void UserComunication()
{
  MySerialEvent();
  if(SerialPacket.m_PackageOK == true)
  {
    SerialPacket.m_PackageOK = false;
    switch(SerialPacket.m_Buffer[4])
    {
      case 0x01:break;
      case 0x02: UpdatePID(); break;
      case 0x03: CarDirection();break;
      case 0x04: SendPID();break;
      case 0x05:  
                  SavingData.KA_P = KA_P;
                  SavingData.KA_D = KA_D;
                  SavingData.KP_P = KP_P;
                  SavingData.KP_I = KP_I;
                  SavingData.K_Base = K_Base;
                  WritePIDintoEEPROM(&SavingData);
                  break;
      case 0x06:  break;
      case 0x07:break;
      default:break;             
    }
  }
  
}
*/
/*
void UpdatePID()
{
  unsigned int Upper,Lower;
  double NewPara;
  Upper = SerialPacket.m_Buffer[2];
  Lower = SerialPacket.m_Buffer[1];
  NewPara = (float)(Upper<<8 | Lower)/100.0;
  switch(SerialPacket.m_Buffer[3])
  {
    case 0x01:KA_P = NewPara; Serial.print("Get KA_P: \n");Serial.println(KA_P);break;
    case 0x02:KA_D = NewPara; Serial.print("Get KA_D: \n");Serial.println(KA_D);break;
    case 0x03:KP_P = NewPara; Serial.print("Get KP_P: \n");Serial.println(KP_P);break;
    case 0x04:KP_I = NewPara; Serial.print("Get KP_I: \n");Serial.println(KP_I);break;
    case 0x05:K_Base = NewPara; Serial.print("Get K_Base: \n");Serial.println(K_Base);break;
    default:break;
  }
}


void CarDirection()
{
  unsigned char Speed = SerialPacket.m_Buffer[1];
  switch(SerialPacket.m_Buffer[3])
  {
    case 0x00: Speed_Need = 0;Turn_Need = 0;break;
    case 0x01: Speed_Need = -Speed; break;
    case 0x02: Speed_Need = Speed; break;
    case 0x03: Turn_Need = Speed; break;
    case 0x04: Turn_Need = -Speed; break;
    default:break;
  }
}

void SendPID()
{
  static unsigned char cnt = 0;
  unsigned char data[3];
  switch(cnt)
  {
    case 0: 
    data[0] = 0x01;
    data[1] = int(KA_P*100)/256;
    data[2] = int(KA_P*100)%256;
    AssemblyAndSend(0x04,data);
    cnt++;break;
    case 1:
     data[0] = 0x02;
    data[1] = int(KA_D*100)/256;
    data[2] = int(KA_D*100)%256;
    AssemblyAndSend(0x04,data);
    cnt++;break;  
    case 2:
    data[0] = 0x03;
    data[1] = int(KP_P*100)/256;
    data[2] = int(KP_P*100)%256;
    AssemblyAndSend(0x04,data);
    cnt++;break;    
    case 3:
    data[0] = 0x04;
    data[1] = int(KP_I*100)/256;
    data[2] = int(KP_I*100)%256;
    AssemblyAndSend(0x04,data);
    cnt++;break; 
    case 4:
    data[0] = 0x05;
    data[1] = int(K_Base*100)/256;
    data[2] = int(K_Base*100)%256;
    AssemblyAndSend(0x04,data);
    cnt = 0;break; 
  default:break;  
  }
}

*/
/*
void AssemblyAndSend(char type ,unsigned char *data)
{
  unsigned char sendbuff[6];
  sendbuff[0] = 0xAA;
  sendbuff[1] = type;
  sendbuff[2] = data[0];
  sendbuff[3] = data[1];
  sendbuff[4] = data[2];
  sendbuff[5] = data[0]^data[1]^data[2];
  for(int i = 0; i < 6; i++)
  {
    Serial.write(sendbuff[i]);
  }
}


void MySerialEvent()
{
  uchar c = '0';
        uchar tmp = 0;
        if(Serial.available()) {
          c = (uchar)Serial.read();
  //Serial.println("here");
          for(int i = 5; i > 0; i--)
          {
             SerialPacket.m_Buffer[i] = SerialPacket.m_Buffer[i-1];
          }
    SerialPacket.m_Buffer[0] = c;
          
          if(SerialPacket.m_Buffer[5] == 0xAA)
          {
             tmp = SerialPacket.m_Buffer[1]^SerialPacket.m_Buffer[2]^SerialPacket.m_Buffer[3];
             if(tmp == SerialPacket.m_Buffer[0])
             {
               SerialPacket.m_PackageOK = true;
             }
          }
        } 
}

*/
