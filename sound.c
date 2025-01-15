
#include "sound.h"
#include "ge0_port_interface.h"

#define isDigit(x) ((x) >= '0' && (x) <= '9')
#define NEXT_CHAR                                                              \
    rtttl.startposition++;                                                     \
    c = *(rtttl.startposition);                                                \
    if (c == 0)                                                                \
        return 0;
#define NEXT_CHAR_IN_P                                                         \
    rtttl.position++;                                                          \
    c = *(rtttl.startposition + rtttl.position);                               \
    if (c == 0)                                                                \
        return 50;
#define OCTAVE_OFFSET 0

struct RTTTL {
    char *address;
    char *startposition;
    uint16_t position;
    uint8_t loop;
    uint8_t play;
    uint8_t default_dur;
    uint8_t default_oct;
    uint16_t bpm;
    uint32_t wholenote;
    uint16_t this_tone;
    int16_t delay;
    uint8_t isPlayed;
};

struct PLAY_TONE {
    uint16_t freq;
    int16_t time;
};

struct RTTTL rtttl;
struct PLAY_TONE play_tone;

void addTone(uint16_t f, uint16_t t) {
    play_tone.freq = f;
    play_tone.time = t;
}
uint8_t loadRtttl() {
    uint16_t num;
    char c;
    rtttl.default_dur = 4;
    rtttl.default_oct = 6;
    rtttl.bpm = 63;
    rtttl.startposition = rtttl.address;
    c = *(rtttl.startposition);
    while (c != ':') {
        // ignore name
        NEXT_CHAR
    }
    NEXT_CHAR // skip ':'
              // get default duration
        if (c == 'd') {
        NEXT_CHAR
        NEXT_CHAR // skip "d="
            num = 0;
        while (isDigit(c)) {
            num = (num * 10) + (c - '0');
            NEXT_CHAR
        }
        if (num > 0)
            rtttl.default_dur = num;
        NEXT_CHAR // skip comma
    }
    // get default octave
    if (c == 'o') {
        NEXT_CHAR
        NEXT_CHAR // skip "o="
            num = c - '0';
        NEXT_CHAR
        if (num >= 3 && num <= 7)
            rtttl.default_oct = num;
        NEXT_CHAR // skip comma
    }
    // get BPM
    if (c == 'b') {
        NEXT_CHAR
        NEXT_CHAR // skip "b="
            num = 0;
        while (isDigit(c)) {
            num = (num * 10) + (c - '0');
            NEXT_CHAR
        }
        rtttl.bpm = num;
    }
    rtttl.wholenote = (60 * 1000 / rtttl.bpm) * 4;
    NEXT_CHAR
    rtttl.position = 0;
    rtttl.delay = 0;
    return 1;
}

void setRtttlAddress(char *adr) {
    rtttl.address = adr;
    loadRtttl();
}
void setRtttlLoop(uint8_t loop) { rtttl.loop = loop; }

void setRtttlPlay(uint8_t play) {
    if (play == 0) {
        rtttl.play = 0;
        ge0_port_noTone();
    } else if (play == 1) {
        rtttl.play = 1;
    } else {
        rtttl.play = 0;
        rtttl.position = 0;
        ge0_port_noTone();
    }
    rtttl.isPlayed = 0;
    rtttl.delay = 0;
}
