# Code to run the directing of the trains in the central station

## Stopper
Rail with color detector, which will signal to the switches,
which color the incoming train has.
Than the Train will drive through the switches until
the rail of his color, where the switch lets him in, 
the communication is done through IR.

## SmartSwitch
Receive color code, test them with their own assigned
color. 
If the color is the switches color code, it will open the switch,
until the next color code is received.
