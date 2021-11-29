## Arduino MEGA2560 ports

 - PORT A 22-29
 - PORT C 30-37
 - PORT L 42-49
 - PORT B 50-7

## How to use

### Set data direction

DDRx

Replace x with port letter, this variable takes a byte of 0-255 where each binary bit repersents weather that pin is a output

### Set port value

PORTx

Replace x with port letter you want to write to, takes a uint8_t type value

### Read port value

PINx

Replace x with port letter you want to read from