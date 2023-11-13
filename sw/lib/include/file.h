#pragma once
#include <stdint.h>

// Pseudo File System
typedef struct {
    int (*write)(char *, uint32_t);
    int (*read)(char *, uint32_t);
} Device_t;

extern Device_t stddev[];

/**
 * @brief Write characters into device from buffer
 * 
 * @param dev device
 * @param buf buffer
 * @param n number of chars
 * @return int 
 */
int dev_write(const Device_t *dev, char *buf, uint32_t n);

/**
 * @brief Read characters into buffer from device
 * 
 * @param dev device
 * @param buf buffer
 * @param n number of chars
 * @return int 
 */
int dev_read(const Device_t *dev, char *buf, uint32_t n);
