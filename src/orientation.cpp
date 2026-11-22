#include "orientation.h"

#include <cmath>

namespace rot {

namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kFlat = 0.87;       // |z| / g above this (about 60° from upright): lying flat
constexpr double kHalfSector = 30;   // a rotation is chosen within ±30° of it: 15° dead zones between
}  // namespace

int Orientation::feed(const Reading& r, double now) {
    double g = std::sqrt(r.x * r.x + r.y * r.y + r.z * r.z);
    if (g < 1e-6) return current_;
    flat_ = std::fabs(r.z) / g > kFlat;
    if (flat_) {
        pending_ = -1;
        return current_;
    }
    // Upright gives +y; turned to the left (top to the left) gives +x and
    // needs the picture turned clockwise by 90°.
    double a = std::atan2(r.x, r.y) * 180 / kPi + opt_.offset;
    a = std::fmod(std::fmod(a, 360) + 360, 360);
    angle_ = a;
    int nearest = int(std::lround(a / 90)) % 4 * 90;
    double off = std::fabs(a - (nearest == 0 && a > 180 ? 360 : nearest));
    if (off > kHalfSector) {
        pending_ = -1;
        return current_;
    }
    if (nearest == 180 && !opt_.upside_down) {
        pending_ = -1;
        return current_;
    }
    if (current_ < 0) return current_ = nearest;  // the first reading decides at once
    if (nearest == current_) {
        pending_ = -1;
        return current_;
    }
    if (nearest != pending_) {
        pending_ = nearest;
        pending_since_ = now;
    } else if (now - pending_since_ >= opt_.settle_s) {
        current_ = nearest;
        pending_ = -1;
    }
    return current_;
}

}  // namespace rot
