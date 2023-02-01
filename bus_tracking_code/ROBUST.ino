/////////////////////////////////////////////////////////////////////////////
#include <SoftwareSerial.h>
#include <LiquidCrystal.h> // lcd
SoftwareSerial sim808(10, 11);//tx 11 rx 10
LiquidCrystal lcd(13, 8, 4, 5, 6, 7);
//////////////////////////////////////////////////////////////////////////////
String id = "63b481d912af33826763027d"; //bus ID
//////////////////////////////////////////////////////////////////////////////
//JsonObject BUS = doc.to<JsonObject>();
//BUS["id"] = id;
#define DEBUG true
#define PowerOn 9 //power pin
///////////////////////////////////////////////////////////////////////////////
int buttonPin = 2;
int buttonState = 0; // 0 -> off 1 -> on
//float lat , lon; // latitude , longitude
///////////////////////////////////////////////////////////////////////////////
String url = "http://robust.u-control.net/addData";  //URL of Server
String apn = "internet.vodafone.net";           //APN
String apn_u = "internet";                     //APN-Username
String apn_p = "internet";                     //APN-password
String data[5];
String state, timegps, latitude, longitude;
String response;
///////////////////////////////////////////////////////////////////////////////
void setup() {
///////////////////////////////////////////////////////////////////////////////
  lcd.begin(16, 2);
  pinMode(PowerOn, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(12, HIGH);
  analogWrite(13, 150);
  sim808.begin(9600);
  Serial.begin(9600);
  power_on();
/////////////////////////////////////////////////////////////////////////////////
  Serial.println("Initializing..........");
  lcd.print("Initializing..........");
  delay(100);
  response = sendData("AT", 1000, DEBUG);
  while (response.indexOf("OK") == -1) {
    response = sendData("AT", 1000, DEBUG);
    Serial.println("ERROR");
    lcd.print("ERROR");
    lcd.clear();
    delay(100);
  }
  delay(100);
  response = sendData("AT+CGNSPWR=1", 1000, DEBUG);
  while (response.indexOf("OK") == -1) {
    response = sendData("AT+CGNSPWR=1", 1000, DEBUG);
    Serial.println("ERROR");
    lcd.print("ERROR");
    lcd.clear();
    delay(100);
  }
  delay(100);
  response = sendData("AT+CGPSSTATUS?",1000,DEBUG);
    while(response.indexOf("is")==-1){
        response = sendData("AT+CGPSSTATUS?",1000,DEBUG); // wether is not fixed or not.
        Serial.println("Location Not Fix");
        lcd.print("Location Not Fix");
        lcd.clear();
        delay(100);
   }
  delay(100);
  response = sendData("AT+CPIN?",1000,DEBUG);
  while(response.indexOf("OK")==-1){
      response = sendData("AT+CPIN?",1000,DEBUG);
      Serial.println("ERROR");
      lcd.print("ERROR");
      lcd.clear();
      delay(100);
   }
  delay(100);
  gsm_config_gprs() //to set up gprs configuration
  lcd.clear();
}
//////////////////////////////////////////////////////////////////////////////////
void loop() {
  
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH){
    latitude = 0.00;
    longitude = 0.00;
    sendTabData("AT+CGNSINF", 1000, DEBUG);
    Serial.println("lat: "+latitude+" lon: "+longitude);
    BUS["lon"] = longitude;
    BUS["lat"] = latitude;
    gsm_http_post(BUS); // send the JSON object over http via post req
    delay(500);
  }
}
void sendTabData(String command , const int timeout , boolean debug) {
  sim808.println(command);
  long int time = millis();
  int i = 0;
  while ((time + timeout) > millis()) {
    while (sim808.available()) {
      char c = sim808.read();
      if (c != ',') {
        data[i] += c;
        delay(100);
      } else {
        i++;
      }
      if (i == 5) {
        delay(100);
        goto exitL;
      }
    }
} exitL:
  if (debug){
    Serial.println(data[3]);
    Serial.println(data[4]);
    state = data[1];
    timegps = data[2];
    latitude = data[3];
    longitude = data[4];
  }
}
///////////////////////////////////////////////////////////////////////////////////////
String sendData(String command , const int timeout , boolean debug) {
  String response = "";
  sim808.println(command);
  long int time = millis();
  int i = 0;
  while ((time + timeout) > millis()) {
    while (sim808.available()) {
      char c = sim808.read();
      response += c;
    }
  }
  if (debug) {
    Serial.print(response);
  }
  return response;
}
////////////////////////////////////////////////////////////////////////////////////////
void power_on() {
  digitalWrite(PowerOn, LOW);
  delay(500);
  digitalWrite(PowerOn, HIGH);
  delay(2000);
  digitalWrite(PowerOn, LOW);
  delay(3000);
}
////////////////////////////////////////////////////////////////////////////////////////
void gsm_send_serial(String command) {
  Serial.println("Send ->: " + command);
  sim808.println(command);
  long wtimer = millis();
  while (wtimer + 3000 > millis()) {
    while (sim808.available()) {
      Serial.write(sim808.read());
    }
  }
  Serial.println();
}
/////////////////////////////////////////////////////////////////////////////////////////
void gsm_http_post( String postdata) {
  Serial.println(" --- Start GPRS & HTTP --- ");
  gsm_send_serial("AT+SAPBR=1,1");
  gsm_send_serial("AT+SAPBR=2,1");
  gsm_send_serial("AT+HTTPINIT");
  gsm_send_serial("AT+HTTPPARA=CID,1");
  gsm_send_serial("AT+HTTPPARA=URL," + url);
  gsm_send_serial("AT+HTTPPARA=CONTENT,application/x-www-form-urlencoded");
  gsm_send_serial("AT+HTTPDATA=192,5000");
  gsm_send_serial(postdata);
  gsm_send_serial("AT+HTTPACTION=1");
  gsm_send_serial("AT+HTTPREAD");
  gsm_send_serial("AT+HTTPTERM");
  gsm_send_serial("AT+SAPBR=0,1");
}
/////////////////////////////////////////////////////////////////////////////////////////////
void gsm_config_gprs() {
  Serial.println(" --- CONFIG GPRS --- ");
  gsm_send_serial("AT+SAPBR=3,1,Contype,GPRS");
  gsm_send_serial("AT+SAPBR=3,1,APN," + apn);
  if (apn_u != "") {
    gsm_send_serial("AT+SAPBR=3,1,USER," + apn_u);
  }
  if (apn_p != "") {
    gsm_send_serial("AT+SAPBR=3,1,PWD," + apn_p);
  }
}
//////////////////////////////////////////////////////////////////////////////////////////////
