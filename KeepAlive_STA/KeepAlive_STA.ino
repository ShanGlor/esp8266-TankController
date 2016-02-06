#include <ESP8266WiFi.h>

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
const char* host          = "192.168.4.1";
const int httpPort = 80;
WiFiClient client;
String DataRecieved="0";
int Wifi_Status =0;//......0=OFF, 1=ON, 2=FAILD,3=START .........................................//
/////////////////////////// Declarations Failure_Detection ////////////////////////////////////////
int Wiifi_cnt_Trigger = 500;//.............. The cnt in STA and in AP should the same? ..........//
int Wifi_Failue_cnt=0;
int Fail_ID;//........... 1=RC, 2=Wifi, 3=Gyro ..................................................//

//////////////////////////////////////// RC Rx ROUTINE ////////////////////////////////////////////
void pgm(){
    digitalWrite(BUILTIN_LED, HIGH);//....... RED LED OFF .......................................//
    delay(20);//............................ Simulates RC control routine .......................//
    digitalWrite(BUILTIN_LED, LOW);//....... RED LED ON .........................................//
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

void Failure_Detection(){
  if(Fail_ID==2){
        Wifi_Failue_cnt = Wifi_Failue_cnt + 1;
        if(Wifi_Failue_cnt > Wiifi_cnt_Trigger-1){
            Wifi_Status =2;//......0=OFF, 1=ON, 2=FAILD,3=START ................................//
            Wifi_Failue_cnt = Wiifi_cnt_Trigger;
        }
  }
  
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
        
        Time_1 = micros();
        DataRecieved = client.readStringUntil('\r');//....Timeout is set to ZERO
        if(DataRecieved !=""){
            cnt=cnt+1;
            Wifi_Status =1;//......0=OFF, 1=ON, 2=FAILD,3=START .................................//
            Wifi_Failue_cnt = 0;
        }
        else{Fail_ID=2;Failure_Detection();}
        pgm();//...................................... Delay required for Wifi .................//
        Serial_print();
        if(Wifi_Status ==2){
            Wifi_Failue_cnt = 0;
            Wifi_Status =3;
            return;
        }
        pgm();//...................................... Delay required for Wifi .................//
      }
   
     
}
