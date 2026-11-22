#include "sensor.h"

#include <dirent.h>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

namespace rot {

namespace {

bool read_file(const std::string& path, std::string& out) {
    std::ifstream f(path);
    if (!f) return false;
    std::getline(f, out);
    return true;
}

bool read_number(const std::string& path, double& v) {
    std::string s;
    if (!read_file(path, s) || s.empty()) return false;
    char* end = nullptr;
    v = std::strtod(s.c_str(), &end);
    return end != s.c_str();
}

bool test_bit(const unsigned long* bits, int bit) {
    constexpr int kBits = sizeof(unsigned long) * 8;
    return (bits[bit / kBits] >> (bit % kBits)) & 1;
}

}  // namespace

Sensor::~Sensor() {
    if (fd_ >= 0) close(fd_);
}

bool Sensor::open() {
    // IIO first: it says how the chip is mounted.
    if (DIR* d = opendir("/sys/bus/iio/devices")) {
        while (dirent* e = readdir(d)) {
            if (std::strncmp(e->d_name, "iio:device", 10) != 0) continue;
            std::string dir = std::string("/sys/bus/iio/devices/") + e->d_name;
            double v;
            if (!read_number(dir + "/in_accel_x_raw", v) || !read_number(dir + "/in_accel_y_raw", v) ||
                !read_number(dir + "/in_accel_z_raw", v))
                continue;
            kind_ = Kind::Iio;
            dir_ = dir;
            read_file(dir + "/name", name_);
            std::string mm;
            if (read_file(dir + "/in_accel_mount_matrix", mm) || read_file(dir + "/mount_matrix", mm)) {
                for (char& c : mm)
                    if (c == ',' || c == ';') c = ' ';
                std::istringstream in(mm);
                double m[9];
                int n = 0;
                while (n < 9 && in >> m[n]) ++n;
                if (n == 9) std::memcpy(m_, m, sizeof m);
            }
            break;
        }
        closedir(d);
    }
    if (kind_ == Kind::Iio) return true;
    if (DIR* d = opendir("/dev/input")) {
        while (dirent* e = readdir(d)) {
            if (std::strncmp(e->d_name, "event", 5) != 0) continue;
            std::string path = std::string("/dev/input/") + e->d_name;
            int fd = ::open(path.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
            if (fd < 0) continue;
            unsigned long abs[ABS_MAX / (sizeof(long) * 8) + 1] = {};
            ioctl(fd, EVIOCGBIT(EV_ABS, sizeof abs), abs);
            if (!test_bit(abs, ABS_X) || !test_bit(abs, ABS_Y) || !test_bit(abs, ABS_Z)) {
                close(fd);
                continue;
            }
            char nm[128] = "?";
            ioctl(fd, EVIOCGNAME(sizeof nm), nm);
            name_ = nm;
            fd_ = fd;
            kind_ = Kind::Evdev;
            break;
        }
        closedir(d);
    }
    return kind_ != Kind::None;
}

bool Sensor::read(Reading& out) {
    double v[3];
    if (kind_ == Kind::Iio) {
        const char* axes[] = {"x", "y", "z"};
        for (int i = 0; i < 3; ++i)
            if (!read_number(dir_ + "/in_accel_" + axes[i] + "_raw", v[i])) return false;
        // The same scale for every axis: the angles do not need it.
        out.x = m_[0] * v[0] + m_[1] * v[1] + m_[2] * v[2];
        out.y = m_[3] * v[0] + m_[4] * v[1] + m_[5] * v[2];
        out.z = m_[6] * v[0] + m_[7] * v[1] + m_[8] * v[2];
        return true;
    }
    if (kind_ == Kind::Evdev) {
        // The current values, without depending on events arriving.
        const int codes[] = {ABS_X, ABS_Y, ABS_Z};
        for (int i = 0; i < 3; ++i) {
            input_absinfo a{};
            if (ioctl(fd_, EVIOCGABS(codes[i]), &a) != 0) return false;
            v[i] = a.value;
        }
        // Drain the event queue so it never fills up.
        input_event ev[64];
        while (::read(fd_, ev, sizeof ev) > 0) {
        }
        out = {v[0], v[1], v[2]};
        return true;
    }
    return false;
}

}  // namespace rot
