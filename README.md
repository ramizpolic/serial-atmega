# AVR USART
Implements bidirectional serial communication via USART, and allows the data packages to be stored internally into non-volatile memory such as EEPROM. 
It supports all the basic AVR microcontrollers, but is mainly designed to work with **Atmega128** and **Atmega328/P**.


## Requirements
#### Board
* Interrupts 
* USART Interface
* EEPROM

#### Softwares and libraries
* Atmel Studio 7.0
* avrdude
* realterm


## Configuration
Make sure to update the following data in order to make the program work with the other boards.
```cpp
#define F_CPU           16000000    // clock speed
#define USART_BAUDRATE  9600        // baudrate
#define RX_BUFFER_SIZE  512         // buffer size
#define RX_LINE_SIZE    128         // word size

UCSR0C|=(1<<UCSZ01)|(1<<UCSZ01);  // no parity, 1 stop bit, 8-bit data
```


## Build
* Open Atmel Studio 7
* Load project
* Build project
* Launch from project path
  ```
  avrdude -C "/path/to/avrdude.conf" -p atmega328p -c arduino -P {PORT} \
  -b 9600 -U flash:w:"$(ProjectDir)Debug\$(TargetName).hex":i 
  ```
* Open and configure **realterm**
* Start communication

---

## Demo
Once the program has been successfully flashed onto the controller, and the serial port opened, we can initiate the communication. When opening the realterm (or any other serial monitor), after properly configuring it first, you should see following:
```bash
Commands: 
 'save' - save all results sent via UART to EEPROM
 'load' - load saved results from EEPROM
 'all' - show buffer data 
 'x' - send data
 ```
 #### Tests
 We will send following data in packages (`\n` indicates end of transmission): 
 
`hello world\n`, `Oh, hi Mark\n`, `all\n`, `save\n`, `load\n`
 
Following response should be visible:
 ```bash
 # input 1
-> input: hello world
 # input 2
-> input: Oh, hi Mark
 # command 'all'
-> all results: 
-> hello world
-> Oh, hi Mark
 # command 'save'
-> Data saved to EEPROM.
-> ----- DATA -----
-> hello world
-> Oh, hi Mark
-> ----------------
 # command 'load'
-> Data loaded from EEPROM.
-> ----- DATA -----
-> hello world
-> Oh, hi Mark
-> ----------------
```


***
