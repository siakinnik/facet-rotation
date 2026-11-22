// Unit tests of the orientation decisions.
#include <cmath>
#include <cstdio>

#include "orientation.h"

static int fails = 0;
#define CHECK(c)                                                     \
    do {                                                             \
        if (!(c)) {                                                  \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); \
            ++fails;                                                 \
        }                                                            \
    } while (0)

static rot::Reading at(double deg, double flat_deg = 0) {
    double r = deg * 3.14159265358979 / 180, f = flat_deg * 3.14159265358979 / 180;
    return {std::sin(r) * std::cos(f) * 9.8, std::cos(r) * std::cos(f) * 9.8, std::sin(f) * 9.8};
}

int main() {
    rot::Orientation o;
    CHECK(o.current() == -1);
    CHECK(o.feed(at(0), 0) == 0);           // upright: decided at once
    CHECK(o.feed(at(90), 1.0) == 0);        // turned left: not yet
    CHECK(o.feed(at(92), 1.3) == 0);
    CHECK(o.feed(at(88), 1.6) == 90);       // held for 0.6 s: turned
    CHECK(o.feed(at(50), 2.0) == 90);       // between two: stays (dead zone)
    CHECK(o.feed(at(0, 75), 3.0) == 90);    // flat on the table: stays
    CHECK(o.flat());
    CHECK(o.feed(at(180), 4.0) == 90);      // upside down not allowed
    CHECK(o.feed(at(180), 5.0) == 90);
    CHECK(o.feed(at(270), 6.0) == 90);
    CHECK(o.feed(at(-90), 6.6) == 270);     // turned right
    CHECK(o.feed(at(0), 7.0) == 270);
    CHECK(o.feed(at(10), 7.2) == 270);      // a short wobble back and forth
    CHECK(o.feed(at(60), 7.3) == 270);
    CHECK(o.feed(at(0), 7.4) == 270);
    CHECK(o.feed(at(0), 8.0) == 0);

    rot::Orientation u;
    rot::Orientation::Options opt;
    opt.upside_down = true;
    opt.offset = 90;  // a sensor mounted turned by 90°
    u.set_options(opt);
    CHECK(u.feed(at(90), 0) == 180);
    CHECK(u.feed(at(-90), 0.1) == 180);
    CHECK(u.feed(at(-90), 0.7) == 0);
    std::printf(fails ? "%d FAILED\n" : "all passed\n", fails);
    return fails ? 1 : 0;
}
