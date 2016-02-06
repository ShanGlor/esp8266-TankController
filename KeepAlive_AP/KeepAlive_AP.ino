//  Description of Project;
//      To increase the capabilities of Hobby grade RC control. Will be using RC Tanks as test bed.

//  Objective working on in this Sketch;

//      1). Add Gryo sketch and send by Wi-Fi Gyro Data from the 
//          turret to the hull.

//      2). Test using the esp8266's i/o controlling RC Tanks functions with the hope of replacing the
//          Arduino Due mentioned below.

//  Objectives achieved already but not yet added to this project;
//      1).  Arduino Due connected to RC Rx 8 channel module's servo outputs and have the Arduino Due
//           control RC tank functions.   

//      2).  Using MPU6050 Gyro connected to Arduino Due to control cannon elevation and 
//           turret rotation to remain fixed on target as hull is moving.

//      3).  Using the capabilities of FS-TH9X RC radio using ER9X software, transmit 6 switch positions
//           encoded on one channel and have the Arduino connected to the Rx module decode these
//           switch position. This gives the ability of & full analog and the position of 6 switches.
//           a total of 13 functions on 8 channels. 

//  Objective Completed in this Sketch;
//      1). Controlling Turret and Hull functions without wiring connecting hull and turret. This will
//          Allow continuous 360 deg. rotations of turret. 
//          Using Two esp8266's one in the turret and one in the Hull sending Wi-Fi Data from the 
//          turret to the hull.


// Notes for this sketch;  
//    The esp8266 sketch for the turret is referred to as AP and the esp8266 sketch in the hull is STA.
// 
//    The time for AP Wi-Fi to Transmit data to STA plus STA to send ACK to AP = approx.0.125ms.
//    This would cause blocking of both AP and STA sketches of 0.125ms each Wi-Fi transmission this
//    would make Safe RC control impossible.
//
//    Because the Sketch blocking nature of the client.print in include <ESP8266WiFi.h> it is 
//    not used in AP instead Non Blocking include "pfodESP8266WiFi.h" is used. Also print timeout
//    in STA is set to zero. This stops the sketch blocking making RC control possible.
//
//    AP Wifi_Failue_cnt = number of AP's client.print skipped and STA's client.reads that 
//    timed out between Wi-Fi actual successful transmissions. Wifi_Failue_cnt in both AP and STA
//    can be used as a gauge as RC and Gyro programming is added. If or when this count is constantly
//    zero because of additional programming the frequency of transmission maybe decreasing.







//#include <ESP8266WiFi.h>
#include "pfodESP8266WiFi.h"

#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#include "Wire.h"

///////////////////////////////////// GYRO, MPU6050. Declarations ///////////////////////////////////////////////
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

/////////////////////////// Declarations Debugging /////////////////////////////////////////////////
int cnt = 0;
unsigned long   Time_1=0,Time_1_prev =0,Time_1_max=0;
float           Time_1_display,Time_1_max_display;
unsigned long   Time_2=0,Time_2_prev =0,Time_2_max=0;
float           Time_2_display,Time_2_max_display;
unsigned long   prev_time =0;
/////////////////////////// Declarations Wifi /////////////////////////////////////////////////////
const char* ssid          = "TANK_TURRET_AP";
const char* password      = "12345679";
WiFiServer server(80);
String DataSent;
int Wifi_Status =0;//......0=OFF, 1=ON, 2=FAILD,3=START .........................................//
/////////////////////////// Declarations Failure_Detection ////////////////////////////////////////
int Wifi_Failure_cnt=0;
int Gyro_Failure_cnt=0;
int Wiifi_cnt_Trigger = 500;//.............. The cnt in STA and in AP should the same?   ........//
int Fail_ID;//........... 1=RC, 2=Wifi, 3=Gyro ..................................................//
//////////////////////////////////////// Failure Dection ROUTINE //////////////////////////////////
void Failure_Detection(){
  if(Fail_ID==2){//.................................... ID2=Wifi ................................//
        Wifi_Failure_cnt = Wifi_Failure_cnt + 1;
        if(Wifi_Failure_cnt > Wiifi_cnt_Trigger-1){
            Wifi_Status =2;//......0=OFF, 1=ON, 2=FAILD,3=START .................................//
            Wifi_Failure_cnt = Wiifi_cnt_Trigger;
            cnt=0;}
  }
  if(Fail_ID==31){//................................... ID31=Tried Gyro Start ...................//
      Gyro_Status=2;}//........................................... Communications failed ........//
  if(Fail_ID==32){//.......... Gyro ID32; Gyro Data flow ........................................//
      Gyro_Failure_cnt=Gyro_Failure_cnt+1;Gyro_Status = 3;//.......... Data fifo Reset ..........//
      if(Gyro_Failure_cnt > 10){//.................................. Eccessive fifi Resets.......//
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
        Serial.print(" AP Data Sent(");Serial.print(DataSent); Serial.print(") ");
        Serial.print("\t");
        Serial.print("Status(s); ");Serial.print("Wifi(");Serial.print(Wifi_Status);Serial.print(") ");
        Serial.print("\t");
        Serial.print("Fail_cnt(s); ");
        Serial.print("Wifi_C(");Serial.print(Wifi_Failure_cnt);Serial.print(") ");
        Serial.print(Gyro_Status);
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
        pgm();//...................................... Simulate Timeing  ............................//
        if(Gyro_Status==1)DataSent = ypr[0];
        else {DataSent = "OFF";}
        
        if(client ==true && client.isSendWaiting()==false) {
            client.print(DataSent+ '\r');
            Wifi_Failure_cnt = 0;
            //cnt=cnt+1;
           // DataSent = cnt;  
        }  
        else {Fail_ID=2;Failure_Detection();}
            Serial_print();
            if(Wifi_Status ==2){
            Wifi_Failure_cnt = 0;
            Wifi_Status =3;
            return;
          }
         
      }   
} 
