#pragma once

// Error codes
#define SD_SUCCESS              0
#define SD_ERR_UNKNOWN_CARD     1
#define SD_ERR_MAYBE_VER1CARD   2

uint8_t sd_init(struct SPI_Config *cfg);
