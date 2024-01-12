#include <Arduino.h>
#include <JY901.h>
#include <FS.h>
#include <SD.h>
#include <Wire.h>
#include <FastLED.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#define NUM_LEDS 1
#define DATA_PIN 21

CRGB leds[NUM_LEDS];

const int save_interval = 60000; // Save data every minute
const int wifi_interval = 3000; // Send data via WiFi every 3 seconds
const int save_time = 30; // Number of samples saved (with a sampling time of 500ms) for a 15s recording
const int recive_time = 500; // Get data every 0.5 seconds
unsigned long last_send_time = 0;

File file;

struct Button {
    const uint8_t PIN;
    uint32_t numberKeyPresses;
    bool pressed;
};

Button button1 = {2, 0, false};

String nomeFile = ""; // String that will contain the file path on the SD card

bool rec_on = false;
unsigned long lastSaveTime = 0;

int n_rec = 0;

// Set these to your desired credentials.
const char *ssid = "Automa";
const char *password = "12345678";

// set wifi port
WiFiServer server(80);
WiFiClient client;

void IRAM_ATTR isr() {
  button1.pressed = true;
}

void saveData() {
  float time = (float)millis()/1000;
  float ax = (float)JY901.stcAcc.a[0]/32768*16;
  float ay = (float)JY901.stcAcc.a[1]/32768*16;
  float az = (float)JY901.stcAcc.a[2]/32768*16;
  float gx = (float)JY901.stcGyro.w[0]/32768*2000;
  float gy = (float)JY901.stcGyro.w[1]/32768*2000;
  float gz = (float)JY901.stcGyro.w[2]/32768*2000;
  float mx = (float)JY901.stcMag.h[0];
  float my = (float)JY901.stcMag.h[1];
  float mz = (float)JY901.stcMag.h[2];
  float lon = JY901.stcLonLat.lLon;
  float lat = JY901.stcLonLat.lLat;
  float SN = JY901.stcSN.sSVNum;
  float PDOP = (float)JY901.stcSN.sPDOP/100;
  float HDOP = (float)JY901.stcSN.sHDOP/100;
  float VDOP = (float)JY901.stcSN.sVDOP/100;
  float q0 = JY901.stcQuater.q0;
  float q1 = JY901.stcQuater.q1;
  float q2 = JY901.stcQuater.q2;
  float q3 = JY901.stcQuater.q3;
  float norm_q = sqrt(q0*q0+q1*q1+q2*q2+q3*q3);

  q0 /=  norm_q;
  q1 /=  norm_q;
  q2 /=  norm_q;
  q3 /=  norm_q;

  file.println(String(n_rec) + "," + String(time) + "," + String(ax) + "," + String(ay) + "," + String(az) + "," + String(gx) + "," + String(gy) + "," + String(gz) + "," + 
    String(mx) + "," + String(my) + "," + String(mz) + "," + String(lon) + "," + String(lat) + "," + String(SN) + "," + String(PDOP) + "," + 
    String(HDOP) + "," + String(VDOP) + "," + String(q0) + "," + String(q1) + "," + String(q2) + "," + String(q3));
  
  delay(recive_time);
}

int findNextFileNumber(String baseName) {
  int fileNumber = 1;

  while (SD.exists(baseName + String(fileNumber) + ".csv")) {
    //Serial.println(String(fileNumber));
    fileNumber++;
  }
  Serial.println("File regitrazione_"+String(fileNumber)+".csv creato");
  return fileNumber;
}

void registrazione(){
  int write_time = 0;
  delay(5);
  file = SD.open(nomeFile, FILE_APPEND);
  n_rec++;
  Serial.println("Avvio registrazione numero "+String(n_rec)+"\n");
  file.println("Avvio registrazione numero "+String(n_rec)+"\n");
  if (client && client.connected()){
    client.print("Avvio registrazione numero "+String(n_rec));
  }
  while(true){
    saveData();
    sent_data_Client();
    write_time++;
    if(write_time == save_time){
      file.println("\nFine registrazione numero "+String(n_rec)+"\n");
      file.close();
      Serial.println("Fine Registrazione numero "+String(n_rec)+"\n");
      if (client && client.connected()){
        client.print("Fine registrazione numero "+String(n_rec));
      }
    
      return;
    
    }
    while (Serial1.available()) {
    JY901.CopeSerialData(Serial1.read());
    }  

  }
}

void sent_data_Client(){
  float time = (float)millis()/1000;
  float PDOP = (float)JY901.stcSN.sPDOP/100;
  float q0 = JY901.stcQuater.q0;
  float q1 = JY901.stcQuater.q1;
  float q2 = JY901.stcQuater.q2;
  float q3 = JY901.stcQuater.q3;
  float lon = JY901.stcLonLat.lLon;
  float lat = JY901.stcLonLat.lLat;
  float norm_q = sqrt(q0*q0+q1*q1+q2*q2+q3*q3);

  q0 /=  norm_q;
  q1 /=  norm_q;
  q2 /=  norm_q;
  q3 /=  norm_q;
  
  client.print(String(time) + "," + String(PDOP) + "," + String(lon) + "," + String(lat) + "," + String(q0) + "," + String(q1) + "," + String(q2) + "," + String(q3));
}


void setup() {
  Serial.begin(115200);
  Serial.println("Beginning Setup...");
  Serial1.begin(115200, SERIAL_8N1, 44, 43); // Serial connection to the WTGAHRS2 module

  pinMode(2, INPUT_PULLUP);                  // Connect the button to pin 2
  attachInterrupt(digitalPinToInterrupt(2), isr, RISING);

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  

  while(!SD.begin()) {
    Serial.println("Card Mount Failed");
    leds[0] = CRGB::Red; 
    FastLED.show();
    delay(5000);
  }
  delay(5000); // Wait for 5 seconds before generating the file
  
  nomeFile = "/registrazione_" + String(findNextFileNumber("/registrazione_")) + ".csv";
  file = SD.open(nomeFile, FILE_WRITE);
  file.println("N Reg,Time,ax,ay,az,gx,gy,gz,mx,my,mz,lon,lat,SN,PDOP,HDOP,VDOP,q0,q1,q2,q3");
  delay(5);

  Serial.println("Configuring access point...");
  
  Serial.print("SSID=");
  Serial.print(ssid);
  Serial.print("\n");
    
  // Use the password parameter if you want the AP to be secured.
  WiFi.softAP(ssid, password);
  Serial.print("PW=");
  Serial.print(password);
  Serial.print("\n");

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: "); // IP to connect to
  Serial.println(myIP);
  server.begin();

  Serial.println("Server started");

  Serial.println("End of setup");
  leds[0] = CRGB::Blue; 
  FastLED.show();
}

void loop() {
  if (!client || !client.connected()) {
    client = server.available();
    delay(10);  // Brief delay for stability
  }

  if(button1.pressed && !rec_on){
    file.close();
    rec_on = true;
    registrazione();
    button1.pressed = false;
    rec_on = false;
    file = SD.open(nomeFile, FILE_APPEND);
  }

  saveData();

  while (Serial1.available()) {
    JY901.CopeSerialData(Serial1.read()); // Call JY901 data cope function
  }

  unsigned long currentMillis = millis();

  if(currentMillis - lastSaveTime >= save_interval){
    Serial.println("Saving file");
    lastSaveTime = currentMillis;
    file.close();
    delay(5);
    file = SD.open(nomeFile, FILE_APPEND);
  }

  if (currentMillis - last_send_time >= wifi_interval) {
    last_send_time = currentMillis;

    if (client && client.connected()) {

      // Add connection debug
      sent_data_Client();
      Serial.println("Data sent to MATLAB");
    } else {
      // Add connection debug
      Serial.print(".");
    }
  }
}