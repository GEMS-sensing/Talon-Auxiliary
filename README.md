
<!-- [![DOI](https://zenodo.org/badge/678911830.svg)](https://zenodo.org/doi/10.5281/zenodo.13377368) -->

# Talon-Auxiliary
Talon daughterboard for [Flight data logger](https://github.com/GEMS-sensing/DFM_-_Flight?tab=readme-ov-file). Designed to interface with [Kestrel](https://github.com/GEMS-sensing/Project-Kestrel) main board. This board is designed to provide data logger with support to interface with analog and pulsed digital signals.
# Repo Roadmap
#### [Documents](Documents/) 

General document and image storage for repo and README

#### [Hardware](Hardware/)

Current board files (Eagle), Bill of Materials (BoM), and other design documents

#### [Production](Production/)

Contains the various Gerber and pick and place files required to have the Printed Circuit Boards (PCBs) manufactured or populated 

#### [Software](Software/)

The software associated with the piece of hardware, this is usually diagnostic software used for verifying or investigating the hardware

<!-- #### [Mechanical](Mechanical/)

Mechanical design files and assembly documents -->

<!-- #### [Testing](Testing/)

Scripts and results from the testing process and development process. Contains more detailed information about documented issues among other testing. 
 -->


## Overview
* **Type:** Talon
* **Interfaces:** Analog, digital, digital (open drain)
* **Release Version:** v1.5 - 2023/05/15

![PCB render image, top](Documents/Images/BoardRender_TOP.png)

![PCB render image, bottom](Documents/Images/BoardRender_BOTTOM.png)

<!-- ![Haar v0.0 - Bottom](Documents/Images/Haar_0v0_Bottom_Cropped.jpg) -->

## Features
### On-Board
* High precision ADC interface
	* 5V input
	* PGA
	* 16 bit (93.75 &mu;V @ 5V input, 3.9 &mu;V @ 0.25V input)
	* up to 860 SPS (reduced resolution)
* Hardware pulse counter
	* Reduces power consumption of logger by eliminating need to wake up SoM for frequent pulse interrupts 
	* 16 bit (65536) pulse count limit
	* Analog front end with hysteresis to deal with noisy inputs
	* Support for self diagnostics to allow for automated testing of operation 
* Push-pull and Open-drain compatible inputs 
* Load switch control for each port with current limiting 
* Manual switches to select each port between 3v3 and 5v power/logic
	* Chosen to be manual to prevent a firmware bug from damaging a sensor
* EEPROM with UUID built in - can store additional values defined by firmware to track offsets, etc
* ESD protected inputs

### Interface - Sensor
* I<sup>2</sup>C data interface
* M12 circular connector 

![Pinout](Documents/Images/Pinout.png)

<!-- <picture>
  <source media="(prefers-color-scheme: dark)" srcset="Documents/Images/Pinout_LIGHT.png">
  <img alt="Hedorah Connector Pinout" src="Documents/Images/Pinout.png">
</picture> -->

| **Pin** | **Connection** | **Description** |
|---------|----------------| --------------- |
| 1     | Power (3.3V  **OR** 5V) | Output power to sensors, limited to 50mA continuous current, voltage level is selected by DIP switch next to each port |
| 2     | Open Drain            | Input for switch based sensors - 10k&Omega; pullup to Power rail|
| 3     | Ground            | Power and data ground for system |
| 4      | Analog            | Analog voltage input |
| 5      | Digital | Digital pulse input |


#### Sensor Wiring

- Switch based sensors
	- e.g. reed switch, hall effect, etc
	- Devices where the sensor open or closes a switch depending on state
	- Wire switch between Ground and Open Drain lines
	- WARNING: The Talon will apply rail power to the switch line via a pullup resistor - ensure the sensor is rated for the rail voltage being used (3.3V or 5V)
	- NOTE: To limit power consumption, only recommended for Nominally Open (NO) switches
- Pulse based sensors
	- e.g. microcontroller pulse outputs, basic digital sensors, etc
	- Devices where the output is either 0V or a logic voltage based based on a sensor state
	- Wire grounds together and wire the pulse output to the Digital pin, wire Power to sensor if needed
	- NOTE: The digital input is 5V tolerant regardless of the voltage on the power bus
- Analog based sensors
	- e.g. Potentiometers, basic sensors with variable voltage output, etc
	- Devices where the output is a voltage which varies with sensed value
	- Wire grounds together and wire the analog output of the sensor to the Analog pin, wire Power to sensor if needed
	- NOTE: The analog input is rated up to 5V regardless of the voltage on the power bus

#### Bus Selection
The power provided by each Aux Talon port can be selected as either 3.3V (default) or 5V - depending on the demands of the sensor.

This setting is changed by flipping the DIP switches (`S1`, `S2`, `S3`) on the Aux Talon itself, the position for a given voltage is noted next to each switch. Each switch controls the Aux Talon port it is adjacent to and each port can be switched independently of the others.

> [!WARNING]
> Application of voltage beyond the sensor specifications may result in irreparable damage to the sensor!


<!-- ### Interface
* I<sup>2</sup>C
* M12 circular connector  -->

<!-- <picture>
  <source media="(prefers-color-scheme: dark)" srcset="Documents/Pinout_LIGHT.png">
  <img alt="Haar Connector Pinout" src="Documents/Pinout.png">
</picture> -->

<!-- ## Specifications
* **Voltage - Supply:** 3.3V
* **Voltage - Logic:** 3.3V
* **Input Current (Avg, 1s period): 30.95&mu;A @ 3.3V**
* **Input Current (Avg, 5s period): 107.43&mu;A @ 3.3V**
* **Input Current (Max): 0.45mA @ 3.3V**

#### Sensor
**Temperature** [^1]
* Range: -40 ~ 125°C 
* Accuracy: &plusmn; 0.2°C
* Repeatability: &plusmn; 0.04°C

**Pressure** [^2]
* Range: 300 ~ 1200 hPa  
* Accuracy: &plusmn; 1 hPa
* Repeatability: &plusmn; 0.06 hPa

**Humidity** [^1]
* Range: 0 ~ 100%
* Accuracy: &plusmn; 2%
* Repeatability: 0.08%

[^1]: [SHT31 Datasheet](https://sensirion.com/media/documents/213E6A3B/63A5A569/Datasheet_SHT3x_DIS.pdf)
[^2]: [DPS368 Datasheet](https://www.infineon.com/dgdl/Infineon-DPS368-DS-v01_00-EN.pdf?fileId=5546d46269e1c019016a0c45105d4b40) -->

<!-- ## Version History

`v0.0` - Initial primal version with screw terminal

`v0.1` - Initial 'hard mount' version, using M12 connector

`v1.0` - Removed pullup select[^3], added logic level buffer and voltage reg on input

`v1.1` - Moved via away from connector pads to prevent bridging when soldering 

`v2.0` - Fixed issue with address select jumper[^3] -->

<!-- ## Jumper Settings 

> [!IMPORTANT]
> Jumper settings valid for version `v2.0` and beyond[^3]

[^3]: See Errata

**Configuration Jumpers**
| Jumper | Purpose | Open | Closed | Default | 
| ------ | ------- | ---------- | ---------- | ----- | 
| `JP1` | I<sup>2</sup>C Address Select | Base Address<sup>&dagger;</sup> | Alt Address<sup>&Dagger;</sup> | Open |

&dagger; SHT31 = `0x44`, DPS368 = `0x76`

&Dagger; SHT31 = `0x44`, DPS368 = `0x77`
 -->

![PCB trace CAD image, top](Documents/Images/Copper_TOP.png)

![PCB trace CAD, bottom](Documents/Images/Copper_BOTTOM.png)

## Known Issues/Errata

None
<!-- #### Solder Jumper Stencil

**Version Affected:** All

**Issue:** Solar input does not support very low impedance sources (e.g. lead acid battery) - see details in issue [#20](https://github.com/GEMS-sensing/Project-Kestrel/issues/20)

**Workaround:** 

* Avoid use of low impedance batteries for input
* Include series impedance to prevent current spikes (not desirable)
* Perform board patch described in issue [#20](https://github.com/GEMS-sensing/Project-Kestrel/issues/20) -->
