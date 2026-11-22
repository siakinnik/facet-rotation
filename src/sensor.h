// The accelerometer, through whichever interface the kernel offers:
// - IIO (/sys/bus/iio/devices/iio:deviceN/in_accel_*_raw), mainline kernels;
//   the mounting matrix the device tree gives is applied;
// - evdev (/dev/input/eventN with absolute X, Y and Z), older Android
//   kernels. The core gives this module only motion sensors, never the
//   touchscreen or a keyboard.
#pragma once

#include <string>

#include "orientation.h"

namespace rot {

class Sensor {
public:
    ~Sensor();
    bool open();  // finds the first accelerometer
    bool ok() const { return kind_ != Kind::None; }
    const std::string& name() const { return name_; }
    bool read(Reading& out);

private:
    enum class Kind { None, Iio, Evdev } kind_ = Kind::None;
    std::string dir_;  // IIO device directory
    int fd_ = -1;      // evdev
    std::string name_;
    double m_[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};  // mounting matrix (IIO)
};

}  // namespace rot
