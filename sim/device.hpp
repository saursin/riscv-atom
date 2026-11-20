#pragma once

/**
 * @brief A model of something attached to the SoC's pins (a flash chip, a sensor, ...)
 * @details A backend keeps a list of attached devices and calls eval() on each once per clock
 * cycle, before the clock edge, so a device reads the SoC's outputs and drives its inputs. A new
 * model is a subclass plus one line where the backend attaches it.
 */
class Device
{
public:
    virtual ~Device() {}

    /**
     * @brief Called once per clock cycle
     */
    virtual void eval() = 0;
};
