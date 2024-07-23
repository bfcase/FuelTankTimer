# FuelTankTimer
Cheap Yellow Display Fuel Tank Timer for 2 fuel Tank For Aircraft 
Created on PlatformIO

Friend was trying to find a timer he liked for monitoring how much fuel he was using out of each of the 2 fuel tanks in his airplane.
I had just learned about CYD (Cheap Yellow Display) so I decided to try it to build something for him.
This is what I came up with using may amateur program skills.

Things to do still are write the code to store the timer values in the EEPROM when the unit powers off
and of course add photos and documentation to this Github (also my 1st time creating a Github project)

unit consists of...
the CYD, (https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)  $23.31
Connectors for the CYD ( Micro JST MX 1.25 4Pins Connector Plug Socket 1.25mm Pitch Female & Male Connector PCB Socket 150mm Cable)
a 12v to 5V voltage converter, (AITRIP 8 Pack Mini MP1584EN DC-DC Buck Converter Adjustable Power Supply Module 24V to 12V 9V 5V 3VCompatible with Arduino Raspberry Pi) $1.00 Each
a capacitor to power it long enough to write value to EEPROM when it loses power. (8.5x17x14mm(0.31x0.67x0.55in) Super Capacitor 5.5V 4F Farad Capacitance Winding Type Energy Storage for On Board Backup Energy Storage) $4.50 each
A potentiometer (voltage divider) to step the voltage down for the CYD to sense the voltage loss.
A small prototype board to mount it on.
Some 3d printed mounting parts, so it will fit in a standard 80mm instrument hole.
USB extension cable (Poyiccot Micro USB Extension Cable, 90 Degree Right Angle Micro USB Male to Micro USB Female Power Extension Panel Mount Type Cable for Data & Charge) $8.99




