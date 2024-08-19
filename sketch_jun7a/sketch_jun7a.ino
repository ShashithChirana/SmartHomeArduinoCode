#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include "DHT.h"

#define FIREBASE_HOST "iotproject-9cf99-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "wfogCpqXSEgRqPPBdlj8Tj3mvSqxpiPCOhthL6Uh"
#define WIFI_SSID "Dialog 4G 175"
#define WIFI_PASSWORD "3iJeLQiM"

#define DHTPIN D1
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);


const int BULB1_PIN = D2;  
const int BULB2_PIN = D3;  
const int BULB3_PIN = D4;  
const int FAN_PIN = D5;    
const int SWITCH1_PIN = D6; 
const int SWITCH2_PIN = D7; 

bool lastSwitch1State = HIGH; 
bool lastSwitch2State = HIGH; 

void setup() {
  pinMode(BULB1_PIN, OUTPUT);
  pinMode(BULB2_PIN, OUTPUT);
  pinMode(BULB3_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(SWITCH1_PIN, INPUT_PULLUP); // Configure switch pin with internal pull-up
  pinMode(SWITCH2_PIN, INPUT_PULLUP); 
  
  Serial.begin(9600);

  Serial.println(F("DHTxx test!"));
  dht.begin(); 
  delay(500);
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.println("Connected to WiFi.");
  Serial.println(WiFi.localIP());
  
  // Connect to Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  // Read the values from Firebase
  bool firebaseBulb1 = Firebase.getBool("devices/bulb_01");
  bool firebaseBulb2 = Firebase.getBool("devices/bulb_02");
  bool bulb3 = Firebase.getBool("devices/bulb_03");
  bool fan = Firebase.getBool("devices/fan");

  // Check for errors in fetching data from Firebase
  if (Firebase.failed()) {
    Serial.print("Failed to read from Firebase: ");
    Serial.println(Firebase.error());
    delay(500); // Retry after a short delay
    return;
  }

  // Read the switch states
  bool switch1State = digitalRead(SWITCH1_PIN) == LOW; // Assuming LOW when pressed
  bool switch2State = digitalRead(SWITCH2_PIN) == LOW; // Assuming LOW when pressed

  // Debug prints for switch states
  Serial.print("Switch 1 State: ");
  Serial.println(switch1State);
  Serial.print("Switch 2 State: ");
  Serial.println(switch2State);

  // Determine the final state for bulb1 based on switch and Firebase
  bool bulb1State;
  if (switch1State != lastSwitch1State) {
    // If the switch state changes, use the switch state
    bulb1State = switch1State;
    // Update Firebase to reflect the switch change
    Firebase.setBool("devices/bulb_01", switch1State);
    if (Firebase.failed()) {
      Serial.print("Failed to update Firebase: ");
      Serial.println(Firebase.error());
    }
  } else {
    // If the switch state hasn't changed, use the Firebase state
    bulb1State = firebaseBulb1;
  }

  // Determine the final state for bulb2 based on switch and Firebase
  bool bulb2State;
  if (switch2State != lastSwitch2State) {
    // If the switch state changes, use the switch state
    bulb2State = switch2State;
    // Update Firebase to reflect the switch change
    Firebase.setBool("devices/bulb_02", switch2State);
    if (Firebase.failed()) {
      Serial.print("Failed to update Firebase: ");
      Serial.println(Firebase.error());
    }
  } else {
    // If the switch state hasn't changed, use the Firebase state
    bulb2State = firebaseBulb2;
  }

  // Control the bulbs and fan
  digitalWrite(BULB1_PIN, bulb1State ? LOW : HIGH);
  digitalWrite(BULB2_PIN, bulb2State ? LOW : HIGH);
  digitalWrite(BULB3_PIN, bulb3 ? LOW : HIGH);
  digitalWrite(FAN_PIN, fan ? LOW : HIGH);

  // Remember the last switch states
  lastSwitch1State = switch1State;
  lastSwitch2State = switch2State;

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C"));

      // Send data to Firebase
  Firebase.setFloat("humidity", h);
  Firebase.setFloat("temperature", t);

  if (Firebase.failed()) {
    Serial.print(F("Setting failed: "));
    Serial.println(Firebase.error());
  }
  

  delay(50);
}
