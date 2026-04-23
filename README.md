# RadarSpeedgun-HLK-LD2415H #
is a simple fun project, inspired by the info posts often seen around schools. License is GPL v3.

## Intention ##
to measure and display speed of passing vehicles (or persons). 

## Simple setup ##
utilizes sensor HLK-LD2415H, 2*16 LCD-display, buzzer and arduino pro mini 3.3V, 8MHz. Power comes from 
either 3S lipo or 3 LiIon cells 18650 of your choice. 

## Build ##
* common ground between battery, sensor, display, arduino
* straight connects between sensor and arduino for software-serial (crossed RX<->TX as always)
* straight connects between display and arduino for i2c
* optional buzzer connected to arduino
* 3 cells in series,
  *  power display from 1S,
  *  power arduino from 2S,
  *  power sensor from 3S
* compile and upload software via arduino IDE. (requires some typical libraries to be installed)    
* stl-files for 3D-printable casing included

## Run ##
connect balancer plug for power (or insert 3 LiIon cells, cell holder not included). Aim at traffic, observe continuous 
periodic measurements of approaching and departing vehicles and persons.
In detail: top row has latest measurement on the left and fastest measurement of last 3 seconds on right, 
the second row shows diagnostic info

![ casing ](sg1.jpeg)

![ display ](sg2.jpg)

## power ##
to avoid the need for various voltages' regulators, using a 3S LiPo or LiIon rechargeable pack works well enough.
Positive side effect: take the respective 1S,2S,3S voltages from the balance plug for the 3 components.
Choices:
 * from existing RC equipment, use 3S LiPo (only connect balance plug)  and appropriate charging equipment, or
 * get 3 LiIon cells plus a usb-charger (for one or more cells); 3Dprint 3 single cell holders, glue together, wire in series and attach balance connector.
   
![ display ](sg3.jpg)
