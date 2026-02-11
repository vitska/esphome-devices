/*
 * ESP8266 D1 Mini Hardware Test
 * Tests various hardware components and reports results over serial
 */

#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

// EEPROM layout for persistence test
#define EEPROM_SIZE 512
#define EEPROM_MAGIC_ADDR 0
#define EEPROM_COUNTER_ADDR 4
#define EEPROM_PATTERN_ADDR 8
#define EEPROM_CHECKSUM_ADDR 256
#define EEPROM_MAGIC_VALUE 0xDEADBEEF

// Test result tracking
struct TestResult {
    const char* name;
    bool passed;
    const char* message;
};

#define MAX_TESTS 20
TestResult results[MAX_TESTS];
int testCount = 0;

// MCP23017 registers
#define MCP23017_IODIRA   0x00
#define MCP23017_IODIRB   0x01
#define MCP23017_GPIOA    0x12
#define MCP23017_GPIOB    0x13
#define MCP23017_OLATA    0x14
#define MCP23017_OLATB    0x15

void addResult(const char* name, bool passed, const char* message) {
    if (testCount < MAX_TESTS) {
        results[testCount].name = name;
        results[testCount].passed = passed;
        results[testCount].message = message;
        testCount++;
    }
}

void printHeader(const char* title) {
    Serial.println();
    Serial.println("========================================");
    Serial.print("  ");
    Serial.println(title);
    Serial.println("========================================");
}

void printProgress(const char* msg) {
    Serial.print("[PROGRESS] ");
    Serial.println(msg);
    Serial.flush();
}

void printPass(const char* msg) {
    Serial.print("[PASS] ");
    Serial.println(msg);
    Serial.flush();
}

void printFail(const char* msg) {
    Serial.print("[FAIL] ");
    Serial.println(msg);
    Serial.flush();
}

// ============================================
// TEST: Basic Serial Communication
// ============================================
bool testSerial() {
    printProgress("Testing serial communication...");
    Serial.println("  Serial TX: OK");
    Serial.println("  Baud rate: 115200");
    printPass("Serial communication working");
    return true;
}

// ============================================
// TEST: Chip Information
// ============================================
bool testChipInfo() {
    printProgress("Reading chip information...");

    Serial.print("  Chip ID: 0x");
    Serial.println(ESP.getChipId(), HEX);

    Serial.print("  Flash Chip ID: 0x");
    Serial.println(ESP.getFlashChipId(), HEX);

    Serial.print("  Flash Size: ");
    Serial.print(ESP.getFlashChipSize() / 1024);
    Serial.println(" KB");

    Serial.print("  Flash Speed: ");
    Serial.print(ESP.getFlashChipSpeed() / 1000000);
    Serial.println(" MHz");

    Serial.print("  Free Heap: ");
    Serial.print(ESP.getFreeHeap());
    Serial.println(" bytes");

    Serial.print("  SDK Version: ");
    Serial.println(ESP.getSdkVersion());

    Serial.print("  Core Version: ");
    Serial.println(ESP.getCoreVersion());

    Serial.print("  CPU Freq: ");
    Serial.print(ESP.getCpuFreqMHz());
    Serial.println(" MHz");

    printPass("Chip info read successfully");
    return true;
}

// ============================================
// TEST: GPIO Pins (Built-in LED)
// ============================================
bool testGPIO() {
    printProgress("Testing GPIO (built-in LED on D4/GPIO2)...");

    pinMode(LED_BUILTIN, OUTPUT);

    Serial.println("  Blinking LED 3 times...");
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_BUILTIN, LOW);  // LED on (active low)
        delay(200);
        digitalWrite(LED_BUILTIN, HIGH); // LED off
        delay(200);
    }

    printPass("GPIO test completed (check if LED blinked)");
    return true;
}

// ============================================
// TEST: I2C Bus Scan
// ============================================
bool testI2C() {
    printProgress("Scanning I2C bus (SDA=D2/GPIO4, SCL=D1/GPIO5)...");

    Wire.begin(4, 5);  // SDA=GPIO4 (D2), SCL=GPIO5 (D1)
    Wire.setClock(100000);  // 100kHz for reliability

    int deviceCount = 0;
    Serial.println("  Scanning addresses 0x00 - 0x7F...");

    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("  Found device at 0x");
            if (addr < 16) Serial.print("0");
            Serial.print(addr, HEX);

            // Identify common devices
            if (addr == 0x20) Serial.print(" (MCP23017 #1 - LEDs)");
            else if (addr == 0x21) Serial.print(" (MCP23017 #2 - Buttons)");
            else if (addr >= 0x20 && addr <= 0x27) Serial.print(" (MCP23017/PCF8574)");
            else if (addr == 0x3C || addr == 0x3D) Serial.print(" (OLED Display)");
            else if (addr == 0x68) Serial.print(" (DS3231 RTC)");
            else if (addr == 0x76 || addr == 0x77) Serial.print(" (BME280/BMP280)");

            Serial.println();
            deviceCount++;
        }
    }

    Serial.print("  Total devices found: ");
    Serial.println(deviceCount);

    if (deviceCount > 0) {
        printPass("I2C scan completed");
        return true;
    } else {
        printFail("No I2C devices found");
        return false;
    }
}

// ============================================
// TEST: MCP23017 at address 0x20 (LEDs)
// ============================================
bool testMCP23017_LEDs() {
    printProgress("Testing MCP23017 at 0x20 (LED controller)...");

    Wire.beginTransmission(0x20);
    if (Wire.endTransmission() != 0) {
        printFail("MCP23017 at 0x20 not responding");
        return false;
    }

    // Set all pins as outputs (IODIRA = 0x00, IODIRB = 0x00)
    Wire.beginTransmission(0x20);
    Wire.write(MCP23017_IODIRA);
    Wire.write(0x00);  // All Port A as outputs
    Wire.endTransmission();

    Wire.beginTransmission(0x20);
    Wire.write(MCP23017_IODIRB);
    Wire.write(0x00);  // All Port B as outputs
    Wire.endTransmission();

    Serial.println("  Configured all pins as outputs");

    // Test pattern: turn all LEDs on
    Serial.println("  Testing LED pattern: ALL ON...");
    Wire.beginTransmission(0x20);
    Wire.write(MCP23017_OLATA);
    Wire.write(0xFF);
    Wire.endTransmission();

    Wire.beginTransmission(0x20);
    Wire.write(MCP23017_OLATB);
    Wire.write(0xFF);
    Wire.endTransmission();
    delay(500);

    // Turn all LEDs off
    Serial.println("  Testing LED pattern: ALL OFF...");
    Wire.beginTransmission(0x20);
    Wire.write(MCP23017_OLATA);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(0x20);
    Wire.write(MCP23017_OLATB);
    Wire.write(0x00);
    Wire.endTransmission();
    delay(500);

    // Walking LED pattern
    Serial.println("  Testing walking LED pattern...");
    for (int i = 0; i < 16; i++) {
        uint8_t portA = (i < 8) ? (1 << i) : 0;
        uint8_t portB = (i >= 8) ? (1 << (i - 8)) : 0;

        Wire.beginTransmission(0x20);
        Wire.write(MCP23017_OLATA);
        Wire.write(portA);
        Wire.endTransmission();

        Wire.beginTransmission(0x20);
        Wire.write(MCP23017_OLATB);
        Wire.write(portB);
        Wire.endTransmission();

        delay(100);
    }

    // Turn all off
    Wire.beginTransmission(0x20);
    Wire.write(MCP23017_OLATA);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.beginTransmission(0x20);
    Wire.write(MCP23017_OLATB);
    Wire.write(0x00);
    Wire.endTransmission();

    printPass("MCP23017 LED test completed");
    return true;
}

// ============================================
// TEST: MCP23017 at address 0x21 (Buttons)
// ============================================
bool testMCP23017_Buttons() {
    printProgress("Testing MCP23017 at 0x21 (Button controller)...");

    Wire.beginTransmission(0x21);
    if (Wire.endTransmission() != 0) {
        printFail("MCP23017 at 0x21 not responding");
        return false;
    }

    // Set all pins as inputs with pull-ups
    // IODIRA = 0xFF (all inputs), IODIRB = 0xFF (all inputs)
    Wire.beginTransmission(0x21);
    Wire.write(MCP23017_IODIRA);
    Wire.write(0xFF);
    Wire.endTransmission();

    Wire.beginTransmission(0x21);
    Wire.write(MCP23017_IODIRB);
    Wire.write(0xFF);
    Wire.endTransmission();

    // Enable pull-ups (GPPUA = 0x0C, GPPUB = 0x0D)
    Wire.beginTransmission(0x21);
    Wire.write(0x0C);  // GPPUA
    Wire.write(0xFF);
    Wire.endTransmission();

    Wire.beginTransmission(0x21);
    Wire.write(0x0D);  // GPPUB
    Wire.write(0xFF);
    Wire.endTransmission();

    Serial.println("  Configured all pins as inputs with pull-ups");

    // Read current button states
    Wire.beginTransmission(0x21);
    Wire.write(MCP23017_GPIOA);
    Wire.endTransmission();
    Wire.requestFrom(0x21, 1);
    uint8_t portA = Wire.read();

    Wire.beginTransmission(0x21);
    Wire.write(MCP23017_GPIOB);
    Wire.endTransmission();
    Wire.requestFrom(0x21, 1);
    uint8_t portB = Wire.read();

    Serial.print("  Port A state: 0b");
    for (int i = 7; i >= 0; i--) Serial.print((portA >> i) & 1);
    Serial.println();

    Serial.print("  Port B state: 0b");
    for (int i = 7; i >= 0; i--) Serial.print((portB >> i) & 1);
    Serial.println();

    // Check for any pressed buttons (active low with pull-ups)
    int pressedCount = 0;
    for (int i = 0; i < 8; i++) {
        if (!(portA & (1 << i))) {
            Serial.print("  BTN");
            Serial.print(i);
            Serial.println(" is PRESSED");
            pressedCount++;
        }
        if (!(portB & (1 << i))) {
            Serial.print("  BTN");
            Serial.print(i + 8);
            Serial.println(" is PRESSED");
            pressedCount++;
        }
    }

    if (pressedCount == 0) {
        Serial.println("  No buttons currently pressed");
    }

    printPass("MCP23017 button test completed");
    return true;
}

// ============================================
// TEST: WiFi Hardware
// ============================================
bool testWiFi() {
    printProgress("Testing WiFi hardware...");

    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);

    Serial.print("  MAC Address: ");
    Serial.println(WiFi.macAddress());

    Serial.println("  Scanning for networks...");
    int networks = WiFi.scanNetworks();

    if (networks == 0) {
        Serial.println("  No networks found");
    } else {
        Serial.print("  Found ");
        Serial.print(networks);
        Serial.println(" networks:");

        for (int i = 0; i < min(networks, 5); i++) {
            Serial.print("    ");
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(" dBm) ");
            Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "[OPEN]" : "[SECURED]");
        }

        if (networks > 5) {
            Serial.print("    ... and ");
            Serial.print(networks - 5);
            Serial.println(" more");
        }
    }

    WiFi.scanDelete();

    printPass("WiFi hardware test completed");
    return true;
}

// ============================================
// TEST: ADC (Analog Input)
// ============================================
bool testADC() {
    printProgress("Testing ADC (A0)...");

    int samples[10];
    int sum = 0;

    for (int i = 0; i < 10; i++) {
        samples[i] = analogRead(A0);
        sum += samples[i];
        delay(10);
    }

    int avg = sum / 10;
    float voltage = avg * 3.3 / 1024.0;

    Serial.print("  ADC Average: ");
    Serial.print(avg);
    Serial.print(" (");
    Serial.print(voltage, 2);
    Serial.println(" V)");

    // Calculate variance
    long variance = 0;
    for (int i = 0; i < 10; i++) {
        variance += (samples[i] - avg) * (samples[i] - avg);
    }
    variance /= 10;

    Serial.print("  ADC Variance: ");
    Serial.println(variance);

    if (variance > 1000) {
        Serial.println("  WARNING: High ADC noise detected");
    }

    printPass("ADC test completed");
    return true;
}

// ============================================
// TEST: SRAM Memory Test
// ============================================
bool testSRAM() {
    printProgress("Testing SRAM memory...");

    uint32_t freeHeapBefore = ESP.getFreeHeap();
    Serial.print("  Free heap before test: ");
    Serial.print(freeHeapBefore);
    Serial.println(" bytes");

    // Determine safe allocation size (use 60% of free heap)
    size_t testSize = (freeHeapBefore * 60) / 100;
    if (testSize > 32768) testSize = 32768;  // Cap at 32KB
    if (testSize < 1024) testSize = 1024;    // Minimum 1KB

    Serial.print("  Test block size: ");
    Serial.print(testSize);
    Serial.println(" bytes");

    // Allocate test buffer
    uint8_t* testBuffer = (uint8_t*)malloc(testSize);
    if (testBuffer == NULL) {
        printFail("SRAM allocation failed");
        return false;
    }

    Serial.println("  Running pattern tests...");

    // Test 1: Walking ones pattern
    Serial.print("    Pattern 0x55/0xAA: ");
    for (size_t i = 0; i < testSize; i++) {
        testBuffer[i] = (i & 1) ? 0xAA : 0x55;
    }
    bool pattern1Pass = true;
    for (size_t i = 0; i < testSize; i++) {
        uint8_t expected = (i & 1) ? 0xAA : 0x55;
        if (testBuffer[i] != expected) {
            pattern1Pass = false;
            Serial.print("FAIL at offset ");
            Serial.println(i);
            break;
        }
    }
    if (pattern1Pass) Serial.println("PASS");

    // Test 2: All zeros
    Serial.print("    Pattern 0x00: ");
    memset(testBuffer, 0x00, testSize);
    bool pattern2Pass = true;
    for (size_t i = 0; i < testSize; i++) {
        if (testBuffer[i] != 0x00) {
            pattern2Pass = false;
            break;
        }
    }
    Serial.println(pattern2Pass ? "PASS" : "FAIL");

    // Test 3: All ones
    Serial.print("    Pattern 0xFF: ");
    memset(testBuffer, 0xFF, testSize);
    bool pattern3Pass = true;
    for (size_t i = 0; i < testSize; i++) {
        if (testBuffer[i] != 0xFF) {
            pattern3Pass = false;
            break;
        }
    }
    Serial.println(pattern3Pass ? "PASS" : "FAIL");

    // Test 4: Address pattern (detect address line faults)
    Serial.print("    Address pattern: ");
    for (size_t i = 0; i < testSize; i++) {
        testBuffer[i] = (uint8_t)(i & 0xFF);
    }
    bool pattern4Pass = true;
    for (size_t i = 0; i < testSize; i++) {
        if (testBuffer[i] != (uint8_t)(i & 0xFF)) {
            pattern4Pass = false;
            break;
        }
    }
    Serial.println(pattern4Pass ? "PASS" : "FAIL");

    // Test 5: Random seed pattern
    Serial.print("    Pseudo-random: ");
    uint32_t seed = 0x12345678;
    for (size_t i = 0; i < testSize; i++) {
        seed = seed * 1103515245 + 12345;  // LCG
        testBuffer[i] = (uint8_t)(seed >> 16);
    }
    seed = 0x12345678;  // Reset seed
    bool pattern5Pass = true;
    for (size_t i = 0; i < testSize; i++) {
        seed = seed * 1103515245 + 12345;
        if (testBuffer[i] != (uint8_t)(seed >> 16)) {
            pattern5Pass = false;
            break;
        }
    }
    Serial.println(pattern5Pass ? "PASS" : "FAIL");

    free(testBuffer);

    uint32_t freeHeapAfter = ESP.getFreeHeap();
    Serial.print("  Free heap after test: ");
    Serial.print(freeHeapAfter);
    Serial.println(" bytes");

    if (freeHeapAfter < freeHeapBefore - 100) {
        Serial.println("  WARNING: Possible memory leak detected");
    }

    bool allPassed = pattern1Pass && pattern2Pass && pattern3Pass && pattern4Pass && pattern5Pass;

    if (allPassed) {
        printPass("SRAM test completed - all patterns verified");
        return true;
    } else {
        printFail("SRAM test failed - pattern mismatch detected");
        return false;
    }
}

// ============================================
// TEST: Flash/EEPROM Persistence Test
// ============================================
bool testFlashPersistence() {
    printProgress("Testing flash persistence (EEPROM emulation)...");

    EEPROM.begin(EEPROM_SIZE);

    // Read existing data
    uint32_t storedMagic = 0;
    uint32_t bootCounter = 0;

    EEPROM.get(EEPROM_MAGIC_ADDR, storedMagic);
    EEPROM.get(EEPROM_COUNTER_ADDR, bootCounter);

    bool isFirstBoot = (storedMagic != EEPROM_MAGIC_VALUE);

    if (isFirstBoot) {
        Serial.println("  First boot detected - initializing EEPROM...");
        bootCounter = 1;

        // Write magic value
        EEPROM.put(EEPROM_MAGIC_ADDR, (uint32_t)EEPROM_MAGIC_VALUE);
        EEPROM.put(EEPROM_COUNTER_ADDR, bootCounter);

        // Write test pattern (248 bytes at offset 8)
        uint8_t checksum = 0;
        for (int i = 0; i < 248; i++) {
            uint8_t val = (uint8_t)((i * 7 + 13) & 0xFF);
            EEPROM.write(EEPROM_PATTERN_ADDR + i, val);
            checksum ^= val;
        }
        EEPROM.write(EEPROM_CHECKSUM_ADDR, checksum);

        if (EEPROM.commit()) {
            Serial.println("  EEPROM initialized successfully");
        } else {
            printFail("EEPROM commit failed");
            EEPROM.end();
            return false;
        }
    } else {
        Serial.println("  Existing data found - verifying persistence...");
        Serial.print("  Boot counter: ");
        Serial.println(bootCounter);

        // Verify stored pattern
        Serial.print("  Verifying 248-byte pattern: ");
        uint8_t checksum = 0;
        bool patternValid = true;
        int errorCount = 0;

        for (int i = 0; i < 248; i++) {
            uint8_t expected = (uint8_t)((i * 7 + 13) & 0xFF);
            uint8_t actual = EEPROM.read(EEPROM_PATTERN_ADDR + i);
            checksum ^= actual;

            if (actual != expected) {
                patternValid = false;
                errorCount++;
                if (errorCount <= 3) {
                    Serial.print("\n    Mismatch at ");
                    Serial.print(i);
                    Serial.print(": expected 0x");
                    Serial.print(expected, HEX);
                    Serial.print(", got 0x");
                    Serial.print(actual, HEX);
                }
            }
        }

        uint8_t storedChecksum = EEPROM.read(EEPROM_CHECKSUM_ADDR);

        if (patternValid && checksum == storedChecksum) {
            Serial.println("PASS");
            Serial.println("  [PERSIST] Data survived power cycle!");
        } else {
            Serial.println("FAIL");
            Serial.print("  Errors found: ");
            Serial.println(errorCount);
            Serial.print("  Checksum - stored: 0x");
            Serial.print(storedChecksum, HEX);
            Serial.print(", calculated: 0x");
            Serial.println(checksum, HEX);
        }

        // Increment boot counter
        bootCounter++;
        EEPROM.put(EEPROM_COUNTER_ADDR, bootCounter);
        EEPROM.commit();
    }

    Serial.print("  Next boot will be #");
    Serial.println(bootCounter + 1);

    // Write/Read test with new data
    Serial.print("  Write/Read cycle test: ");
    uint32_t testValue = millis();
    EEPROM.put(260, testValue);
    EEPROM.commit();

    uint32_t readBack;
    EEPROM.get(260, readBack);

    if (readBack == testValue) {
        Serial.println("PASS");
    } else {
        Serial.println("FAIL");
        printFail("EEPROM write/read mismatch");
        EEPROM.end();
        return false;
    }

    EEPROM.end();

    printPass("Flash persistence test completed");
    return true;
}

// ============================================
// TEST: Comprehensive Flash Memory Test
// ============================================
bool testFlashComprehensive() {
    printProgress("Testing flash memory (comprehensive)...");

    uint32_t flashSize = ESP.getFlashChipSize();
    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t flashId = ESP.getFlashChipId();

    Serial.print("  Flash Chip ID: 0x");
    Serial.println(flashId, HEX);

    // Decode flash ID
    uint8_t manufacturer = flashId & 0xFF;
    uint8_t memType = (flashId >> 8) & 0xFF;
    uint8_t capacity = (flashId >> 16) & 0xFF;

    Serial.print("  Manufacturer: 0x");
    Serial.print(manufacturer, HEX);
    switch (manufacturer) {
        case 0xEF: Serial.println(" (Winbond)"); break;
        case 0xC8: Serial.println(" (GigaDevice)"); break;
        case 0x68: Serial.println(" (Boya)"); break;
        case 0x20: Serial.println(" (Micron)"); break;
        case 0x01: Serial.println(" (Spansion)"); break;
        case 0x1F: Serial.println(" (Adesto/Atmel)"); break;
        default:   Serial.println(" (Unknown)"); break;
    }

    Serial.print("  Memory Type: 0x");
    Serial.println(memType, HEX);

    Serial.print("  Capacity Code: 0x");
    Serial.print(capacity, HEX);
    Serial.print(" (");
    Serial.print(1 << (capacity - 10));
    Serial.println(" Mbit)");

    Serial.print("  Configured size: ");
    Serial.print(flashSize / 1024);
    Serial.println(" KB");

    Serial.print("  Real size: ");
    Serial.print(realSize / 1024);
    Serial.println(" KB");

    if (flashSize != realSize) {
        Serial.println("  [WARNING] Flash size mismatch - check board settings!");
    }

    // Flash mode
    FlashMode_t mode = ESP.getFlashChipMode();
    Serial.print("  Flash mode: ");
    switch (mode) {
        case FM_QIO:  Serial.println("QIO (Quad I/O - fastest)"); break;
        case FM_QOUT: Serial.println("QOUT (Quad Output)"); break;
        case FM_DIO:  Serial.println("DIO (Dual I/O)"); break;
        case FM_DOUT: Serial.println("DOUT (Dual Output - slowest)"); break;
        default:      Serial.println("Unknown"); break;
    }

    Serial.print("  Flash speed: ");
    Serial.print(ESP.getFlashChipSpeed() / 1000000);
    Serial.println(" MHz");

    // CRC check
    Serial.print("  Sketch CRC: ");
    if (ESP.checkFlashCRC()) {
        Serial.println("VALID");
    } else {
        Serial.println("INVALID - flash may be corrupted!");
        printFail("Flash CRC check failed");
        return false;
    }

    // Sketch info
    Serial.print("  Sketch size: ");
    Serial.print(ESP.getSketchSize() / 1024);
    Serial.println(" KB");

    Serial.print("  Free sketch space: ");
    Serial.print(ESP.getFreeSketchSpace() / 1024);
    Serial.println(" KB");

    printPass("Flash memory test completed");
    return true;
}

// ============================================
// Print Final Summary
// ============================================
void printSummary() {
    printHeader("TEST SUMMARY");

    int passed = 0;
    int failed = 0;

    for (int i = 0; i < testCount; i++) {
        Serial.print("  ");
        Serial.print(results[i].passed ? "[PASS]" : "[FAIL]");
        Serial.print(" ");
        Serial.println(results[i].name);

        if (results[i].passed) passed++;
        else failed++;
    }

    Serial.println();
    Serial.println("----------------------------------------");
    Serial.print("  Total: ");
    Serial.print(testCount);
    Serial.print(" | Passed: ");
    Serial.print(passed);
    Serial.print(" | Failed: ");
    Serial.println(failed);
    Serial.println("----------------------------------------");

    if (failed == 0) {
        Serial.println("  ALL TESTS PASSED!");
    } else {
        Serial.println("  SOME TESTS FAILED - Check above for details");
    }

    Serial.println();
    Serial.println("========================================");
    Serial.println("  HARDWARE TEST COMPLETE");
    Serial.println("========================================");
    Serial.println();
}

// ============================================
// SETUP
// ============================================
void setup() {
    Serial.begin(115200);
    delay(1000);  // Wait for serial to stabilize

    Serial.println();
    Serial.println();
    printHeader("ESP8266 D1 MINI HARDWARE TEST");
    Serial.println("  Starting hardware tests...");
    Serial.println("  All results will be reported via serial");
    Serial.println();
    delay(500);

    // Run all tests
    addResult("Serial Communication", testSerial(), "");
    addResult("Chip Information", testChipInfo(), "");
    addResult("GPIO (Built-in LED)", testGPIO(), "");
    addResult("SRAM Memory", testSRAM(), "");
    addResult("Flash Memory (Comprehensive)", testFlashComprehensive(), "");
    addResult("Flash Persistence (EEPROM)", testFlashPersistence(), "");
    addResult("ADC (Analog Input)", testADC(), "");
    addResult("WiFi Hardware", testWiFi(), "");
    addResult("I2C Bus Scan", testI2C(), "");
    addResult("MCP23017 LEDs (0x20)", testMCP23017_LEDs(), "");
    addResult("MCP23017 Buttons (0x21)", testMCP23017_Buttons(), "");

    // Print summary
    printSummary();
}

// ============================================
// LOOP - Interactive button monitor
// ============================================
void loop() {
    static unsigned long lastRead = 0;
    static uint8_t lastPortA = 0xFF;
    static uint8_t lastPortB = 0xFF;

    if (millis() - lastRead > 50) {  // Poll every 50ms
        lastRead = millis();

        // Check if MCP23017 at 0x21 is available
        Wire.beginTransmission(0x21);
        if (Wire.endTransmission() == 0) {
            // Read button states
            Wire.beginTransmission(0x21);
            Wire.write(MCP23017_GPIOA);
            Wire.endTransmission();
            Wire.requestFrom(0x21, 1);
            uint8_t portA = Wire.read();

            Wire.beginTransmission(0x21);
            Wire.write(MCP23017_GPIOB);
            Wire.endTransmission();
            Wire.requestFrom(0x21, 1);
            uint8_t portB = Wire.read();

            // Check for changes
            if (portA != lastPortA || portB != lastPortB) {
                for (int i = 0; i < 8; i++) {
                    bool wasPressed = !(lastPortA & (1 << i));
                    bool isPressed = !(portA & (1 << i));
                    if (isPressed && !wasPressed) {
                        Serial.print("[BUTTON] BTN");
                        Serial.print(i);
                        Serial.println(" PRESSED");

                        // Toggle corresponding LED
                        Wire.beginTransmission(0x20);
                        Wire.write(MCP23017_GPIOA);
                        Wire.endTransmission();
                        Wire.requestFrom(0x20, 1);
                        uint8_t ledState = Wire.read();
                        ledState ^= (1 << i);
                        Wire.beginTransmission(0x20);
                        Wire.write(MCP23017_OLATA);
                        Wire.write(ledState);
                        Wire.endTransmission();
                    }

                    wasPressed = !(lastPortB & (1 << i));
                    isPressed = !(portB & (1 << i));
                    if (isPressed && !wasPressed) {
                        Serial.print("[BUTTON] BTN");
                        Serial.print(i + 8);
                        Serial.println(" PRESSED");

                        // Toggle corresponding LED
                        Wire.beginTransmission(0x20);
                        Wire.write(MCP23017_GPIOB);
                        Wire.endTransmission();
                        Wire.requestFrom(0x20, 1);
                        uint8_t ledState = Wire.read();
                        ledState ^= (1 << i);
                        Wire.beginTransmission(0x20);
                        Wire.write(MCP23017_OLATB);
                        Wire.write(ledState);
                        Wire.endTransmission();
                    }
                }

                lastPortA = portA;
                lastPortB = portB;
            }
        }
    }

    yield();  // Let ESP8266 handle background tasks
}
