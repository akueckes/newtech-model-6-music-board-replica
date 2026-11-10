/*
	Score Converter

	Converting a score from a computer readable notation into a compact
	binary format ready for efficiently being stored and processed by
	computer controlled audio hardware.

	Both score notation and binary format are widely hardware independent
	and support different sound generation schemes such as oscillator or 
	wavetable synthesis.

	1. Binary Format
	----------------

	The binary format is primarily optimized for efficient processing and
	compactness, and basically consists of a sequence of music or noise
	events, plus general control commands. Each event is at least one byte,
	the control byte, describing a command number in bits 4 to 6,
	plus a parameter in the lower nibble. A value of zero in both fields
	terminates the score.

	A special flag (X) controls whether a general control command is
	effective for all voices, or for just a selected single voice, which
	needs to be specified in a byte following the control command.
	
	Bit   7   6   5   4   3   2   1   0
	    +---+---+---+---+---+---+---+---+
	    | X |  COMMAND  |   PARAMETER   |
	    +---+---+---+---+---+---+---+---+  

	Currently the following commands are supported:

	Command description

	   0    End of score (terminates the playback)
	   1    Play music/noise event (parameter holds duration, extra bytes hold
	        pitches for individual voices plus tie flag)
	   2    Set volume/amplitude (parameter holds volume)
	   3    Set waveform (parameter holds waveform)
	   4    Set envelope (parameter holds envelope preset)
	   5    Set number of voices (parameter holds number of voices)
	   6    Set metronome/tempo (extra byte holds tempo in beats per minute)
	   7    Set tune vs. noise mode for all voices (parameter assigns mode
	        with 0=tune and 1=noise)

	The X bit (bit 7) of the control byte, if set, indicates that the
	command shall be applied to a specific voice only. In this case,
	another complementary byte with voice information follows the
	control byte:

	Bit   7   6   5   4   3   2   1   0
	    +---+---+---+---+---+---+---+---+
	    |             VOICE             |
	    +---+---+---+---+---+---+---+---+

	If X is clear, the change applies to all voices, and no additional
	byte is following the control byte.

	A value of 1 in the command field denotes a music or noise event,
	with the assigned duration in the parameter field, which is
	valid for 1-8 following bytes, depending on the current number
	of parts/voices.

	Value Ratio     Meaning

	   0:           (reserved)
	   1: 1.5       full note with dot
	   2: 1.0       full note
	   3: 0.75      half note with dot
	   4: 0.5       half note
	   5: 0.375     quarter note with dot
	   6: 0.25      quarter note
	   7: 0.1875    eigth note with dot
	   8: 0.166666  1/3 half note
	   9: 0.125     eigth note
	  10: 0.09375   sixteenth note with dot
	  11: 0.083333  1/3 quarter note
	  12: 0.0625    sixteenth note
	  13: 0.046875  thirty-second note with dot
	  14: 0.041666  1/3 eigth note
	  15: 0.03125   thirty-second note

	The volume/dynamics can be controlled separately for each voice	in
	16 steps including silence (0=silence, 15=full volume) and is set with
	the volume command.

	Likewise, the desired waveform can be assigned separately for each
	voice, allowing different sound variations or instruments for
	each player. By default, waveform #0 can normally be assumed as a
	pure sinus wave (unless assigned otherwise) whereas waveform #15
	holds data for periodic noise (such as engines sound).

	Encoding for dynamics/playback volume is
	
	   0: silence
	   1: pianississimo (ppp)
	   2: pianissimo (pp)
	   3: piano (p)
	   4: mezzo piano (mp)
	   5: mezzo forte (mf)
	   6: forte (f)
	   7: fortissimo (ff)
	   8: fortissimissimo (fff)
   
	For a command value of 1, the control byte is followed by a fixed
	number of note bytes, each representing one voice, which will be
	processed in parallel in order to describe chords or parts for
	different voices or instruments. Each note byte consists of a tone
	value for 127 individual pitches spanning up to 10 octaves in the
	lower 7 bits, plus a tie flag in the MSB.

	Bit   7   6   5   4   3   2   1   0
	    +---+---+---+---+---+---+---+---+
	    | T |         PITCH             |
	    +---+---+---+---+---+---+---+---+  

	A note byte with zero value tells the computer to play a rest (silence)
	with its length controlled by the duration in the control byte. The
	tie flag T controls whether the note shall be combined with the
	following note, normally with articulation only on the first note
	(slur). A cleared tie bit will introduce an articulation as short
	period of silence between two subsequent notes.

	The idea for processing multiple voices is that the playback algorithm
	has a fixed starting clock for achieving synchronous playback of
	all voices, combining all voices into a single event, which then can
	be implemented in a much simpler way, than having to manage separate
	events for every voice.

	Because individual notes over different voices can differ in their
	duration, longer notes need to be created by "glueing" together
	multiple tones with the tie flag set. As an example, if there
	is a half note in one voice starting together with two quarter notes
	in another voice, the two quarter notes demand two events to be
	processed. The half note in the first voice is created by linking
	two quarter notes with the tie flag into one half note.


	2. ASCII score notation
	-----------------------

	The notation described here is based on Hal Taylor's SCORTOS suggestion
	for an ASCII based music language from the September 1977 issue of the
	BYTE magazine, but modified and extended for general sound processors
	including wavetable synthesis and especially for use with the Cromemco
	D+7A digital analog converter.

	Each score consists of general controls (such as a key signature,
	a time signature, a metronome mark, the number of parts/voices, current
	part /voice, current octave, volume/dynamics, waveforms for instruments
	and envelopes), flow control (left hand repeat, right hand repeat),
	structure information (bars), events (notes, noise or rests), and
	comments.

	The score notation supports up to 8 parts/voices (sufficient for e.g.
	six tone parts and two noise parts). Volume/dynamics, waveform and
	envelope can be selected either for all voices or per individual voice.

	General controls can be placed between events anywhere in the score
	and are effective until changed with another control command:

	  Control                   Syntax                 Default
	  
	  Key signature:            K[1..7][#|b]           K    (C major/A minor)
	  Time signature:           T[2..8]/[2..8]         T4/4
	  Metronome mark:           M[1-256]               M120 (120 beats per minute)
	  Number of parts/voices:   N[1..16]               N1   (1 part)
	  Current part/voice:       P[1..16]               P1
	  Current octave:           O[-1..9]               O4
	  Volume/dynamics:          V<volume>[@<voice>]    V9   (mezzo forte)
	  Waveform:                 W<waveform>[@<voice>]  W0   (normally sinus wave)
	  Envelope:                 E<envelope>[@<voice>]  E0   (no envelope)
	  Transposition:            [+|-]<halftones>       +0   (no transposition)

	where

	  <voice> := [1..16]
	  <volume> := [0..15]       (0=silence)
	  <waveform> := [1..16]     (1-15 wavetable synthesis, 16=noise)
	  <envelope> := [0..15]     (0=no envelope)

	Encoding for dynamics/playback volume is

	    0: silence
	    1: pianississimo (ppp)
	    3: pianissimo (pp)
	    5: piano (p)
	    7: mezzo piano (mp)
	    9: mezzo forte (mf)
	   11: forte (f)
	   13: fortissimo (ff)
	   15: fortissimissimo (fff)

	It is up to the sound processor how to deal with the above specs.
	This especially applies to the assignment of voices to (stereo)
	output channels. Different volumes can be implemented by waveforms
	with matching amplitudes.
	
	Envelope presets can be implemented via	amplitude tables (similar
	to waveforms) or parameterized e.g. via attack, alternate, hold and
	continue.
	
	Noise can be implemeneted as periodic noise with waveforms or as
	white noise with appropriate white noise generators.
  

	Events
	------

	An event is either a tone event (denoted by the note as a letter with
	octave for the pitch, such as C4 or D5, with optional modifiers - # for
	sharp, b for flat and n for natural), a noise event (denoted by N
	followed by a pitch of 1 to 255), or a rest event (denoted by R).
	The tone pitch is calibrated to A4 = 440 Hz.

	Each event has a duration as a value of 1, 2, 4, 8, 16 or 32 for
	1/1, 1/2, 1/4, 1/8, 1/16 or 1/32 notes, and can be combined with a
	dot for 1.5 times lengthening.

	Example: play a singe voice D major scale of eigth notes

	  8D4,8E4,8F#4,8G4,8A4,8B4,8C#4,8D5

	Each event can be "glued" to its following event with ^. Normally,
	there is an accent between two events, so that the listener can
	separate them. ^ stands for a slur or tie, playing an event with
	its follower without accent in between.

	Example: same as above, but with tie

	  8D4^,8E4^,8F#4^,8G4^,8A4^,8B4^,8C#4^,8D5

	Each voice has a current volume, waveform and envelope assigned to it.
	Each event is described by its duration, followed by what happens on
	each voice (playing a tone, playing noise or playing a rest).

	Each voice can include a ^ to indicate a tie to the same voice in
	the next event.

	  <event> := <duration>[<tone>|<noise>|<rest>][^][,[<tone>|<noise>|<rest>][^]]

	where

	  <duration> := [1,2,4,8,16,32][.]
	  <tone> := <note>[<modifier>][<octave>]
	  <note> := [C,D,E,F,G,A,B]
	  <noise> := N
	  <rest> := R
	  <modifier> := #|b|n
	  <octave> := [0..9]
	  <pitch> := [0..127]

	The notation supports up to 127 tone values. Each tone is made up of a
	note (one of C, D, E, F, G, A, B) plus an optional modifier (either #
	for sharp, b for flat or n for neutral) and also optionally the octave
	(-1 to 9). If the octave is omitted, the octave	will be either the default
	octave 4, or the octave set with the O command for the current part/voice.
	A pitch of 0 denotes silence for playing breaks.

	Same applies to pitch values for noise events (128 pitch values where 0
	again denotes silence).

	In fact the wide range of midi pitches is of limited use, especially
	for the extreme high and low ranges.


	Structure
	---------

	Notes can be grouped into measures with bars and combined into tuplets
	using parenthesis. Also, repeat sequences may be defined.  Currently,
	only triplets are supported as the are by far most common type of
	tuplets.

	  Bar:          | or /
	  Tuplet:       (...)
	  Repeat:       [ or ]
	  End of score: @ or End-of-file (EOF)
  
	If framed by brackets, sequences will be repeated once.

	The total duration of each triplet is defined by the duration value.
	For example, three sixteenth notes (value 16) in a triple combine to
	the duration of one eigth note, three eigth notes (value 8) in a
	triple combine to the duration of one quarter note and so on.

	For better readability, whitespace and commas my be inserted as
	needed.

	Example:

	A valid example for an ASCII notation for two voices:
   
	(comma separators increase readability but can be omitted).


	3. MIDI
	-------
	
	In addition to the ASCII syntax descibed above, also MIDI files can be
	used as a source for music data. But don't expect simply using any
	MIDI file out there to be converted into a format optimize for the
	computer sound & music hardware of the 70s. MIDI support here is rather
	intended for using the authoring tool of your choice to comfortably
	create your score.
	
	MIDI files are usually structured into channels (one for each
	instrument) each of which can hold an arbitrary number of notes to be
	played simultaneously as chords. So the overall number of voices can
	easily exceed the number of voices supported by your sound hardware.
	
	Voices in a MIDI file can be mapped to the binary format basically
	in two different ways. The most natural way is to assign each MIDI
	channel to a fixed part, since MIDI channels in general represent
	instruments. This is also the only way for positioning a channel in
	the stereo space, and the default way mapping is done by this program.
	
	Some instruments, however, are polyphonic in their nature, such as
	keyboards or guitars, so that MIDI channels can contain more than a
	single voice. In that case, notes in chords need to be assigned
	dynamically to "free" parts when they appear, which can be configured
	on demand if necessary.

	MIDI has almost no limitation on the duration of notes, since the
	duration of each note is specified in (very short) ticks instead of
	multiples of a fraction of a whole note, as it is with the compact
	binary format used here, which uses multiples of 1/32 notes
	including dotted notes.

	MIDI allows any kind of tuplet, whereas the binary format used here
	currently is supporting only triplets, which is by far the most common
	type of tuplet.

	Dynamics in MIDI are specified with a resolution of 128 different volumes.
	The binary format is using up to 16 different volumes, including silence.
	The values used by MIDI will automatically be translated in the proper
	binary format.

	Finally, MIDI offers much more control, such as aftertouch or pitch wheels,
	which currently can't be reproduced with the binary format. None of the
	MIDI format 3 special features are supported.

	So please make sure that the total number of voices in the MIDI	file
	matches the maximum number of voices you want for your hardware, and that
	no notes shorter than 1/32 are included, and no tuplets other than triplets
	are used.

	MIDI files will be automatically processed if the input file has a
	filename extension .mid or .midi. If any of the limitations listed
	above applies, the program will output an appropriate warning.

	Usage:

	  score2bin <input-file> [-a [<assembler-file>]] [-b <binary-file>}

	<assembler-file> denotes the filename of a file which shall hold the
	music code in Intel 8080 format for copy & paste into a assembler
	program.

	<binary-file> specifies a file which shall hold the output as byte
	code.

	When using this program without parameters, the Intel 8080 assembler
	code will be output to the console.
	
	By default, MIDI channels are mapped to parts in a 1:1 fashion. You
	may change this mapping with the -m option, such as

	  score2bin musicfile.mid -m 1:3,2:4

	which reassigns MIDI channel 3 to part #1 and MIDI channel 4 to part #2.
	The mapping for all other voices remains unchanged.
	
	Alternatively, the program can run in a mode assigning the MIDI voices
	dynamically to the available parts.

	Normally, the number of parts for the output will match the number
	of channels in the MIDI file. You may change the number of voices with
	the -n option, such as

	  score2bin musicfile.mid -n 2

	which reduces the number of parts to 2, using just the first two channels
	in the MIDI file. You still may map the two parts to other tracks with
	the -m option.
	
	Dynamic assignment of voices can be activated with the -d option. The program
	then handles notes when they have to be played in a round robin scheme,
	assigning them to any free part:
	
	  score2bin musicfile.mid -d
	  
	In case no free part is available, notes will be dropped. As the notes are
	assigned randomly, they can't be positioned in the stereo sound space.

	In fact it is recommended not to use more than 8 parts/voices for the binary
	format, since for synchronization puposes longer notes which are played
	together with short notes automatically will be broken up into shorter
	notes tied together, which simplifies playback, but may produce inefficiently
	large binary files from complex MIDI files.
	
	MIDI formats 0 and 1 are supported. So all existing MIDI tracks are processed,
	but there will be no synchronization of channels across tracks. So make sure
	no channels are used across multiples tracks.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define DEF_NUM_PARTS		2			/* default number of parts/voices */
#define MAX_NUM_PARTS		16			/* maximum number of parts/voices */
#define MAX_NUM_EVENTS		1000		/* maximum number of music events */

enum NoiseGenerator {
	WAVETABLE_SYNTHESIS	= 0,			/* use wavetable synthesis for periodic noise */
	WHITE_NOISE	= 1						/* use white noise generator for noise */
};

enum Command {
	CMD_END = 0,
	CMD_EVENT = 1,
	CMD_VOLUME = 2,
	CMD_WAVEFORM = 3,
	CMD_ENVELOPE = 4,
	CMD_PARTS = 5,
	CMD_METRONOME = 6,
	CMD_MODE = 7
};

typedef struct event {
	int duration;
	int dot;
	int pitch;
	int tie;
	int is_triplet;
	int is_noise;
} Event;

typedef struct part {
	Event event[MAX_NUM_EVENTS];
	int current_event;
	int num_events;
} Part;

static bool verbose = false;				/* output more verbose information */

/* ------------ MIDI stuff ----------- */

#define MAX_ACTIVE_PITCHES 2048

typedef struct {
	uint32_t start_tick;		/* start of the note in ticks */
	bool is_active;				/* note is depressed */
	uint8_t part;				/* part that is assigned (0 = no assignment) */
} ActiveNote;

typedef struct {
	bool is_active;
	uint32_t start_tick; 
	uint8_t pitch;
} BaseNote;

/* global array to track active notes */
static ActiveNote note_tracker[16][128]; 

/* global info for tuplet detection */
typedef struct {
	int type;			/* 0=unused, 3=triplet, 5=quintuplet etc. (currently only triplets supported) */
	int current_slot;	/* current slot starting at 0 */
	int note_count[3];	/* populated notes per slot */
	int tick;			/* tick of last slot in tuplet */
	int num;			/* numerator of last slot duration */
	int den;			/* denominator of last slot duration */
	int pitch[3][16];	/* slots with pitches */
	int tie[3][16];		/* slots with tie information */
	int	part[3][16];	/* assigned part */
} Tuplet;

Tuplet tuplet;

/* --- helper functions (prototypes) --- */

uint16_t read_u16_be(FILE *ifp);
uint32_t read_u32_be(FILE *ifp);
uint32_t read_vlq(FILE *ifp);
void skip_chunk_data(FILE *ifp, uint32_t bytes_to_skip);
char* get_pitch_name(uint8_t pitch);
long gcd(long a, long b);
char* calculate_musical_fraction(uint32_t duration_ticks, uint16_t division);

/* --- helper functions (implementation) --- */

uint16_t read_u16_be(FILE *ifp) {
	uint8_t byte1, byte2;
	if (fread(&byte1, 1, 1, ifp) != 1 || fread(&byte2, 1, 1, ifp) != 1) return 0;
	return (uint16_t)((byte1 << 8) | byte2);
}

uint32_t read_u32_be(FILE *ifp) {
	uint32_t val = 0;
	for (int i = 0; i < 4; i++) {
		uint8_t byte;
		if (fread(&byte, 1, 1, ifp) != 1) return 0;
		val = (val << 8) | byte;
	}
	return val;
}

uint32_t read_vlq(FILE *ifp) {
	uint32_t value = 0;
	uint8_t byte;

	for (int i = 0; i < 4; i++) {
		if (fread(&byte, 1, 1, ifp) != 1) return 0;

		value = (value << 7) | (byte & 0x7F);

		if (!(byte & 0x80)) {
			return value;
		}
	}
	return 0;
}

void skip_chunk_data(FILE *ifp, uint32_t bytes_to_skip) {
	if (bytes_to_skip > 0) {
		fseek(ifp, bytes_to_skip, SEEK_CUR);
	}
}

char* get_pitch_name(uint8_t pitch) {
	static char pitch_names[128][4];
	const char *note_names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	
	int octave = (pitch / 12) - 1;
	int note_index = pitch % 12;
	
	sprintf(pitch_names[pitch], "%s%d", note_names[note_index], octave);
	return pitch_names[pitch];
}

/* find the Greatest Common Divisor (Euclidean algorithm) */
long gcd(long a, long b) {
	while (b) {
		a %= b;
		long temp = a;
		a = b;
		b = temp;
	}
	return a;
}

/* Calculates the duration as a fraction of a whole note (1/N) */
char* calculate_musical_fraction(uint32_t duration_ticks, uint16_t division) {
	static char fraction_str[16];
	
	/* 
		Ticks per quarter note (QN) is 'division'
		Ticks per whole note (WN) = division * 4
	*/
	uint32_t ticks_per_whole_note = (uint32_t)division * 4;
	
	/* Check for zero duration or zero division to prevent division by zero */
	if (duration_ticks == 0 || ticks_per_whole_note == 0) {
		strcpy(fraction_str, "0/1");
		return fraction_str;
	}

	/* Numerator is the duration in ticks. Denominator is ticks per whole note */
	long num = duration_ticks;
	long den = ticks_per_whole_note;
	
	/* Simplify the fraction using GCD */
	long common = gcd(num, den);
	num /= common;
	den /= common;
	
	/*
		We want the numerator to be 1 for standard musical fractions (1/4, 1/8, 1/16, etc.)
		If the numerator is not 1, it might be a dotted note or a triplet duration.
		For this basic parser, we'll try to simplify it to a single unit fraction (1/N).
	*/
	
	/* if the new numerator is 1, it's a simple fraction (1/4, 1/8, etc.) */
	if (num == 1) {
		sprintf(fraction_str, "1/%ld", den);
	} 
	/* if the new numerator is 3 and the denominator is 2*N (e.g., 3/8, 3/16), it's a dotted note */
	else if (num == 3 && (den % 2 == 0)) {
		/* e.g., 3/8 whole note = 3/2 quarter note (Dotted QN) */
		/* This area is complex, but we'll print the simplified fraction */
		sprintf(fraction_str, "%ld/%ld (Compound)", num, den);
	} 
	/* otherwise, print the simplified ratio */
	else {
		sprintf(fraction_str, "%ld/%ld (Ratio)", num, den);
	}

	return fraction_str;
}

void print_event(int part, Event *event)
{	printf("Registering event: part=%d duration=%d dot=%d pitch=%d tie=%d is_triplet=%d is_noise=%d\n",
	part,
	event->duration,
	event->dot,
	event->pitch,
	event->tie,
	event->is_triplet,
	event->is_noise);
}


/* --- Event Parsing Logic --- */

static uint32_t last_note_off_tick[16];
static BaseNote current_base_note[16];
static bool part_is_assigned[MAX_NUM_PARTS];

void register_midi_event(uint8_t channel, Part *part, int p, long num, long den, uint8_t pitch, uint32_t ticks_per_whole_note, uint32_t current_tick, int num_parts, bool dynamic_assignment)
{
	int i,j;

	last_note_off_tick[channel] = current_tick;

	part[p].event[part[p].current_event].dot = 0;
	part[p].event[part[p].current_event].tie = 0;
	part[p].event[part[p].current_event].is_triplet = 0;
	part[p].event[part[p].current_event].is_noise = 0;
	
	/* check for pending tuplet */
	if ((tuplet.type > 0) && (den % 3)) {
		/* processs tuplet */
		if (tuplet.current_slot == 2) {
			if (verbose) printf("Triplet detected (1/%d note)\n", (2 * tuplet.den) / 3);
		} else printf("Warning: incomplete triplet detected (1/%d note)\n", (2 * tuplet.den) / 3);
			
		for (i=0; i<=tuplet.current_slot; i++) {
			for (j=0; j<tuplet.note_count[i]; j++) {
				int p = tuplet.part[i][j];
				part[p].event[part[p].current_event].duration = (2 * tuplet.den) / 3;
				part[p].event[part[p].current_event].dot = 0;
				part[p].event[part[p].current_event].pitch = tuplet.pitch[i][j];
				part[p].event[part[p].current_event].tie = tuplet.tie[i][j];
				part[p].event[part[p].current_event].is_triplet = 1;
				part[p].event[part[p].current_event].is_noise = 0;
				if (verbose) print_event(p, &part[p].event[part[p].current_event]);
				part[p].current_event++;
				part[p].num_events++;
			}
		}

		/* free up tuplet */
		memset(&tuplet, 0, sizeof(tuplet));
	}

	/* check for current/new tuplet */
	if ((den % 5) == 0) {
		fprintf(stderr, "Warning: quintuplet detected (unsupported, ignored)\n");
	}
	else if ((den % 3) == 0) {
		if ((tuplet.type == 3) && (num == tuplet.num) && (den == tuplet.den) && (current_tick == tuplet.tick)) {
			/* next note in slot */
			tuplet.pitch[tuplet.current_slot][tuplet.note_count[tuplet.current_slot]] = pitch;
			tuplet.tie[tuplet.current_slot][tuplet.note_count[tuplet.current_slot]] = 0;			// TODO: correct tie
			tuplet.part[tuplet.current_slot][tuplet.note_count[tuplet.current_slot]] = p;
			tuplet.note_count[tuplet.current_slot]++;
			if (tuplet.note_count[tuplet.current_slot] > 16) {
				tuplet.note_count[tuplet.current_slot] = 16;
				fprintf(stderr, "Warning: too many simultaneous notes in tuplet (ignored)\n");
			}
			else if (verbose) printf("Triplet slot %d note %d\n", tuplet.current_slot, tuplet.note_count[tuplet.current_slot]);
		}
		else if ((tuplet.type == 3) && (num == tuplet.num) && (den == tuplet.den) && (current_tick == tuplet.tick + ((ticks_per_whole_note * num) / den)) && (tuplet.current_slot < 2)) {
			/* next slot */
			tuplet.current_slot++;
			tuplet.tick = current_tick;
			tuplet.pitch[tuplet.current_slot][0] = pitch;
			tuplet.tie[tuplet.current_slot][0] = 0;			// TODO: correct tie
			tuplet.part[tuplet.current_slot][0] = p;
			tuplet.note_count[tuplet.current_slot] = 1;
			if (verbose) printf("Triplet slot %d note %d\n", tuplet.current_slot, tuplet.note_count[tuplet.current_slot]);
		}
		else {
			/* pending tuplet? */
			if (tuplet.type == 3) {
				if (tuplet.current_slot == 2) {
					if (verbose) printf("Triplet detected (1/%d note)\n", (2 * tuplet.den) / 3);
				} else printf("Warning: incomplete triplet detected (1/%d note)\n", (2 * tuplet.den) / 3);

				/* processs tuplet */
				for (i=0; i<=tuplet.current_slot; i++) {
					for (j=0; j<tuplet.note_count[i]; j++) {
						int p = tuplet.part[i][j];
						part[p].event[part[p].current_event].duration = (2 * tuplet.den) / 3;
						part[p].event[part[p].current_event].dot = 0;
						part[p].event[part[p].current_event].pitch = tuplet.pitch[i][j];
						part[p].event[part[p].current_event].tie = tuplet.tie[i][j];
						part[p].event[part[p].current_event].is_triplet = 1;
						part[p].event[part[p].current_event].is_noise = 0;
						if (verbose) print_event(p, &part[p].event[part[p].current_event]);
						part[p].current_event++;
						part[p].num_events++;
					}
				}
			}
			/* reset tuplet */
			memset(&tuplet, 0, sizeof(tuplet));
			tuplet.type = 3;
			tuplet.num = num;
			tuplet.den = den;
			tuplet.tick = current_tick;
			tuplet.pitch[0][0] = pitch;
			tuplet.tie[0][0] = 0;			// TODO: correct tie
			tuplet.part[0][0] = p;
			tuplet.note_count[0] = 1;
			if (verbose) printf("New triplet\n");
		}
	} else if (num == 1) {
		/* normal note */
		part[p].event[part[p].current_event].duration = den;
		part[p].event[part[p].current_event].pitch = pitch;
		if (verbose) print_event(p, &part[p].event[part[p].current_event]);
		part[p].current_event++;
		part[p].num_events++;
	} else if (num == 3 && (den % 2 == 0)) {
		/* dotted note */
		part[p].event[part[p].current_event].duration = den / 2;
		part[p].event[part[p].current_event].dot = 1;
		part[p].event[part[p].current_event].pitch = pitch;
		if (verbose) print_event(p, &part[p].event[part[p].current_event]);
		part[p].current_event++;
		part[p].num_events++;
	} else if (den % 64 == 0) {
		/* all others - split up into tied notes */
		int remaining = 64 / den * num;
		int divider = 64;
		for (i=0; i<7; i++) {
			while (remaining > 0) {
				part[p].event[part[p].current_event].pitch = pitch;
				part[p].event[part[p].current_event].dot = 0;
				part[p].event[part[p].current_event].tie = 0;
				part[p].event[part[p].current_event].is_triplet = 0;
				part[p].event[part[p].current_event].is_noise = 0;

				if (remaining == 3) {
					/* special case: last note 1/32 with dot */
					part[p].event[part[p].current_event].duration = 32;
					part[p].event[part[p].current_event].dot = 1;
					part[p].event[part[p].current_event].tie = 1;
					remaining = 0;
				} else if ((remaining - divider) > 0) {
					/* partial */
					part[p].event[part[p].current_event].duration = 64 / divider;
					part[p].event[part[p].current_event].tie = 1;
					remaining -= divider;
				}
				else if ((remaining - divider) == 0) {
					/* last note */
					part[p].event[part[p].current_event].duration = 64 / divider;
					remaining = 0;
				}
				else if (divider > remaining) {
					fprintf(stderr, "Warning: %lu/%lu note (will be converted to 1/32)\n", num, den);
					part[p].event[part[p].current_event].duration = 32;
					remaining = 0;
				}
				if (verbose) print_event(p, &part[p].event[part[p].current_event]);
				part[p].current_event++;
				part[p].num_events++;
			}
			divider >>= 1;
		}
	} else {
		fprintf(stderr, "\nWarning: unsupported note length with ratio %ld/%ld (ignored)\n", num, den);
		return;
	}
}

void parse_midi_event(FILE *ifp, FILE *afp, FILE *bfp, Part *part, uint32_t *bytes_read_ptr, uint8_t *last_status, uint32_t *current_tick, uint16_t division, int *midi_mapping, int num_parts, bool dynamic_assignment, float *time_signature) {
	/* read Delta-Time and update current_tick */
	long start_pos = ftell(ifp);
	uint32_t delta_time = read_vlq(ifp);
	long end_pos = ftell(ifp);
	uint32_t vlq_bytes = end_pos - start_pos;
	*bytes_read_ptr += vlq_bytes;
	uint8_t meta_data[256];
	int p;
	
	*current_tick += delta_time;
	
	/* read Status Byte (with Running Status handling) */
	uint8_t status_byte;
	long status_pos = ftell(ifp);
	
	if (fread(&status_byte, 1, 1, ifp) != 1) {
		*bytes_read_ptr = 0; 
		return;
	}
	*bytes_read_ptr += 1;

	if (!(status_byte & 0x80)) { 
		status_byte = *last_status;
		fseek(ifp, status_pos, SEEK_SET); 
		*bytes_read_ptr -= 1;
	} else {
		if (status_byte < 0xF0) {
			*last_status = status_byte;
		}
	}

	uint8_t command = status_byte & 0xF0;
	uint8_t channel = status_byte & 0x0F;

	if (command == 0x90 || command == 0x80) { // Note On (0x90) or Note Off (0x80)
		uint8_t pitch, velocity;
		
		if (fread(&pitch, 1, 1, ifp) != 1 || fread(&velocity, 1, 1, ifp) != 1) return;
		*bytes_read_ptr += 2;
		
		bool is_note_on = (command == 0x90) && (velocity > 0);
		bool is_note_off = (command == 0x80) || ((command == 0x90) && (velocity == 0));

		if (verbose) printf("[%5u ticks] Ch=%u Cmd=%s (0x%02X) Pitch=%u (%s) Vel=%u", 
			   *current_tick, channel + 1, 
			   is_note_on ? "Note ON" : "Note OFF", status_byte, 
			   pitch, get_pitch_name(pitch), velocity);

		if (is_note_on) {

			/* set tone active */
			note_tracker[channel][pitch].start_tick = *current_tick;
			note_tracker[channel][pitch].is_active = true;
			if (verbose) printf("\n");
			
			/* map midi channel to part */
			if (dynamic_assignment) {
				/* map to next free part */
				for (p=0; p<num_parts; p++) {
					if (!part_is_assigned[p]) {
						if (note_tracker[channel][pitch].part == 0) {
							part_is_assigned[p] = true;
							note_tracker[channel][pitch].part = p + 1;
							break;
						}
						else {
							fprintf(stderr, "Warning: simultaneous notes with pitch %d on the same MIDI channel %d detected (ignored)\n", pitch, channel);
							return;
						}
					}
				}

				if (p == num_parts) {
					fprintf(stderr, "Warning: number of simultaneous MIDI notes exceeds max. number of %d parts (dropping note)\n", num_parts);
					return;
				}
			}
			/* if no other pitch is active in the channel, select current pitch for new base note in a chord */
			else if (!current_base_note[channel].is_active) {
				current_base_note[channel].is_active = true;
				current_base_note[channel].start_tick = *current_tick;
				current_base_note[channel].pitch = pitch;
			}
		}	   
		else if (is_note_off && note_tracker[channel][pitch].is_active) {

			/* note termination */
			uint32_t start_time = note_tracker[channel][pitch].start_tick;
			uint32_t duration_ticks = *current_tick - start_time;
			char *fraction = calculate_musical_fraction(duration_ticks, division);
			
			if (verbose) printf(" Duration=%u ticks (%s Whole Note)\n", duration_ticks, fraction);
			
			uint32_t ticks_per_whole_note = (uint32_t)division * 4;

			/* numerator is the duration in ticks. Denominator is ticks per whole note. */
			long num = duration_ticks;
			long den = ticks_per_whole_note;
			
			/* simplify the fraction using GCD */
			long common = gcd(num, den);
			num /= common;
			den /= common;

			/* check for active base note */
			if ((current_base_note[channel].is_active && (current_base_note[channel].pitch == pitch)) || dynamic_assignment) {
				if (dynamic_assignment) {
					/* get part number from note_tracker */
					if (note_tracker[channel][pitch].part > 0) {
						p = note_tracker[channel][pitch].part - 1;
						note_tracker[channel][pitch].part = 0;
						part_is_assigned[p] = false;
					}
					else {
						/* Note OFF for ignored note */
						return;
					}
				}
				else {
					/* only the first note in a chord will be processed */
					current_base_note[channel].is_active = false;
	
					/* map channel 1:1 to part */
					for (p=0; p<num_parts; p++) {
						if (midi_mapping[p] == channel) break;
					}

					if (p == num_parts) {
						fprintf(stderr, "Warning: channel count exceeds max. number of %d parts (dropping note)\n", num_parts);
						return;
					}
				}
		
				/* register event */			
				if ((*current_tick > last_note_off_tick[channel]) || dynamic_assignment) {
					register_midi_event(channel, part, p, num, den, pitch, ticks_per_whole_note, *current_tick, num_parts, dynamic_assignment);		
				}
			}
			note_tracker[channel][pitch].is_active = false;
			if (dynamic_assignment)	part_is_assigned[p] = false;
		} else {
			 if (verbose) printf(" (ignored) \n"); 
		}
	} 
	else if (status_byte == 0xFF) {
		/* handle Meta-Events (0xFF) */
		uint8_t meta_type;
		if (fread(&meta_type, 1, 1, ifp) != 1) return;
		*bytes_read_ptr += 1;

		uint32_t meta_length = read_vlq(ifp);
		*bytes_read_ptr += (ftell(ifp) - end_pos) - 1; 

		if (verbose) printf("[%5u ticks] Status: 0xFF (Meta Event) | Type: 0x%02X | Length: %u\n", 
			   *current_tick, meta_type, meta_length);
		
		switch (meta_type) {
		case 0x2f:	/* end of track */
			*bytes_read_ptr = 0;
			break;
		case 0x03:	/* track name */
			if (meta_length < 256) {
				meta_data[meta_length] = 0;
			} else{
				meta_data[255] = 0;
			}
			if (fread(&meta_data, 1, meta_length, ifp) != meta_length) return;
			if (verbose) printf("Track name: \"%s\"\n", (char*)meta_data);
			   	*bytes_read_ptr += meta_length;
			break;
		case 0x59:	/* key signature */
			if (fread(&meta_data, 1, meta_length, ifp) != meta_length) return;
			uint8_t key_signature = (int8_t)meta_data[0];
			if (verbose) printf("Key signature: %d\n", key_signature);
		   	*bytes_read_ptr += meta_length;
			break;
		case 0x58:	/* time signature */
			if (fread(&meta_data, 1, meta_length, ifp) != meta_length) return;
			uint8_t num = meta_data[0];
			uint8_t den = 1 << meta_data[1];
			*time_signature = num / den;
			if (verbose) printf("Time signature: %d/%d %f\n", num, den, *time_signature);
		   	*bytes_read_ptr += meta_length;
			break;
		case 0x51:	/* tempo in beats per minute */
			if (fread(&meta_data, 1, meta_length, ifp) != meta_length) return;
			uint32_t usecs = (meta_data[0] << 16) | (meta_data[1] << 8) | meta_data[2];
			uint8_t tempo = (1000000.0 / usecs) * 60.0;
			*bytes_read_ptr += (ftell(ifp) - end_pos) - 1; 
			if (verbose) printf("Tempo: %d bpm\n", tempo);
			if (afp) fprintf(afp, "\tDB\t%03XH,%03XH\n", CMD_METRONOME << 4, tempo);
			if (bfp) {
				fputc(CMD_METRONOME << 4, bfp);
				fputc(tempo, bfp);
			}
		   	*bytes_read_ptr += meta_length;
			break;
		default:	/* all others */
				skip_chunk_data(ifp, meta_length);
				*bytes_read_ptr += meta_length;
		}		
	}
	else {
		/* handle other MIDI events */
		int data_bytes = 0;
		if (command == 0xA0 || command == 0xB0 || command == 0xE0) data_bytes = 2;
		if (command == 0xC0 || command == 0xD0) data_bytes = 1;
		
		if (data_bytes > 0) {
			skip_chunk_data(ifp, data_bytes);
			*bytes_read_ptr += data_bytes;
		}
		if (verbose) printf("[%5u ticks] Status: 0x%02X (other MIDI event)\n", *current_tick, status_byte);
	}
}

/* ------------ End of MIDI stuff ----------- */

void encode_duration(Event *event, int *ret_length_code, float *ret_note_length)
{
  /* encode duration */
	if (event->is_triplet) {
		if (event->duration == 4) *ret_length_code =  8;		// 1/3 half note
		else if (event->duration == 8) *ret_length_code =  11;	// 1/3 quarter note
		else if (event->duration == 16) *ret_length_code =  14;	// 1/3 eighth note
	}
	else if (event->dot) {
		if (event->duration == 1) *ret_length_code =  1;		// full note with dot
		else if (event->duration == 2) *ret_length_code =  3;	// half note with dot
		else if (event->duration == 4) *ret_length_code =  5;	// quarter note with dot
		else if (event->duration == 8) *ret_length_code =  7;	// eighth note with dot
		else if (event->duration == 16) *ret_length_code =  10;	// sixteenth note with dot
		else if (event->duration == 32) *ret_length_code =  13;	// thirty-second note with dot
	}
	else {
		if (event->duration == 1) *ret_length_code =  2;		// full note
		else if (event->duration == 2) *ret_length_code =  4;	// half note
		else if (event->duration == 4) *ret_length_code =  6;	// quarter note
		else if (event->duration == 8) *ret_length_code =  9;	// eighth note
		else if (event->duration == 16) *ret_length_code =  12;	// sixteenth note
		else if (event->duration == 32) *ret_length_code =  15;	// thirty-second note
	}
  
	/* translate into 1/32 note counts and multples of a full note */
	switch (*ret_length_code) {
		case  1: *ret_note_length = 1.5; break;			/* full note with dot */
		case  2: *ret_note_length = 1.0; break;			/* full note */
		case  3: *ret_note_length = 0.75; break;		/* half note with dot */
		case  4: *ret_note_length = 0.5; break;			/* half note */
		case  5: *ret_note_length = 0.375; break;		/* quarter note with dot */
		case  6: *ret_note_length = 0.25; break;		/* quarter note */
		case  7: *ret_note_length = 0.1875; break;		/* eigth note with dot */
		case  8: *ret_note_length = 0.5 / 3.0; break;	/* 1/3 half note */
		case  9: *ret_note_length = 0.125; break;		/* eigth note */
		case 10: *ret_note_length = 0.09375; break;		/* sixteenth note with dot */
		case 11: *ret_note_length = 0.25 / 3.0; break;	/* 1/3 quarter note */
		case 12: *ret_note_length = 0.0625; break;		/* sixteenth note */
		case 13: *ret_note_length = 0.046875; break;	/* thirty-second note with dot */
		case 14: *ret_note_length = 0.125 / 3.0; break; /* 1/3 eigth note */
		case 15: *ret_note_length = 0.03125; break;		/* thirty-second note */
		default:;
	}
}

void parse_number(FILE *ifp, int *result)
{
	char data;
	int digits = 0;
	int number = 0;

	number = 0;
	while (digits < 8) {
		data = fgetc(ifp);
	  	if ((data > 47) && (data < 58)) {
	  		number *= 10;
	  		number += data - 48;
	  		digits++;
	  	}
	  	else {
			fseek(ifp, ftell(ifp) - 1, SEEK_SET);			/* step back */	
	  		break;
	  	}
	}
	if (digits > 0) *result = number;
}

/*
	encodes key signature as negative number of b's and positive number of #'s
*/
int parse_key_signature(FILE *ifp, int *signature)
{
	char data;
	
	parse_number(ifp, signature);
	if (*signature > 0) {
		data = fgetc(ifp);
		if (data == 'b') *signature = 0 - *signature;
		else if (data != '#') return -1;		/* syntax error */
//		fseek(ifp, ftell(ifp) - 1, SEEK_SET);		/* step back */	
	}
	return 0;						/* ok */
}

int parse_time_signature(FILE *ifp, float *signature)
{
	char data;
	int nominator, denominator;
	
	data = fgetc(ifp);
  	if ((data > 47) && (data < 58)) {
  		nominator = data - 48;
  	}
  	else return -1;

	data = fgetc(ifp);
	if (data != '/') return -1;

	data = fgetc(ifp);
  	if ((data > 47) && (data < 58)) {
  		denominator = data - 48;
  	}
  	else return -1;
  	
  	*signature = (float)nominator / (float)denominator;

	return 0;						/* ok */
}

void parse_duration(FILE *ifp, int *duration, int *dot)
{
	char data;

	/* get duration */
	parse_number(ifp, duration);
	
	/* check for dot */
	*dot = 0;
	data = fgetc(ifp);
	if (data == '.') {
		*dot = 1;
		data = fgetc(ifp);
	}
	fseek(ifp, ftell(ifp) - 1, SEEK_SET);			/* step back */	
}

int parse_event(FILE *ifp, Event *event, int octave, int key_signature, int transposition)
{
	char data, letter;
	int sharp, flat, natural;

	/* get duration */
	parse_duration(ifp, &event->duration, &event->dot);
	
	/* get note with modifiers and octave */
	data = fgetc(ifp);

	sharp = 0;
	flat = 0;
	natural = 0;
	event->is_noise = 0;

	letter = data;
	data = fgetc(ifp);

	/* accidentials */
	if (data == '#') {
		/* sharp */
		sharp = 1;
		data = fgetc(ifp);
	}
	else if (data == 'b') {
		/* flat */
		flat = 1;
		data = fgetc(ifp);
	}
	else if (data == 'n') {
		/* natural */
		natural = 1;
		data = fgetc(ifp);
	}

	/* apply key signature  */
	if (letter == 'C') {
		event->pitch = 0;
		if (!natural) {
			if (key_signature > 1) event->pitch++;
			if (key_signature < -5) event->pitch--;
			event->pitch += transposition;
		}
	}
	else if (letter == 'D') {
		event->pitch = 2;
		if (!natural) {
			if (key_signature > 3) event->pitch++;
			if (key_signature < -3) event->pitch--;
			event->pitch += transposition;
		}
	}
	else if (letter == 'E') {
		event->pitch = 4;
		if (!natural) {
			if (key_signature > 5) event->pitch++;
			if (key_signature < -1) event->pitch--;
			event->pitch += transposition;
		}
	}
	else if (letter == 'F') {
		event->pitch = 5;
		if (!natural) {
			if (key_signature > 0) event->pitch++;
			if (key_signature < -6) event->pitch--;
			event->pitch += transposition;
		}
	}
	else if (letter == 'G') {
		event->pitch = 7;
		if (!natural) {
			if (key_signature > 2) event->pitch++;
			if (key_signature < -4) event->pitch--;
			event->pitch += transposition;
		}
	}
	else if (letter == 'A') {
		event->pitch = 9;
		if (!natural) {
			if (key_signature > 4) event->pitch++;
			if (key_signature < -2) event->pitch--;
			event->pitch += transposition;
		}
	}
	else if (letter == 'B') {
		event->pitch = 11;
		if (!natural) {
			if (key_signature > 6) event->pitch++;
			if (key_signature < 0) event->pitch--;
			event->pitch += transposition;
		}
	}
	else if (letter == 'N') {
		/* noise with pitch */
		event->pitch = 0;
		fseek(ifp, ftell(ifp) - 1, SEEK_SET);	/* step back in file */
		parse_number(ifp, &event->pitch);
		event->is_noise = 1;
	}
	else if (letter == 'R') {
		/* rest */
		event->pitch = 0;
	}
	else {
		fseek(ifp, ftell(ifp) - 1, SEEK_SET);	/* step back in file */
		return -1;				/* return on error */
	}

	if (sharp) event->pitch++;
	if (flat) event->pitch--;

	/* octave */
	if (letter == 'R'){
		fseek(ifp, ftell(ifp) - 1, SEEK_SET);	/* step back in file */
	} else if (letter != 'N') {
	  	if ((data > 47) && (data < 58)) {
			event->pitch += (data - 47) * 12;
			data = fgetc(ifp);
		}
		else {
			event->pitch += (octave + 1) * 12;
		}
		if (data == '^') {
			/* tie */
			event->tie = 1;
			data = fgetc(ifp);
		}
		fseek(ifp, ftell(ifp) - 1, SEEK_SET);	/* step back in file */
	}

	return 0;
}

void process_parts(FILE *afp, FILE *bfp, Part *part, int num_parts, int *mode)
{	
	int p, done;
	int max_code, max_events;
	float min_remaining;
	int current[MAX_NUM_PARTS];
	int code[MAX_NUM_PARTS];
	float remaining[MAX_NUM_PARTS];
	
	/* initialize */
	for (p=0; p<num_parts; p++) {
		encode_duration(&part[p].event[0], &code[p], &remaining[p]);
		current[p] = 0;
		//printf("i=%d duration=%f, code=%d\n", i, remaining[i], code[i]);
	}
	done = 0;

#if 1
	/* output all events */
	int event;
	for (p=0; p<num_parts; p++) {
		for (event=0; event<part[p].num_events; event++) {
			if (verbose) printf("part=%d duration=%d dot=%d is_triplet=%d pitch=%02X tie=%d, is_noise=%d\n",
			 p,
			 part[p].event[event].duration,
			 part[p].event[event].dot,
			 part[p].event[event].is_triplet,
			 part[p].event[event].pitch + ((part[p].event[event].tie == 1) ? 128 : 0),
			 part[p].event[event].tie,
			 part[p].event[event].is_noise);
		}
	}
#endif

	max_events = 0;
	for (p=0; p<num_parts; p++) {
		if (part[p].num_events > max_events) {
			max_events = part[p].num_events;
		}
	}

	while (!done) {

		/* if we have a change in mode, issue a mode command */
		for (p=0; p<num_parts; p++) {
			if ((part[p].event[current[p]].is_noise != mode[p]) && (part[p].event[current[p]].pitch > 0)) {
				mode[p] = part[p].event[current[p]].is_noise;
				if (afp) fprintf(afp, "\n\tDB\t%03XH,%03XH", 128 | (CMD_MODE << 4) | mode[p], p);
				if (bfp) {
					fputc(128 | (CMD_MODE << 4) | mode[p], bfp);	
					fputc(p, bfp);
				}
			}
		}

		/* get shortest note length across all parts */
		min_remaining = 1.5;
		max_code = 0;
		for (p=0; p<num_parts; p++) {
			if ((part[p].num_events > 0) && (current[p] < part[p].num_events)) {
				if (remaining[p] <= min_remaining) {
					min_remaining = remaining[p];
					if (code[p] > max_code)
						max_code = code[p];
				}
			}
		}
		
		//printf("min_remaining=%f max_code=%d\n", min_remaining, max_code);
		
#if 1
		for (p=0; p<num_parts; p++) {
			if (part[p].num_events > 0) break;
		}
		if (p == num_parts) return;
#endif

		if (!done) {
			/* sync score on notes with shortest length  */
			if (afp) fprintf(afp, "\n\tDB\t%03XH", CMD_EVENT << 4 | max_code);
	 		if (bfp) fputc(CMD_EVENT << 4 | max_code , bfp);

			for (p=0; p<num_parts; p++) {
				if (current[p] >= part[p].num_events) {
					/* silence for empty parts  */
				  	if (afp) fprintf(afp, ",000H");
			 		if (bfp) fputc(0, bfp);
				}
				else {
					//printf("%d-%d\n", i, current[i]);
					remaining[p] -= min_remaining;
					if (fabs(remaining[p]) < 0.0001) {
						/* event done, tie only if requested  */
					  	if (afp) fprintf(afp, ",%03XH", part[p].event[current[p]].pitch | (part[p].event[current[p]].tie ? 128 : 0));
				 		if (bfp) fputc(part[p].event[current[p]].pitch | (part[p].event[current[p]].tie ? 128 : 0) , bfp);
					  	
						/* next event */
						current[p]++;
						encode_duration(&part[p].event[current[p]], &code[p], &remaining[p]);
						if (current[p] >= max_events) done = true;						
					}
					else {
						/* split up longer event with tie */
					  	if (afp) fprintf(afp, ",%03XH", part[p].event[current[p]].pitch | 128);
						if (bfp) fputc(part[p].event[current[p]].pitch | 128 , bfp);
					}
				}
			}
		}
	}

	/* cleanup parts */
	for (p=0; p<num_parts; p++) {
		part[p].num_events = 0;
		part[p].current_event = 0;
		current[p] = 0;
		remaining[p] = 0;
	}

#if 0
	if (mode != tmp_mode) {
		/* output mode command */
 		if (afp) fprintf(afp, "n\tDB\t%03XH,%03XH", CMD_MODE << 4, mode);
 		if (bfp) {
 			fputc(CMD_MODE << 4, bfp);
 			fputc(mode, bfp);
 		}
	}
#endif

}

int main(int argc, char **argv)
{
	Part part[MAX_NUM_PARTS];			/* parts */
	int octave[MAX_NUM_PARTS];			/* current octave */
	int midi_mapping[MAX_NUM_PARTS];	/* midi channel mapping*/
	int mode[MAX_NUM_PARTS];			/* current mode (0=tune, 1=noise) */
	int num_parts = DEF_NUM_PARTS;		/* current number of parts/voices */
	int current_part = 0;				/* current part/voice */
	int current_measure = 1;			/* the measure we're currently in */
	int transposition = 0;				/* current transposition */
	int volume = 7;						/* volume between 0 (silence) and 15 (full volume) */
	int waveform = 0;					/* current waveform */
	int envelope = 0;					/* current envelope preset between 0 (no envelope) and 15 */
	int key_signature = 0;				/* current key signature (negative number of b's, positive number of #'s) */
	int metronome = 120;				/* metronome/tempo in beats per minute */
	int line = 1;						/* current line number */
	int do_repeat = 0;					/* repeat mode active */
	int repeat_start = 0;				/* pointer to start of repeat period */
	int is_triplet = 0;					/* triplet mode active */
	float time_signature;				/* measure length with reference to full notes (usually 0.5, 0.75 or 1.0) */
	long line_start = 0;				/* start of current line position in file */
	int measure, last_measure;
	
	bool use_midi = false;
	bool dynamic_assignment = false;
	
	int i, j, data, pos, voice;
	char c, *binary_outfile, *ascii_outfile, *mapping_string, *index;
	FILE *ifp, *afp, *bfp;
	
	if (argc < 2) {
usage:
		fprintf(stderr, "Usage:\n\n");
		fprintf(stderr, "%s <scorefile> [-b <binaryfile>][-a <asciifile>][-n <partcount>][-m <midi-mapping>]\n\n", argv[0]);
		exit(0);
	}

	/* initializations */
	ascii_outfile = NULL;
	binary_outfile = NULL;
	bfp = NULL;
	afp = stdout;
	mapping_string = NULL;
	for (i=0; i<MAX_NUM_PARTS; i++) {
	  	part[i].current_event = 0;
	  	part[i].num_events = 0;
	  	octave[i] = 4;
	  	midi_mapping[i] = i;
	  	mode[i] = 0;
	}
	for (i=0; i<16; i++) {
		current_base_note[i].is_active = false;
		current_base_note[i].pitch = 0;
		last_note_off_tick[i] = 0;
	}
	
	/* command line */
	i = 2;
	while (i<argc) {
	  	if (!strcmp(argv[i], "-b")) {
	  		i++;
	  		if (argc <= i) goto usage;
	  		binary_outfile = argv[i];
	  		i++;
	  	}
	  	else if (!strcmp(argv[i], "-a")) {
	  		i++;
	  		if (argc <= i) goto usage;
	  		ascii_outfile = argv[i];
	  		i++;
	  	}
	  	else if (!strcmp(argv[i], "-n")) {
	  		i++;
	  		if (argc <= i) goto usage;
	  		num_parts = atoi(argv[i]);
	  		if (num_parts > MAX_NUM_PARTS) {
				fprintf(stderr, "Parts count must be between 1 and %d\n", MAX_NUM_PARTS);
				exit(-1);
	  		}
	  		i++;
	  	}
	  	else if (!strcmp(argv[i], "-m")) {
	  		i++;
	  		if (argc <= i) goto usage;
	  		mapping_string = argv[i];
	  		i++;
	  	}
	  	else if (!strcmp(argv[i], "-d")) {
	  		i++;
	  		dynamic_assignment = true;
	  	}
	  	else if (!strcmp(argv[i], "-v")) {
	  		i++;
	  		verbose = true;
	  	}
	  	else goto usage;
	}
	
	/* parse user defined mapping */
	if (mapping_string) {
		bool done = false;
		index = mapping_string;
		pos = 0;
		for (j=0;!done;j++) {
			if (mapping_string[j] == ':') {
				mapping_string[j] = 0;
				pos = atoi(index);
				if (pos > num_parts) {
					fprintf(stderr, "Part number must be between 1 and %d\n", num_parts);
					exit(-1);
				}
				index = mapping_string+j+1;
			} else if ((mapping_string[j] == ',') || (mapping_string[j] == 0)) {
				if (mapping_string[j] == 0) done = true;
				if (pos == 0) goto usage;
				mapping_string[j] = 0;
				midi_mapping[pos-1] = atoi(index) - 1;
				index = mapping_string+j+1;
				pos = 0;
			}	
		}
		//for(j=0;j<num_parts;j++) printf("%d:%d,", j+1, midi_mapping[j]+1);
		//printf("\n");
	}
	
	/* check for .mid or .midi extension */
	  
	for (i=strlen(argv[1]); i>0; i--)
	  	if (argv[1][i] == '.') break;
	  
	if (i>0) {
	  	if (!strcmp(argv[1]+i, ".mid") || !strcmp(argv[1]+i, ".midi")) use_midi = true;
	}
	
	/* open input file */
	if (!(ifp = fopen(argv[1], "rb"))) {
		fprintf(stderr, "Could not open file \"%s\"\n", argv[1]);
		exit(-1);
	}
	fseek(ifp, 0, SEEK_SET);
	  
	/* open output files */
	if (binary_outfile) {
		  if (!(bfp = fopen(binary_outfile, "wb"))) {
			fprintf(stderr, "Could not open file \"%s\"\n", binary_outfile);
			exit(-1);
		  }
		  afp = NULL;
	}
	if (ascii_outfile) {
		  if (!(afp = fopen(ascii_outfile, "w"))) {
			fprintf(stderr, "Could not open file \"%s\"\n", ascii_outfile);
			exit(-1);
		  }
	}
	
	if (use_midi) {	/* parse MIDI binary notation */
		/* initialize the note tracker state */
		memset(note_tracker, 0, sizeof(note_tracker));
		memset(part_is_assigned, 0, sizeof(part_is_assigned));

		/* output part count */
		if (afp) fprintf(afp, "\n\tDB\t%03XH\n", CMD_PARTS << 4 | num_parts);
		if (bfp) fputc(CMD_PARTS << 4 | num_parts, bfp);
	
		/* read header chunk (MThd) */
		char chunk_id[4];
		if (fread(chunk_id, 1, 4, ifp) != 4 || strncmp(chunk_id, "MThd", 4) != 0) {
			fprintf(stderr, "Error: %s is not a valid MIDI file (missing MThd chunk).\n", argv[1]);
			fclose(ifp);
			if (afp) fclose(afp);
			if (bfp) fclose(bfp);
			return 1;
		}
	
		read_u32_be(ifp);							/* MIDI header length */
		uint16_t format = read_u16_be(ifp);			/* MIDI format type */
		uint16_t ntracks = read_u16_be(ifp);		/* number of tracks */
		uint16_t division = read_u16_be(ifp); 		/* ticks per quarter note */
	
		if (verbose) {
			printf("---------- MIDI file header ----------\n");
			printf("Format: %u\n", format);
			printf("Number of tracks: %u\n", ntracks);
		}
		
		if (division & 0x8000) {
			int fps = -(int8_t)(division >> 8);
			int tpf = division & 0xff;
			division = fps * tpf;
			if (verbose) printf("Time division (SMPTE): frames/second=%d ticks/frame=%d ticks/second=%d\n", fps, tpf, division);
			fprintf(stderr, "Error: MIDI file uses SMPTE based timing (not supported)\n");
			fclose(ifp);
			if (afp) fclose(afp);
			if (bfp) fclose(bfp);
			return 1;
		}
		else {
			if (verbose) printf("Time division (ticks/quarternote): %u\n", division);
		}
		
		if (verbose) printf("--------------------------------------\n\n");

		if (division == 0) {
			fprintf(stderr, "Error: Time Division (Ticks/QN) is 0. Cannot calculate fractions.\n");
			fclose(ifp);
			if (afp) fclose(afp);
			if (bfp) fclose(bfp);
			return 1;
		}
		
		/* 2. read track chunks (MTrk) */
		for (int i = 0; i < ntracks; i++) {
			char track_id[4];
			
			last_measure = 0;
			
			if (fread(track_id, 1, 4, ifp) != 4 || strncmp(track_id, "MTrk", 4) != 0) {
				fprintf(stderr, "Error: Missing MTrk chunk for track %d. Aborting.\n", i + 1);
				break;
			}
	
			uint32_t track_length = read_u32_be(ifp);
			
			if (verbose) {
				printf("\n--- Track Chunk %d ---\n", i + 1);
				printf("Track Length: %u bytes\n", track_length);
			}
	
			uint32_t bytes_read = 0;
			uint8_t last_status = 0; 
			uint32_t current_tick = 0; 
			
			memset(&tuplet, 0, sizeof(tuplet));

			if (track_length > 0) {
				long track_end_pos = ftell(ifp) + track_length;
				
				while (bytes_read < track_length) {
					if (ftell(ifp) >= track_end_pos) break; 
	
					/* parse MIDI track */
					parse_midi_event(ifp, afp, bfp, part, &bytes_read, &last_status, &current_tick, division, midi_mapping, num_parts, dynamic_assignment, &time_signature);
					measure = (current_tick / (division * 4) / time_signature);
					if (measure > last_measure) {
						last_measure = measure;
						if (verbose) printf("-------------- %d --------------\n", measure + 1);
					}
	
					if (bytes_read == 0) break;		/* end of track */
				}
				
				long remaining_bytes = track_end_pos - ftell(ifp);
				if (remaining_bytes > 0) {
					fprintf(stderr, "Warning: %ld unparsed bytes at end of track %d (ignored)\n", remaining_bytes, i + 1);
					skip_chunk_data(ifp, remaining_bytes);
				}
			}
		}
		process_parts(afp, bfp, part, num_parts, mode);
	}
	else {		/* parse ASCII notation */
		/* now parse score & print compact binary representation to standard output */
		while (1) {
			data = fgetc(ifp);
			
			/* key signature */
			if (data == 'K') {
				if (afp) fprintf(afp, "\n");
				if (parse_key_signature(ifp, &key_signature) < 0) {
					fprintf(stderr, "Syntax error in key signature in line %d\n", line);
					goto error;
				}
				continue;
			}
			
			/* time signature */
			if (data == 'T') {
				if (parse_time_signature(ifp, &time_signature) < 0) {
					fprintf(stderr, "Syntax error in time signature in line %d\n", line);
					goto error;
				}
				continue;
			}
			
			/* metronome/tempo  */
			if (data == 'M') {
				process_parts(afp, bfp, part, num_parts, mode);
				parse_number(ifp, &metronome);
				if (afp) fprintf(afp, "\n\tDB\t%03XH,%03XH", CMD_METRONOME << 4, metronome);
				if (bfp) {
					fputc(CMD_METRONOME << 4, bfp);
					fputc(metronome, bfp);
				}
				continue;
			}
			
			/* volume */
			if (data == 'V') {
				process_parts(afp, bfp, part, num_parts, mode);
				voice = 0;
				parse_number(ifp, &volume);
				data = fgetc(ifp);
				if (data == '@') {
					/* specific voice */
			 		parse_number(ifp, &voice);
			 		if (afp) fprintf(afp, "\n\tDB\t%03XH,%03XH", 128 | CMD_VOLUME << 4 | volume, voice);
			 		if (bfp) {
			 			fputc(128 | CMD_VOLUME << 4 | volume, bfp);
			 			fputc(voice, bfp);
			 		}
				}
				else {
					/* all voices */
			 		if (afp) fprintf(afp, "\n\tDB\t%03XH", CMD_VOLUME << 4 | volume);
			 		if (bfp) fputc(CMD_VOLUME << 4 | volume, bfp);
					fseek(ifp, ftell(ifp) - 1, SEEK_SET);			/* step back */
				}
				continue;
			}
			
			/* waveform */
			if (data == 'W') {
				process_parts(afp, bfp, part, num_parts, mode);
				voice = 0;
				parse_number(ifp, &waveform);
				data = fgetc(ifp);
				if (data == '@') {
					/* specific voice */
			 		parse_number(ifp, &voice);
			 		if (afp) fprintf(afp, "\n\tDB\t%03XH,%03XH", 128 | CMD_WAVEFORM << 4 | waveform, voice);
			 		if (bfp) {
			 			fputc(128 | CMD_WAVEFORM << 4 | waveform, bfp);
			 			fputc(voice, bfp);
			 		}
				}
				else {
					/* all voices */
			 		if (afp) fprintf(afp, "\n\tDB\t%03XH", CMD_WAVEFORM << 4 | waveform);
			 		if (bfp) fputc(CMD_WAVEFORM << 4 | waveform, bfp);
					fseek(ifp, ftell(ifp) - 1, SEEK_SET);			/* step back */
				}
				continue;
			}
			
			/* envelope */
			if (data == 'E') {
				process_parts(afp, bfp, part, num_parts, mode);
				voice = 0;
				parse_number(ifp, &envelope);
				data = fgetc(ifp);
				if (data == '@') {
					/* specific voice */
			 		parse_number(ifp, &voice);
			 		if (afp) fprintf(afp, "\n\tDB\t%03XH,%03XH", 128 | CMD_ENVELOPE << 4 | envelope, voice);
			 		if (bfp) {
			 			fputc(128 | CMD_ENVELOPE << 4 | envelope, bfp);
			 			fputc(voice, bfp);
			 		}
				}
				else {
					/* all voices */
			 		if (afp) fprintf(afp, "\n\tDB\t%03XH", CMD_ENVELOPE << 4 | envelope);
			 		if (bfp) fputc(CMD_ENVELOPE << 4 | envelope, bfp);
					fseek(ifp, ftell(ifp) - 1, SEEK_SET);			/* step back */
				}
				continue;
			}
			
			/* octave */
			if (data == 'O') {
				parse_number(ifp, &octave[current_part]);
				continue;
			}
			
			/* number of parts/voices */
			if (data == 'N') {
				parse_number(ifp, &num_parts);
				if (afp) fprintf(afp, "\n\tDB\t%03XH", CMD_PARTS << 4 | num_parts);
				if (bfp) fputc(CMD_PARTS << 4 | num_parts, bfp);
				continue;
			}
			
			/* current parts/voice */
			if (data == 'P') {
				int parts;
				parse_number(ifp, &parts);
				current_part = parts - 1;
				if (afp) fprintf(afp, "\n");
				continue;
			}
			
			/* transposition */
			if (data == '+') {
				parse_number(ifp, &transposition);
				continue;
			}
			if (data == '-') {
				parse_number(ifp, &transposition);
				transposition = -transposition;
				continue;
			}
			
			/* tuplet (currently only triplets are supported) */
			if (data == '(') {
				is_triplet = 1;
			}
			if (data == ')') {
				is_triplet = 0;
			}
			
			/* music event */
			if ((data > 47) && (data < 58)) {
				fseek(ifp, ftell(ifp) - 1, SEEK_SET);			/* step back in file */

				if (parse_event(ifp, &part[current_part].event[part[current_part].current_event],
					octave[current_part], key_signature, transposition) < 0) {
					fprintf(stderr, "Syntax error in event in line %d\n", line);
					goto error;
				}
				
				part[current_part].event[part[current_part].current_event].is_triplet = is_triplet;
				part[current_part].current_event++;
				part[current_part].num_events++;
			
				continue;
			}
		
			/* bar */
			if ((data == '|') || (data == '/')) {
				current_measure++;
			// if (afp) fprintf(afp, "\t; --------------------\n");
				continue;
			}
				
			/* repeat start */
			if (data == '[') {
				if (do_repeat) {
					fprintf(stderr, "Nesting of repeat sequences not allowed (line %d)\n", line);
					goto error;
				}
				do_repeat = 1;
				repeat_start = ftell(ifp);
				continue;
			}
			
			/* repeat end */
			if (data == ']') {
				if (do_repeat) {
		  		do_repeat = 0;
		  		fseek(ifp, repeat_start, SEEK_SET);
		  	}
				continue;
			}
		
			/* end of file */
			if ((data == '@') || (data == EOF)) {
				process_parts(afp, bfp, part, num_parts, mode);
				break;
			}
			
			/* end of line */
			if (data == '\n') {
				line_start = ftell(ifp);
				line++;
				continue;
			}
			
			/* skip whitespace & separators */
			if ((data == ' ') || (data == '\t') || (data == ',')) continue;
			
			/* comments */
			if (data == ';') {
				if (afp) {
					if (line_start == ftell(ifp) - 1)
						fprintf(afp, "\n;");
					else
						fprintf(afp, "\t;");
				}
				while ((data = fgetc(ifp)) != '\n') {
					if (data == EOF) {
						if (afp) fprintf(afp, "\n");
						goto done;
					}
					if (afp) fprintf(afp, "%c", data);
				}
				line_start = ftell(ifp);
				line++;
			}
			
			/* convert to uppercase */
			if ((data > 96) && (data < 123)) data -= 32;		
		}
		done:
		if (afp) fprintf(afp, "\n\tDB\t000H\n");
		if (bfp) fputc(0, bfp);
		fclose(ifp);
		if (afp) fclose(afp);
		if (bfp) fclose(bfp);
		exit(0);
		
		error:
		pos = ftell(ifp) - line_start - 1;
		fseek(ifp, line_start, SEEK_SET);
		while (1) {
			c = fgetc(ifp);
			if ((c == '\n') || (c == EOF)) break;
			printf("%c", c);
		}
		printf("\n");
		for (i=0; i<pos; i++) printf(" ");
		printf("^\n");
		if (ifp) fclose(ifp);
		if (afp) fclose(afp);
		if (bfp) fclose(bfp);
		exit(-1);
	}
}