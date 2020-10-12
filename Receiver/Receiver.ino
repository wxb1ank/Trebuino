/*
 * FILENAME: Receiver.ino
 * CREATED:  11/24/2019
 * MODIFIED: 10/11/2020
 * AUTHOR:   wxblank
 *
 * DESCRIPTION:
 * 		Configures Bluetooth services, detects launch signals,
 * 		controls the motor upon launch, and provides a debugging
 * 		interface over serial.
 *
 */

#include <Arduino.h>
#include <LowPower.h>
#include <SPI.h>

#include <Adafruit_BLE.h>
#include <Adafruit_BluefruitLE_SPI.h>

#include "BluefruitConfig.h"

#define PIN_MOTOR         2
/*      BLUEFRUIT_SPI_RST 4 */
/*      BLUEFRUIT_SPI_CS  7 */
/*      BLUEFRUIT_SPI_IRQ 8 */

#define BLE_NAME             "Meg Trebuchet"
#define BLE_SERVICE_UUID     "57-69-6c-6c-2b-48-75-6e-74-65-72-2b-45-76-61-6e"
#define BLE_DATA             "02-01-06-11-06-6e-61-76-45-2b-72-65-74-6e-75-48-2b-6c-6c-69-57"

#define BLE_CHAR_UUID        "0x0001"
#define BLE_CHAR_PROPERTIES  "0x10"
#define BLE_CHAR_MIN_LEN     "1"
#define BLE_CHAR_VALUE       "0"
#define BLE_CHAR_DESCRIPTION "is_launching"

#define serviceString(uuid) "UUID128=" uuid
#define characteristicString(uuid, properties, min_len, value, description) "UUID=" uuid ",PROPERTIES=" properties ",MIN_LEN=" min_len ",VALUE=" value ",DESCRIPTION=" description

#define BLE_AT_NAME    "AT+GAPDEVNAME="
#define BLE_AT_SERVICE "AT+GATTADDSERVICE="
#define BLE_AT_CHAR    "AT+GATTADDCHAR="
#define BLE_AT_DATA    "AT+GAPSETADVDATA="
#define BLE_AT_ADV     "AT+GAPSTARTADV"
#define BLE_AT_NOTIFY  "AT+GATTCHAR="

Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

enum {
	kBlinkBluefruit,
	kBlinkFactoryReset,
	kBlinkSetName,
	kBlinkAddService,
	kBlinkAddCharacteristic,
	kBlinkSetData
};

static int32_t serviceId     = 0;
static int32_t isLaunchingId = 0;

#define sleep() \
	do { \
		LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF); \
	} while(0)

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
		blink(kBlinkBluefruit);
		sleep();
	}

	ble.setMode(BLUEFRUIT_MODE_COMMAND);

	Serial.println(F(" done!"));
}

static void factoryReset(void) {
	Serial.print(F(">> Performing factory reset..."));

	if(!ble.factoryReset()) {
		Serial.println(F("\n[!] Couldn't perform factory reset!"));
		blink(kBlinkFactoryReset);
		sleep();
	}

	Serial.println(F(" done!"));
}

static void setName(const char *name) {
	Serial.print(F(">> Changing Bluetooth name..."));
	
	ble.print(BLE_AT_NAME);
	ble.println(name);

	if(!ble.waitForOK()) {
		Serial.println(F("\n[!] Couldn't change Bluetooth name!"));
		blink(kBlinkSetName);
		sleep();
	}

	Serial.println(F(" done!"));
}

static void addService(const char *service, int32_t *id) {
	Serial.print(F(">> Adding trebuchet service..."));

	ble.print(BLE_AT_UUID);
	ble.println(service);

	if(!ble.waitForOK()) {
		Serial.println(F("\n[!] Couldn't add trebuchet service!"));
		blink(kBlinkAddService);
		sleep();
	}

	*id = ble.readline_parseInt();

	Serial.println(F(" done!"));
}

static void addCharacteristic(const char *characteristic, int32_t *id) {
	Serial.print(F(">> Adding service characteristic..."));

	ble.print(BLE_AT_CHAR);
	ble.println(characteristic);

	if(!ble.waitForOK()) {
		Serial.println(F("\n[!] Couldn't add service characteristic!"));
		blink(kBlinkAddCharacteristic);
		sleep();
	}

	*id = ble.readline_parseInt();

	Serial.println(F(" done!"));
}

static void setData(const char *data) {
	Serial.print(F(">> Setting advertising data..."));
	
	ble.print(BLE_AT_DATA);
	ble.println(data);

	if(!ble.waitForOK()) {
		Serial.println(F("\n[!] Couldn't set advertising data!"));
		blink(kBlinkSetData);
		sleep();
	}

	Serial.println(F(" done!"));
}

static void launch(void) {
	Serial.print(F(">> Launching!"));
	digitalWrite(LED_BUILTIN, HIGH);

	ble.print(BLE_AT_NOTIFY);
	ble.print(isLaunchingId);
	ble.println(",1");
	
	digitalWrite(PIN_MOTOR, HIGH);
	delay(200);
	digitalWrite(PIN_MOTOR, LOW);
	delay(500);

	ble.print(BLE_AT_NOTIFY);
	ble.print(isLaunchingId);
	ble.println(",0");

	digitalWrite(LED_BUILTIN, LOW);
}

void setup() {
	Serial.begin(9600);

	Serial.println(F("=== MEG SERIAL START ==="));
	pinMode(LED_BUILTIN, OUTPUT);

	initBluetooth();
	factoryReset();

	setName(BLE_NAME);
	addService(serviceString(BLE_SERVICE_UUID), &serviceId);
	addCharacteristic(characteristicString(BLE_CHAR_UUID,
	                                       BLE_CHAR_PROPERTIES
	                                       BLE_CHAR_MIN_LEN
	                                       BLE_CHAR_VALUE
	                                       BLE_CHAR_DESCRIPTION),
	                  &isLaunchingId);
	setData(BLE_DATA);

	ble.reset();

	ble.println(BLE_AT_ADV);
	ble.waitForOK();

	Serial.println(F(">> Retrieving Bluefruit info..."));
	ble.info();

	ble.echo(false);
	ble.verbose(false);

	ble.setMode(BLUEFRUIT_MODE_DATA);

	pinMode(PIN_MOTOR, OUTPUT);

	Serial.println();

	while(!ble.isConnected()) {
		delay(500);
	}
}

void loop() {
	if(ble.isConnected()) {
		if(ble.read() == 'L') {
			launch();

			delay(50);
		}
	}
}
