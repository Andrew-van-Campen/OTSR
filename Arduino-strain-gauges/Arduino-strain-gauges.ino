#include <SPI.h>
#include <SD.h>

// Chip select (CS) pins
#define minADC A0
#define maxADC A1
#define SDpin 9

int gaugeData[4];

void setup() {
    Serial.begin(9600);

    // Initialize SPI
    SPI.begin();
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
    for (int i = minADC; i <= maxADC; i++) {  // Initialize ADC pins
        pinMode(i, OUTPUT);
        digitalWrite(i, HIGH);
    }

    // Initialize SD
    pinMode(SDpin, OUTPUT);
    digitalWrite(SDpin, HIGH);
    SD.begin(SDpin);

    // Reset ADCs
    for (int i = minADC; i <= maxADC; i++) {
        digitalWrite(i, LOW);
        SPI.transfer(0x06);  // Instruction to reset ADC
        digitalWrite(i, HIGH);
    }
    delay(250);

    // Set ADC configuration registers
    for (int i = minADC; i <= maxADC; i++) {
        digitalWrite(i, LOW);
        SPI.transfer(0x42);  // Instruction to write 3 bytes starting at register 0
        SPI.transfer(0x0E);  // Settings for register 0
        SPI.transfer(0xC4);  // Settings for register 1
        SPI.transfer(0xC0);  // Settings for register 2
        digitalWrite(i, HIGH);
        //Serial.println(readConfig(i), HEX);  // Print register settings
    }

    // Start conversions
    for (int i = minADC; i <= maxADC; i++) {
        digitalWrite(i, LOW);
        SPI.transfer(0x08);  // Instruction to start conversions
        digitalWrite(i, HIGH);
    }
}

void loop() {
}

// Convert output from ADC into a readable value
int convertOutput(unsigned long data) {
    //long minval = -1;
    //for (int i = 1; i <= 23; i++) minval = minval * 2;
    long maxval = 1;
    for (int i = 1; i <= 23; i++) maxval = maxval * 2;
    maxval = maxval - 1;
    return (1000 * (twosComplement(data, 24) )) / (maxval);
}

// Convert an N-bit two's complement number to a long integer
long twosComplement(unsigned long num, unsigned char N) {
    long output;
    if (num >> (N - 1)) {
        output = -1 * (~((num - 1) << (33 - N)) >> (33 - N));
        if (output == 0) {
            output = -1;
            for (int i = 1; i <= N - 1; i++) {
                output = output * 2;
            }
        }
    } else {
        output = num;
    }
    return output;
}

// Read raw data from ADC
unsigned long readData(int chipSelect) {
    digitalWrite(chipSelect, LOW);
    SPI.transfer(0x10);  // Instruction to read data
    unsigned long data = 0;
    for (int i = 1; i <= 3; i++) {  // Get data bytes
        data = (data << 8) + SPI.transfer(0);
    }
    digitalWrite(chipSelect, HIGH);
    return data;
}

// Read ADC configuration registers
unsigned long readConfig(int chipSelect) {
    unsigned long output;
    digitalWrite(chipSelect, LOW);
    SPI.transfer(0x23);  // Instruction to read 4 bytes starting at register 0
    for (int i = 1; i <= 4; i++) {
        output = (output << 8) + SPI.transfer(0);
    }
    digitalWrite(chipSelect, HIGH);
    return output;
}
