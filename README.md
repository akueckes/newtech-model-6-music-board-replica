# Heading level 1
Welcome to another replica project for the Newtech Model 6 Music Board

The Model 6 Music Board from Newtech Computer Systems was launched in 1977 as a low cost alternative to other,
more expensive sound solutions for the S-100 bus like the D+7A from Cromemco.
In contrast to the D+7A board with its up to seven DACs, the Music Board implements just a single audio channel
and does not provide ADC channels for e.g. connecting joysticks.

The minimalistic design was based on a simple bus interface with address decoder,
a digital to analog converter (DAC) built by a simple resistor ladder, and a single LM380 IC as simple low power amplifier.

The Music Board can be equipped with a single 2" speaker, or connected to an external amplifier via a phono jack.
Multiple boards can be combined for stereo output.

An optional daughter board can be added to improve the audio quality by an active low pass filter.

Due to the simplistic design, audio generation is achieved by directly and continuously driving the
DAC by he CPU with a constant rate, producing a wave output.
Although this keeps the CPU busy from doing other things, there are no limits concerning the type of audio output.
Music can be produced as well as human speach or noise.
Multiple voices can be mixed into a single channel, which is then output via the hardware.
Synthesis is just a matter of software, and primarily limited by the performance of the CPU.
Assembler programming, however, is mandatory in order to feed a continuous stream of samples into the board.

This repository contains the replica KiCAD files for both the Music Board and the low pass filter PCB, as well as a BOM.

There are other great replica projects around. This aim of this project is to be as close as possible to the original hardware.
Just a few components need to be added in order to build your own working sound solution for your S-100 based system.

Just as a reference, here is a sample picture of the original hardware without low pass filter

<img width="3148" height="1022" alt="S100_Sound_Model_6" src="https://github.com/user-attachments/assets/6e4ede28-74fc-4d9a-80ee-93497c730858" />

And here a picture of the replica, again without low pass filter, built up with components close to the original

<img width="2124" height="667" alt="image" src="https://github.com/user-attachments/assets/efc5b197-cb38-4db7-b03b-907f0026ad49" />

Note that the bus contact fingers of the replica are not gold plated as the original.
Gold plating is a process which needs to be negotiated directly with the PCB producer of your choice and can't be specified by a directive in the Gerber files.

The replica is a nice project requiring limited budget and skills to create your first own hardware for 

For your convenience, a copy of the original Model 6 Music Board user manual is provided,
as well as an audio test program I once have written for diverse sound solutions for the S-100 bus,
which also supports up to two Newtech Model 6 Music Boards.
