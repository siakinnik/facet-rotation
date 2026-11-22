// Russian translation of the screen rotation module. Keys are the English source strings.
#include "i18n/i18n.h"

namespace rot {

namespace {

const facet::i18n::Table& ru() {
    static const facet::i18n::Table table = {
        {"Screen rotation", "Поворот экрана"},
        {"Sensor", "Датчик"},
        {"Accelerometer", "Акселерометр"},
        {"not found", "не найден"},
        {"No accelerometer this module may read. On phones with older Android kernels the sensor may have to be "
         "switched on by the system first.",
         "Нет акселерометра, доступного модулю. На телефонах со старыми ядрами Android датчик, возможно, сначала "
         "должна включить система."},
        {"Reading (x y z)", "Показания (x y z)"},
        {"Tilt", "Наклон"},
        {"lying flat", "лежит горизонтально"},
        {"Picture", "Изображение"},
        {"not decided yet", "ещё не определено"},
        {"Settings", "Настройки"},
        {"Turn upside down too", "Переворачивать вверх ногами"},
        {"Sensor mounting", "Установка датчика"},
        {"as it reports", "как сообщает"},
        {"turned 90°", "повёрнут на 90°"},
        {"turned 180°", "повёрнут на 180°"},
        {"turned 270°", "повёрнут на 270°"},
        {"If the picture turns the wrong way, change the sensor mounting until \"Picture\" says 0° while you hold "
         "the device upright.",
         "Если изображение поворачивается не туда, меняйте установку датчика, пока «Изображение» не покажет 0°, "
         "когда устройство держат вертикально."},
        {"Switch turning on or off in Settings > Appearance > Rotation (Automatic) or in the shade.",
         "Включить или выключить поворот можно в Настройках > Оформление > Поворот (Автоматически) или в шторке."},
    };
    return table;
}

}  // namespace

void register_translations(facet::i18n::Catalog& catalog) { catalog.add("ru", ru()); }

}  // namespace rot
