#include <platform.h>
#include <assert.h>
#include <file.h>

Device_t stddev[N_STDDEV];

int dev_write(const Device_t *dev, char *buf, uint32_t n){
    assert(dev);
	if(!dev->write)
		return 1;
	return dev->write(buf, n);
}

int dev_read(const Device_t *dev, char *buf, uint32_t n){
    assert(dev);
	if(!dev->read)
		return 1;
	return dev->read(buf, n);
}
