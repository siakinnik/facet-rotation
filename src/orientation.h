// From gravity to the screen's rotation, as phones do it: the picture turns
// only when the device is held clearly in a new way for a moment, never while
// it lies flat, and (by default) never upside down.
#pragma once

namespace rot {

struct Reading {
    double x = 0, y = 0, z = 0;  // gravity in the device's axes: x right, y up, z out of the screen
};

class Orientation {
public:
    struct Options {
        bool upside_down = false;  // 180° allowed
        int offset = 0;            // the sensor is mounted turned by this (0, 90, 180, 270)
        double settle_s = 0.5;     // a new way of holding must last this long
    };

    void set_options(const Options& o) { opt_ = o; }
    // The rotation to show (0, 90, 180, 270 clockwise), -1 before the first decision.
    int feed(const Reading& r, double now);
    int current() const { return current_; }
    // The last tilt around the screen's axis (degrees, 0 = upright) and whether it lay flat.
    double angle() const { return angle_; }
    bool flat() const { return flat_; }

private:
    Options opt_;
    int current_ = -1, pending_ = -1;
    double pending_since_ = 0, angle_ = 0;
    bool flat_ = false;
};

}  // namespace rot
