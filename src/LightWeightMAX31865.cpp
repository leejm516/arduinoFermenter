#include "LightweightMAX31865.h"

// Hardware SPI constructor
LightweightMAX31865::LightweightMAX31865(uint8_t cs) : _cs(cs), _use_hardware_spi(true) {
    pinMode(_cs, OUTPUT);
    digitalWrite(_cs, HIGH);
}

// Software SPI constructor
LightweightMAX31865::LightweightMAX31865(uint8_t cs, uint8_t mosi, uint8_t miso, uint8_t clk) 
    : _cs(cs), _mosi(mosi), _miso(miso), _clk(clk), _use_hardware_spi(false) {
    pinMode(_cs, OUTPUT);
    pinMode(_mosi, OUTPUT);
    pinMode(_miso, INPUT);
    pinMode(_clk, OUTPUT);
    
    digitalWrite(_cs, HIGH);
    digitalWrite(_clk, LOW);
    digitalWrite(_mosi, LOW);
}

bool LightweightMAX31865::begin(max31865_numwires_t wires) {
    if (_use_hardware_spi) {
        SPI.begin();
        SPI.setDataMode(SPI_MODE1);
        SPI.setClockDivider(SPI_CLOCK_DIV16);
    }
    
    // Configure the sensor
    uint8_t config = MAX31865_CONFIG_BIAS;
    if (wires == MAX31865_3WIRE) {
        config |= MAX31865_CONFIG_3WIRE;
    }
    
    writeRegister8(MAX31865_CONFIG_REG, config);
    delay(10);
    
    // Check if sensor is responding
    uint8_t readback = readRegister8(MAX31865_CONFIG_REG);
    return (readback == config);
}

float LightweightMAX31865::temperature(float rtd_nominal, float ref_resistor) {
    uint16_t rtd_raw = readRTD();
    return calculateTemperature(rtd_raw, rtd_nominal, ref_resistor);
}

uint16_t LightweightMAX31865::readRTD() {
    // Clear any existing faults
    clearFault();
    
    // Enable bias and wait for settling
    enableBias(true);
    delay(10);
    
    // Trigger one-shot conversion
    triggerOneShot();
    delay(65); // Conversion time
    
    // Read RTD value
    uint16_t rtd = readRegister16(MAX31865_RTD_MSB_REG);
    
    // Disable bias to save power
    enableBias(false);
    
    // Check fault bit (LSB of LSB register)
    if (rtd & 0x0001) {
        return 0xFFFF; // Fault detected
    }
    
    // Remove fault bit and return 15-bit value
    return rtd >> 1;
}

uint8_t LightweightMAX31865::readFault() {
    return readRegister8(MAX31865_FAULT_STATUS_REG);
}

void LightweightMAX31865::clearFault() {
    uint8_t config = readRegister8(MAX31865_CONFIG_REG);
    config |= MAX31865_CONFIG_FAULTCLEAR;
    writeRegister8(MAX31865_CONFIG_REG, config);
    config &= ~MAX31865_CONFIG_FAULTCLEAR;
    writeRegister8(MAX31865_CONFIG_REG, config);
}

void LightweightMAX31865::enableBias(bool enable) {
    uint8_t config = readRegister8(MAX31865_CONFIG_REG);
    if (enable) {
        config |= MAX31865_CONFIG_BIAS;
    } else {
        config &= ~MAX31865_CONFIG_BIAS;
    }
    writeRegister8(MAX31865_CONFIG_REG, config);
}

void LightweightMAX31865::triggerOneShot() {
    uint8_t config = readRegister8(MAX31865_CONFIG_REG);
    config |= MAX31865_CONFIG_1SHOT;
    writeRegister8(MAX31865_CONFIG_REG, config);
}

void LightweightMAX31865::writeRegister8(uint8_t reg, uint8_t value) {
    reg |= 0x80; // Set write bit
    
    if (_use_hardware_spi) {
        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
        digitalWrite(_cs, LOW);
        SPI.transfer(reg);
        SPI.transfer(value);
        digitalWrite(_cs, HIGH);
        SPI.endTransaction();
    } else {
        digitalWrite(_cs, LOW);
        softSPITransfer(reg);
        softSPITransfer(value);
        digitalWrite(_cs, HIGH);
    }
}

uint8_t LightweightMAX31865::readRegister8(uint8_t reg) {
    uint8_t result;
    
    if (_use_hardware_spi) {
        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
        digitalWrite(_cs, LOW);
        SPI.transfer(reg);
        result = SPI.transfer(0xFF);
        digitalWrite(_cs, HIGH);
        SPI.endTransaction();
    } else {
        digitalWrite(_cs, LOW);
        softSPITransfer(reg);
        result = softSPITransfer(0xFF);
        digitalWrite(_cs, HIGH);
    }
    
    return result;
}

uint16_t LightweightMAX31865::readRegister16(uint8_t reg) {
    uint16_t result;
    
    if (_use_hardware_spi) {
        SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE1));
        digitalWrite(_cs, LOW);
        SPI.transfer(reg);
        result = SPI.transfer(0xFF) << 8;
        result |= SPI.transfer(0xFF);
        digitalWrite(_cs, HIGH);
        SPI.endTransaction();
    } else {
        digitalWrite(_cs, LOW);
        softSPITransfer(reg);
        result = softSPITransfer(0xFF) << 8;
        result |= softSPITransfer(0xFF);
        digitalWrite(_cs, HIGH);
    }
    
    return result;
}

uint8_t LightweightMAX31865::softSPITransfer(uint8_t data) {
    uint8_t result = 0;
    
    for (int i = 7; i >= 0; i--) {
        digitalWrite(_mosi, (data >> i) & 0x01);
        digitalWrite(_clk, HIGH);
        delayMicroseconds(1);
        result = (result << 1) | digitalRead(_miso);
        digitalWrite(_clk, LOW);
        delayMicroseconds(1);
    }
    
    return result;
}

float LightweightMAX31865::calculateTemperature(uint16_t rtd_raw, float rtd_nominal, float ref_resistor) {
    if (rtd_raw == 0xFFFF) {
        return NAN; // Fault condition
    }
    
    // Convert ADC value to resistance
    float rtd_resistance = (rtd_raw * ref_resistor) / 32768.0;
    
    // Callendar-Van Dusen equation (simplified for PT100)
    // R(T) = R0 * (1 + A*T + B*T^2)
    // Solving for T using quadratic formula
    const float A = 3.9083e-3;
    const float B = -5.775e-7;
    
    float ratio = rtd_resistance / rtd_nominal;
    float temp = (ratio - 1.0) / A;
    
    // For better accuracy, use quadratic correction
    if (temp < 0) {
        // For negative temperatures, different coefficients are used
        // This is a simplified calculation
        temp = (ratio - 1.0) / A;
    } else {
        // Quadratic correction for positive temperatures
        float discriminant = A * A - 4 * B * (1.0 - ratio);
        if (discriminant >= 0) {
            temp = (-A + sqrt(discriminant)) / (2 * B);
        }
    }
    
    return temp;
}