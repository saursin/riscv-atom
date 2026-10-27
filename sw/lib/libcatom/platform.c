#include <platform.h>

const uint32_t *__platform;

const uint32_t *platform_find(uint32_t tag, unsigned n)
{
    const uint32_t *p = __platform;
    if(!p || p[0] != PLATFORM_MAGIC || p[1] != PLATFORM_VERSION)
        return 0;

    // walk the tags, never past the table's size
    const uint8_t *t = (const uint8_t *)(p + 3);
    const uint8_t *end = (const uint8_t *)p + p[2];
    while(t + 8 <= end) {
        const uint32_t *tg = (const uint32_t *)t;
        if(tg[1] < 8 || tg[1] > (uint32_t)(end - t))
            return 0;   // corrupt
        if(tg[0] == tag && n-- == 0)
            return tg;
        t += tg[1];
    }
    return 0;
}

uint32_t platform_clk(void)
{
    const uint32_t *t = platform_find(PLATFORM_TAG_CLK, 0);
    return (t && t[1] >= 12) ? t[2] : 0;
}
