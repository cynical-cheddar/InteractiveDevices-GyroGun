#include <SPI.h>
#include <RH_RF69.h>
#include <Wire.h>

// Relevant physical pins
#define RFM69_INT   7
#define RFM69_CS    8
#define SOLENOID_SWITCH_PIN 9
#define BTN_PIN     10
#define RELAY_PIN   11
#define RFM69_RST   12
#define LED        13



// Radio frequency (MHz)
#define RF69_FREQ 915.0

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);



// ========== Gyroscope variables ================

const int MPU = 0x68; // MPU6050 I2C address
float AccX, AccY, AccZ;
float GyroX, GyroY, GyroZ;
float accAngleX, accAngleY, gyroAngleX, gyroAngleY, gyroAngleZ;
float roll, pitch, yaw;
float AccErrorX, AccErrorY, GyroErrorX, GyroErrorY, GyroErrorZ;
float elapsedTime, currentTime, previousTime;
int c = 0;
long gyroSendPeriod_ms = 100;
long lastGyroSendTime_ms = 0;

// ========== Trigger variables ================

bool btnPressed = false;
long lastButtonPressMillis = 0;
bool longPress = false;

// ===================================

void setup()
{

  // Set pinmodes
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(SOLENOID_SWITCH_PIN, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  pinMode(BTN_PIN, INPUT);

  // Ensure radio reset pin is set to low
  digitalWrite(RFM69_RST, LOW);

  // wait some time for RFM69HCW breakout radio board to configure itself
  delay(2000);

  // Begin serial at 230400 Bauds
  Serial.begin(230400);
  
  // Delay before outputting first serial message (otherwise message may be lost!)
  delay(200);
  Serial.println("Begin");




  // RFM69HCW Setup ====================================================================
  Serial.println("Feather RFM69 RX Test!");
  Serial.println();
  Serial.println("Feather RFM69 resetting");
  
  // Manually reset the RFM69HCW Radio Module
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
  

  // MPU-6050 Gyro Setup ====================================================================
  Wire.begin();                      // Initialize comunication
  Wire.beginTransmission(MPU);       // Start communication with MPU6050 // MPU=0x68
  Wire.write(0x6B);                  // Talk to the register 6B
  Wire.write(0x00);                  // Make reset - place a 0 into the 6B register
  Wire.endTransmission(true);        //end the transmission
  calculate_IMU_error();
  delay(20);

  // Ensure solenoid is not active.
  digitalWrite(RELAY_PIN, LOW);

}

// send telemetry packet without timeout
void SendTelemetryPacket(String telemetryPacket) {
  char telemetryPacketArray[12];
  telemetryPacket.toCharArray(telemetryPacketArray, 12);
  rf69.send((uint8_t *)telemetryPacketArray, strlen(telemetryPacketArray));

  // if serial is connected, send the command over serial instead.
  if (Serial){
    Serial.println(telemetryPacket);
  }
}


// Function fires the solenoid by closing the relay.
// The code is a bit dirty and should use a state machine instead of a delay - but this isn't a critical system!
void FireSolenoid(){
  if(digitalRead(SOLENOID_SWITCH_PIN)){
    digitalWrite(RELAY_PIN, LOW);
    delay(10);
    digitalWrite(RELAY_PIN, HIGH);
    delay(40);
    digitalWrite(RELAY_PIN, LOW);
  }
  
}



void loop() {
  // ensure that the solenoid is not powered - this can quickly drain the battery otherwise.
  digitalWrite(RELAY_PIN, LOW);

  // TRIGGER SENSING ===================================================================
  // Detect moment when trigger is initially pulled
  if (digitalRead(BTN_PIN) && !btnPressed) {
    lastButtonPressMillis = millis();
    btnPressed = true;
  }
  // Detect if trigger has been held for more than 400ms. Only triggers once per long press.
  else if (btnPressed && (millis() - lastButtonPressMillis > 400) && !longPress) {
    SendTelemetryPacket("@L");
    zero_readings();
    longPress = true;
  }
  // Detect when trigger is released *after* a long press. Do not fire.
  else if (!digitalRead(BTN_PIN) && btnPressed && longPress) {
    longPress = false;
    btnPressed = false;
  }
  // Detect when trigger is released *before* a long press (normal operation). Open fire!
  else if (!digitalRead(BTN_PIN) && btnPressed && !longPress){
    SendTelemetryPacket("@F");
    btnPressed = false;
    longPress = false;
  }


  // COMMUNICATION ===================================================================
  // Poll the radio module. If there exists messages in the buffer, then read them.
  if (rf69.available()) {

    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len)) {
      if (!len) return;
      buf[len] = 0;
      String msg = (char*)buf;
      // if the message equals "@F" - the fire command, then fire the solenoid.
      if(msg[0] == '@' && msg[1] == 'F'){
        FireSolenoid();
      }
    }
  }

  // If we are connected via serial, then poll the serial buffer. We are implicitly not using the radio.
  if (Serial){
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

    // if the message equals "@F" - the fire command, then fire the solenoid.
    if(message[0] == '@' && message[1] == 'F'){
        FireSolenoid();
      }
  }


  // 3-AXIS GYRO ====================================
  // === Read acceleromter data === //
  Wire.beginTransmission(MPU);
  Wire.write(0x3B); // Start with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 6 registers total, each axis value is stored in 2 registers
  //For a range of +-2g, we need to divide the raw values by 16384, according to the datasheet
  AccX = (Wire.read() << 8 | Wire.read()) / 16384.0; // X-axis value
  AccY = (Wire.read() << 8 | Wire.read()) / 16384.0; // Y-axis value
  AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0; // Z-axis value
  // Calculating Roll and Pitch from the accelerometer data
  accAngleX = (atan(AccY / sqrt(pow(AccX, 2) + pow(AccZ, 2))) * 180 / PI); // AccErrorX ~(0.58) See the calculate_IMU_error()custom function for more details
  accAngleY = (atan(-1 * AccX / sqrt(pow(AccY, 2) + pow(AccZ, 2))) * 180 / PI); // AccErrorY ~(-1.58)
  // === Read gyroscope data === //
  previousTime = currentTime;        // Previous time is stored before the actual time read
  currentTime = millis();            // Current time actual time read
  elapsedTime = (currentTime - previousTime) / 1000; // Divide by 1000 to get seconds
  Wire.beginTransmission(MPU);
  Wire.write(0x43); // Gyro data first register address 0x43
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 6, true); // Read 4 registers total, each axis value is stored in 2 registers
  GyroX = (Wire.read() << 8 | Wire.read()) / 131.0; // For a 250deg/s range we have to divide first the raw value by 131.0, according to the datasheet
  GyroY = (Wire.read() << 8 | Wire.read()) / 131.0;
  GyroZ = (Wire.read() << 8 | Wire.read()) / 131.0;
  // Correct the outputs with the calculated error values
  GyroX = GyroX - GyroErrorX; // GyroErrorX ~(-0.56)
  GyroY = GyroY - GyroErrorY; // GyroErrorY ~(2)
  GyroZ = GyroZ - GyroErrorZ; // GyroErrorZ ~ (-0.8)

  // High pass filter to combat drift. Only deal with sufficiently large angular velocities.
  // Threshold values selected through trial and error.
  if(abs(GyroX) < 0.02)GyroX = 0;
  if(abs(GyroY) < 0.02)GyroY = 0;
  if(abs(GyroZ) < 0.02)GyroZ = 0;
  // Currently the raw values are in degrees per seconds, deg/s, so we need to multiply by sendonds (s) to get the angle in degrees
  if (abs(gyroAngleX + GyroX) > 0.05 )gyroAngleX = gyroAngleX + GyroX * elapsedTime; // deg/s * s = deg
  if (abs(gyroAngleY + GyroY) > 0.05 )gyroAngleY = gyroAngleY + GyroY * elapsedTime;
  if (abs(gyroAngleZ + GyroZ) > 0.05 )gyroAngleZ = gyroAngleZ + GyroZ * elapsedTime;

  yaw =  gyroAngleZ;
  // Complementary filter - combine acceleromter and gyro angle values
  roll =  gyroAngleX;
  pitch = gyroAngleY;






  // TELEMETRY =====================
  if (millis() - lastGyroSendTime_ms > gyroSendPeriod_ms) {
    lastGyroSendTime_ms = millis();
    // send telemetry
    String telemetry = "#" + String((int)(roll)) + ","  + String((int)(yaw)) + ",";
    SendTelemetryPacket(telemetry);
  }
}



// Function called after a long press. Zeroes values and re-calibrates 3-Axis Gyro.
void zero_readings(){


  gyroAngleX = 0;
  gyroAngleY = 0;
  gyroAngleZ = 0;

  yaw = 0;
  roll = 0;
  pitch = 0;

  AccX = 0; 
  AccY = 0;
  AccZ = 0;

  GyroX = 0;
  GyroY = 0;
  GyroZ = 0;

  accAngleX = 0;
  accAngleY = 0;

  AccErrorX = 0;
  AccErrorY = 0;

  GyroErrorX = 0;
  GyroErrorY = 0;
  GyroErrorZ = 0;
 
  calculate_IMU_error();
}




// t is a variable to denote number of samples 
int t = 500;
void calculate_IMU_error() {
  // We can call this funtion in the setup section to calculate the accelerometer and gyro data error. From here we will get the error values used in the above equations printed on the Serial Monitor.
  // Note that we should place the IMU flat in order to get the proper values, so that we then can the correct values
  // Read accelerometer values t times
  c = 0;
  while (c < t) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    AccX = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccY = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    AccZ = (Wire.read() << 8 | Wire.read()) / 16384.0 ;
    // Sum all readings
    AccErrorX = AccErrorX + ((atan((AccY) / sqrt(pow((AccX), 2) + pow((AccZ), 2))) * 180 / PI));
    AccErrorY = AccErrorY + ((atan(-1 * (AccX) / sqrt(pow((AccY), 2) + pow((AccZ), 2))) * 180 / PI));
    c++;
  }
  //Divide the sum by 200 to get the error value
  AccErrorX = AccErrorX / t;
  AccErrorY = AccErrorY / t;
  c = 0;
  // Read gyro values 200 times
  while (c < t) {
    Wire.beginTransmission(MPU);
    Wire.write(0x43);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 6, true);
    GyroX = Wire.read() << 8 | Wire.read();
    GyroY = Wire.read() << 8 | Wire.read();
    GyroZ = Wire.read() << 8 | Wire.read();
    // Sum all readings
    GyroErrorX = GyroErrorX + (GyroX / 131.0);
    GyroErrorY = GyroErrorY + (GyroY / 131.0);
    GyroErrorZ = GyroErrorZ + (GyroZ / 131.0);
    c++;
  }
  //Divide the sum by t to get the error value
  GyroErrorX = GyroErrorX / t;
  GyroErrorY = GyroErrorY / t;
  GyroErrorZ = GyroErrorZ / t;
  // Print the error values on the Serial Monitor
  Serial.print("AccErrorX: ");
  Serial.println(AccErrorX);
  Serial.print("AccErrorY: ");
  Serial.println(AccErrorY);
  Serial.print("GyroErrorX: ");
  Serial.println(GyroErrorX);
  Serial.print("GyroErrorY: ");
  Serial.println(GyroErrorY);
  Serial.print("GyroErrorZ: ");
  Serial.println(GyroErrorZ);
}
