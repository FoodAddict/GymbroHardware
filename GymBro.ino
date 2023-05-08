#include<Wire.h>
#include <math.h>


//consst uint8_t *esp_bt_dev_get_address(void);


//HARDWARE WIRES//
const int MPU=0x68;
int16_t AcX,AcY,AcZ;
int flexResistancePIN = A1;
int armContactPIN = A0;




//ACCELEROMETER//
double pitch,roll,flex,arm;
boolean A = true;
float F_C = 0;
float F_F = 0;
int Repcounter = 0;


//CALORIE//
//CONSTANTS//
const float VO2Rest = 3.5 ;// in mL/kg/min
const float MaxHR = (212 - 21);
float VO2Max = 212 - 21;
float VO2Reserve = (212 - 21) - 3.5;
int weight = 58.9;
float Intensity = 0;
float totalCalorie = 0;
float CalPMin = 0;
double emg;
float F_M = 0;


//HEART RATE CONSTANTS//
boolean B = false;
int heartRead = A2;
int heartCount = 0;
float BPM = 0.0f;


//Timer//
unsigned long chrono = 0;
float seconds = 0;


//BlueTooth//


#define window_size 25


struct Filter{
  int X[window_size] = {0};
  int Y[window_size] = {0};
  int Z[window_size] = {0};
  int F[window_size] = {0};
  int C[window_size] = {0};
  int M[window_size] = {0};
  int x_size = 1, y_size = 1, z_size = 1, f_size = 1, c_size = 1, m_size = 1;
};


void push(Filter& f, int x, int y, int z, int FR, int C, int m)
{
  if(f.x_size < window_size)
  {
    f.X[f.x_size - 1] = x;
    ++f.x_size;
  }
  else
  {
    for(int i = window_size - 1; i >= 1; --i)
    {
      f.X[i - 1] = f.X[i];
    }
    f.X[f.x_size - 1] = x;
  }


//================================================================//
 
  if(f.y_size < window_size)
  {
    f.Y[f.y_size - 1] = y;
    ++f.y_size;
  }
  else
  {
    for(int i = window_size - 1; i >= 1; --i)
    {
      f.Y[i - 1] = f.Y[i];
    }
    f.Y[f.y_size - 1] = y;
  }


//================================================================//


  if(f.z_size < window_size)
  {
    f.Z[f.z_size - 1] = z;
    ++f.z_size;
  }
  else
  {
    for(int i = window_size - 1; i >= 1; --i)
    {
      f.Z[i - 1] = f.Z[i];
    }
    f.Z[f.z_size - 1] = z;
  }


//================================================================//
 
  if(f.f_size < window_size)
  {
    f.F[f.f_size - 1] = FR;
    ++f.f_size;
  }
  else
  {
    for(int i = window_size - 1; i >= 1; --i)
    {
      f.F[i - 1] = f.F[i];
    }
    f.F[f.f_size - 1] = FR;
  }


//================================================================//


  if(f.c_size < window_size)
  {
    f.C[f.c_size - 1] = C;
    ++f.c_size;
  }
  else
  {
    for(int i = window_size - 1; i >= 1; --i)
    {
      f.C[i - 1] = f.C[i];
    }
    f.C[f.c_size - 1] = C;
  }


  //================================================================//


  if(f.m_size < window_size)
    {
     f.M[f.m_size - 1] = m;
      ++f.m_size;
    }
    else
    {
      for(int i = window_size - 1; i >= 1; --i)
      {
        f.M[i - 1] = f.M[i];
      }
      f.M[f.m_size - 1] = m;
    }
}


float take_avg(int avg[], int count)
{
  float sum=0;
  int i=0;
  for(i=0;i<count;i++){
    sum+=avg[i];
  }
  sum = sum / count;
  return sum;
}


//pitch, F_F, F_C, F_M


void countRep()
{
    int condition = 0;
    if(!A)
    {
      if(F_F >= 560)
      {
        ++condition;
      }
      if(F_C <= 320)
      {
        ++condition;
      }
      if(F_M <= 575)
      {
        ++condition;
      }


      if(condition >= 2)
      {
        if(pitch <= -50)
        {
          A = true;
          ++Repcounter;
        }
      }
    }
    if(A)
    {
      if(pitch >= 10)
      {
        if(F_F <= 560)
        {
          if(F_C <= 320)
          {
            if(F_M <= 575)
            {
              A = false;
            }
          }
          else
          {
            A = false;  
          }
        }
      }
    }
   
     //Serial.print(" Flag: ");
     //Serial.println(A);
     //Serial.print(" Reps: ");
    //Serial.println(Repcounter);
   
}


void heartRate(float read){
  if(!B){
    if(read > 600){
      B = true;
    }
  }
  if(B){
    if(read < 550){
      B = false;
      heartCount++;
    }
  }
}


void HR_bpm(int seconds)
{  
  if(seconds % 60 == 0){
    BPM = heartCount;
    heartCount = 0;
    totalCalorie = totalCalorie + CalPMin;
  }
  Serial.println(BPM);  
}


float intensity(float HR)
{
  float i = 0.0f;
  return i = HR / VO2Max;
}


Filter filter;
void setup(){
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  chrono = millis();
  Serial.begin(9600);
  pinMode(10, INPUT); // Setup for leads off detection LO +
  pinMode(11, INPUT); // Setup for leads off detection LO -
}


void loop(){
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,14,true);


  if((digitalRead(8) == 1) || (digitalRead(9) == 1))
  { //check if leads are removed
    Serial.println("leads off!");
    return;
  }


  if (millis() - chrono >= 1000)
  {
    chrono = millis();
    result();
  }


  int AcXoff,AcYoff,AcZoff;


  //Acceleration data correction
  AcXoff = -950;
  AcYoff = -300;
  AcZoff = 0;


  //read accel data
  AcX=(Wire.read()<<8|Wire.read()) + AcXoff;
  AcY=(Wire.read()<<8|Wire.read()) + AcYoff;
  AcZ=(Wire.read()<<8|Wire.read()) + AcZoff;


  flex = analogRead(flexResistancePIN);
  arm = analogRead(armContactPIN);
  emg = analogRead(A2);


  //delay(100);


  push(filter, AcX, AcY, AcZ, flex, arm, emg);


  float F_X = take_avg(filter.X, filter.x_size);
  float F_Y = take_avg(filter.Y, filter.y_size);
  float F_Z = take_avg(filter.Z, filter.z_size);
  F_C = take_avg(filter.C, filter.c_size);
  F_F = take_avg(filter.F, filter.f_size);
  F_M = take_avg(filter.M, filter.m_size);


  //get pitch/roll
  getAngle(F_X,F_Y,F_Z);
  countRep();


  //send the data out the serial port


  //  Serial.print(" | Angle: ");
  //  Serial.print("Pitch = "); Serial.print(pitch);
  //  Serial.print(" | Roll = "); Serial.print(roll);
  //  Serial.print(" | Photoresistor: ");
  //  Serial.print(F_C);
  //  Serial.print(" | Potentiometer: ");
  //  Serial.print(F_F);
  //  Serial.println(" ");


  heartRate(analogRead(A2));
}


//convert the accel data to pitch/roll
void getAngle(float Vx,float Vy,float Vz) {
  float x = Vx;
  float y = Vy;
  float z = Vz;


  pitch = atan(x/sqrt((y*y) + (z*z)));
  roll = (atan(y/sqrt((x*x) + (z*z))));
  //convert radians into degrees
  pitch = pitch * (180.0/3.14);
  roll = roll * (180.0/3.14);


}


void result() {
 
  //TIMER//
  seconds++;
 
  Serial.print("Seconds: ");
  Serial.println(seconds);
 
  //Movement
  Serial.print("Reps: ");
  Serial.println(Repcounter);


  // //HEARTRATE//
   Serial.print("BPM: ");
   HR_bpm(seconds);
   Serial.print("Heart Beats: ");
   Serial.println(heartCount);


   //CALORIES//
   Intensity = intensity(heartCount);
   float VO2Final = (((VO2Reserve * Intensity)) - VO2Rest) * weight;
   CalPMin = (VO2Final / 1000) * 5;


   Serial.print("Total Calorie: " );
   Serial.println(totalCalorie);
   Serial.println("************************************");


}
