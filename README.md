# rotation — Facet module

Turns [Facet](https://github.com/siakinnik/facet-core)'s screen to match how
the device is held, from the accelerometer.

## How it works

- Reads the accelerometer through IIO (`/sys/bus/iio`, mainline kernels; the
  device tree's mounting matrix is applied) or evdev (older Android kernels).
  Facet gives the module only motion sensors (permission `sensors`), never the
  touchscreen or a keyboard.
- Turns like a phone: only once the device is clearly held a new way for half
  a second, not while it lies flat, and not upside down unless allowed.
- Whether the screen turns at all is chosen in Facet: Settings > Appearance >
  Rotation > Automatic, or the rotation tile of the shade.
- If the picture turns the wrong way, the module's page (Settings > Modules >
  Screen rotation) shows the live reading and lets you set how the sensor is
  mounted.

Phones with older Android kernels (MediaTek's hwmsen and similar) often switch
the accelerometer on only when the Android sensor service asks; such a sensor
reports nothing here.

## Requirements

Facet 0.9 or newer. Permissions: `display.rotation`, `sensors` (dangerous),
`background`. Rotation works while Facet draws with the processor.

## Install

```bash
curl -fsSL https://raw.githubusercontent.com/siakinnik/facet-core/main/scripts/get.sh | sudo bash -s -- --plugin siakinnik/facet-rotation
```

## Build

```bash
cmake -S . -B build -G Ninja && cmake --build build && ctest --test-dir build
```

## Licence

GPL-3.0, see [LICENSE](LICENSE).
