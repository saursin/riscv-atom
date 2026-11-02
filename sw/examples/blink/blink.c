// GPIO out and in: patterns on the LEDs; a button switches to the next one.
// Pins as on the TangPrimer 20K (synth/gowin/tangprimer20k/tangprimer20k_shell.v); change the
// defines for another board.
#include <stdio.h>
#include <gpio.h>
#include <time.h>

#define NUM_LEDS    6       // LEDs on GPIO 0..5
#define BUTTON      12      // button S1 on GPIO 12, HIGH while pressed
#define STEP_MS     100

#define NUM_PATTERNS 4
static const char *names[NUM_PATTERNS] = {"blink", "chase", "bounce", "count"};

// LEDs to light at step n of pattern p
static uint32_t pattern(int p, unsigned n) {
    switch(p) {
        case 0:                                     // all on, all off
            return (n & 1) ? (1u << NUM_LEDS) - 1 : 0;
        case 1:                                     // one LED moving along
            return 1u << (n % NUM_LEDS);
        case 2: {                                   // one LED going back and forth
            unsigned k = n % (2 * NUM_LEDS - 2);
            return 1u << (k < NUM_LEDS ? k : 2 * NUM_LEDS - 2 - k);
        }
        default:                                    // binary counter
            return n & ((1u << NUM_LEDS) - 1);
    }
}

// true once per press of the button (not again while it is held)
static int button_pressed() {
    static int was_down = 0;
    int down = gpio_read(BUTTON) == HIGH;
    int pressed = down && !was_down;
    was_down = down;
    return pressed;
}

int main() {
    for(int i = 0; i < NUM_LEDS; i++)
        gpio_setmode(i, OUTPUT);
    puts("blink: press S1 for the next pattern\n");

    int p = 0;
    printf("pattern: %s\n", names[p]);
    for(unsigned n = 0; ; n++) {
        if(button_pressed()) {
            p = (p + 1) % NUM_PATTERNS;
            printf("pattern: %s\n", names[p]);
        }
        gpio_writew(pattern(p, n));
        sleep_ms(STEP_MS);
    }
    return 0;
}
