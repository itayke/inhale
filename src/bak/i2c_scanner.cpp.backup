#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "config.h"

static Adafruit_BMP280 bmp;

const char* getI2CError(byte error) {
  switch(error) {
    case 0: return "Success";
    case 1: return "Data too long";
    case 2: return "NACK on address";
    case 3: return "NACK on data";
    case 4: return "Other error";
    case 5: return "Timeout";
    default: return "Unknown error";
  }
}

void verifyBMP280(byte addr) {
  // Read chip ID register (0xD0) - should return 0x58 for BMP280
  Wire.beginTransmission(addr);
  Wire.write(0xD0);
  byte error = Wire.endTransmission();

  if (error == 0) {
    Wire.requestFrom(addr, (byte)1);
    if (Wire.available()) {
      byte chipID = Wire.read();
      Serial.print(" [Chip ID: 0x");
      if (chipID < 16) Serial.print("0");
      Serial.print(chipID, HEX);

      if (chipID == 0x58) {
        Serial.print(" - Verified BMP280!");
      } else if (chipID == 0x60) {
        Serial.print(" - BME280 detected");
      } else {
        Serial.print(" - Unknown chip");
      }
      Serial.print("]");
    }
  }
}

int scanAtSpeed(uint32_t speed, const char* speedName) {
  Serial.print("\nScanning at ");
  Serial.print(speedName);
  Serial.println("...");

  Wire.setClock(speed);
  int devicesFound = 0;

  // Only scan BMP280 addresses
  for (byte addr : {0x76, 0x77}) {
    Wire.beginTransmission(addr);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("  Found device at 0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      Serial.print(" (BMP280/BME280)");
      verifyBMP280(addr);
      Serial.println();
      devicesFound++;
    } else {
      Serial.print("  0x");
      if (addr < 16) Serial.print("0");
      Serial.print(addr, HEX);
      Serial.print(" - ");
      Serial.println(getI2CError(error));
    }
  }

  if (devicesFound == 0) {
    Serial.println("  No BMP280 found!");
  } else {
    Serial.print("  Total: ");
    Serial.print(devicesFound);
    Serial.println(" device(s)");
  }

  return devicesFound;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\nI2C Scanner with Diagnostics");
  Serial.println("============================");
  Serial.print("Using pins: SDA=GPIO");
  Serial.print(BMP_SDA);
  Serial.print(", SCL=GPIO");
  Serial.println(BMP_SCL);

  // Test I2C bus health
  Serial.println("\nI2C Bus Health Check:");

  // Check GPIO pin states BEFORE Wire.begin()
  pinMode(BMP_SDA, INPUT_PULLUP);
  pinMode(BMP_SCL, INPUT_PULLUP);
  delay(10);
  Serial.print("GPIO ");
  Serial.print(BMP_SDA);
  Serial.print(" (SDA) state: ");
  Serial.println(digitalRead(BMP_SDA) ? "HIGH" : "LOW");
  Serial.print("GPIO ");
  Serial.print(BMP_SCL);
  Serial.print(" (SCL) state: ");
  Serial.println(digitalRead(BMP_SCL) ? "HIGH" : "LOW");

  Wire.begin();
  delay(100);

  // Check if we can even communicate on I2C
  Serial.println("\nTesting if I2C is working at all...");
  Wire.beginTransmission(0x00);
  byte err = Wire.endTransmission();
  Serial.print("Test transmission error code: ");
  Serial.print(err);
  Serial.print(" (");
  Serial.print(getI2CError(err));
  Serial.println(")");

  // Manual read of chip ID register to verify hardware
  Serial.println("\nManual chip ID read test:");

  for (byte addr : {0x76, 0x77}) {
    Serial.print("Testing address 0x");
    Serial.print(addr, HEX);
    Serial.print(": ");

    Wire.beginTransmission(addr);
    Wire.write(0xD0);  // Chip ID register
    byte error = Wire.endTransmission();

    if (error == 0) {
      Wire.requestFrom(addr, (byte)1);
      if (Wire.available()) {
        byte chipID = Wire.read();
        Serial.print("Read chip ID = 0x");
        Serial.println(chipID, HEX);
        if (chipID == 0x58) Serial.println("  ^ This is a valid BMP280!");
      } else {
        Serial.println("No data available");
      }
    } else {
      Serial.print("Error ");
      Serial.print(error);
      Serial.print(" (");
      Serial.print(getI2CError(error));
      Serial.println(")");
    }
  }

  Serial.println("\nNow running full I2C scan...");
}

void loop() {
  Serial.println("\n========================================");
  Serial.println("Starting I2C scan...");
  Serial.println("========================================");

  // Scan at multiple speeds
  int found100k = scanAtSpeed(100000, "100kHz (standard)");
  delay(100);
  int found400k = scanAtSpeed(400000, "400kHz (fast)");

  // Summary
  Serial.println("\n========================================");
  Serial.println("Summary:");
  Serial.println("========================================");

  if (found100k == 0 && found400k == 0) {
    Serial.println("No devices found at any speed!");
    Serial.println("\nTroubleshooting:");
    Serial.print("  - Check wiring: SDA->GPIO");
    Serial.print(BMP_SDA);
    Serial.print(", SCL->GPIO");
    Serial.println(BMP_SCL);
    Serial.println("  - Check power: VCC->3.3V, GND->GND");
    Serial.println("  - Check for loose connections");
    Serial.println("  - Verify BMP280 module is working");
    Serial.println("  - Check if pull-up resistors are needed");
  } else {
    Serial.println("Devices detected successfully!");
    if (found100k != found400k) {
      Serial.println("Note: Different devices found at different speeds");
      Serial.println("      This may indicate signal integrity issues");
    }
  }

  Serial.println("\nNext scan in 5 seconds...\n");
  delay(5000);
}
