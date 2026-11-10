# Welcome to another replica project for the Newtech Model 6 Music Board

The Model 6 Music Board from Newtech Computer Systems was launched in 1977 as a low cost alternative to other,
more expensive sound solutions for the S-100 bus like the D+7A from Cromemco.
In contrast to the D+7A with its up to seven bidirectional analog channels, the Newtech board implements just a single audio output channel
and does not provide input channels for e.g. connecting joysticks or keyboards.

The minimalistic design was based on a simple bus interface with address decoder,
a digital to analog converter (DAC) built by a six-step resistor ladder, and a single LM380 IC as simple low power amplifier.

The Music Board can be equipped with a single 2" speaker, or connected to an external amplifier via a phono jack.
Multiple boards can be combined for stereo output.

An optional daughter board can be added to improve the audio quality by an active low pass filter.

Due to the simplistic design, audio generation is achieved by directly and continuously driving the
DAC by the CPU with a constant rate, producing a wave output.
Although this keeps the CPU busy from doing other things, there are no limits concerning the type of audio output.
Music can be produced as well as arbitrary sounds, including human speech or noise.
Multiple voices can be mixed into a single channel, which is then output via the hardware.
Synthesis is just a matter of software, and primarily limited by the performance of the CPU.
Assembler programming, however, is mandatory in order to feed a continuous stream of 6-bit audio samples into the board.

This repository contains the replica KiCAD files for both the Music Board and the low pass filter PCB.
Please consult the Newtech User Manual for a parts list / bill of material.

There are other great replica projects around. The aim of this project is to be as close as possible to the original hardware.
Just a few components need to be added in order to build your own working sound solution for your S-100 based system.

Just as a reference, here is a sample picture of the original hardware without low pass filter

<img width="3148" height="1022" alt="S100_Sound_Model_6" src="https://github.com/user-attachments/assets/6e4ede28-74fc-4d9a-80ee-93497c730858" />

And here a picture of the replica, again without low pass filter, built up with components close to the original

<img width="2124" height="667" alt="image" src="https://github.com/user-attachments/assets/efc5b197-cb38-4db7-b03b-907f0026ad49" />

Note that the bus contact fingers of the replica are not gold plated as the original.
Gold plating is a process which needs to be negotiated directly with the PCB producer of your choice and can't be specified by a directive in the Gerber files.

The replica is a nice little project requiring limited budget and skills to create your first own hardware for your personal S-100 system.

This by the way is the very first version of the replica design, please feel free to comment on any kind of improvement,
as long as the replica itself stays as close as possible to the original. A couple of limitations are intentional, so there is for instance
no jumper for activating/deactivating the internal speaker, and as with the original, the low pass filter needs to be added with a daughter board
and is intentionally not integrated into the board design as with other replicas.

For your convenience, a copy of the original Model 6 Music Board user manual is provided,
as well as a BDOS based audio test program I once have written for diverse sound solutions for the S-100 bus,
which also supports up to two Newtech Model 6 Music Boards in parallel. I have created some kind of notation
for playing music scores with multiple channels, an example is the file FANFAR, which works with two boards configured at ports 24H and 28H.
