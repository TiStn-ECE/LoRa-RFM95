#include <ModbusMaster.h>
#include <SoftwareSerial.h>
#include <loraid.h>

///////// PIN /////////
#define MAX485_DE      3
#define MAX485_RE_NEG  2

long interval = 2000;
long previousMillis = 0;
uint8_t result;
float temp, hum;
String dataString;

ModbusMaster node;
SoftwareSerial mySerial(4, 5);      //RX TX RS485
void preTransmission()
{
  digitalWrite(MAX485_RE_NEG, 1);
  digitalWrite(MAX485_DE, 1);
}

void postTransmission()
{
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
}

void setup()
{
  pinMode(MAX485_RE_NEG, OUTPUT);
  pinMode(MAX485_DE, OUTPUT);
  digitalWrite(MAX485_RE_NEG, 0);
  digitalWrite(MAX485_DE, 0);
  Serial.begin(9600);
  mySerial.begin(9600);
  node.begin(1, mySerial);
  node.preTransmission(preTransmission);
  node.postTransmission(postTransmission);

  ////////// LORA ///////////
  lora.init();
  lora.setDeviceClass(CLASS_A);
  lora.setDataRate(2);
  lora.setAccessKey("**********");
  lora.setDeviceId("**********");
}

void loop()
{
  int recvStatus = 0;
  char outStr[255];
  unsigned long currentMillis = millis();
  // Data Frame --> 01 04 00 01 00 02 20 0B
  result = node.readInputRegisters(0x0001, 2);
  if (result == node.ku8MBSuccess)
  {
    temp = node.getResponseBuffer(0) / 10.0f;
    hum = node.getResponseBuffer(1) / 10.0f;
  }

  //Kirim data 2s sekali
  if (currentMillis - previousMillis > interval)
  {
    previousMillis = currentMillis;

    // String to unsigned char*
    dataString = "Suhu: " + String(temp) + ", Kelembapan: " + String(hum);
    unsigned char* data = (unsigned char*) dataString.c_str();
    lora.sendToAntares(data, strlen(data) , 0);
  }
  recvStatus = lora.readData(outStr);
  if (recvStatus) {
    Serial.println(outStr);
  }
  lora.update();
}
