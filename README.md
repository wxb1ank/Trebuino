# Trebuino
BLE Arduino receiver and iOS app to wirelessly launch "Meg the Trebuchet"

## Hardware

* [Adafruit Bluefruit LE SPI Friend](https://www.adafruit.com/product/2633)
* Arduino Pro Mini 5V

## Usage

Trebuchets implementing this receiver will expose the **Trebuchet Service** to clients (such as Meg Launcher):

* **Trebuchet Service:**
	+ **UUID:** `57-69-6c-6c-2b-48-75-6e-74-65-72-2b-45-76-61-6e`
	+ **Characteristic 0:**
		- **UUID:** `0x0001`
		- **PROPERTIES:** `0x12` (Notify, Read)
		- **DATATYPE:** `3` (INTEGER)
		- **DESCRIPTION:** `is_launching`

The trebuchet will start launching when it receives "GO" over BLE. The characteristic `is_launching` will be set to `1` for the duration of the launch and `0` when it finishes.

## Meg Launcher Demo

![Demo of the iOS app](MegLauncherDemo.gif)
