/*
   From github https://gist.github.com/igrr
   21 oct 2017 modified by LEMD49 maricq@ieee.org

   Status: WORKS

   Tested ok working for publish and subcribe. Sending messages from terminal on OSX (10.13) with local MOSQUITTO
   using mosquitto_pub -t testin -m "1" (activates LED) or "0" (deactivates LED)

   Verified on nodeMCU from Amica and on D1 mini Wemos clones

   I could not make the example from PubSub lib work as got consistent rc -2 errors. By elimination the sketch example has error.
   Monitoring on the serial monitor and also open a terminal window after ssh in MQTT server and enter
   mosquitto_sub -v -t testout
*/

#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <credentials.h>

//const char* ssid = "?"; //in case not using a credentials library
//const char* password = "?";
const char* topicOut = "testout";
const char* topicIn = "testin";  // put "#" to monitor all traffic inbound from MQTT broker
char* server = "10.0.8.50"; //need ip as server.local did not worl
const char* seed = "LEMD49-"; //seed naming for Client ID

WiFiClient wifiClient;

// Callback function to handle message arrived
void callback(char* topicIn, byte* payload, unsigned int length) {
  Serial.print("Message arrived from topic [");
  Serial.print(topicIn);
  Serial.print("] with payload: ");
  for (int i = 0; i < length; i++) {
    Serial.println((char)payload[i]);
    Serial.println("Subscribed ok");
  }
 
  if ((char)payload[0] == '1' && length == 1) {   // Switch on the LED if an 1 was received as first and only character
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (reverse logic)
  } else {
    if ((char)payload[0] == '0' && length == 1) {
      digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    }
  }
}
// END Callback function

PubSubClient client(server, 1883, callback, wifiClient);  //instantiate object client

// <<<SET UP>>>
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);  // Serial monitor setup
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);  // WiFi setup
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(",");
  }
  Serial.println("");
  Serial.print("WiFi connected to ");
  Serial.print("IP address: ");
  Serial.print(WiFi.localIP());
  Serial.print(" with RSSI: ");
  Serial.println(WiFi.RSSI());
  Serial.print("and MAC address: ");
  Serial.println(WiFi.macAddress());


  // Generate client name
  String clientName;
  clientName += "LEMD49-";
  clientName += String(micros() & 0xff, 16);

  Serial.print("Connecting to MQTT server ");
  Serial.print(server);
  Serial.print(" as clientID (CID): ");
  Serial.println(clientName);

  if (client.connect((char*) clientName.c_str())) {
    Serial.println("Connected to MQTT server");
    Serial.print("Topic is: ");
    Serial.println(topicOut);
    const char* welcomeMessage;
    welcomeMessage = "Hello from ESP8266 ";

    if (client.publish(topicOut, welcomeMessage)) {
      Serial.println("Publish ok");
    }
    else {
      Serial.println("Publish failed (setup)");
    }
    if (client.subscribe(topicIn)) {
      Serial.println("Subscribe ok");
    }
    else {
      Serial.println("Subscribe failed");
    }
  }
  else {
    Serial.println("MQTT connect failed");
    Serial.println("Will reset and try again...");
    abort();
  }
  Serial.println (". . . . . ");
}

// <<<MAIN LOOP>>>
void loop() {
  client.loop();  // need to sub
  static int counter = 0;
  // Formating pub in JSON format
  String payload = "{\"micros\":";
  payload += micros();
  payload += ",\"counter\":";
  payload += counter;
  payload += "}";
  // END of formating

  if (client.connected()) {
    Serial.print("Sending from topic [");
    Serial.print(topicOut);
    Serial.print("], the payload: ");
    Serial.println(payload);


    if (client.publish(topicOut, (char*) payload.c_str())) {
      Serial.println("Publish ok");
    }
    else {
      Serial.println("Publish failed (loop)");
    }
  }
  ++counter;
  delay(4000);
}


