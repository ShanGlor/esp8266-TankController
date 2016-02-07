
//#include <ESP8266WiFi.h>
#include "pfodESP8266WiFi.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"
/////////////////////////// Declarations Debugging /////////////////////////////////////////////////
int cnt = 0;
unsigned long   Time_1=0,Time_1_prev =0,Time_1_max=0;
float           Time_1_display,Time_1_max_display;
unsigned long   Time_2=0,Time_2_prev =0,Time_2_max=0;
float           Time_2_display,Time_2_max_display;
unsigned long   prev_time =0;
///////////////////////////////////// GYRO, MPU6050. Declarations //////////////////////////////////
MPU6050 mpu;
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
Quaternion q;           // [w, x, y, z]         quaternion container
VectorInt16 aa;         // [x, y, z]            accel sensor measurements
VectorInt16 aaReal;     // [x, y, z]            gravity-free accel sensor measurements
VectorInt16 aaWorld;    // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity;    // [x, y, z]            gravity vector
float euler[3];         // [psi, theta, phi]    Euler angle container
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
float yaw;
int Gyro_Status=0;

///////////////////////////////////// RC. Declarations /////////////////////////////////////////////
int RC_Loop_Time=0;
int RC_Status = 0;
int RC_prev_time =0;
int elevation_time;
int switches_time;
int triger;
int base;
int Alloff;
int TriggerOffSet;
int Trigger1,Trigger2,Trigger3,Trigger4,Trigger5,Trigger6;
int Sub1,Sub2,Sub3,Sub4,Sub5,Sub6;
int Display=0;
boolean sw1=0,sw2=0,sw3=0,sw4=0,sw5=0,sw6=0;

/////////////////////////// Declarations Wifi //////////////////////////////////////////////////////
const char* ssid          = "TANK_TURRET_AP";
const char* password      = "12345679";
WiFiServer server(80);
String DataSent;
int Wifi_Status =0;//......0=OFF, 1=ON, 2=FAILD,3=START ..........................................//

/////////////////////////// Declarations Failure_Detection /////////////////////////////////////////
int Wifi_Failure_cnt=0;
int Gyro_Failure_cnt=0;
int RC_Failure_cnt=0;
int RC_cnt_Trigger = 4;//.................. Missed Signals before Safe mod for RC ................//
int Gyro_cnt_Trigger = 10;//............... fifi overflows before turning gyro off ...............//
int Wifi_cnt_Trigger = 500;//.............. Missed transmissions before reseting Wyfi ............//
int Fail_ID;//........... 1=RC, 2=Wifi, 3=Gyro ...................................................//

//////////////////////////////////////// Failure Dection ROUTINE ///////////////////////////////////
void Failure_Detection(){
    if(Fail_ID==1){//.................................... ID1=RC .................................//
      RC_Failure_cnt=RC_Failure_cnt+1;RC_Status = 2;// failed to complete in time for RC Rx ......//
      if(RC_Failure_cnt > RC_cnt_Trigger){//...................... RC in SAFE mode ...............//
          RC_Status = 3;RC_Failure_cnt=RC_Failure_cnt-1;}
    }
    if(Fail_ID==2){//.................................... ID2=Wifi ...............................//
        Wifi_Failure_cnt = Wifi_Failure_cnt + 1;//... Not ready for client.print .................//
        if(Wifi_Failure_cnt > Wifi_cnt_Trigger-1){// Wifi in SAFE mode ...........................//
            Wifi_Status =2;//......0=OFF, 1=ON, 2=FAILD,3=START ..................................//
            Wifi_Failure_cnt = Wifi_cnt_Trigger;
            cnt=0;}
    }
    if(Fail_ID==31){//................................... ID31=Tried Gyro Start ..................//
        Gyro_Status=2;}//........................................... Communications failed .......//
    if(Fail_ID==32){//.......... Gyro ID32; Gyro Data flow .......................................//
        Gyro_Failure_cnt=Gyro_Failure_cnt+1;Gyro_Status = 3;//.......... Data fifo Reset .........//
        if(Gyro_Failure_cnt >  Gyro_cnt_Trigger){//................... Exccessive fifi Resets.....//
            Gyro_Status = 0;Gyro_Failure_cnt=Gyro_Failure_cnt-1;}}  
  }
///////////////////////////////////// Start GYRO, SubRoutine //////////////////////////////////////
void startMPU6050(){
        delay(10);//.......... delay for power up ...............................................//
        Wire.begin();
        Wire.setClock(400000);
        mpu.initialize();
        if(mpu.testConnection()){
            mpu.dmpInitialize();
            mpu.setXGyroOffset(220);
            mpu.setYGyroOffset(76);
            mpu.setZGyroOffset(-85);
            mpu.setZAccelOffset(1788);
            mpu.setDMPEnabled(true);
            packetSize = mpu.dmpGetFIFOPacketSize();
            Gyro_Status=1;//.......... 1=Started 
        }
        else {Fail_ID=31;Failure_Detection();}
}
///////////////////////////////////// Get GYRO DATA, SubRoutine /////////////////////////////////////////////////
void getMPU6050data(){
    fifoCount = mpu.getFIFOCount();
    if (fifoCount == 1024) {mpu.resetFIFO();fifoCount = mpu.getFIFOCount();Fail_ID=32;Failure_Detection();}
    while (fifoCount > packetSize-1){
        mpu.getFIFOBytes(fifoBuffer, packetSize);
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        fifoCount = mpu.getFIFOCount();
    }
    Gyro_Status=1;
}

/////////////////////////// RC Rx ROUTINE /////////////////////////////////////////////////////////
void pgm(){
    
    delay(20);//........... Simulates RC control routine ........................................//
    //if(sw4==1 && Gyro_Status==0){digitalWrite(D8, HIGH);startMPU6050();}
    //if(sw4==0 && Gyro_Status!=0){digitalWrite(D8, LOW);Gyro_Status=0;}
    if(Gyro_Status==0){digitalWrite(16, HIGH);startMPU6050();}
    if(Gyro_Status==1)getMPU6050data();
    
}

///////////////////////////////////// RC, SubRoutine ////////////////////////////////////////////////////////////
void RC(){ 
    RC_Loop_Time=micros();
    //..........................................Channel 3, Cannon Elevation ...................................//
    while(digitalRead(D5)==HIGH){//..................Prevent short timing .....................................//
      if(micros()-RC_Loop_Time > 20000){Fail_ID=1;Failure_Detection();return;}
    }
    while(digitalRead(D5)==LOW){
      if(micros()-RC_Loop_Time > 20000){Fail_ID=1;Failure_Detection();return;}
    }
    RC_prev_time = micros();
    while(digitalRead(D5)==HIGH);{
       if(micros()-RC_Loop_Time > 20000){Fail_ID=1;Failure_Detection();return;}
    }
    elevation_time = micros()-prev_time;
    //..........................................Channel 5, 6 x On/Off switches encoded ........................//
    while(digitalRead(D7)==LOW);{
      if(micros()-RC_Loop_Time > 20000){Fail_ID=1;Failure_Detection();return;}
    }
    RC_prev_time = micros();
    while(digitalRead(D7)==HIGH);{
      if(micros()-RC_Loop_Time > 20000){Fail_ID=1;Failure_Detection();return;}
    }
    switches_time = micros()-prev_time;Display=switches_time;
        //................................ Sw. 1 = Wt.16  .....................................................//
        if(switches_time > Trigger1){switches_time =switches_time-Sub1;sw1=1;}
        else {sw1=0;}
        //................................ Sw. 2 = Wt.32  .....................................................//
        if(switches_time > Trigger2){switches_time = switches_time-Sub2;sw2=1;}
        else {sw2=0;}
        //................................ Sw. 3 = Wt.64  .....................................................//
        if(switches_time > Trigger3){switches_time = switches_time-Sub3;sw3=1;}
        else {sw3=0;}
        //................................ Sw. 4 = Wt.128  ....................................................//
        if(switches_time > Trigger4){ switches_time = switches_time-Sub4;sw4=1;}
        else {sw4=0;}
        //................................ Sw. 5 = Wt.128  ....................................................//
        if(switches_time >Trigger5){switches_time = switches_time-Sub5;sw5=1;}
        else {sw5=0;}
        //................................ Sw. 6 = Wt.128  ....................................................//
        if(switches_time > Trigger6){switches_time = switches_time-Sub6;sw6=1;}
        else {sw6=0;}
    RC_Failure_cnt=0;
    RC_Status = 1;    
}
//////////////////////////////////////// Wiifi Start ROUTINE //////////////////////////////////////
void Wifi_disconnect(){
    WiFi.disconnect();
    delay(500);
}
void Wifi_AP_start(){ 
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid,password);
    server.begin();
    delay(500);
    Wifi_Status =1;//......0=OFF, 1=ON, 2=FAILD,3=START .........................................//
}

void Serial_print(){
        Serial.print("Ap Data to Send(");Serial.print(DataSent); Serial.print(") ");
        Serial.print("\t");
        
        Serial.print("Status; ");
        Serial.print("W(");Serial.print(Wifi_Status);Serial.print(") ");
        Serial.print("G(");Serial.print(Gyro_Status);Serial.print(") ");
        Serial.print("R(");Serial.print(RC_Status);Serial.print(") ");
        Serial.print("\t");
        
        Serial.print("Fail_cnts; ");
        Serial.print("W(");Serial.print(Wifi_Failure_cnt);Serial.print(") ");
        Serial.print("G(");Serial.print(Gyro_Failure_cnt);Serial.print(") ");
        Serial.print("R(");Serial.print(RC_Failure_cnt);Serial.print(") ");
        Serial.print("\t");
        
        Serial.print("Wifi Transmissions(");Serial.print(cnt);Serial.print("}");
        Serial.println(); 
}
/////////////////////////// SETUP ROUTINE /////////////////////////////////////////////////////////
void setup() {
    pinMode(D7,INPUT); 
    pinMode(16, OUTPUT); digitalWrite(16, HIGH);//........ Gyro Power ...........................//
    Serial.begin(250000);
    //.................. delay serial communication to prevent MS Window ........................//
    delay(500); //...... fron thinking it a serial mouse ........................................//
    Serial.println();Serial.println();
    
}
/////////////////////////// LOOP ROUTINE //////////////////////////////////////////////////////////
void loop() {
  
    Serial.println("Restarted Loop");
    Wifi_disconnect();
    Wifi_AP_start();
    WiFiClient client = server.available();

    while(1==1){
        //pgm();//...................................... Simulate Timeing  ............................//
        RC();
        if(Gyro_Status==0){digitalWrite(16, HIGH);startMPU6050();}
        if(Gyro_Status==1)getMPU6050data();
        if(Gyro_Status==1)DataSent = ypr[0];
        else {DataSent = "OFF";}
        if(client ==true && client.isSendWaiting()==false) {
            cnt=cnt+1;
            client.print(DataSent+ '\r');
            Wifi_Failure_cnt = 0;
        }  
        else {Fail_ID=2;Failure_Detection();}
        if(Wifi_Status ==2){
            Wifi_Failure_cnt = 0;
            Wifi_Status =3;
            return;
        }  
        Serial_print();
   }   
} 
