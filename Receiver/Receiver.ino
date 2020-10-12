/*
 * FILENAME: Receiver.ino
 * CREATED:  11/24/2019
 * MODIFIED: 10/11/2020
 * AUTHOR:   wxblank
 *
 * DESCRIPTION:
 *     Configures Bluetooth services, detects a launch signal,
 *     controls the motor upon launch, and provides a serial
 *     interface.
 *
 */

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <SPI.h>

/* Adafruit BluefruitLE nrF51 <https://github.com/adafruit/Adafruit_BluefruitLE_nRF51> */
#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>

/* Rocketscream Low-Power <https://github.com/rocketscream/Low-Power> */
#include <LowPower.h>

#include "BluefruitConfig.h"

#define PIN_MOTOR 2

#define BLE_SERVICE_STR(uuid) "UUID128=" uuid
#define BLE_CHAR_STR(uuid, properties, min_len, value, description) "UUID=" uuid ",PROPERTIES=" properties ",MIN_LEN=" min_len ",VALUE=" value ",DESCRIPTION=" description

#define BLE_DEFAULT_NAME    "Meg Trebuchet"
#define BLE_DEFAULT_SERVICE BLE_SERVICE_STR("57-69-6c-6c-2b-48-75-6e-74-65-72-2b-45-76-61-6e")
#define BLE_DEFAULT_CHAR    BLE_CHAR_STR("0x0001", "0x10", "1", "0", "is_launching")
#define BLE_DEFAULT_DATA    "02-01-06-11-06-6e-61-76-45-2b-72-65-74-6e-75-48-2b-6c-6c-69-57"

#define BLE_AT_NAME    "AT+GAPDEVNAME="
#define BLE_AT_SERVICE "AT+GATTADDSERVICE="
#define BLE_AT_CHAR    "AT+GATTADDCHAR="
#define BLE_AT_DATA    "AT+GAPSETADVDATA="
#define BLE_AT_ADV     "AT+GAPSTARTADV"
#define BLE_AT_NOTIFY  "AT+GATTCHAR="

#define BLINK_BLUEFRUIT           2
#define BLINK_FACTORY_RESET       4
#define BLINK_SET_NAME            6
#define BLINK_ADD_SERVICE         8
#define BLINK_ADD_CHARACTERISTIC 10
#define BLINK_SET_DATA           12

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

static int32_t serviceId     = 0;
static int32_t isLaunchingId = 0;

static inline void sleep(void) {
	LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);
}

static void blink(unsigned amount) {
	for(unsigned i = 0; i < amount; i++) {
		digitalWrite(LED_BUILTIN, HIGH);
		delay(500);
		digitalWrite(LED_BUILTIN, LOW);
		delay(500);
	}
}

static void initBluetooth(void) {
	Serial.print(F(">> Initializing Bluetooth..."));

	if(!ble.begin(VERBOSE_MODE)) {
		Serial.println(F("\n[!] Couldn't connect to Bluefruit!"));
		blink(BLINK_BLUEFRUIT);
		sleep();
	}

	ble.setMode(BLUEFRUIT_MODE_COMMAND);

	Serial.println(F(" done!"));
}

static void factoryReset(void) {
	Serial.print(F(">> Performing factory reset..."));

	if(!ble.factoryReset()) {
		Serial.println(F("\n[!] Couldn't perform factory reset!"));
		blink(BLINK_FACTORY_RESET);
		sleep();
	}

	Serial.println(F(" done!"));
}

static void setName(void) {
	Serial.print(F(">> Changing Bluetooth name..."));
	
	ble.println(F(BLE_AT_NAME BLE_DEFAULT_NAME));

	if(!ble.waitForOK()) {
		Serial.println(F("\n[!] Couldn't set Bluetooth name!"));
		blink(BLINK_SET_NAME);
		sleep();
	}

	Serial.println(F(" done!"));
}

static void addService(void) {
	Serial.print(F(">> Adding trebuchet service..."));

	ble.println(F(BLE_AT_SERVICE BLE_DEFAULT_SERVICE));

	if(!ble.waitForOK()) {
		Serial.println(F("\n[!] Couldn't add trebuchet service!"));
		blink(BLINK_ADD_SERVICE);
		sleep();
	}

	serviceId = ble.readline_parseInt();

	Serial.println(F(" done!"));
}

static void addCharacteristic(void) {
	Serial.print(F(">> Adding service characteristic..."));

	ble.println(F(BLE_AT_CHAR BLE_DEFAULT_CHAR));

	if(!ble.waitForOK()) {
		Serial.println(F("\n[!] Couldn't add service characteristic!"));
		blink(BLINK_ADD_CHARACTERISTIC);
		sleep();
	}

	isLaunchingId = ble.readline_parseInt();

	Serial.println(F(" done!"));
}

static void setData(void) {
	Serial.print(F(">> Setting advertising data..."));
	
	ble.println(F(BLE_AT_DATA BLE_DEFAULT_DATA));

	if(!ble.waitForOK()) {
		Serial.println(F("\n[!] Couldn't set advertising data!"));
		blink(BLINK_SET_DATA);
		sleep();
	}

	Serial.println(F(" done!"));
}

static void launch(void) {
	Serial.println(F(">> Launching!"));
	digitalWrite(LED_BUILTIN, HIGH);

	ble.print(F(BLE_AT_NOTIFY));
	ble.print(isLaunchingId);
	ble.println(F(",1"));
	
	digitalWrite(PIN_MOTOR, HIGH);
	delay(200);
	digitalWrite(PIN_MOTOR, LOW);
	delay(500);

	ble.print(F(BLE_AT_NOTIFY));
	ble.print(isLaunchingId);
	ble.println(F(",0"));

	digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
	Serial.begin(9600);

	Serial.println(F("=== MEG SERIAL START ==="));
	pinMode(LED_BUILTIN, OUTPUT);

	initBluetooth();
	factoryReset();

	setName();
	addService();
	addCharacteristic();
	setData();

	ble.reset();

	ble.println(F(BLE_AT_ADV));
	ble.waitForOK();

	Serial.println(F(">> Retrieving Bluefruit info..."));
	ble.info();

	ble.echo(false);
	ble.verbose(false);

	ble.setMode(BLUEFRUIT_MODE_DATA);

	pinMode(PIN_MOTOR, OUTPUT);

	Serial.println();
}

void loop() {
	if(ble.isConnected()) {
		if((ble.read() == 'G') && (ble.read() == 'O')) {
			launch();
		}
	}

	delay(50);
}
