#include "IoTNode.h"
#include "WeatherLevel.h"
#include "MQTT.h"

SYSTEM_THREAD(ENABLED);
// SYSTEM_MODE(MANUAL);

#define SERIAL_DEBUG

// Defined the total distance in inches from the head of the range sensor to the bottom of the muck
float RANGE_DISTANCE_TO_BOTTOM = 140;

#ifdef SERIAL_DEBUG
  #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
  #define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
  #define DEBUG_PRINTLNF(...) Serial.printlnf(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...)
  #define DEBUG_PRINTLN(...)
  #define DEBUG_PRINTF(...)
  #define DEBUG_PRINTLNF(...)
#endif

#define UNITS_US

typedef struct {
    float range;

    float windMPH;
    float gustMPH;
    float windDegrees;

    float airTemp;
    float airHumidity;
    float airPressure;

    float rain;

    float mv;
    bool isCharged;
    bool isCharging;


} reading_t;
reading_t reading;

String i2cDevices = "";

String i2cNames[] = {
    "RTC",
    "Exp",
    "RTC EEPROM",
    "ADC",
    "FRAM",
    "AM2315",
    "MPL3115"
};

byte i2cAddr[] = {
    0x6F, //111
    0x20, //32
    0x57, //87
    0x4D, //77
    0x50, //80
    0x5C, //
    0x60
};

/* number of elements in `array` */
static const size_t i2cLength = sizeof(i2cAddr) / sizeof(i2cAddr[0]);

bool i2cExists[] = {
  false,
  false,
  false,
  false,
  false,
  false,
  false
};

#define SENSOR_COLLECT_SEND_MA 6000 // Standard 60000
bool collectSensorsNow = false;
void setCollectensorsFlag() {
  collectSensorsNow = true;
}
Timer collectSensorTimer(SENSOR_COLLECT_SEND_MA, setCollectensorsFlag);

#define SENSOR_SEND_RATE_MA 60000 // Standard 60000
bool sendSensorsNow = false;
void setSendSensorsFlag() {
  sendSensorsNow = true;
}
Timer sendSensorTimer(SENSOR_SEND_RATE_MA, setSendSensorsFlag);

IoTNode node;
Maxbotix maxbotix(node, 99);
Weather weatherSensors;

const uint8_t mqttServer[] = { 172, 17, 1, 1 };
MQTT mqClient(mqttServer, 1883, mqCallback);
void mqCallback(char* topic, byte* payload, unsigned int length) {
  // Ignoring mqtt callbacks
}

void collectSensors() {
  weatherSensors.capturePressure();
  weatherSensors.captureAirTempHumid();
  weatherSensors.captureWindVane();  
  weatherSensors.captureTempHumidityPressure();
  weatherSensors.captureWaterTemp();
  weatherSensors.captureBatteryVoltage();
  DEBUG_PRINTLN("Collected sensors");
}

void readRange(reading_t &reading) {
  float range;
  range = (float)maxbotix.range1Median();
  if (!maxbotix.isValid() || range <= 0.1) {
    DEBUG_PRINTLN("Maxbotix timeout"); 
    return;
  }
  
  // Convert to inches and find the real depth
  reading.range = RANGE_DISTANCE_TO_BOTTOM - (range * 0.0393701);
}

void readWind(reading_t &reading) {
  reading.windMPH = weatherSensors.getAndResetAnemometerMPH(&reading.gustMPH);
  reading.windDegrees = weatherSensors.getAndResetWindVaneDegrees();
  reading.rain = weatherSensors.getAndResetRainInches();
}

void readAir(reading_t &reading) {
  //TODO: I think there are stats we can pull off the node too
  reading.airTemp = weatherSensors.getAndResetTempF();
  reading.airHumidity = weatherSensors.getAndResetHumidityRH();
  reading.airPressure = weatherSensors.getAndResetPressurePascals() * 0.0002953;
}

void readPower(reading_t &reading) {
  reading.mv = node.voltage();
  reading.isCharged = node.isLiPoCharged();
  reading.isCharging = node.isLiPoCharging();
}

void sendSensors() {
  readRange(reading);
  readWind(reading);
  readAir(reading);

  String status = "";
  status += String::format("battery=%f;", reading.mv); //TODO
  status += String::format("humidity=%f;", reading.airHumidity);

  status += String::format("liPoCharged=%i;", reading.isCharged); //TODO
  status += String::format("liPoCharging=%i;", reading.isCharging); //TODO
  status += String::format("pressure=%f;", reading.airPressure);
  status += String::format("rain=%f;", reading.rain);
  status += String::format("waterLevel=%f;", reading.range); //Verify

  status += String::format("windDir=%f;", reading.windDegrees);
  status += String::format("windGust=%f;", reading.gustMPH);
  status += String::format("windSpeed=%f;", reading.windMPH);

  DEBUG_PRINTLN(status);
  mqClient.publish("/weatherStation/state", status, true)
}

void connect() {
  if (!WiFi.hasCredentials()) {
    DEBUG_PRINTLN("Please add WiFi credentials");
    DEBUG_PRINTLN("Resetting in 60 seconds");
    delay(60000);
    System.reset();
  }

  if (!WiFi.ready()) {
    DEBUG_PRINTLN("Attempting to connect to WiFi...");
    WiFi.on();
    WiFi.connect();
    waitFor(WiFi.ready, 60000);
    if (!WiFi.ready()) {
      DEBUG_PRINTLN("WiFi not ready - resetting");
      delay(200);
      System.reset();
    }

  } else {
    DEBUG_PRINTLN("WiFi ready");
  }

  if (!Particle.connected()) {
    DEBUG_PRINTLN("Attempting to connect to Particle...");
    Particle.connect();
    // Note: that conditions must be a function that takes a void argument function(void) with the () removed,
    // e.g. Particle.connected instead of Particle.connected().
    waitFor(Particle.connected, 90000);
    if (!Particle.connected()) {
      DEBUG_PRINTLN("Particle not connected - resetting");
      delay(200);
      System.reset();
    } 

  } else {
    DEBUG_PRINTLN("Particle connected");
    Particle.publishVitals(600);
  }
}

// check i2c devices with i2c names at i2c address of length i2c length returned in i2cExists
bool checkI2CDevices() {
  byte error, address;
  bool result = true;
  i2cDevices = "";
  for (size_t i=0; i<i2cLength; ++i) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    address = i2cAddr[i];
    WITH_LOCK(Wire) {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
    }

    //Try again if !error=0
    if (!error == 0) {
      delay(10);
      WITH_LOCK(Wire) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
      }
    }

    //Try reset if !error=0
    if (!error == 0) {
      WITH_LOCK(Wire) {      
        Wire.reset();
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
      }
    }
 
    if (error == 0) {
      DEBUG_PRINTLN(String("Device " + i2cNames[i] + " at address:0x" + String(address, HEX)));
      i2cExists[i] = true;
      i2cDevices.concat(i2cNames[i]);
      i2cDevices.concat(":");

    } else {
      DEBUG_PRINTLN(String("Device " + i2cNames[i] + " NOT at address:0x" + String(address, HEX)));
      i2cExists[i] = false;
      result = false;
      i2cDevices.concat(i2cNames[i]);
      i2cDevices.concat("-missing:");
    }
  }
  return result;
}


void printI2C(int inx) {
  for (uint8_t i=0; i < i2cLength; i++) {
    if (i2cAddr[i] == inx) {
      DEBUG_PRINTLN(String("Device " + i2cNames[i] + " at address:0x" + String(i2cAddr[i], HEX)));
    }
  }        
}

void scanI2C() {
  byte error, address;
  int nDevices;
 
  DEBUG_PRINTLN("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    WITH_LOCK(Wire) {
      Wire.beginTransmission(address);
      error = Wire.endTransmission();
    }
 
    if (error == 0) {
      printI2C(address);
      nDevices++;
    } else if (error == 4) {
      DEBUG_PRINT("Unknown error at address 0x");
      if (address < 16) {
        DEBUG_PRINT("0");
      }
      DEBUG_PRINTLN(address, HEX);
    }    
  }

  if (nDevices == 0) {
    DEBUG_PRINTLN("No I2C devices found");
  } else {
    DEBUG_PRINTLN("done");
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setup() runs once, when the device is first turned on.
void setup() {
  Particle.function("systemreset", systemReset);

  String err = trySetup();
  connect();
  if (err.length() > 0) {
    if (waitFor(Particle.connected, 10000)) {
      Particle.publish("Error", "IoT Node expander not responding, resetting.", PRIVATE);
    } else {
      DEBUG_PRINTLN("Could not connect to particle and had error during setup.");
    }

    System.reset();
    return;
  }

  if (Particle.connected()) {
    Particle.publish("Started", i2cDevices, PRIVATE);

  } else {
    DEBUG_PRINTLN("Starting measurements while attempting to connect.");
  }

  mqClient.connect("WeatherStation");
  if (mqClient.isConnected()) {
    Particle.publish("Started with MQTT", i2cDevices, PRIVATE);
  }
  weatherSensors.begin();
  collectSensorTimer.start();
  sendSensorTimer.start();
}

String trySetup() {
  pinMode(D1, OUTPUT);
  for(int i = 0; i < 15; i++) {
    digitalWrite(D1, HIGH);
    delayMicroseconds(100);
    digitalWrite(D1, LOW);
    delayMicroseconds(100);
  }

  pinMode(D1, INPUT);

  Serial.begin(115200);
  #ifdef SERIAL_DEBUG
  delay(1000);
  #endif
  DEBUG_PRINTLN("Starting");

  Wire.begin();
  if (!node.begin()) {
    delay(1000);
    return String("oT Node expander not responding");

  } else {
    DEBUG_PRINTLN("IoT Node expander responded");
  }

  node.setPowerON(EXT3V3, true);
  node.setPowerON(EXT5V, true);

  delay(1000);

  checkI2CDevices();
  if (!node.ok()) {
    return String("IoT Node not ok.");

  } else {
    DEBUG_PRINTLN("IoT Node connected");
  }

  if (!(maxbotix.setup()>0)) {
    return String("No Maxbotix sensor detected.");

  } else {
    DEBUG_PRINTLN("Maxbotix sensors connected");
  }

  return String("");
}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  maxbotix.readMaxbotixCharacter();

  if (collectSensorsNow) {
    collectSensors();
    collectSensorsNow = false;
  }

  if (sendSensorsNow) {
    sendSensors();
    sendSensorsNow = false;
  }

  if (mqClient.isConnected()) {
    mqClient.loop();
  }
}

int systemReset(String command) {
  if (command.equals("systemreset")) {
    System.reset();
  }
  return 0;  
}
