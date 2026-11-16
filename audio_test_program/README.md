The program is supporting four different sound boards. Since there is no way to
autodetect them, you need to first select your hardware with the SELECT HARDWARE
function. Then check whether the hardware is configured to the I/O port address
suggested by the program (e.g. 24H as base address for the Newtech or 18H as
base address for the D+7A).

Then check with the PLAY TONE function whether the audio hardware is working in
general, e.g. with

- t
VOLUME (0-7): 7
DURATION (HEX 0-FF): 40
PITCH (HEX 0-FFFF): 1000

If you now hear a short sound everything should work. If not, double check the
hardware configuration. You may try different pitch values and leave the play
mode by just hitting <ENTER> without any input.

Next would be to try playing a music score file, there is just one score file
included as an example:

- s
FILENAME: fanfar

This of course requires the file FANFAR to be in te same directory as AUDIO.COM.

The AUDIO program by default uses sine waves for output. The files STRINGS and
TRUMPS are example wave form files, which can be loaded to change
instrumentation e.g. via

- w
FILENAME: strings

To check the full capabilities of a hardware you can also use the AUDIO TEST
function. It will sequentially try a couple of individual tests, which will take
some time. A D+7A has the advantage that you already have multiple analog
channels, so that you can use them already for stereo output. In use are analog
channels 1 (port 19H) and 3 (port 1BH), which are the defaults for using the
D+7A with JS-1 joysticks for audio. For the Newtech you would need two boards,
one for each channel.