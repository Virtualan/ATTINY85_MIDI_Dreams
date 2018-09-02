#include <SendOnlySoftwareSerial.h>
SendOnlySoftwareSerial midiSerial(0); // Tx PB 0 pin 5

const byte
drumChan = 9,
arraySizeSet = 64,
tempoPin = 2,
LDRPin = 1,
tunings[5] = { 40, 52, 64, 76, 88 };

unsigned int scales[10] = {
	2773U, 2906U,2708U, 2418U ,  2901U, 2905U, 2925U, 2733U, 3290U,  2730U
};

unsigned int
loopCount = 0,
tickCount = 0,
tickCount2 = 0,
straightCount = 0,
scale = scales[0];

int
tuneSpeed = 0,
lightLevel = 0,
randomSpeed = 0,
oldLightLevel = 0;

unsigned long
playControl = 0,
bassPatt = 0,
pianoPatt = 0,
windPatt = 0,
synthPatt = 0,
kickPatt = 0,
hhPatt = 0,
snarePatt = 0,
notePatt = 0,
chordPatt = 0,
tickTime2 = 0,
tickTime = 0;

byte
arraySize = arraySizeSet,
articnote = 0,
filter = 0,
rcc = 0,
chord = 0,
bassChan = 0,
bassNote = 0,
bassIndex = 0,
chordIndex = 0,
chordRange = 16,
lastBassNote = 0,
bassInterval = 0,
pianoChan = 1,
pianoNote = 0,

pianoChordType = 0,
pianoIndex = 0,
windNote = 0,
windIndex = 0,
lightIndex = 0,
windChan = 2,
synthChan = 3,
synthNote = 0,
synthIndex = 0,
tickNote = 0,
tickChan = 10,
speedChange = 0,
rate = 1,
keyIndex = 0,
bpr = 32,
ppr = 32,
wpr = 32,
ta = 0,
lc = 0,
cc = 0,
sc = 0,
tt = 0,
key = 0,
kn = 0,
lightNote = 0,
drum = 0;

//volatile char keys[16];
volatile byte pianoArray[arraySizeSet];
volatile byte bassArray[arraySizeSet];
volatile byte windArray[arraySizeSet];
volatile byte chordArray[16];
char llc = 1;



/*
Here are how chords are arranged

C C# D D# E F F# G G# A A# B C C# D D# Chord HEX Value
1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 Note 8000
1 0 0 0 0 1 0 1 0 0 0 0 0 0 0 0 Sus 8500
1 0 0 0 0 1 0 1 0 0 1 0 0 0 0 0 7Sus 8520
1 0 0 0 1 0 0 0 1 0 0 0 0 0 0 0 Aug  8880
1 0 0 0 1 0 0 1 0 0 0 0 0 0 0 0 Maj  8900
1 0 0 0 1 0 0 1 0 0 0 0 1 0 0 0 Maj 9th 8908
1 0 0 0 1 0 0 1 0 0 0 1 0 0 0 0 Maj 7th 8910
1 0 0 0 1 0 0 1 0 0 1 0 0 0 0 0 7th  8920
1 0 0 0 1 0 0 1 0 1 0 0 0 0 0 0 6th  8940
1 0 0 1 0 0 0 1 0 0 0 0 0 0 0 0 Min  9100
1 0 0 1 0 0 0 1 0 0 1 0 0 0 0 0 Min 7th 9120
1 0 0 1 0 0 0 1 0 1 0 0 0 0 0 0 Min 6th 9140
1 0 0 1 0 0 1 0 0 0 0 0 0 0 0 0 Dim  9200
1 0 0 1 1 0 0 1 0 1 1 1 0 0 0 0 Const 9970
1 0 1 0 0 0 0 1 0 0 0 0 0 0 0 0 Sus2 A100
1 0 1 0 0 0 0 1 0 0 1 0 0 0 0 0 7Sus2 A120
1 0 1 1 0 0 0 1 0 0 1 0 1 0 0 0 Min 9th B128
*/

unsigned int chords[15] = {
	0x8900U, 0x8940U, 0x8920U, 0x8910U, 0x8908U, //maj chords
	0x9100U, 0x9140U, 0x9120U, 0xB128U, // min chords
	0x9200U, 0x8880U, 0xA100U, 0x8500U, 0xA120U, 0x8520U // others
}, pianoChord = 0, tickChord = 0;


void setup() {
	pinMode(2, INPUT); //the tuneSpeed speed pot - attiny85 PB2 pin 7
	pinMode(3, INPUT); //the note/lightLevel offset pot - attiny85 PB3 pin 2
	pinMode(1, OUTPUT);//output for the beat/activity LED - attiny85 PB1 pin 6
	pinMode(4, INPUT); //the LDR input for the lightLevel - attiny85 PB4 pin 3
	midiSerial.begin(31250); // Start serial port at the midi 31250 baud - out on attiny85 PB0 pin 5
	gsReset();  // reset the Boss DR-330 synth and switch to multitimberal mode
	delay(2000); //GS Reset needs a delay


	for (byte x = 0; x < 16; x++) {
		CC(x, 7, 80);  //volumes
		CC(x, 10, 64);  //pans
		CC(x, 123, 0);  //all notes off
		randomSeed(analogRead(1) + analogRead(2) + analogRead(3));
		//keys[x] = 0;
		chordArray[x] = random(15);
	}
	CC(drumChan, 7, 70);  // drumvolumes
	for (byte x = 0; x < arraySize; x++) {
		bassArray[x] = 36;// random(24, 48);//
		pianoArray[x] = 60;//  random(36, 60);//
		windArray[x] = 48;//  random(48, 70);//
	}

	/*ProgChange(bassChan, 35);
	ProgChange(pianoChan, 1);
	ProgChange(windChan, 79);*/
	randomSpeed = random(30, 80);
	ProgChange(drumChan, 0);
	playControl = random(0xffffU);
	/*kickPatt = randomPatt();
	snarePatt = randomPatt();
	hhPatt = randomPatt();
	bassPatt = randomPatt();
	pianoPatt = randomPatt();
	windPatt = randomPatt();*/
}



void loop() {
	tuneSpeed = (unsigned long)(map(analogRead(tempoPin), 0, 1023, 10, 600) + randomSpeed);

	////various loop pointers
	ta = tickCount % arraySize;
	sc = straightCount % arraySizeSet;
	tt = tickCount2 % arraySizeSet;
	lc = lightIndex % arraySizeSet;

	// check the light changes
	oldLightLevel = lightLevel;
	lightLevel = analogRead(LDRPin);//map(analogRead(LDRPin), 0, 1023, 0, 500); // read the LDR value // 
	analogWrite(1, abs(lightLevel - oldLightLevel) * 30);
	llc = (char)(lightLevel - oldLightLevel);   //light level change - brighter is +ve,  darker is -ve

	// light changes to the melody arrays are either 
	// 50% chance of light change amount, 
	// 10 % chance of circle of fiths or nothing 

	if (llc != 0) {
		pianoArray[lc] = random(2) ? ScaleFilter(scale, pianoArray[lc] + llc, key) : pianoArray[lc];// random(10) ? windArray[lc] : bassArray[lc];// ((pianoArray[lc] + 7) % 12); //
		windArray[lc] = random(2) ? ScaleFilter(scale, windArray[lc] + llc, key) : windArray[lc];// random(10) ? pianoArray[lc] : bassArray[lc];// ((windArray[lc] + 7) % 12); //
		bassArray[lc] = random(2) ? ScaleFilter(scale, bassArray[lc] + llc, key) : bassArray[lc];// random(10) ? windArray[lc] : pianoArray[lc];// ((bassArray[lc] + 7) % 12); //
		lightIndex++;
	}


	if (millis() > tickTime) {

		DoArticulations();
		//SNARE
		if ((bitRead(playControl, 0) && sc % (3 + rate) == 0) && ((snarePatt >> (sc % 32)) & 1)) {
			drum = 38 + pianoArray[sc] % 2 * 2;
			NoteOn(drumChan, drum, random(50, 100));
			NoteOff(drumChan, drum);
		}
		//KICK
		if ((bitRead(playControl, 1) && sc % 2 == 0) && ((kickPatt >> (sc % 32)) & 1)) {
			cc = random(16);
			if (cc != drumChan && cc != bassChan) {
				CC(cc, 10, random(127));
			}
			drum = 35 + bassArray[sc] % 2;
			NoteOn(drumChan, drum, random(60, 100));
			NoteOff(drumChan, drum);
		}
		// HIGH HATS
		if (bitRead(playControl, 2) && ((hhPatt >> (sc % 32)) & 1)) {
			drum = 42 + windArray[sc] % 3 * 2;
			NoteOn(drumChan, drum, random(30, 100));
			NoteOff(drumChan, drum);
		}

		//PIANO
		if (bitRead(playControl, 3) && ((pianoPatt >> (sc % ppr)) & 1)) {
			NoteOff(pianoChan, pianoNote);
			playChord(pianoChord, pianoChan, pianoNote, 0, 0, pianoChordType);
			pianoChord = chords[chord];
			pianoNote = key + 24 + (ScaleFilter(scale, pianoArray[ta], key) % 60);
			chord = chordArray[pianoIndex % chordRange];
			//chordIndex++;
			pianoChordType = random(2);
			DoFilter(pianoChan, 80, (bassArray[pianoIndex%arraySize] % 16) * 8);
			ta % rate == 0 ?
				playChord(pianoChord, pianoChan, pianoNote, random(60, 100), 1, pianoChordType) :  //**************NOTE ON*********************
				NoteOn(pianoChan, pianoNote, 50);
			pianoIndex++;
			if (llc > 0) {
				chordArray[chordIndex % 16] = random(0, 5);
				chordIndex++;
			}
			else if (llc < 0) {
				chordArray[chordIndex % 16] = random(5, 10);
				chordIndex++;
			}

			if (chordIndex % 16 == 0) {
				chordIndex++;
				arraySize = random(4, arraySizeSet << 1) >> 1;
				killPlayers();
				playControl |= 0x07;
				NoteOff(tickChan, tickNote);
				playChord(chords[chord], tickChan, tickNote, 0, 0, 1);
				tickChan = random(10, 16);
			}

		}
		//BASS
		if (bitRead(playControl, 4) && ((bassPatt >> sc % bpr) & 1)) {
			NoteOff(bassChan, bassNote);
			bassNote = key + 24 + ScaleFilter(scale, (ScaleFilter(pianoChord, bassArray[ta], key) % 36), key);
			DoFilter(bassChan, 80, rr());
			NoteOn(bassChan, bassNote, random(70, 100));//**************NOTE ON*********************
			bassIndex++;
		}
		//WINDS
		if (bitRead(playControl, 5) && ((windPatt >> sc % wpr) & 1)) {
			NoteOff(windChan, windNote);
			windNote = key + 12 + ScaleFilter(scale, (ScaleFilter(pianoChord, windArray[ta], key) % 60), key);
			NoteOn(windChan, windNote, random(30, 100));//**************NOTE ON*********************
			windIndex++;
		}
		//SYNTH
		if (bitRead(playControl, 6) && ((synthPatt >> sc % 32) & 1)) {
			NoteOff(synthChan, synthNote);
			synthNote = key + ScaleFilter(scale, ScaleFilter(pianoChord, (24 + (windArray[synthIndex%arraySizeSet]) % 60), key), key);
			NoteOn(synthChan, synthNote, random(30, 100));//**************NOTE ON*********************
			synthIndex++;
		}
		//ALT CHORDS
		if (bitRead(playControl, 7)) {  
			//&& !((pianoPatt >> (ta % 32)) & 1)
			//&& !((windPatt >> (ta % 32)) & 1)
			//&& !((bassPatt >> (ta % 32)) & 1)
			NoteOff(tickChan, tickNote);
			playChord(tickChord, tickChan, tickNote, 0, 0, 1);
			tickChord = chords[chord];
			tickNote = key + ScaleFilter(scale, ScaleFilter(tickChord,
				24 +
				(windArray[tt] % 24 +
					pianoArray[tt] % 12 +
					bassArray[tt] % 12),
				key), key);
			DoFilter(tickChan, 80, (windArray[sc] % 32) * 4);
			sc % (rate << 1) == 0 ? NoteOn(tickChan, tickNote, 50) :
				playChord(tickChord, tickChan, tickNote, random(30, 100), 1, 1); //**************NOTE ON*********************
		}


		if (straightCount % int(arraySize << 2) == 0) {
			key = random(12);
			randomSpeed += random(3) - 1;
			rate = random(1, 5);
			//chordRange = random(1, 5) * 4;
			
			snarePatt = random(5) ? snarePatt : randomPatt();
			kickPatt = random(5) ? kickPatt : randomPatt();
			hhPatt = random(5) ? hhPatt : randomPatt();
			bassPatt = random(5) ? bassPatt : randomPatt();
			pianoPatt = random(5) ? pianoPatt : randomPatt();
			windPatt = random(5) ? windPatt : randomPatt();


			scale = scales[random(10)];
			if (llc > 0) {
				scale = scales[0];  // force major
			}
			else if (llc < 0) {
				scale = scales[1];  // force minor
			}
			ProgChange(bassChan, random(32, 40));
			ProgChange(pianoChan, random(9));
			ProgChange(windChan, random(70, 80));

			//drum tunings
			CC(drumChan, 0x63, 0x18);
			CC(drumChan, 0x62, random(12, 60));
			CC(drumChan, 6, random(0x2e, 0x5f));

			//drum reverbs
			CC(drumChan, 0x63, 0x1D);
			CC(drumChan, 0x62, random(12, 60));
			CC(drumChan, 6, random(127));

			//drum chorus
			CC(drumChan, 0x63, 0x1E);
			CC(drumChan, 0x62, random(12, 60));
			CC(drumChan, 6, random(127));

			cc = random(16);
			while (cc == drumChan) {
				cc = random(16);
				
			}

			NoteOff(tickChan, tickNote);
			playChord(tickChord, tickChan, tickNote, 0, 0, 1);
			tickChan = cc;
			//if (cc != drumChan ) {  // && cc > pianoChan
				//CC(cc, 10, random(127));
			ProgChange(cc, random(119));
			if (bitRead(playControl, 8)) {
				MasterTune(cc, tunings[random(5)]); // -24,-12,0,12,24 (octaves)   40 - 88 // 
				ADSR(cc, rr(), rr(), rr());
			}
			if (bitRead(playControl, 9)) {
				//vib Rate
				CC(cc, 0x63, 0x01);
				CC(cc, 0x62, 0x08);
				CC(cc, 6, rr());

				//vib depth
				CC(cc, 0x63, 0x01);
				CC(cc, 0x62, 0x09);
				CC(cc, 6, rr() >> 1);

				//vib delay
				CC(cc, 0x63, 0x01);
				CC(cc, 0x62, 0x0A);
				CC(cc, 6, rr() >> 1);
			}
			//	}
		}

		else if (straightCount % arraySize == 0) {
			playControl |= 1 << random(10);
			bpr = 32 >> random(3);
			ppr = 32 >> random(3);
			wpr = 32 >> random(3);
			cc = random(16);
			CC(cc, 0x5B, random(127)); // reverb send
			CC(cc, 0x5D, random(127)); // chorus send
		}

		tickCount += rate;
		if (tickCount % (1 + (bassArray[tickCount2%arraySizeSet] % 4)) == 0) {
			tickCount2++;
		}
		straightCount++;
		tickTime = (unsigned long)(millis() + tuneSpeed);

	}
	
	//PitchBend(random(16), loopCount);
	loopCount++;

}  // end of loop


byte rr() {  // random values for asdr and filter settings
	return random(0x0e, 0x72);
}

byte rp() {  // random bit pointers
	return random(32);
}

//// pitchBend
//void PitchBend(byte chan, unsigned int bend) {
//	byte one = bend >> 7;
//	byte two = bend & 0x007F;
//	midiSerial.write((chan + 0xE0));
//	midiSerial.write(two); //lsb
//	midiSerial.write(one); //msb
//}

unsigned long randomPatt() { // random long patterns formed from nibbles, bytes or ints :)
	byte r = random(3);

	unsigned long n = random(1, 0xFFFFU);
	if (r == 0) {
		n = (((n & 0x0F) << 28) + ((n & 0x0F) << 24) + ((n & 0x0F) << 20) +
			((n & 0x0F) << 16) + ((n & 0x0F) << 12) + ((n & 0x0F) << 8) +
			((n & 0x0F) << 4) + (n & 0x0F));
	}
	else if (r == 1) {
		n = (((n & 0xFF) << 24) + ((n & 0xFF) << 16) + ((n & 0xFF) << 8) + (n & 0xFF));
	}
	else {
		n = ((n << 16) + n);
	}
	return n;
}



void killPlayers() {  // wipe all playing notes
	for (byte x = 0; x < 16; x++) {
		CC(x, 123, 0);
		NoteOn(9, 56, 100); NoteOff(9, 56);
	}
}

// send note on stuff
void NoteOn(byte chan, byte note, byte vel) {
	if (note) {
		chan &= 0x0F;
		midiSerial.write((chan + 0x90));
		midiSerial.write(note & 0x7F);
		midiSerial.write(vel & 0x7F);
	}
}
// send note off stuff
void NoteOff(byte chan, byte note) {
	chan &= 0x0F;
	midiSerial.write((chan + 0x80));
	midiSerial.write(note & 0x7F);
	midiSerial.write(byte(0));
}

// scale correction - the input note is moved until it belongs to the current scale
byte ScaleFilter(unsigned int s, byte n, int k) {
	byte nm = n;
	byte x = 0;
	byte range = 16;
	if (s & 0xF000U == 0U) {
		s <<= 4;  // its a scale not a chord
		range = 12;
	}
	while (!(bitRead(s, (15 - (((n % 12) + (k % 12)) % range)))) && x < 16) {
		n++;
		x++;
	}
	if (x < 16) {
		return n;
	}
	else {
		return nm;
	}
}



//chord plays the scale corrected chord
void playChord(unsigned int cord, byte chan, byte note, byte vel, byte cont, byte type) {
	// cont is either play or kill
	//chan &= 0x0f;
	for (byte c = 0; c < 15; c++) {
		if (((cord << c) & 0x8000U) > 0U) {
			//delay(100);
			if (cont) {
				NoteOn(type ? chan : (chan + c % 16), ScaleFilter(scale, note + c, key), vel); //ScaleFilter(scale, note + c + key, key)
			}
			else {
				CC(type ? chan : (chan + c % 16), 123, 0);
				//NoteOff(type ? chan : (chan + c) % 16, note + c); //ScaleFilter(scale, note + c + key, key)
			}
			if (cord == 0x8000U) { // no point going any further
				break;
			}
		}
	}
}


//specificaly NRPN for BOSS DR-Synth 330
void MasterTune(byte chan, byte b) {
	//chan &= 0x0F;
	CC(chan, 0x65, 0);
	CC(chan, 0x64, 2); // should be 2 - 0 for pitch bend range
	CC(chan, 6, b);
}

//specificaly NRPN for BOSS DR-Synth 330
void ADSR(byte chan, byte a, byte d, byte r) {
	chan &= 0x0F;
	//Attack
	CC(chan, 0x63, 0x01);
	CC(chan, 0x62, 0x63);
	CC(chan, 6, a);
	//decay
	CC(chan, 0x63, 0x01);
	CC(chan, 0x62, 0x64);
	CC(chan, 6, d);
	//release
	CC(chan, 0x63, 0x01);
	CC(chan, 0x62, 0x66);
	CC(chan, 6, r);
}

//Basic Channel contro message
void CC(byte chan, byte cont, byte val) {
	chan &= 0x0F;
	midiSerial.write((chan + 0xB0));
	midiSerial.write(cont & 0x7F);
	midiSerial.write(val & 0x7F);
}


//specificaly NRPN for BOSS DR-Synth 330
void DoFilter(byte ch, byte res, byte coff) {
	ch &= 0x0F;
	CC(ch, 0x63, 0x01);
	CC(ch, 0x62, 0x21);
	CC(ch, 6, res & 0x7F); //resonance can go to 0x72
	CC(ch, 0x63, 0x01);
	CC(ch, 0x62, 0x20);
	CC(ch, 6, coff & 0x7F);//cut off frequency
}

void DoArticulations() {
	articnote = random(23, 127);
	rcc = random(16);
	NoteOn(rcc, articnote, 1);
	NoteOff(rcc, articnote);
}

//specificaly sysex for BOSS DR-Synth DS330
void gsReset() {
	byte gs[11] = { 0xF0, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xF7 };
	for (byte g = 0; g < 11; g++) {
		midiSerial.write(gs[g]);
	}
}

// Program change for midi channel
void ProgChange(byte chan, byte prog) {
	chan &= 0x0F;
	midiSerial.write((chan + 0xC0));
	midiSerial.write(prog & 0x7F);
}