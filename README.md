# esp8266-TankController
RC Tank Controller

Description of Project;
      To increase the capabilities of Hobby grade RC control. 
      
      Will be using One Hen Long RC Tank, One FS-TH9X RC radio using ER9X software and Two esp8266-12 and 
      Two FS-R9B receivers.

  Objectives Completed;
  
      1). Controlling Turret(Sketch KeepAlive_AP) and Hull functions(Sketch KeepAlive_STA) without 
          wiring connecting hull and turret. This will allow continuous 360 deg. rotations of turret. 
          Using Two esp8266's one in the turret and one in the Hull sending Wi-Fi Data from the 
          turret to the hull. 
          Note: (Tank function Outputs not yet added to sketches.)

      2). Add Gyro MPU6050 to Turret(Sketch KeepAlive_AP) and send by Wi-Fi Gyro Data from the 
          to Hull(Sketch KeepAlive_STA). 

      3).  Using the capabilities of FS-TH9X RC radio using ER9X software, transmit 6 switch 
           positions encoded on one channel and have  both esp8266's connected to the Rx modules  
           decode these switch position. This gives the ability of 6 full analog and the position  
           of 6 switches. A total of 13 functions on 8 channels. 
           Note:(Tank function OutputsOutputs not yet added to sketches.)

  Objective working on;

      Test using the esp8266's i/o controlling RC Tanks functions with the hope of replacing 
      the Arduino Due mentioned below.

  Objectives achieved already with an Arduino but not yet added to this project; 

      Using MPU6050 Gyro connected to Arduino Due to control cannon elevation and 
      turret rotation to remain fixed on target as hull is moving.


 Notes for this project;  
 
    The esp8266 sketch for the turret is KeepAlive_AP and the esp8266 sketch in the hull is KeepAlive_STA.
 
    The time for AP Wi-Fi to Transmit data to STA plus STA to send ACK to AP = approx.0.125ms.
    This would cause blocking of both AP and STA sketches of 0.125ms each Wi-Fi transmission this
    would make Safe RC control impossible.

    Because the Sketch blocking nature of the client.print in include <ESP8266WiFi.h> it is 
    not used in AP instead Non Blocking include "pfodESP8266WiFi.h" is used. Also print timeout
    in STA is set to zero. This stops the sketch blocking making RC control possible.

    AP Wifi_Failue_cnt = number of AP's client.print skipped and STA's client.reads that 
    timed out between Wi-Fi actual successful transmissions. Wifi_Failue_cnt in both AP and STA
    can be used as a gauge as RC and Gyro programming is added. If or when this count is constantly
    zero because of additional programming the frequency of transmission maybe decreasing.
