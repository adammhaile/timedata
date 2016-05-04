#pragma once

#include <stdlib.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <strstream>

#include <tdsp/color/names.h>
#include <tdsp/base/enum.h>
#include <tdsp/base/throw.h>
#include <tdsp/base/math_inl.h>

namespace tdsp {

template <typename Collection, typename Function>
void forEachPair(Collection const& coll, Function f) {
    for (size_t i = 0; i + 1 < coll.size(); ++i)
        for (size_t j = i + 1; j < coll.size(); ++j)
            f(coll[i], coll[j]);
}

template <typename Collection>
typename Collection::value_type maxPairedDistanceAbs(Collection const& coll) {
    using Number = typename Collection::value_type;
    Number result = 0;
    forEachPair(coll, [&](Number x, Number y) {
        result = std::max(result, std::abs(std::abs(x) - std::abs(y)));
    });
    return result;
}

inline std::string colorToString(float r, float g, float b) {
    return toString(Color{{r, g, b}});
}

inline Color toColor(unsigned int hex) {
    static const auto BYTE = 256;
    auto b = hex % BYTE;
    hex /= BYTE;

    auto g = hex % BYTE;
    hex /= BYTE;

    auto r = hex % BYTE;
    return {{r / 255.0f, g / 255.0f, b / 255.0f}};
};

inline bool isNearHex(float decimal) {
    return isNearFraction(decimal, 255);
}

inline bool isGray(Color color) {
    return maxPairedDistanceAbs(color) < 0.0001;
}

inline float strtof(const char *nptr, char const **endptr) {
    char* ep;
    auto r = ::strtof(nptr, &ep);
    *endptr = ep;
    return r;
}

inline uint32_t fromHex(Color c) {
    uint32_t total = 0;
    static uint32_t const max = 256;
    for (auto& i : c)
        (total *= max) += std::min(max - 1, uint32_t(256 * std::abs(i)));
    return total;
}


inline std::string toString(Color c) {
    auto addNegatives = [&](std::string const& value) {
        auto s = value;
        auto negative = Sample<RGB, bool>{{c[0] < 0, c[1] < 0, c[2] < 0}};
        if (negative[0] or negative[1] or negative[2]) {
            for (auto n : negative)
                s += "+-"[n];
        }
        return s;
    };

    if (std::all_of(c.begin(), c.end(), isNearHex)) {
        auto hex = fromHex(c);

        auto i = colorNamesInverse().find(hex);
        if (i != colorNamesInverse().end())
            return addNegatives(i->second);
    }

    if (isGray(c))
        return addNegatives("gray " + toString(100 * std::abs(c[0]), 5));

    return commaSeparated(c, 7);
}

inline bool toColorNonNegative(char const* name, Color& result) {
    if (not *name)
        return false;

    auto i = colorNames().find(name);
    if (i != colorNames().end()) {
        result = toColor(i->second);
        return true;
    }

    static const auto hexPrefixes = {"0x", "0X", "#"};
    for (auto& prefix : hexPrefixes) {
        if (strstr(name, prefix) == name) {
            result = toColor(fromHex(name + strlen(prefix)));
            return true;
        }
    }

    char* endptr;

    // Special case for grey and gray.
    if (not (strstr(name, "gray ") and strstr(name, "grey "))) {
        auto gray = static_cast<float>(strtod(name + 5, &endptr)) / 100;
        if (not *endptr) {
            result = {{gray, gray, gray}};
            return true;
        }
        return false;
    }

    auto getNumber = [&]() {
        auto x = strtod(name, &endptr);
        name = endptr;

        skipSpaces(name);
        return static_cast<float>(x);
    };

    result[0] = getNumber(); // RGB::red
    if (*name++ != ',')
        return false;
    skipSpaces(name);

    result[1] = getNumber();
    if (*name++ != ',')
        return false;
    skipSpaces(name);

    result[2] = getNumber();
    if (*name)
        return false;
    return true;
}

inline bool toColor(char const* name, Color& result) {
    auto isSign = [](char ch) { return ch == '-' or ch == '+'; };
    auto len = strlen(name), end = len;
    for (; end > 0 && isSign(name[end - 1]); --end);

    if (end == len)
        return toColorNonNegative(name, result);

    if ((not end) or len - end != 3)
        return false;

    std::string n(name, len - 3);
    if (not toColorNonNegative(n.c_str(), result))
        return false;

    for (auto i = 0; i < 3; ++i) {
        if (name[len - 3 + i] == '-')
            result[i] = - result[i];
    }
    return true;
}

inline Color colorFromCommaSeparated(char const* p) {
    auto originalP = p;
    auto getNumber = [&]() {
        auto x = strtof(p, &p);
        skipSpaces(p);
        return static_cast<float>(x);
    };

    auto skipComma = [&]() {
        THROW_IF_NE(*p++, ',', "Expected a comma", originalP);
        skipSpaces(p);
    };

    auto r = getNumber();
    skipComma();

    auto g = getNumber();
    skipComma();

    auto b = getNumber();
    THROW_IF(*p, "Extra characters after end", originalP);

    return {{r, g, b}};
}

inline Color toColor(char const* name) {
    Color result;
    THROW_IF(not toColor(name, result), "Bad color name", name);
    return result;
}

inline ColorNamesInverse const& colorNamesInverse() {
    auto construct = []() {
        ColorNamesInverse inverse;
        for (auto& i: colorNames()) {
            auto& name = i.first;
            auto& hex = i.second;
            if (secondaryColors().count(name))
                continue;
            THROW_IF(inverse.count(hex), "Duplicate name", name, hex);
            inverse[hex] = name;
        }
        return inverse;
    };

    auto static const inverse = construct();
    return inverse;
}

inline std::set<std::string> const& secondaryColors() {
    static const std::set<std::string> colors{
        "aqua",
        "aquamarine 1",
        "aquamarine 3",
        "azure 1",
        "bisque 1",
        "blue 3",
        "blue 4",
        "chartreuse 1",
        "chocolate 4",
        "corn silk 1",
        "cyan 4",
        "dark olivegreen",
        "deep pink 1",
        "deep sky blue 1",
        "dodger blue 1",
        "fuchsia",
        "gold 1",
        "grey",
        "green 1",
        "honeydew 1",
        "ivory 1",
        "lavender blush 1",
        "lemon chiffon 1",
        "light cyan 1",
        "light salmon 1",
        "light yellow 1",
        "lime",
        "limegreen",
        "magenta 4",
        "medium seagreen",
        "medium slateblue",
        "medium violetred",
        "misty rose 1",
        "navajo white 1",
        "none",
        "olive drab 3",
        "orange 1",
        "orange red 1",
        "pale green 2",
        "peachpuff 1",
        "red 1",
        "red 4",
        "sea green 4",
        "seashell 1",
        "snow 1",
        "tan 3",
        "tomato 1",
        "yellow 1"
    };
    return colors;
}

inline ColorNames const& colorNames() {
    static const ColorNames names{{
        {"alice blue", 0xf0f8ff},
        {"antique white 1", 0xffefdb},
        {"antique white 2", 0xeedfcc},
        {"antique white 3", 0xcdc0b0},
        {"antique white 4", 0x8b8378},
        {"antique white", 0xfaebd7},
        {"aqua", 0x00ffff},
        {"aquamarine 1", 0x7fffd4},
        {"aquamarine 2", 0x76eec6},
        {"aquamarine 3", 0x66cdaa},
        {"aquamarine 4", 0x458b74},
        {"aquamarine", 0x7fffd4},
        {"azure 1", 0xf0ffff},
        {"azure 2", 0xe0eeee},
        {"azure 3", 0xc1cdcd},
        {"azure 4", 0x838b8b},
        {"azure", 0xf0ffff},
        {"banana", 0xe3cf57},
        {"beige", 0xf5f5dc},
        {"bisque 1", 0xffe4c4},
        {"bisque 2", 0xeed5b7},
        {"bisque 3", 0xcdb79e},
        {"bisque 4", 0x8b7d6b},
        {"bisque", 0xffe4c4},
        {"black", 0x000000},
        {"blanched almond", 0xffebcd},
        {"blue 2", 0x0000ee},
        {"blue 3", 0x0000cd},
        {"blue 4", 0x00008b},
        {"blue violet", 0x8a2be2},
        {"blue", 0x0000ff},
        {"brick", 0x9c661f},
        {"brown 1", 0xff4040},
        {"brown 2", 0xee3b3b},
        {"brown 3", 0xcd3333},
        {"brown 4", 0x8b2323},
        {"brown", 0xa52a2a},
        {"burly wood 1", 0xffd39b},
        {"burly wood 2", 0xeec591},
        {"burly wood 3", 0xcdaa7d},
        {"burly wood 4", 0x8b7355},
        {"burly wood", 0xdeb887},
        {"burnt sienna", 0x8a360f},
        {"burnt umber", 0x8a3324},
        {"cadet blue 1", 0x98f5ff},
        {"cadet blue 2", 0x8ee5ee},
        {"cadet blue 3", 0x7ac5cd},
        {"cadet blue 4", 0x53868b},
        {"cadet blue", 0x5f9ea0},
        {"cadmium orange", 0xff6103},
        {"cadmium yellow", 0xff9912},
        {"carrot", 0xed9121},
        {"chartreuse 1", 0x7fff00},
        {"chartreuse 2", 0x76ee00},
        {"chartreuse 3", 0x66cd00},
        {"chartreuse 4", 0x458b00},
        {"chartreuse", 0x7fff00},
        {"chocolate 1", 0xff7f24},
        {"chocolate 2", 0xee7621},
        {"chocolate 3", 0xcd661d},
        {"chocolate 4", 0x8b4513},
        {"chocolate", 0xd2691e},
        {"cobalt green", 0x3d9140},
        {"cobalt", 0x3d59ab},
        {"cold grey", 0x808a87},
        {"coral 1", 0xff7256},
        {"coral 2", 0xee6a50},
        {"coral 3", 0xcd5b45},
        {"coral 4", 0x8b3e2f},
        {"coral", 0xff7f50},
        {"corn silk 1", 0xfff8dc},
        {"corn silk 2", 0xeee8cd},
        {"corn silk 3", 0xcdc8b1},
        {"corn silk 4", 0x8b8878},
        {"corn silk", 0xfff8dc},
        {"cornflower blue", 0x6495ed},
        {"crimson", 0xdc143c},
        {"cyan 2", 0x00eeee},
        {"cyan 3", 0x00cdcd},
        {"cyan 4", 0x008b8b},
        {"cyan", 0x00ffff},
        {"dark blue", 0x00008b},
        {"dark cyan", 0x008b8b},
        {"dark goldenrod 1", 0xffb90f},
        {"dark goldenrod 2", 0xeead0e},
        {"dark goldenrod 3", 0xcd950c},
        {"dark goldenrod 4", 0x8b6508},
        {"dark goldenrod", 0xb8860b},
        {"dark green", 0x006400},
        {"dark grey", 0x555555},
        {"dark khaki", 0xbdb76b},
        {"dark magenta", 0x8b008b},
        {"dark olive green", 0x556b2f},
        {"dark olivegreen 1", 0xcaff70},
        {"dark olivegreen 2", 0xbcee68},
        {"dark olivegreen 3", 0xa2cd5a},
        {"dark olivegreen 4", 0x6e8b3d},
        {"dark olivegreen", 0x556b2f},
        {"dark orange 1", 0xff7f00},
        {"dark orange 2", 0xee7600},
        {"dark orange 3", 0xcd6600},
        {"dark orange 4", 0x8b4500},
        {"dark orange", 0xff8c00},
        {"dark orchid 1", 0xbf3eff},
        {"dark orchid 2", 0xb23aee},
        {"dark orchid 3", 0x9a32cd},
        {"dark orchid 4", 0x68228b},
        {"dark orchid", 0x9932cc},
        {"dark red", 0x8b0000},
        {"dark salmon", 0xe9967a},
        {"dark sea green 1", 0xc1ffc1},
        {"dark sea green 2", 0xb4eeb4},
        {"dark sea green 3", 0x9bcd9b},
        {"dark sea green 4", 0x698b69},
        {"dark sea green", 0x8fbc8f},
        {"dark slate blue", 0x483d8b},
        {"dark slate grey 1", 0x97ffff},
        {"dark slate grey 2", 0x8deeee},
        {"dark slate grey 3", 0x79cdcd},
        {"dark slate grey 4", 0x528b8b},
        {"dark slate grey", 0x2f4f4f},
        {"dark turquoise", 0x00ced1},
        {"dark violet", 0x9400d3},
        {"deep pink 1", 0xff1493},
        {"deep pink 2", 0xee1289},
        {"deep pink 3", 0xcd1076},
        {"deep pink 4", 0x8b0a50},
        {"deep pink", 0xff1493},
        {"deep sky blue 1", 0x00bfff},
        {"deep sky blue 2", 0x00b2ee},
        {"deep sky blue 3", 0x009acd},
        {"deep sky blue 4", 0x00688b},
        {"deep sky blue", 0x00bfff},
        {"dim grey", 0x696969},
        {"dodger blue 1", 0x1e90ff},
        {"dodger blue 2", 0x1c86ee},
        {"dodger blue 3", 0x1874cd},
        {"dodger blue 4", 0x104e8b},
        {"dodger blue", 0x1e90ff},
        {"eggshell", 0xfce6c9},
        {"emerald green", 0x00c957},
        {"fire brick 1", 0xff3030},
        {"fire brick 2", 0xee2c2c},
        {"fire brick 3", 0xcd2626},
        {"fire brick 4", 0x8b1a1a},
        {"fire brick", 0xb22222},
        {"flesh", 0xff7d40},
        {"floral white", 0xfffaf0},
        {"forest green", 0x228b22},
        {"fuchsia", 0xff00ff},
        {"gainsboro", 0xdcdcdc},
        {"ghost white", 0xf8f8ff},
        {"gold 1", 0xffd700},
        {"gold 2", 0xeec900},
        {"gold 3", 0xcdad00},
        {"gold 4", 0x8b7500},
        {"gold", 0xffd700},
        {"goldenrod 1", 0xffc125},
        {"goldenrod 2", 0xeeb422},
        {"goldenrod 3", 0xcd9b1d},
        {"goldenrod 4", 0x8b6914},
        {"goldenrod", 0xdaa520},
        {"gray", 0x808080},
        {"green 1", 0x00ff00},
        {"green 2", 0x00ee00},
        {"green 3", 0x00cd00},
        {"green 4", 0x008b00},
        {"green yellow", 0xadff2f},
        {"green", 0x00ff00},
        {"grey", 0x808080},
        {"honeydew 1", 0xf0fff0},
        {"honeydew 2", 0xe0eee0},
        {"honeydew 3", 0xc1cdc1},
        {"honeydew 4", 0x838b83},
        {"honeydew", 0xf0fff0},
        {"hot pink 1", 0xff6eb4},
        {"hot pink 2", 0xee6aa7},
        {"hot pink 3", 0xcd6090},
        {"hot pink 4", 0x8b3a62},
        {"hot pink", 0xff69b4},
        {"indian red 1", 0xff6a6a},
        {"indian red 2", 0xee6363},
        {"indian red 3", 0xcd5555},
        {"indian red 4", 0x8b3a3a},
        {"indian red", 0xcd5c5c},
        {"indigo", 0x4b0082},
        {"ivory 1", 0xfffff0},
        {"ivory 2", 0xeeeee0},
        {"ivory 3", 0xcdcdc1},
        {"ivory 4", 0x8b8b83},
        {"ivory black", 0x292421},
        {"ivory", 0xfffff0},
        {"khaki 1", 0xfff68f},
        {"khaki 2", 0xeee685},
        {"khaki 3", 0xcdc673},
        {"khaki 4", 0x8b864e},
        {"khaki", 0xf0e68c},
        {"lavender blush 1", 0xfff0f5},
        {"lavender blush 2", 0xeee0e5},
        {"lavender blush 3", 0xcdc1c5},
        {"lavender blush 4", 0x8b8386},
        {"lavender blush", 0xfff0f5},
        {"lavender", 0xe6e6fa},
        {"lawn green", 0x7cfc00},
        {"lemon chiffon 1", 0xfffacd},
        {"lemon chiffon 2", 0xeee9bf},
        {"lemon chiffon 3", 0xcdc9a5},
        {"lemon chiffon 4", 0x8b8970},
        {"lemon chiffon", 0xfffacd},
        {"light blue 1", 0xbfefff},
        {"light blue 2", 0xb2dfee},
        {"light blue 3", 0x9ac0cd},
        {"light blue 4", 0x68838b},
        {"light blue", 0xadd8e6},
        {"light coral", 0xf08080},
        {"light cyan 1", 0xe0ffff},
        {"light cyan 2", 0xd1eeee},
        {"light cyan 3", 0xb4cdcd},
        {"light cyan 4", 0x7a8b8b},
        {"light cyan", 0xe0ffff},
        {"light goldenrod 1", 0xffec8b},
        {"light goldenrod 2", 0xeedc82},
        {"light goldenrod 3", 0xcdbe70},
        {"light goldenrod 4", 0x8b814c},
        {"light goldenrod yellow", 0xfafad2},
        {"light green", 0x90ee90},
        {"light grey", 0xd3d3d3},
        {"light pink 1", 0xffaeb9},
        {"light pink 2", 0xeea2ad},
        {"light pink 3", 0xcd8c95},
        {"light pink 4", 0x8b5f65},
        {"light pink", 0xffb6c1},
        {"light salmon 1", 0xffa07a},
        {"light salmon 2", 0xee9572},
        {"light salmon 3", 0xcd8162},
        {"light salmon 4", 0x8b5742},
        {"light salmon", 0xffa07a},
        {"light sea green", 0x20b2aa},
        {"light sky blue 1", 0xb0e2ff},
        {"light sky blue 2", 0xa4d3ee},
        {"light sky blue 3", 0x8db6cd},
        {"light sky blue 4", 0x607b8b},
        {"light sky blue", 0x87cefa},
        {"light slate blue", 0x8470ff},
        {"light slate grey", 0x778899},
        {"light steel blue 1", 0xcae1ff},
        {"light steel blue 2", 0xbcd2ee},
        {"light steel blue 3", 0xa2b5cd},
        {"light steel blue 4", 0x6e7b8b},
        {"light steel blue", 0xb0c4de},
        {"light yellow 1", 0xffffe0},
        {"light yellow 2", 0xeeeed1},
        {"light yellow 3", 0xcdcdb4},
        {"light yellow 4", 0x8b8b7a},
        {"light yellow", 0xffffe0},
        {"lime green", 0x32cd32},
        {"lime", 0x00ff00},
        {"limegreen", 0x32cd32},
        {"linen", 0xfaf0e6},
        {"magenta 2", 0xee00ee},
        {"magenta 3", 0xcd00cd},
        {"magenta 4", 0x8b008b},
        {"magenta", 0xff00ff},
        {"manganese blue", 0x03a89e},
        {"maroon 1", 0xff34b3},
        {"maroon 2", 0xee30a7},
        {"maroon 3", 0xcd2990},
        {"maroon 4", 0x8b1c62},
        {"maroon", 0x800000},
        {"medium aquamarine", 0x66cdaa},
        {"medium blue", 0x0000cd},
        {"medium orchid 1", 0xe066ff},
        {"medium orchid 2", 0xd15fee},
        {"medium orchid 3", 0xb452cd},
        {"medium orchid 4", 0x7a378b},
        {"medium orchid", 0xba55d3},
        {"medium purple 1", 0xab82ff},
        {"medium purple 2", 0x9f79ee},
        {"medium purple 3", 0x8968cd},
        {"medium purple 4", 0x5d478b},
        {"medium purple", 0x9370db},
        {"medium sea green", 0x3cb371},
        {"medium seagreen", 0x3cb371},
        {"medium slate blue", 0x7b68ee},
        {"medium slateblue", 0x7b68ee},
        {"medium spring green", 0x00fa9a},
        {"medium turquoise", 0x48d1cc},
        {"medium violet red", 0xc71585},
        {"medium violetred", 0xc71585},
        {"melon", 0xe3a869},
        {"midnight blue", 0x191970},
        {"mint cream", 0xf5fffa},
        {"mint", 0xbdfcc9},
        {"misty rose 1", 0xffe4e1},
        {"misty rose 2", 0xeed5d2},
        {"misty rose 3", 0xcdb7b5},
        {"misty rose 4", 0x8b7d7b},
        {"misty rose", 0xffe4e1},
        {"moccasin", 0xffe4b5},
        {"navajo white 1", 0xffdead},
        {"navajo white 2", 0xeecfa1},
        {"navajo white 3", 0xcdb38b},
        {"navajo white 4", 0x8b795e},
        {"navajo white", 0xffdead},
        {"navy", 0x000080},
        {"none", 0x000000},
        {"old lace", 0xfdf5e6},
        {"olive drab 1", 0xc0ff3e},
        {"olive drab 2", 0xb3ee3a},
        {"olive drab 3", 0x9acd32},
        {"olive drab 4", 0x698b22},
        {"olive drab", 0x6b8e23},
        {"olive", 0x808000},
        {"orange 1", 0xffa500},
        {"orange 2", 0xee9a00},
        {"orange 3", 0xcd8500},
        {"orange 4", 0x8b5a00},
        {"orange red 1", 0xff4500},
        {"orange red 2", 0xee4000},
        {"orange red 3", 0xcd3700},
        {"orange red 4", 0x8b2500},
        {"orange red", 0xff4500},
        {"orange", 0xffa500},
        {"orchid 1", 0xff83fa},
        {"orchid 2", 0xee7ae9},
        {"orchid 3", 0xcd69c9},
        {"orchid 4", 0x8b4789},
        {"orchid", 0xda70d6},
        {"pale goldenrod", 0xeee8aa},
        {"pale green 1", 0x9aff9a},
        {"pale green 2", 0x90ee90},
        {"pale green 3", 0x7ccd7c},
        {"pale green 4", 0x548b54},
        {"pale green", 0x98fb98},
        {"pale turquoise 1", 0xbbffff},
        {"pale turquoise 2", 0xaeeeee},
        {"pale turquoise 3", 0x96cdcd},
        {"pale turquoise 4", 0x668b8b},
        {"pale turquoise", 0xafeeee},
        {"pale violet red 1", 0xff82ab},
        {"pale violet red 2", 0xee799f},
        {"pale violet red 3", 0xcd6889},
        {"pale violet red 4", 0x8b475d},
        {"pale violet red", 0xdb7093},
        {"papaya whip", 0xffefd5},
        {"peachpuff 1", 0xffdab9},
        {"peachpuff 2", 0xeecbad},
        {"peachpuff 3", 0xcdaf95},
        {"peachpuff 4", 0x8b7765},
        {"peachpuff", 0xffdab9},
        {"peacock", 0x33a1c9},
        {"peru", 0xcd853f},
        {"pink 1", 0xffb5c5},
        {"pink 2", 0xeea9b8},
        {"pink 3", 0xcd919e},
        {"pink 4", 0x8b636c},
        {"pink", 0xffc0cb},
        {"plum 1", 0xffbbff},
        {"plum 2", 0xeeaeee},
        {"plum 3", 0xcd96cd},
        {"plum 4", 0x8b668b},
        {"plum", 0xdda0dd},
        {"powder blue", 0xb0e0e6},
        {"purple 1", 0x9b30ff},
        {"purple 2", 0x912cee},
        {"purple 3", 0x7d26cd},
        {"purple 4", 0x551a8b},
        {"purple", 0x800080},
        {"raspberry", 0x872657},
        {"raw sienna", 0xc76114},
        {"red 1", 0xff0000},
        {"red 2", 0xee0000},
        {"red 3", 0xcd0000},
        {"red 4", 0x8b0000},
        {"red", 0xff0000},
        {"rosy brown 1", 0xffc1c1},
        {"rosy brown 2", 0xeeb4b4},
        {"rosy brown 3", 0xcd9b9b},
        {"rosy brown 4", 0x8b6969},
        {"rosy brown", 0xbc8f8f},
        {"royal blue 1", 0x4876ff},
        {"royal blue 2", 0x436eee},
        {"royal blue 3", 0x3a5fcd},
        {"royal blue 4", 0x27408b},
        {"royal blue", 0x4169e1},
        {"saddle brown", 0x8b4513},
        {"salmon 1", 0xff8c69},
        {"salmon 2", 0xee8262},
        {"salmon 3", 0xcd7054},
        {"salmon 4", 0x8b4c39},
        {"salmon", 0xfa8072},
        {"sandy brown", 0xf4a460},
        {"sap green", 0x308014},
        {"sea green 1", 0x54ff9f},
        {"sea green 2", 0x4eee94},
        {"sea green 3", 0x43cd80},
        {"sea green 4", 0x2e8b57},
        {"sea green", 0x2e8b57},
        {"seashell 1", 0xfff5ee},
        {"seashell 2", 0xeee5de},
        {"seashell 3", 0xcdc5bf},
        {"seashell 4", 0x8b8682},
        {"seashell", 0xfff5ee},
        {"sepia", 0x5e2612},
        {"sienna 1", 0xff8247},
        {"sienna 2", 0xee7942},
        {"sienna 3", 0xcd6839},
        {"sienna 4", 0x8b4726},
        {"sienna", 0xa0522d},
        {"silver", 0xc0c0c0},
        {"sky blue 1", 0x87ceff},
        {"sky blue 2", 0x7ec0ee},
        {"sky blue 3", 0x6ca6cd},
        {"sky blue 4", 0x4a708b},
        {"sky blue", 0x87ceeb},
        {"slate blue 1", 0x836fff},
        {"slate blue 2", 0x7a67ee},
        {"slate blue 3", 0x6959cd},
        {"slate blue 4", 0x473c8b},
        {"slate blue", 0x6a5acd},
        {"slate grey 1", 0xc6e2ff},
        {"slate grey 2", 0xb9d3ee},
        {"slate grey 3", 0x9fb6cd},
        {"slate grey 4", 0x6c7b8b},
        {"slate grey", 0x708090},
        {"snow 1", 0xfffafa},
        {"snow 2", 0xeee9e9},
        {"snow 3", 0xcdc9c9},
        {"snow 4", 0x8b8989},
        {"snow", 0xfffafa},
        {"spring green 1", 0x00ee76},
        {"spring green 2", 0x00cd66},
        {"spring green 3", 0x008b45},
        {"spring green", 0x00ff7f},
        {"steel blue 1", 0x63b8ff},
        {"steel blue 2", 0x5cacee},
        {"steel blue 3", 0x4f94cd},
        {"steel blue 4", 0x36648b},
        {"steel blue", 0x4682b4},
        {"tan 1", 0xffa54f},
        {"tan 2", 0xee9a49},
        {"tan 3", 0xcd853f},
        {"tan 4", 0x8b5a2b},
        {"tan", 0xd2b48c},
        {"teal", 0x008080},
        {"thistle 1", 0xffe1ff},
        {"thistle 2", 0xeed2ee},
        {"thistle 3", 0xcdb5cd},
        {"thistle 4", 0x8b7b8b},
        {"thistle", 0xd8bfd8},
        {"tomato 1", 0xff6347},
        {"tomato 2", 0xee5c42},
        {"tomato 3", 0xcd4f39},
        {"tomato 4", 0x8b3626},
        {"tomato", 0xff6347},
        {"turquoise 1", 0x00f5ff},
        {"turquoise 2", 0x00e5ee},
        {"turquoise 3", 0x00c5cd},
        {"turquoise 4", 0x00868b},
        {"turquoise blue", 0x00c78c},
        {"turquoise", 0x40e0d0},
        {"violet red 1", 0xff3e96},
        {"violet red 2", 0xee3a8c},
        {"violet red 3", 0xcd3278},
        {"violet red 4", 0x8b2252},
        {"violet red", 0xd02090},
        {"violet", 0xee82ee},
        {"warm grey", 0x808069},
        {"wheat 1", 0xffe7ba},
        {"wheat 2", 0xeed8ae},
        {"wheat 3", 0xcdba96},
        {"wheat 4", 0x8b7e66},
        {"wheat", 0xf5deb3},
        {"white smoke", 0xf5f5f5},
        {"white", 0xffffff},
        {"yellow 1", 0xffff00},
        {"yellow 2", 0xeeee00},
        {"yellow 3", 0xcdcd00},
        {"yellow 4", 0x8b8b00},
        {"yellow green", 0x9acd32},
        {"yellow", 0xffff00}
    }};
    return names;
}

}  // tdsp
