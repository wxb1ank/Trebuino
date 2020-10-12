#ifndef __WX_BLUEFRUIT_CONFIG_H
#define __WX_BLUEFRUIT_CONFIG_H

// COMMON SETTINGS
// ----------------------------------------------------------------------------------------------
// These settings are used in both SW UART, HW UART and SPI mode
// ----------------------------------------------------------------------------------------------
#define BUFSIZE                        160   // Size of the read buffer for incoming data
#define VERBOSE_MODE                   false // If set to 'true' enables debug output


// SHARED SPI SETTINGS
// ----------------------------------------------------------------------------------------------
// The following macros declare the pins to use for HW and SW SPI communication.
// SCK, MISO and MOSI should be connected to the HW SPI pins on the Uno when
// using HW SPI.  This should be used with nRF51822 based Bluefruit LE modules
// that use SPI (Bluefruit LE SPI Friend).
// ----------------------------------------------------------------------------------------------
#define BLUEFRUIT_SPI_CS               7
#define BLUEFRUIT_SPI_IRQ              8
#define BLUEFRUIT_SPI_RST             -1    // Optional but recommended, set to -1 if unused

#endif /* __WX_BLUEFRUIT_CONFIG_H */
