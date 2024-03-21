#include <Arduino.h>
#include <LiquidCrystal.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <iterator>

#define bleServerName "Bnb Bluetooth"
const int LED_BUILTIN=27;
const int LCD_COLUMNS = 16;
const int LCD_ROWS = 2;
LiquidCrystal lcd(19, 23, 18, 17, 16, 15); // Define LCD display pins RS, E, D4, D5, D6, D7
bool deviceConnected = false;
BLECharacteristic *pcharacteristic;

byte bluetooth[8] = {
  B00100, //4
  B10110, //22
  B01101, //13
  B00110, //6
  B01101, //13
  B10110, //22
  B00100, //4
  B00000  //0
};
byte favorite[8] = {
  B00000, //0
  B01010, //10
  B11111, //31
  B11111, //31
  B01110, //14
  B00100, //4
  B00000, //0
  B00000  //0
};
byte bolt[8] = {
  B00100, //4
  B01100, //12
  B11100, //28
  B11111, //31
  B00111, //7
  B00110, //6
  B00100, //4
  B00000  //0
};
byte adb[8] = {
  B10001, //17
  B01110, //14
  B10101, //21
  B11111, //31
  B00000, //0
  B11111, //31
  B11111, //31
  B01110  //14
};
byte notifications[8] = {
  B00100, //4
  B01110, //14
  B01110, //14
  B01110, //14
  B11111, //31
  B00000, //0 
  B00100, //4
  B00000,  //0
};
byte hourglass_bottom[8] = {
  B11111, //31
  B10001, //17
  B01010, //10
  B00100, //4 
  B01110, //14
  B11111, //31
  B11111, //31
  B00000  //0
};
byte dialpad[8] = {
  B10101, //21
  B00000, //0
  B10101, //21
  B00000, //0
  B10101, //21
  B00000, //0
  B00100, //4
  B00000, //0
};
byte lock[8] = {
  B01110, //14
  B10001, //17
  B10001, //17
  B11111, //31
  B11111, //31
  B11011, //27
  B11111, //31
  B11111 //31
};

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLECharacteristic dataCharacteristic("beb5483e-36e1-4688-b7f5-ea07361b26a8");

BLECharacteristic characteristic(
  CHARACTERISTIC_UUID,
  BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
);
BLEDescriptor *characteristicDescriptor = new BLEDescriptor(BLEUUID((uint16_t)0x2902));

// Setup callbacks onConnect and onDisconnect (no change necessary)
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
  }
};

std::string teamId="A71";

void getLCDs(BLECharacteristic* pcharacteristic){
  DynamicJsonDocument responseDoc(4096);
  JsonObject responseObj = responseDoc.to<JsonObject>();
  //LCD
  JsonObject lcd = responseObj.createNestedObject("lcd");
  responseObj["type"]="16x2";
  responseObj["interface"]="Parallel 4-bit";
  responseObj["resolution"]="16x2";
  responseObj["id"]=1;
  responseObj["teamId"]=teamId;
  //serialize the response to a JSON String
  String responseString;
  serializeJson(responseDoc, responseString);
  //set the response string to the ble characteristic
  pcharacteristic->setValue(responseString.c_str());
  pcharacteristic->notify();
}

void setText(BLECharacteristic* pcharacteristic, const JsonObject& request){
  int lcdId = request["id"].as<int>();
  JsonArray textArray = request["text"].as<JsonArray>();
  lcd.clear();
  // print text on lcd
  for (int i = 0; i < textArray.size() && i < LCD_ROWS; i++) {
    String text = textArray[i].as<String>();
    lcd.setCursor(0, i);
    lcd.print(text);
  }
  // Prepare the response object
  DynamicJsonDocument responseDoc(4096);
  JsonObject responseObj = responseDoc.to<JsonObject>();
  responseObj["id"] = lcdId;
  responseObj["text"] = textArray;
  responseObj["teamId"] = teamId;

  // Serialize the response to a JSON string
  String responseString;
  serializeJson(responseDoc, responseString);

  // Set the response string to the BLE characteristic
  pcharacteristic->setValue(responseString.c_str());
  pcharacteristic->notify();
}

void setIconsRequest(BLECharacteristic* pcharacteristic, JsonObject& requestObj) {
  int lcdId = requestObj["id"].as<int>();
  JsonArray iconsArray = requestObj["icons"].as<JsonArray>();

  // Process the icons
  int numIcons = iconsArray.size();
  // Perform the necessary operations to set the icons on the LCD
  lcd.begin(16,2);
  lcd.clear();
  
  for (const auto& icon : iconsArray) {
    // Extract the icon name and data
    String iconName = icon["name"].as<String>();
    JsonArray iconDataArray = icon["data"].as<JsonArray>();

    // Check the condition to determine which icon to display
    if (iconName == "bluetooth") {
      lcd.createChar(0, bluetooth);
      lcd.setCursor(0, 0);
      lcd.write((byte)0);
    }
    else
      if(iconName == "favorite"){
        lcd.createChar(1, favorite);
        lcd.setCursor(1, 1);
        lcd.write((byte)1);
      }
      else
        if(iconName == "bolt"){
          lcd.createChar(2, bolt);
          lcd.setCursor(2,0);
          lcd.write((byte)2);
        }
        else
          if(iconName == "adb"){
            lcd.createChar(3, adb);
            lcd.setCursor(3,1);
            lcd.write((byte)3);
          }
          else
            if(iconName == "notifications"){
              lcd.createChar(4, notifications);
              lcd.setCursor(4,0);
              lcd.write((byte)4);
            }
            else
              if(iconName == "hourglass-bottom"){
                lcd.createChar(5, hourglass_bottom);
                lcd.setCursor(5,1);
                lcd.write((byte)5);
              }
              else
                if(iconName == "dialpad"){
                  lcd.createChar(6, dialpad);
                  lcd.setCursor(6,0);
                  lcd.write((byte)6);
                }
                else
                  if(iconName == "lock"){
                    lcd.createChar(7, lock);
                    lcd.setCursor(7,1);
                    lcd.write((byte)7);
                  }
  }
  // Prepare the response
  DynamicJsonDocument responseDoc(4096);
  JsonObject responseObj = responseDoc.to<JsonObject>();
  responseObj["id"] = lcdId;
  responseObj["number_icons"] = numIcons;
  responseObj["teamId"] = teamId;

  // Serialize the response to a JSON string
  String responseString;
  serializeJson(responseDoc, responseString);

  // Send the response to the app
  pcharacteristic->setValue(responseString.c_str());
  pcharacteristic->notify();
}

bool isScrolling = false;
bool scrollDirectionLeft = true;

void scrollText() { {
    if (scrollDirectionLeft) {
      lcd.scrollDisplayLeft();
    } else {
      lcd.scrollDisplayRight();
    }
    delay(100);
  }
}
void scrollRequest(BLECharacteristic *pcharacteristic, JsonObject &requestObj){
  int lcdId = requestObj["id"].as<int>();
  String scrollDirection = requestObj["direction"].as<String>();

  // Process the scroll direction
  if (scrollDirection == "Left") {
    // Start scrolling the text
    scrollDirectionLeft = true;
    isScrolling = true;
  } else if (scrollDirection == "Right") {
    // Start scrolling the text
    scrollDirectionLeft = false;
    isScrolling = true;
  } else if (scrollDirection == "Off") {
    // Stop scrolling the text
    isScrolling = false;
  }

  // Prepare the response
  DynamicJsonDocument responseDoc(4096);
  JsonObject responseObj = responseDoc.to<JsonObject>();
  responseObj["id"] = lcdId;
  responseObj["scrolling"] = scrollDirection;
  responseObj["teamId"] = teamId;

  // Serialize the response to a JSON string
  String responseString;
  serializeJson(responseObj, responseString);

  // Send the response to the app
  pcharacteristic->setValue(responseString.c_str());
  pcharacteristic->notify();
}

class CharacteristicsCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pcharacteristic) {
      std::string value = pcharacteristic->getValue();

      if (value.length() > 0) {
      // Parse the received JSON request
        DynamicJsonDocument requestDoc(4096);
        DeserializationError error = deserializeJson(requestDoc, value.c_str());

        if (error) {
          Serial.print("JSON parsing error: ");
          Serial.println(error.c_str());
          return;
        }
        JsonObject requestObj = requestDoc.as<JsonObject>();
        String action = requestObj["action"].as<String>();
        if (action == "getLCDs") {
          getLCDs(pcharacteristic);
        }
        else 
          if (action == "setText") {
            setText(pcharacteristic, requestObj);
          }
          else
            if(action == "setIcons"){
              setIconsRequest(pcharacteristic, requestObj);
            }
            else
              if(action == "scroll"){
                scrollRequest(pcharacteristic, requestObj);
              }
      }
    }
};     
void setup() {
  // Start serial communication 
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  // BEGIN DON'T CHANGE
  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  // Set server callbacks
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *bleService = pServer->createService(SERVICE_UUID);

  // Create BLE characteristics and descriptors
  bleService->addCharacteristic(&characteristic);  
  characteristic.addDescriptor(characteristicDescriptor);

  // Set characteristic callbacks
  characteristic.setCallbacks(new CharacteristicsCallbacks());

  // Start the service
  bleService->start();
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
  // END DON'T CHANGE
}

void loop() {
  if (isScrolling) {
    scrollText();
  }
}
