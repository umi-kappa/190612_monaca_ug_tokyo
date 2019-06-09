#include <M5Stack.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

// BLE
#define SERVICE_UUID        "8a61d7f7-888e-4e72-93be-0df87152fc6d"
#define CHARACTERISTIC_UUID "fae2e24f-aea2-48cb-b449-55ec20518e93"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

// HeartRate
unsigned char counter;
unsigned long temp[11];
unsigned long sub;
bool data_effect = true;
unsigned int heart_rate;
const int max_heartpluse_duty = 2000;

/**
 * BLE
 */

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      M5.Lcd.println("connect");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      M5.Lcd.println("disconnect");
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    M5.Lcd.println("read");
    pCharacteristic->setValue("Hello World!");
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
    M5.Lcd.println("write");
    std::string value = pCharacteristic->getValue();
    M5.Lcd.println(value.c_str());
  }
};

/**
 * HeartRate
 */

void sum()
{
  if (data_effect)
  {
    heart_rate = 600000 / (temp[10] - temp[0]); //60*20*1000/20_total_time
    Serial.print("Heart_rate_is:\t");
    Serial.println(heart_rate);
    pCharacteristic->setValue(heart_rate);
    pCharacteristic->notify();
  }
  data_effect = 1; //sign bit
}

void interrupt()
{
  temp[counter] = millis();
  
  Serial.println(counter, DEC);
  Serial.println(temp[counter]);
  
  switch (counter)
  {
    case 0:
      sub = temp[counter] - temp[10];
      Serial.println(sub);
      break;
    default:
      sub = temp[counter] - temp[counter - 1];
      Serial.println(sub);
      break;
  }
  
  if (sub > max_heartpluse_duty) //set 2 seconds as max heart pluse duty
  {
    data_effect = 0; //sign bit
    counter = 0;
    Serial.println("Heart rate measure error,test will restart!" );
    arrayInit();
  }
  
  if (counter == 10 && data_effect)
  {
    counter = 0;
    sum();
  }
  else if (counter != 10 && data_effect)
  {
    counter++;
  }
  else
  {
    counter = 0;
    data_effect = 1;
  }
}

void arrayInit()
{
  for (unsigned char i = 0; i < 10; i ++)
  {
    temp[i] = 0;
  }
  temp[10] = millis();
}

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.setWakeupButton(BUTTON_A_PIN);
  M5.Lcd.println("BLE start.");
  m5.Speaker.mute();

  // デバイスを初期化
  BLEDevice::init("m5-stack");
  // サーバーを生成
  BLEServer *pServer = BLEDevice::createServer();
  // コールバック関数を設定
  pServer->setCallbacks(new MyServerCallbacks());

  // アドバタイズオブジェクトを取得
  BLEAdvertising *pAdvertising = pServer->getAdvertising();

  // Serviceを生成
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Characteristicを生成
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );

  // コールバック関数を設定
  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->addDescriptor(new BLE2902());
  
  // Serviceを起動
  pService->start();
  // アドバタイズ起動
  pAdvertising->start();

  // HeartRate
  arrayInit();
  Serial.println("Heart rate test begin.");
  attachInterrupt(22, interrupt, RISING);//set interrupt 0,digital port 2
}

void loop() {
  if(M5.BtnA.wasPressed()) {
    M5.powerOFF();
  }
  if (deviceConnected) {
    if(M5.BtnB.wasPressed()) {
      M5.Lcd.println("Button B pressed!");
      pCharacteristic->setValue("Button B pressed!");
      pCharacteristic->notify();
    }
  }
  M5.update();
}
