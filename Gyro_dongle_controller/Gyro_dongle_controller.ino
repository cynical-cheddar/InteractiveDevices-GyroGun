#include <SPI.h>
#include <RH_RF69.h>

#define RFM69_CS    8
#define RFM69_INT   7
#define RFM69_RST   12
#define LED        13
#define BTN_PIN     2
#define RELAY_PIN   11

#define RF69_FREQ 915.0
// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

bool btnPressed = false;

//============= The gyro dongle controller is designed to act as a transceiver/intermediatry between radio and serial communication.

void setup()
{
  pinMode(RFM69_RST, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(BTN_PIN, INPUT);

  
  digitalWrite(RFM69_RST, LOW);
  delay(2000);
  Serial.begin(230400);


  delay(200);
  Serial.println("Begin");



  digitalWrite(RFM69_RST, LOW);

  Serial.println("Feather RFM69 RX Test!");
  Serial.println();

  Serial.println("Feather RFM69 resetting");
  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);


  Serial.println("Feather RFM69 resetted");

  if (!rf69.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }

  Serial.println("RFM69 radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                  };
  rf69.setEncryptionKey(key);



  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
}








// send telemetry packet without timeout
void SendCommandPacket(String telemetryPacket) {
  char telemetryPacketArray[31];
  telemetryPacket.toCharArray(telemetryPacketArray, 30);
  rf69.send((uint8_t *)telemetryPacketArray, strlen(telemetryPacketArray));
}



void loop() {
  // If there is a message in the radio buffer, receive it and send to the PC's serial buffer
  if (rf69.available()) {
    // Should be a message for us now
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len)) {
      if (!len) return;
      buf[len] = 0;
      // print results
      Serial.println((char*)buf);
      //String myMessage = (char*)buf;
      // Send a reply!
      //uint8_t data[] = "acknowledged";
      //rf69.send(data, sizeof(data));
      //rf69.waitPacketSent();

    } else {
      Serial.println("Receive failed");
    }
  }


  // Force a solenoid fire command
  if (digitalRead(BTN_PIN) && !btnPressed) {
    SendCommandPacket("@F");
    btnPressed = true;
  }
  else if (!digitalRead(BTN_PIN) && btnPressed) {
    btnPressed = false;
  }


  String  message = "";
  while (Serial.available() > 0) {
    delay(1);
    digitalWrite(LED_BUILTIN, HIGH);
    if (Serial.available() > 0) {

      char c = Serial.read();  //gets one byte from serial buffer
      //Serial.print(c);
      message += c; //makes the string readString
    }
  }
  if (message != "") {
    SendCommandPacket(message);
  }








}
