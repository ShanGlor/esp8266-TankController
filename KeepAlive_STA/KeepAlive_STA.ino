#include <ESP8266WiFi.h>       
/////////////////////////// Declarations Debugging /////////////////////////////////////////////////
int cnt = 0;
unsigned long   Time_1=0,Time_1_prev =0,Time_1_max=0;
float           Time_1_display,Time_1_max_display;
unsigned long   Time_2=0,Time_2_prev =0,Time_2_max=0;
float           Time_2_display,Time_2_max_display;
unsigned long   prev_time =0;

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

/////////////////////////// Declarations Wifi /////////////////////////////////////////////////////
const char* ssid          = "TANK_TURRET_AP";
const char* password      = "12345679";
const char* host          = "192.168.4.1";
const int httpPort = 80;
WiFiClient client;
String DataRecieved="0";
int Wifi_Status =0;//......0=OFF, 1=ON, 2=FAILD,3=START .........................................//

/////////////////////////// Declarations Failure_Detection ////////////////////////////////////////
int RC_Failure_cnt=0;
int Wifi_Failue_cnt=0;
int RC_cnt_Trigger = 4;//.................. Missed Signals before Safe mod for RC ................//
int Wifi_cnt_Trigger = 500;//.............. Missed transmissions before reseting Wyfi ............//
int Fail_ID;//........... 1=RC, 2=Wifi, 3=Gyro ...................................................//

//////////////////////////////////////// Failure Dection ROUTINE ///////////////////////////////////
void Failure_Detection(){
    if(Fail_ID==1){//.................................... ID1=RC .................................//
        RC_Failure_cnt=RC_Failure_cnt+1;RC_Status = 2;// failed to complete in time for RC Rx ....//
        if(RC_Failure_cnt > RC_cnt_Trigger){//...................... RC in SAFE mode .............//
            RC_Status = 3;RC_Failure_cnt=RC_Failure_cnt-1;}
    }
    if(Fail_ID==2){
        Wifi_Failue_cnt = Wifi_Failue_cnt + 1;
        if(Wifi_Failue_cnt > Wifi_cnt_Trigger-1){
            Wifi_Status =2;//......0=OFF, 1=ON, 2=FAILD,3=START ................................//
            Wifi_Failue_cnt = Wifi_cnt_Trigger;
        }
  }
  
}
//////////////////////////////////////// RC Rx ROUTINE ////////////////////////////////////////////
void pgm(){
    digitalWrite(BUILTIN_LED, HIGH);//....... RED LED OFF .......................................//
    delay(20);//............................ Simulates RC control routine .......................//
    digitalWrite(BUILTIN_LED, LOW);//....... RED LED ON .........................................//
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
void Wifi(){
    WiFi.disconnect();delay(500);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    delay(500);
    while ( WiFi.status() != WL_CONNECTED){Serial.print(".");pgm();}
    Serial.println(WiFi.localIP());  
    if(!client.connect(host, httpPort)){return;}
    client.setTimeout(0);
    Wifi_Status =1;//......0=OFpgm();F, 1=ON, 2=FAILD,3=START .........................................//
    Wifi_Failue_cnt = 0;
}


void Serial_print(){
    Serial.print("STA Data Rec'd(");
    Serial.print(DataRecieved); Serial.print(") ");
    Serial.print("\t");
    Serial.print("Status(s); ");
    Serial.print("Wifi(");Serial.print(Wifi_Status);Serial.print(") ");
    Serial.print("\t");
    Serial.print("Fail_cnt(s); ");
    Serial.print("Wifi(");Serial.print(Wifi_Failue_cnt);Serial.print(") ");
    Serial.println();
  
}

//////////////////////////////////////// SETUP ROUTINE ////////////////////////////////////////////
void setup() {
    pinMode(BUILTIN_LED, OUTPUT);
    Serial.begin(250000);
    //.................. delay serial communication to prevent MS Window ........................//
    delay(500);//....... fron thinking it a serial mouse ........................................//
    Serial.println();Serial.println();
}
//////////////////////////////////////// LOOP ROUTINE /////////////////////////////////////////////
void loop() {
    Serial.println("Loop restarted");
    Wifi();
    while(1==1){
        RC();
        Time_1 = micros();
        DataRecieved = client.readStringUntil('\r');//....Timeout is set to ZERO
        if(DataRecieved !=""){
            cnt=cnt+1;
            Wifi_Status =1;//......0=OFF, 1=ON, 2=FAILD,3=START .................................//
            Wifi_Failue_cnt = 0;
        }
        else{Fail_ID=2;Failure_Detection();}
        //pgm();//...................................... Delay required for Wifi .................//
        Serial_print();
        if(Wifi_Status ==2){
            Wifi_Failue_cnt = 0;
            Wifi_Status =3;
            return;
        }
        //pgm();//...................................... Delay required for Wifi .................//
      }
   
     
}
