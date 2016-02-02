//#include <ESP8266WiFi.h>
#include "pfodESP8266WiFi.h"

//New Comment

const char* ssid          = "TANK_TURRET_AP";
const char* password      = "12345679";
WiFiServer server(80);


/////////////////////////// Declarations Deggering /////////////////////////////////////////////////
int cnt = 0;
unsigned long   Time_1=0,Time_1_prev =0,Time_1_max=0;
float           Time_1_display,Time_1_max_display;
unsigned long   Time_2=0,Time_2_prev =0,Time_2_max=0;
float           Time_2_display,Time_2_max_display;
unsigned long   prev_time =0;

/////////////////////////// Declarations Wifi /////////////////////////////////////////////////////
String DataSent;
int Wifi_Status =0;//......0=OFF, 1=ON, 2=FAILD,3=START .........................................//
/////////////////////////// Declarations Failure_Detection ////////////////////////////////////////
int Wifi_Failue_cnt=0;
int Fail_ID;//........... 1=RC, 2=Wifi, 3=Gyro ..................................................//


/////////////////////////// RC Rx ROUTINE /////////////////////////////////////////////////////////
void pgm(){
    digitalWrite(BUILTIN_LED, HIGH);//............ RED LED OFF ..................................//
    delay(20);//........... Simulates RC control routine ........................................//
    digitalWrite(BUILTIN_LED, LOW);//............. RED LED ON ...................................//
}
//////////////////////////////////////// Wiifi Start ROUTINE //////////////////////////////////////
void Wifi(){
    digitalWrite(BUILTIN_LED, LOW);//............. RED LED ON ...................................//
    WiFi.disconnect();delay(100);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid,password);
    delay(500);
    Serial.println("Local server only @ ");Serial.println(WiFi.softAPIP());
    server.begin();
    Wifi_Status =1;//......0=OFF, 1=ON, 2=FAILD,3=START .........................................//
    digitalWrite(BUILTIN_LED, HIGH);//............ RED LED OFF ..................................//
}
void Failure_Detection(){
  if(Fail_ID==2){
        Wifi_Failue_cnt = Wifi_Failue_cnt + 1;
        if(Wifi_Failue_cnt > 99){
            Wifi_Status =2;//......0=OFF, 1=ON, 2=FAILD,3=START ................................//
            Wifi_Failue_cnt = 100;
        }
  }
  
}
void Serial_print(){
        Serial.print("AP_TURRET"); Serial.print(" ");
        Serial.print("\t");
        //Serial.print("T1(");Serial.print(Time_1_display,4);Serial.print(" ");
        //Serial.print(Time_1_max_display,4); Serial.print(") ");
        //Serial.print("cnt(");Serial.print(cnt);Serial.print(") ");
        Serial.print("Status(s); ");
        Serial.print("Wifi(");Serial.print(Wifi_Status);Serial.print(") ");
        Serial.print("\t");
        Serial.print("Fail_cnt(s); ");
        Serial.print("Wifi(");Serial.print(Wifi_Failue_cnt);Serial.print(") ");
        Serial.println();
  
}
/////////////////////////// SETUP ROUTINE /////////////////////////////////////////////////////////
void setup() {
    pinMode(D7,INPUT); 
    pinMode(BUILTIN_LED, OUTPUT);
    Serial.begin(250000);
    //.................. delay serial communication to prevent MS Window .........................//
    delay(500); //...... fron thinking it a serial mouse .........................................//
    Serial.println();Serial.println();
    
}
/////////////////////////// LOOP ROUTINE ///////////////////////////////////////////////////////////
void loop() {
    Serial.println("Restarted");
    Wifi();
    WiFiClient client = server.available();
    //if(!client)return;
    while(1==1){
         if(client ==true && client.isSendWaiting()==false) {
              Time_1=micros();
              client.print(DataSent+ '\r');
              Time_1 = (micros()-Time_1);
              Time_1_display = Time_1;Time_1_display =Time_1_display/1000000;
              if(Time_1>Time_1_max){Time_1_max=Time_1;}
              Time_1_max_display = Time_1_max;
              Time_1_max_display =Time_1_max_display/1000000;
              Wifi_Failue_cnt = 0;
              cnt=cnt+1;
              DataSent = cnt;  
          }  
          else {Fail_ID=2;Failure_Detection();}
          pgm();
          Serial_print();
          if(Wifi_Status ==2){
            Wifi_Failue_cnt = 0;
            Wifi_Status =3;
            return;
          }
      }   
} 
