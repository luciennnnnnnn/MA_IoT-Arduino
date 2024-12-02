#include <CayenneLPP.h>
#include "Seeed_vl53l0x.h"
#include "LIS3DHTR.h"
#include <Wire.h>
#include "src/SwitchHandler.h"
#include "AHT20.h"

// VL53L0X Time-of-Flight sensor object
Seeed_vl53l0x VL53L0X;

// Define the serial port depending on the board
#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define SERIAL SerialUSB
#else
    #define SERIAL Serial
#endif

// Define LIS3DHTR accelerometer object
LIS3DHTR<TwoWire> LIS;
#define WIRE Wire

// Define pin for the switch and switch handler object
#define SWITCH_PIN 1
SwitchHandler switchHandler(SWITCH_PIN);

// AHT20 temperature and humidity sensor object
AHT20 AHT;

// CayenneLPP object for formatting data
CayenneLPP lpp(51);

void setup() {
    // Initialize serial communication for debugging
    SERIAL.begin(115200);
    while (!SERIAL) {
        ; // Wait for the serial connection to establish
    }

    // Initialize VL53L0X
    VL53L0X_Error Status = VL53L0X.VL53L0X_common_init();
    if (Status != VL53L0X_ERROR_NONE) {
        SERIAL.println("Failed to initialize VL53L0X!");
        VL53L0X.print_pal_error(Status);
        while (1);
    }

    VL53L0X.VL53L0X_continuous_ranging_init();
    if (Status != VL53L0X_ERROR_NONE) {
        SERIAL.println("Failed to start continuous ranging for VL53L0X!");
        VL53L0X.print_pal_error(Status);
        while (1);
    }

    // Initialize I2C bus
    WIRE.begin();

    // Initialize LIS3DHTR accelerometer
    LIS.begin(WIRE, LIS3DHTR_ADDRESS_UPDATED); // The method returns void
    if (!LIS.isConnection()) { // Use a method that verifies the connection
        SERIAL.println("Failed to initialize LIS3DHTR!");
        while (1);
    }
    LIS.setOutputDataRate(LIS3DHTR_DATARATE_50HZ);

    // Initialize the switch handler
    switchHandler.begin();

      // Initialize AHT20
    SERIAL.println("Initializing AHT20 sensor...");
    AHT.begin(); // Simple initialization, no return value

    delay(100);  // Allow the sensor to stabilize after initialization

    // Perform a quick test to ensure the sensor is operational
    float humidity, temperature;
    int status = AHT.getSensor(&humidity, &temperature);
    if (status == 0) { // 0 indicates failure
        SERIAL.println("Failed to initialize AHT20!");
        while (1); // Stop execution if the sensor initialization fails
    }

    SERIAL.println("All sensors initialized successfully!");
}


void loop() {
    // Check accelerometer connection
    if (!LIS.isConnection()) {
        SERIAL.println("LIS3DHTR disconnected!");
        while (1);
    }

    // Read accelerometer values
    float ax = LIS.getAccelerationX();
    float ay = LIS.getAccelerationY();
    float az = LIS.getAccelerationZ();
    SERIAL.print("Acceleration - X: ");
    SERIAL.print(ax);
    SERIAL.print(" Y: ");
    SERIAL.print(ay);
    SERIAL.print(" Z: ");
    SERIAL.println(az);

    // Add accelerometer data to CayenneLPP
    lpp.addAccelerometer(1, ax, ay, az);

    // Read switch state
    bool isSwitchOn = switchHandler.readSwitch();
    SERIAL.println(isSwitchOn ? "Switch is ON" : "Switch is OFF");
    lpp.addDigitalInput(2, isSwitchOn ? 1 : 0);

    // Read distance from VL53L0X
    VL53L0X_RangingMeasurementData_t RangingMeasurementData;
    VL53L0X.PerformContinuousRangingMeasurement(&RangingMeasurementData);
    if (RangingMeasurementData.RangeMilliMeter >= 2000) {
        SERIAL.println("Distance: Out of range");
    } else {
        SERIAL.print("Distance: ");
        SERIAL.print(RangingMeasurementData.RangeMilliMeter);
        SERIAL.println(" mm");
        lpp.addDistance(3, RangingMeasurementData.RangeMilliMeter / 1000.0f); // Convert mm to meters
    }

    // Read temperature and humidity from AHT20
    float humidity, temperature;
    if (AHT.getSensor(&humidity, &temperature)) {
        SERIAL.print("Humidity: ");
        SERIAL.print(humidity * 100);
        SERIAL.print("%, Temperature: ");
        SERIAL.println(temperature);

        lpp.addRelativeHumidity(4, humidity * 100); // Multiply by 100 for percentage
        lpp.addTemperature(5, temperature);
    } else {
        SERIAL.println("Failed to read data from AHT20!");
    }

    // Transmit CayenneLPP payload via serial for demonstration
    SERIAL.print("CayenneLPP payload: ");
    for (size_t i = 0; i < lpp.getSize(); i++) {
        SERIAL.print(lpp.getBuffer()[i], HEX);
        SERIAL.print(" ");
    }
    SERIAL.println();

    // Clear CayenneLPP buffer for next transmission
    lpp.reset();

    delay(500); // Delay to avoid flooding the serial monitor
}
