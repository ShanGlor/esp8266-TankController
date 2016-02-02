#include <ESP8266WiFi.h>


const char* ssid          = "TANK_TURRET_AP";
const char* password      = "12345679";
const char* host          = "192.168.4.1";
const int httpPort = 80;
WiFiClient client;

String DataRecieved="0";
int cnt = 0;
unsigned long   Time_1=0,Time_1_prev =0,Time_1_max=0;
float           Time_1_display,Time_1_max_display;
unsigned long   Time_2=0,Time_2_prev =0,Time_2_max=0;
float           Time_2_display,Time_2_max_display;
unsigned long   prev_time =0;

//////////////////////////////////////// RC Rx ROUTINE /////////////////////////////////////////////////////////
void pgm(){
    digitalWrite(BUILTIN_LED, HIGH);//............ RED LED OFF ...............................................//
    delay(20);//............................ Simulates RC control routine ....................................//
    digitalWrite(BUILTIN_LED, LOW);//............. RED LED ON ................................................//
}
//////////////////////////////////////// Wiifi Start ROUTINE ///////////////////////////////////////////////////
void Wifi(){
    digitalWrite(BUILTIN_LED, LOW);//............. RED LED ON ................................................//
    WiFi.disconnect();delay(100);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while ( WiFi.status() != WL_CONNECTED){Serial.print(".");delay(100);}
    Serial.println("WIFI connected");
    Serial.println(WiFi.localIP());  
    Serial.println("Waiting for Tank Wifi");
    if(!client.connect(host, httpPort)){return;}
    client.setTimeout(0);
    digitalWrite(BUILTIN_LED, HIGH);//............ RED LED OFF ...............................................//
}
//////////////////////////////////////// SETUP ROUTINE /////////////////////////////////////////////////////////
void setup() {
    pinMode(BUILTIN_LED, OUTPUT);
    Serial.begin(250000);
    //.................. delay serial communication to prevent MS Window .....................................//
    delay(500);//....... fron thinking it a serial mouse ....................................................//
    Serial.println();Serial.println();
}
//////////////////////////////////////// LOOP ROUTINE //////////////////////////////////////////////////////////
void loop() {
    Serial.println("Loop restarted");
    Wifi();
    while(1==1){
        //---------------------------- Keep the above client.readStringUntil() sequenced ---------------------//
         Time_1 = micros();
         DataRecieved = client.readStringUntil('\r');//....Timepout is set to ZERO
         if(DataRecieved !=""){
            Time_1=(micros()-Time_1);
            Time_1_display = Time_1;Time_1_display =Time_1_display/1000000;
            if(cnt>100){if(Time_1>Time_1_max)Time_1_max = Time_1;}
            Time_1_max_display = Time_1_max;
            Time_1_max_display =Time_1_max_display/1000000;
            Serial.print("STA TANK HULL"); Serial.print(" ");
            Serial.print(Time_1_display,4);    Serial.print(" ");
            Serial.print(Time_1_max_display,4);Serial.print(" ");
            Serial.println(DataRecieved); Serial.print(" ");
            cnt=cnt+1;
         }
         pgm();
      }
   
     
}
