#include "display.h"
#include "engine.h"
#include "ge0_port_interface.h"
#include "screen.h"
#include "sound.h"
#include <stdint.h>
#define DELAY(x)

uint32_t timeF, timeR, timeSpr, timeGpu;
uint32_t cadr_count;
uint32_t redraw;
uint8_t fps;

extern volatile uint8_t thiskey;
extern volatile uint16_t timers[8];

void screen_worker(void) {
    while (1) {
        DELAY(timeForRedraw);
        timeR = ge0_port_millis();
        clearSpriteScr();
        redrawSprites();
        moveSprites();
        testSpriteCollision();
        redrawParticles();
        timeSpr += ge0_port_millis() - timeR;
        timeR = ge0_port_millis();
        redrawScreen();
        redraw = 1;
        timeGpu += ge0_port_millis() - timeR;
        cadr_count++;
        if (ge0_port_millis() - timeF > 1000) {
            timeF = ge0_port_millis();
            fps = cadr_count;
            cadr_count = cadr_count % 2;
        }
    }
}

void key_worker(void) {
    while (1) {
        DELAY(100);
        thiskey = ge0_port_get_key(); // 这是否会影响组合键？
    }
}

void game_logic_worker(void) {
    user_main();
    while (1) {
        DELAY(1000);
    }
}

void sound_worker(void) {
    while (1) {
        DELAY(1);
        if (delay_rtttl <= 0)
            delay_rtttl = playRtttl();
    }
}

void timer_tick(void) {
    for (int16_t i = 0; i < 8; i++) {
        if (timers[i] >= 1)
            timers[i]--;
    }
    delay_rtttl--;
    updateRtttl();
}
