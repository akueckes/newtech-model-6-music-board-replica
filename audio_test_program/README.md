The AUDIO program needs CP/M to run and is currently supporting four different sound boards:

- ADS Noisemaker (GI-AY-8910)
- Cromemco D+7A (DAC)
- Newtech Model 6 Music Board (DAC)
- Solid State Music SB-1 (synthesizer)

All files are also included in the disc image file audio.dsk (IBM 3740 format).

When executed, the AUDIO program first asks for the CPU clock of the current processor.

The program then prints a config message and a command overview:

```
-> CURRENT HARDWARE IS D+7A AT I/O BASE 18H

SELECT ONE OF:

H SELECT HARDWARE
T PLAY TONE
N PLAY NOISE
F PLAY SAMPLE FILE
S PLAY SCORE FILE
W LOAD WAVE TABLE FILE
A AUDIO TEST
J JOYSTICK TEST
Q QUIT

-
```
Default hardware is a Cromemco D+7A. Since there usually is no way to
autodetect the sound hardware, you might need to change the
hardware with the SELECT HARDWARE function:
```
- h
SELECT ONE OF:

0 ADS NOISEMAKER (AY-3-8910)
1 CROMEMCO D+7A (D/A CONVERTER)
2 NEWTECH MUSIC BOARD (D/A CONVERTER)
3 SOLID STATE MUSIC SB-1 (SYNTHESIZER)

- 2
-> CURRENT HARDWARE IS MUSIC BOARD AT I/O BASE 24H+28H
```
Then check whether the hardware is configured to the I/O port address
suggested by the program (e.g. 24H as base address for the Newtech or 18H as
base address for the D+7A).

Now check with the PLAY TONE function whether the audio hardware is working in
general, e.g. with
```
- t
VOLUME (0-7): 7
DURATION (HEX 0-FF): 40
PITCH (HEX 0-FFFF): 1000
```
If you now hear a short sound everything should work. If not, double check the
hardware configuration. You may try different pitch values and leave the play
mode by just hitting \<ENTER\> without any input.

Some sound boards like the Cromemco D+7A and the Newtech Model 6 Music Board
support playback of PCM audio files. Although the capacities on early S-100
computers like the Altair and the IMSAI are limited to the amount of RAM,
it is possible to play PCM files with the PLAY SAMPLE FILE command:

```
- f
FILENAME: voice
....................................................................................................................................
SPEED (HEX 0-FF OR <ENTER> FOR DEFAULT): 
LOOP (Y/N): n
STEREO (Y/N): y
- f
FILENAME: hal
......................................................................................................................................................................................................................
SPEED (HEX 0-FF OR <ENTER> FOR DEFAULT): 
LOOP (Y/N): n
STEREO (Y/N): n
- 
```
Being asked for SPEED just hit \<ENTER\> (you later can tune playback speed here). Files should hold raw data
without file header, either with signed 8 bit per sample (mono) or two times signed 8 bit per sample (stereo),
recorded with a sampling rate of 8 kHz. VOICE is an example file for stereo audio and needs 16K of free RAM,
whereas HAL is an example for mono audio, requiring 27K of free RAM (at least 48K RAM for the whole system).

Next would be to try playing a music score file, there is just one score file
included as an example:
```
- s
FILENAME: fanfar
```
This of course requires the file FANFAR to be in te same directory as AUDIO.COM.

See the documentation for the score2bin program for a reference on the music
score notation. You may use the file fanfar.txt as an example to create your own music scores.

The AUDIO program by default uses sine waves for output. The files STRINGS and
TRUMPS are example wave form files, which can be loaded to change
instrumentation e.g. via
```
- w
FILENAME: strings
```
To check the full capabilities of a hardware you can also use the AUDIO TEST
function. It will sequentially try a couple of individual tests, which will take
some time. A D+7A has the advantage that you already have multiple analog
channels, so that you can use them already for stereo output. In use are analog
channels 1 (port 19H) and 3 (port 1BH), which are the defaults for using the
D+7A with JS-1 joysticks for audio. For the Newtech you would need two boards,
one for each channel.

DAC based sound boards also can play files containing 2-channel signed 8-bit
audio samples with the command PLAY SAMPLE FILE.
Please note that the file must contain sample data only, without file header.
So the common audio file formats such as .WAV have to be converted first.
There currently is no sample file included.

The joystick test simply outputs the input data from the appropriate I/O
ports and is applicable for D+7A boards only.


Limitations:

The AUDIO program ist currently limited to two voices/channels and can't mix multiple
voices into one channel during playback. The next release is intended to support true
polyphony, which probably will require a CPU clock of at least 4 MHz with DAC based boards
like the D+7A and the Newtech Music Board.
