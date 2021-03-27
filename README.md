# RadioMaster TX16S 6-Position Switch firmware

This is my trial to rewrite from scratch the firmware for the 6-position switch controller on the [RadioMaster TX16S](https://www.radiomasterrc.com/article-77.html) breakout board.

Why such an endeavor, you might ask? By experimenting with [DragonLink](http://www.dragonlinkrc.com/) transmitter nearby, the *chicken feet*, incl. the 6 position switch output, stopped suddenly working on my RadioMaster TX16S. On further inspection, I was able to identify the [SiLabs C8051F330](https://www.silabs.com/mcu/8-bit/c8051f33x/device.c8051f330-gm) microcontroller on the breakout board, where the 6-pos. switches and LEDs are located, to be the central part of this functionality. Right next to the microcontroller is a 3-pin C2 debug header. When connecting [SiLabs 8-bit Debug Adapter](https://www.silabs.com/development-tools/mcu/8-bit/8-bit-usb-debug-adapter) to it, the chip answered, so was alive. Why, the code did not run anymore although, remains a mystery, as the chip was powered correctly, and as it turned out in a later testing, was in good condition. Anyhow, as the original flash was code protected, I was unable to read it out, in order to e.g. try to run it in debug mode to see where the problem was. Without any source, I erased the chip and started this project.

If you wonder if I contacted RadioMaster Support and asked for a source - I sure did, but what I got back is a peculiar answer: *Sorry we are unable to provide that information sir, may need to consult the manufacturer of the chip.* Why on earth I need to contact SiLabs for the source code of the µC in RadioMaster product...  makes absolutely no sense to me.

Do note that the 6-position switch firmware does not have much to do with the [OpenTX](https://www.open-tx.org/) firmware (<- code that is running on the TX16S main STM32F429BI microcontroller). The 6-position switch is handled by the dedicated C8051F330 µC on the breakout board and connected to the main STM32 µC only via a single wire - and over this wire analog voltage is transferred - the same way as sticks, sliders and pots do. C8051F330 has a DAC (digital-to-analog converter) that is used to generate the analog voltage.

My goal was to match the original functionality as good as possible, incl. the start-up *Knight-Rider* LED animation, as well as to match the raw value outputs of the position switches to the radio's main controller. I am pretty happy with the end result, but am open to improvement suggestions.

I started by locating the I/O configuration of the chip by tracing the tracks on the PCB:
* P0.0 - internal VRef, connection to capacitor
* P0.1 - analog out to main RadioMaster TX16S microcontroller (STM32F429BI)
* P0.2 - unused
* P0.3 - unused
* P0.4 - Position 6 Switch (rightmost), switched to GND
* P0.5 - Position 5 Switch
* P0.6 - Position 4 Switch
* P0.7 - Position 3 Switch
* P1.0 - Position 2 Switch
* P1.1 - Position 1 Switch (leftmost)
* P1.2 - Position 6 LED anode
* P1.3 - Position 5 LED anode
* P1.4 - Position 4 LED anode
* P1.5 - Position 3 LED anode
* P1.6 - Position 2 LED anode
* P1.7 - Position 1 LED anode
* P2.0 - C2 Debug data
* C2CK - C2 Debug clock

<img src="/media/TX16S_Breakout_top.jpg" alt="Top side of the PCB" width="800"/>
<img src="/media/TX16S_Breakout_bottom.jpg" alt="Bottom side of the PCB" width="800"/>

I used [SiLabs Configuration Wizard 2](https://www.silabs.com/documents/login/software/ConfigAndConfig2Install.exe) to generate the initial settings. The coding I carried out in [SiLabs Simplicity Studio v4](https://www.silabs.com/developers/simplicity-studio), using the [Keil V51 v9.60 C-compiler](https://www.keil.com/c51/c51.asp).

<img src="/media/TX16S_6-pos_debug.jpg" alt="Debug setup" width="800"/>

For own compiling, you only need to fetch and import into Simplicity Studio v4 the file *TX16S-6POS.sls* (use menu File -> Import). It includes all necessary files for setting up the project for building and debugging.

In the repository you will additionally find for reference only:
* *TX16S-6POS.hex* - pre-compiled binary for direct flashing
* *main.c* - the source file for easy access without having to first go through the project import process in Simplicity Studio v4
* *Config2.cwg* - SiLabs Configuration Wizard 2 file

### Discussion

Project discussion in RC Groups forum blog post:

<https://www.rcgroups.com/forums/showthread.php?3855505-Blog-13-RadioMaster-TX16S-6-Position-Switch-firmware>
