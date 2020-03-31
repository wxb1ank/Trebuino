/*
 * FILENAME: Receiver.ino
 * CREATED:  11/24/2019
 * AUTHOR:   wxblank
 *
 * DESCRIPTION:
 * 		 Configures Bluetooth services, detects launch signals,
 * 		 controls the motor upon launch, and provides a debugging
 * 		 interface over serial.
 *
 */

#include <Arduino.h>
#include <SPI.h>

/* DISABLE IN PRODUCTION */
//#define __MEG_DEBUG__
//#define VERBOSE_LOG
//#define NO_SLEEP_ON_FATAL
//#define PREFLIGHT
//#define NO_BLUETOOTH
//#define BLINK_ON_ERROR
/* DISABLE IN PRODUCTION */

#ifndef NO_SLEEP_ON_FATAL
#include <LowPower.h>
#endif // NO_SLEEP_ON_FATAL

#ifndef NO_BLUETOOTH
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>

#include "BluefruitConfig.h"
#endif // NO_BLUETOOTH

#define PIN_MOTOR       2
// BLUEFRUIT_SPI_RST	4
// BLUEFRUIT_SPI_CS		7
// BLUEFRUIT_SPI_IRQ	8

#define BLE_NAME "Meg Trebuchet"
#define BLE_UUID "UUID128=57-69-6c-6c-2b-48-75-6e-74-65-72-2b-45-76-61-6e"
#define BLE_ISLAUNCHING_UUID "UUID=0x0001,PROPERTIES=0x10,MIN_LEN=1,VALUE=0,DESCRIPTION=is_launching"
#define BLE_DATA "02-01-06-11-06-6e-61-76-45-2b-72-65-74-6e-75-48-2b-6c-6c-69-57"

// Flags: LE General Discoverable, BR/EDR Not Supported
// Services: Trebuchet Service (128-bit)

#define AT_NAMECMD "AT+GAPDEVNAME="
#define AT_UUIDCMD "AT+GATTADDSERVICE="
#define AT_CHARCMD "AT+GATTADDCHAR="
#define AT_DATACMD "AT+GAPSETADVDATA="
#define AT_ADVCMD "AT+GAPSTARTADV"
#define AT_NOTIFY "AT+GATTCHAR="

#define LAUNCH_CMD 'G' // Send 'G' over UART to launch

#ifndef NO_BLUETOOTH

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

int32_t serviceId;
int32_t isLaunchingId;

#endif // NO_BLUETOOTH

void sleep() {
	#ifndef NO_SLEEP_ON_FATAL
	LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
	#endif // NO_SLEEP_ON_FATAL
}

#ifdef __MEG_DEBUG__

#define BLINK_BLUEFRUIT_ERROR 4
#define BLINK_CHANGENAME_ERROR 6
#define BLINK_ADDSERVICE_ERROR 8
#define BLINK_ADDCHAR_ERROR 10
#define BLINK_SETDATA_ERROR 12
#define BLINK_RESET_ERROR 14

void blink(unsigned short amount) {
	#ifdef BLINK_ON_ERROR
	for(unsigned short i = 0; i < amount; i++) {
		digitalWrite(LED_BUILTIN, HIGH);
		delay(500);
		digitalWrite(LED_BUILTIN, LOW);
		delay(500);
	}
	#endif // BLINK_ON_ERROR
}

#endif // __MEG_DEBUG__

#ifndef NO_BLUETOOTH

void factory_reset() {
	#ifdef __MEG_DEBUG__
	Serial.print(F(">> Performing factory reset..."));
	#endif // __MEG_DEBUG__
	if(!ble.factoryReset()) {
		#ifdef __MEG_DEBUG__
		Serial.print(F("\n[!] Couldn't perform factory reset!\n"));
		blink(BLINK_RESET_ERROR);
		#endif // __MEG_DEBUG__
		sleep();
	} else {
		#ifdef __MEG_DEBUG__
		Serial.print(F(" done!\n"));
		#endif // __MEG_DEBUG__
	}
}

void change_name(char name[]) {
	#ifdef __MEG_DEBUG__
	Serial.print(F(">> Changing Bluetooth name..."));
	#endif // __MEG_DEBUG__
	ble.print(AT_NAMECMD);
	ble.println(name);
	if(!ble.waitForOK()) {
		#ifdef __MEG_DEBUG__
		Serial.print(F("\n[!] Couldn't change Bluetooth name!\n"));
		blink(BLINK_CHANGENAME_ERROR);
		#endif // __MEG_DEBUG__
		sleep();
	} else {
		#ifdef __MEG_DEBUG__
		Serial.print(F(" done!\n"));
		#endif // __MEG_DEBUG__
	}
}

void add_service(char service[], int32_t *id) {
	#ifdef __MEG_DEBUG__
	Serial.print(F(">> Adding trebuchet service..."));
	#endif // __MEG_DEBUG__
	ble.print(AT_UUIDCMD);
	ble.println(service);
	if(!ble.waitForOK()) {
		#ifdef __MEG_DEBUG__
		Serial.print(F("\n[!] Couldn't add trebuchet service!\n"));
		blink(BLINK_ADDSERVICE_ERROR);
		#endif // __MEG_DEBUG__
		sleep();
	} else {
		id = ble.readline_parseInt();
		#ifdef __MEG_DEBUG__
		Serial.print(F(" done!\n"));
		#endif // __MEG_DEBUG__
	}
}

void add_characteristic(char characteristic[], int32_t *id) {
	#ifdef __MEG_DEBUG__
	Serial.print(F(">> Adding service characteristic..."));
	#endif // __MEG_DEBUG__
	ble.print(AT_CHARCMD);
	ble.println(characteristic);
	if(!ble.waitForOK()) {
		#ifdef __MEG_DEBUG__
		Serial.print(F("\n[!] Couldn't add service characteristic!\n"));
		blink(BLINK_ADDCHAR_ERROR);
		#endif // __MEG_DEBUG__
		sleep();
	} else {
		id = ble.readline_parseInt();
		#ifdef __MEG_DEBUG__
		Serial.print(F(" done!\n"));
		#endif // __MEG_DEBUG__
	}
}

void set_data(char data[]) {
	#ifdef __MEG_DEBUG__
	Serial.print(F(">> Setting advertising data..."));
	#endif // __MEG_DEBUG__
	ble.print(AT_DATACMD);
	ble.println(data);
	if(!ble.waitForOK()) {
		#ifdef __MEG_DEBUG__
		Serial.print(F("\n[!] Couldn't set advertising data!\n"));
		blink(BLINK_SETDATA_ERROR);
		#endif // __MEG_DEBUG__
		sleep();
	} else {
		#ifdef __MEG_DEBUG__
		Serial.print(F(" done!\n"));
		#endif // __MEG_DEBUG__
	}
}

#endif // NO_BLUETOOTH

void setup() {
	#ifdef __MEG_DEBUG__
	Serial.begin(9600); // open serial connection (reads at 19200 bps)
	Serial.print(F("=== MEG SERIAL START ===\n"));
	pinMode(LED_BUILTIN, OUTPUT);
	#endif // __MEG_DEBUG__

	#ifndef NO_BLUETOOTH

	#ifdef __MEG_DEBUG__
	Serial.print(F(">> Initializing Bluetooth!\n"));
	#endif // __MEG_DEBUG__

	if(!ble.begin(VERBOSE_MODE)) {
		#ifdef __MEG_DEBUG__
		Serial.print(F("[!] Couldn't connect to Bluefruit!\n"));
		blink(BLINK_BLUEFRUIT_ERROR);
		#endif // __MEG_DEBUG__
		sleep();
	}

	ble.setMode(BLUEFRUIT_MODE_COMMAND);

	factory_reset();

	change_name(BLE_NAME);
	add_service(BLE_UUID, &serviceId);
	add_characteristic(BLE_ISLAUNCHING_UUID, &isLaunchingId);
	set_data(BLE_DATA);

	ble.reset(); // software reset to add service

	ble.println(AT_ADVCMD); // start advertising (if not already)
	ble.waitForOK();

	#ifdef __MEG_DEBUG__
	#ifdef PREFLIGHT
	Serial.print(F(">> Retrieving Bluefruit info...\n"));
	ble.info();
	#endif // PREFLIGHT
	#endif // __MEG_DEBUG__

	ble.echo(false); // disable command echo
	ble.verbose(false);

	ble.setMode(BLUEFRUIT_MODE_DATA);

	#endif // NO_BLUETOOTH

	pinMode(PIN_MOTOR, OUTPUT);

	#ifdef __MEG_DEBUG__
	Serial.println();
	#endif // __MEG_DEBUG__

	#ifndef NO_BLUETOOTH

	while(!ble.isConnected()) {
		delay(500);
	}

	#endif // NO_BLUETOOTH
}

void launch() {
	#ifdef __MEG_DEBUG__
	Serial.print(F(">> Launching!"));
	digitalWrite(LED_BUILTIN, HIGH);

	unsigned long start = millis();
	#endif // __MEG_DEBUG__

	#ifndef NO_BLUETOOTH

	ble.print(AT_NOTIFY);
	ble.print(isLaunchingId);
	ble.println(",1");

	#endif // NO_BLUETOOTH
	
	digitalWrite(PIN_MOTOR, HIGH); // enable power output
	delay(200);                    // TODO: find correct delay for one revolution
	digitalWrite(PIN_MOTOR, LOW);  // disable power output

	delay(500);

	#ifndef NO_BLUETOOTH

	ble.print(AT_NOTIFY);
	ble.print(isLaunchingId);
	ble.println(",0");

	#endif // NO_BLUETOOTH

	#ifdef __MEG_DEBUG__
	char *tmp = (char *)malloc(sizeof(unsigned long));
	sprintf(tmp, " (%lums)\n", millis() - start);
	Serial.print(tmp);
	free(tmp);

	digitalWrite(LED_BUILTIN, LOW);
	#endif // __MEG_DEBUG__
}

void loop() {
	#ifndef NO_BLUETOOTH

	if(ble.isConnected()) {
		int cmd = ble.read();
		if(cmd == LAUNCH_CMD) {
			launch();
		}

		delay(50);
	}

	#endif // NO_BLUETOOTH
}
