#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <WiFiClientSecure.h> 
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include "SdsDustSensor.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "certs.h"
int rxPin = D5;
int txPin = D6;
SdsDustSensor sds(rxPin, txPin);

Adafruit_BME680 bme680; // I2C
bool hasBME280 = false;

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

const char* host = "pulse.eco";
const char* server = "pulse.eco";
const char* eco_host = "pulse.eco";
const uint16_t eco_port = 443;

const char* fingerprint = "4E 9F 97 B8 6C 8F 70 C0 2A C9 6A 83 6D 5F 3B C7 81 C5 D6 3D";
#define SEALEVELPRESSURE_HPA (1013.25)


#ifdef DEBUG_PROFILE
  #define NUM_MEASURE_SESSIONS 10
  #define CYCLE_DELAY 2000
#else
  #define NUM_MEASURE_SESSIONS 30
  #define CYCLE_DELAY 30000
#endif

#define SH_DEBUG_PRINTLN(a) Serial.println(a)
#define SH_DEBUG_PRINT(a) Serial.print(a)
#define SH_DEBUG_PRINT_DEC(a,b) Serial.print(a,b)
#define SH_DEBUG_PRINTLN_DEC(a,b) Serial.println(a,b)

//Noise sensor pins
#define NOISE_MEASURE_PIN A0
#define NUM_NOISE_SAMPLES 1200

bool hasBME680 = true;
int temp = 0; 
int humidity = 0;
int pressure = 0;
int altitude = 0;
int gasResistance = 0; 
int noise;
//EEPROM Data
const int EEPROM_SIZE = 256;
String deviceName="YOUR DEVICE NAME HERE";
String ssid="YOUR WIFI SSID HERE";
String password="YOUR WIFI PASSWORD HERE";


const char* ISRG_root_X1_CA = "-----BEGIN CERTIFICATE-----\n"
"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\n" \
"-----END CERTIFICATE-----\n";
//const char* ssid = STASSID;
//const char* password = STAPSK;

X509List cert(ISRG_root_X1_CA);


// TCP + TLS
//IPAddress apIP(192, 168, 1, 1);
//const char *ssidDefault = "PulseEcoSensor";
WiFiClientSecure client;
Adafruit_BME680 bme; // I2C

int status = -1;
int loopCycleCount = 0;
int noiseTotal = 0;
int pm10 = 0;
int pm25 = 0;
int noConnectionLoopCount = 0;
int currentSessionNoise = 0;


void measureNoise(){
  //measure noise
  int noiseSessionMax = 0;
  int noiseSessionMin = 1024; 

  while (loopCycleCount<=NUM_MEASURE_SESSIONS){
    loopCycleCount++;
    int currentSample = 0;
    int noiseMeasureLength = millis();
    for (int sample = 0; sample < NUM_NOISE_SAMPLES; sample++) {
      currentSample = analogRead(NOISE_MEASURE_PIN);
      if (currentSample >0 && currentSample < 1020) {
        if (currentSample > noiseSessionMax) {
          noiseSessionMax = currentSample;
        }
        if (currentSample < noiseSessionMin) {
          noiseSessionMin = currentSample;
        }
      }
    }
    int currentSessionNoise = noiseSessionMax - noiseSessionMin;
    if (currentSessionNoise < 0) {
      currentSessionNoise = 0;
      //something bad has happened, but rather send a 0.
    }

    noiseMeasureLength = millis() - noiseMeasureLength;
    SH_DEBUG_PRINT("Noise measurement took: ");
    SH_DEBUG_PRINT_DEC(noiseMeasureLength, DEC);
    SH_DEBUG_PRINT("ms with ");
    SH_DEBUG_PRINT_DEC(NUM_NOISE_SAMPLES, DEC);
    SH_DEBUG_PRINT(" samples. Minumum = ");
    SH_DEBUG_PRINT_DEC(noiseSessionMin, DEC);
    SH_DEBUG_PRINT(", Maxiumum = ");
    SH_DEBUG_PRINT_DEC(noiseSessionMax, DEC);
    SH_DEBUG_PRINT(" samples. Value = ");
    SH_DEBUG_PRINT_DEC(currentSessionNoise, DEC);
    SH_DEBUG_PRINT(", normalized: ");
    SH_DEBUG_PRINTLN_DEC(currentSessionNoise / 4, DEC);
    SH_DEBUG_PRINT(", loopCycleCount: ");
    SH_DEBUG_PRINTLN_DEC(loopCycleCount, DEC);

    SH_DEBUG_PRINT(", NUM_MEASURE_SESSIONS: ");
    SH_DEBUG_PRINTLN_DEC(NUM_MEASURE_SESSIONS , DEC);
    noiseTotal += currentSessionNoise;

  }

 noise = ((int)noiseTotal / loopCycleCount) / 4 + 10;


}

void discoverAndSetStatus() {
  String data = "";
  bool validData = false;

  char readValue = (char)EEPROM.read(0);
  if ((char)readValue == '[') {
    //we're good
    //read till you get to the end
    SH_DEBUG_PRINTLN("Found data in EEPROM");
    int nextIndex = 1;
    while (nextIndex < EEPROM_SIZE && (readValue = (char) EEPROM.read(nextIndex++)) != ']') {
      data += readValue;
    }
  }

  if ((char)readValue == ']') {
    validData = true;
    #ifdef DEBUG_PROFILE
    SH_DEBUG_PRINTLN("Read data:");
    SH_DEBUG_PRINTLN(data);
    #endif
  } else {
    SH_DEBUG_PRINTLN("No data found in EEPROM");
  }

  String readFields[3];
  if (validData) {
    //Try to properly split the string
    //String format: SSID:password:mode
    
    int count = splitCommand(&data, ':', readFields, 3);
    if (count != 3) {
      validData = false;
      SH_DEBUG_PRINTLN("Incorrect data format.");
    } else {
      #ifdef DEBUG_PROFILE
      SH_DEBUG_PRINTLN("Read data parts:");
      SH_DEBUG_PRINTLN(readFields[0]);
      SH_DEBUG_PRINTLN(readFields[1]);
      SH_DEBUG_PRINTLN(readFields[2]); 
      #endif
      deviceName = readFields[0];
      ssid = readFields[1];
      password = readFields[2];
      validData = true;
    }
  }

  if (ssid == NULL || ssid.equals("")) {
    SH_DEBUG_PRINTLN("No WiFi settings found.");
    //no network set yet
    validData = false;
  }

  if (!validData) {
    //It's still not connected to anything
    //broadcast net and display form
    SH_DEBUG_PRINTLN("Setting status code to 0: dipslay config options.");
    //digitalWrite(STATUS_LED_PIN, LOW);
    status = 0;
    
  } else {
    
    
    status = 1;
    //digitalWrite(STATUS_LED_PIN, HIGH);
    SH_DEBUG_PRINTLN("Initially setting status to 1: try to connect to the network.");

  }
}

void measureBME(){
      int countTempHumReadouts = 10;
      int temp = 0; 
      int humidity = 0;
      int pressure = 0;
      int altitude = 0;
      int gasResistance = 0;
      while (--countTempHumReadouts > 0) {
        if (hasBME680) {
          if (! bme680.performReading()) {
            SH_DEBUG_PRINTLN("Failed to perform BME reading!");
            //return;
          } else {
            temp = bme680.temperature;
            humidity = bme680.humidity;
            pressure = bme680.pressure / 100;
            gasResistance = bme680.gas_resistance;
            altitude = bme680.readAltitude(SEALEVELPRESSURE_HPA);
          }
        } else {
          // No temp/hum sensor
          break;
        }
        if (humidity <= 0 || humidity > 100 || temp > 100 || temp < -100 || pressure <= 0) {
          //fake result, pause and try again.
          delay(3000);
        } else {
          // OK result
          break;
        }
      }

      if (countTempHumReadouts <=0) {
        //failed to read temp/hum/pres/gas
        //disable BME sensors
        hasBME680 = false;
        hasBME280 = false;
      }  
}

void dobme(){
  int countTempHumReadouts = 10;


  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }    

  while (--countTempHumReadouts > 0) {
    temp = bme.temperature;
    humidity = bme.humidity;
    pressure = bme.pressure / 100.0;
    gasResistance = bme.gas_resistance;
    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
    Serial.print("Temperature = ");
    Serial.print(bme.temperature);
    Serial.println(" *C");

    Serial.print("Pressure = ");
    Serial.print(bme.pressure / 100.0);
    Serial.println(" hPa");

    Serial.print("Humidity = ");
    Serial.print(bme.humidity);
    Serial.println(" %");

    Serial.print("Gas = ");
    Serial.print(bme.gas_resistance / 1000.0);
    Serial.println(" KOhms");

    Serial.print("Approx. Altitude = ");
    Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    Serial.println(" m");  
  }


}

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while(!Serial) { }

  dobme();
  measureNoise();
  //measureBME();
  sds.begin();
  Serial.println(sds.queryFirmwareVersion().toString()); // prints firmware version
  //Serial.println(sds.setActiveReportingMode().toString()); // ensures sensor is in 'active' reporting mode
  //Serial.println(sds.setContinuousWorkingPeriod().toString()); // ensures sensor has continuous working period - default 
  Serial.println(sds.setQueryReportingMode().toString()); // ensures sensor is in 'query' reporting mode  
  int i = 0;
 
  sds.wakeup();
  delay(5000);

  PmResult pm = sds.queryPm();
  while (i<20){
    i++;
    //PmResult pm = sds.readPm();
    
    //rpmesult.pm25; // float
    //result.pm10; // floa
    
    if (pm.isOk()) {
      Serial.print("PM2.5 = ");
      pm25 = pm.pm25;
      Serial.print(pm.pm25);
      Serial.print(", PM10 = ");
      Serial.println(pm.pm10);
      pm10 = pm.pm10;

      // if you want to just print the measured values, you can use toString() method as well
      //Serial.println(pm.toString());
    } else {
      Serial.print("Could not read values from sensor, reason: ");
      Serial.println(pm.statusToString());
    }    
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(1 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 1676426912) { // a random timestamp from the past
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));

  BearSSL::WiFiClientSecure client;
  client.setTrustAnchors(&cert);


  // Report the data
  String url = "/wifipoint/store";
  url += "?devAddr=" + deviceName;
  url += "&version=2";
  if (pm.isOk()) {
    url += "&pm10=" + String(pm10);
    url += "&pm25=" + String(pm25);
  }
  if (noise > 10) {
    url += "&noise=" + String(noise);
  }
  if (hasBME280 || hasBME680) {
    url += "&temperature=" + String(temp);
    url += "&humidity=" + String(humidity);
    url += "&pressure=" + String(pressure);
    url += "&altitude=" + String(altitude);
  }
  if (hasBME680) {
    url += "&gasresistance=" + String(gasResistance);
  }

  SH_DEBUG_PRINTLN(url);
  String userAgent = "WIFI_SENSOR_V2_1";
  userAgent = userAgent + "_U";

  Serial.println("\nStarting connection to server...");
  if (!client.connect(server, 443))
    Serial.println("Connection failed!");
  else {
    Serial.println("Connected to server!");
    // Make a HTTP request:
    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" +
              "User-Agent: " + userAgent + "\r\n" +
              "Connection: close\r\n\r\n");

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        Serial.println("headers received");
        break;
      }
    }
    // if there are incoming bytes available
    // from the server, read them and print them:
    while (client.available()) {
      char c = client.read();
      Serial.write(c);
    }

    client.stop();
  }

  client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + eco_host + "\r\n" + "User-Agent: BuildFailureDetectorESP8266\r\n" + "Connection: close\r\n\r\n");




  WorkingStateResult state = sds.sleep();
  if (state.isWorking()) {
    Serial.println("Problem with sleeping the sensor.");
  } else {
    Serial.println("Sensor is sleeping");
    //delay(60000); // wait 1 minute
  }
  //WorkingStateResult result = sds.sleep();
  //result.isWorking(); // false

  // Deep sleep mode for 30 seconds, the ESP8266 wakes up by itself when GPIO 16 (D0 in NodeMCU board) is connected to the RESET pin
  Serial.println("I'm awake, but I'm going into deep sleep mode for 30 minutes");
  ESP.deepSleep(1800e6); 



  // Deep sleep mode until RESET pin is connected to a LOW signal (for example pushbutton or magnetic reed switch)
  //Serial.println("I'm awake, but I'm going into deep sleep mode until RESET pin is connected to a LOW signal");
  //ESP.deepSleep(0); 
}

void loop() {
}

int splitCommand(String* text, char splitChar, String returnValue[], int maxLen) {
  int splitCount = countSplitCharacters(text, splitChar);
  SH_DEBUG_PRINT("Split count: ");
  SH_DEBUG_PRINTLN_DEC(splitCount, DEC);
  if (splitCount + 1 > maxLen) {
    return -1;
  }

  int index = -1;
  int index2;

  for(int i = 0; i <= splitCount; i++) {
//    index = text->indexOf(splitChar, index + 1);
    index2 = text->indexOf(splitChar, index + 1);

    if(index2 < 0) index2 = text->length();
    returnValue[i] = text->substring(index+1, index2);
    index = index2;
  }

  return splitCount + 1;
}

int countSplitCharacters(String* text, char splitChar) {
 int returnValue = 0;
 int index = -1;

 while (true) {
   index = text->indexOf(splitChar, index + 1);

   if(index > -1) {
    returnValue+=1;
   } else {
    break;
   }
 }

 return returnValue;
} 

void bubbleSort(short A[],int len) {
  unsigned long newn;
  unsigned long n=len;
  short temp=0;
  do {
    newn=1;
    for(int p=1;p<len;p++){
      if(A[p-1]>A[p]){
        temp=A[p];           //swap places in array
        A[p]=A[p-1];
        A[p-1]=temp;
        newn=p;
      } //end if
    } //end for
    n=newn;
  } while(n>1);
}


short median(short sorted[],int m) //calculate the median
{
  //First bubble sort the values: https://en.wikipedia.org/wiki/Bubble_sort
  bubbleSort(sorted,m);  // Sort the values
  if (bitRead(m,0)==1) {  //If the last bit of a number is 1, it's odd. This is equivalent to "TRUE". Also use if m%2!=0.
    return sorted[m/2]; //If the number of data points is odd, return middle number.
  } else {    
    return (sorted[(m/2)-1]+sorted[m/2])/2; //If the number of data points is even, return avg of the middle two numbers.
  }
}
