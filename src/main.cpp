// Screen rotation for Facet: reads the accelerometer and tells the core how
// to turn the picture. Whether the screen turns at all is the user's choice
// in Facet (Settings > Appearance > Rotation, or the shade); this module
// only says which way is up.
#include <chrono>
#include <cmath>
#include <cstdio>
#include <string>

#include "facet/plugin.h"
#include "i18n/i18n.h"
#include "orientation.h"
#include "sensor.h"

using facet::Json;
using facet::sdk::Plugin;
using facet::sdk::Screen;

#ifndef ROT_VERSION
#define ROT_VERSION "0.0.0-dev"
#endif

namespace {

double now_s() {
    using namespace std::chrono;
    return duration<double>(steady_clock::now().time_since_epoch()).count();
}

class Rotation {
public:
    explicit Rotation(Plugin& p) : plugin_(p) {}

    void start() {
        facet::Json j;
        if (facet::load_json_file(plugin_.data_dir() + "/settings.json", j)) {
            opt_.upside_down = j["upside_down"].as_bool(false);
            opt_.offset = (j["offset"].as_int(0) % 360 + 360) % 360 / 90 * 90;
        }
        orientation_.set_options(opt_);
        if (sensor_.open()) Plugin::log("rotation: accelerometer %s", sensor_.name().c_str());
        else Plugin::log("rotation: no accelerometer found");
        if (!plugin_.has_permission("sensors")) Plugin::log("rotation: the sensors permission is missing");
        refresh(true);
    }

    void tick() {
        double t = now_s();
        if (!sensor_.ok() && t - last_open_ > 10) {  // a sensor whose driver loads late
            last_open_ = t;
            if (sensor_.open()) Plugin::log("rotation: accelerometer %s", sensor_.name().c_str());
        }
        if (sensor_.ok() && sensor_.read(reading_)) {
            have_reading_ = true;
            int r = orientation_.feed(reading_, t);
            if (r >= 0 && r != sent_) {
                sent_ = r;
                plugin_.set_rotation(r);
            }
        }
        // The live values on the module's page, twice a second.
        if (plugin_.visible() && t - shown_at_ > 0.5) refresh(false);
    }

    void on_event(const std::string& id, const Json& v) {
        if (id == "upside_down") opt_.upside_down = v.as_bool();
        else if (id == "offset") opt_.offset = (v.as_int(0) % 4) * 90;
        orientation_.set_options(opt_);
        facet::Json j = facet::Json::object();
        j["upside_down"] = opt_.upside_down;
        j["offset"] = opt_.offset;
        facet::save_json_file(plugin_.data_dir() + "/settings.json", j);
        refresh(true);
    }

    void refresh(bool) {
        shown_at_ = now_s();
        Screen s(plugin_.tr("Screen rotation"));
        s.section(plugin_.tr("Sensor"));
        if (!sensor_.ok()) {
            s.info(plugin_.tr("Accelerometer"), plugin_.tr("not found"), "warn");
            s.note(plugin_.tr("No accelerometer this module may read. On phones with older Android kernels the "
                              "sensor may have to be switched on by the system first."));
        } else {
            s.info(plugin_.tr("Accelerometer"), sensor_.name(), "good");
            if (have_reading_) {
                char buf[64];
                std::snprintf(buf, sizeof buf, "%.1f  %.1f  %.1f", reading_.x, reading_.y, reading_.z);
                s.info(plugin_.tr("Reading (x y z)"), buf, "dim");
                std::string held = orientation_.flat() ? plugin_.tr("lying flat")
                                                       : std::to_string(int(std::lround(orientation_.angle()))) + "°";
                s.info(plugin_.tr("Tilt"), held);
                int r = orientation_.current();
                s.info(plugin_.tr("Picture"), r < 0 ? plugin_.tr("not decided yet") : std::to_string(r) + "°");
            }
        }
        s.section(plugin_.tr("Settings"));
        s.toggle("upside_down", plugin_.tr("Turn upside down too"), opt_.upside_down);
        s.select("offset", plugin_.tr("Sensor mounting"),
                 {plugin_.tr("as it reports"), plugin_.tr("turned 90°"), plugin_.tr("turned 180°"),
                  plugin_.tr("turned 270°")},
                 opt_.offset / 90);
        s.note(plugin_.tr("If the picture turns the wrong way, change the sensor mounting until \"Picture\" says 0° "
                          "while you hold the device upright."));
        s.note(plugin_.tr("Switch turning on or off in Settings > Appearance > Rotation (Automatic) or in the "
                          "shade."));
        plugin_.set_ui(s);
    }

private:
    Plugin& plugin_;
    rot::Sensor sensor_;
    rot::Orientation orientation_;
    rot::Orientation::Options opt_;
    rot::Reading reading_;
    bool have_reading_ = false;
    int sent_ = -1;
    double shown_at_ = 0, last_open_ = 0;
};

}  // namespace

int main() {
    Plugin plugin("rotation", ROT_VERSION);
    rot::register_translations(plugin.catalog());
    Rotation app(plugin);
    plugin.on_hello = [&](const Json&) { app.start(); };
    plugin.on_event = [&](const std::string& id, const Json& v) { app.on_event(id, v); };
    plugin.on_visible = [&](bool) { app.refresh(true); };
    plugin.on_locale = [&](const std::string&) { app.refresh(true); };
    plugin.on_tick = [&] { app.tick(); };
    return plugin.run(200);
}
