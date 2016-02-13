#pragma once

#include <fcolor/Fcolor.h>
#include <fcolor/ColorName.h>
#include <fcolor/Math.h>

namespace fcolor {
namespace color {

class FColor {
  public:
    FColor() {}

    FColor(float red, float green, float blue, float alpha=1.0)
            : red_(red), green_(green), blue_(blue), alpha_(alpha) {
    }

    FColor(uint32 parts) {
        static const auto C = 0xFF * 1.0f;
        blue_ = (parts & 0xFF) / C;
        parts >>= 8;
        green_ = (parts & 0xFF) / C;
        parts >>= 8;
        red_ = (parts & 0xFF) / C;
        parts >>= 8;
        alpha_ = parts / C;
    }

    const float& red() const { return red_; }
    const float& green() const { return green_; }
    const float& blue() const { return blue_; }

    float& red() { return red_; }
    float& green() { return green_; }
    float& blue() { return blue_; }

    const float& alpha() const { return alpha_; }
    float& alpha() { return alpha_; }

    static uint8 floatToUInt8(const float n) {
        return static_cast<uint8>(std::max(0.0f, std::min(255.0f, n * 255.1f)));
    }

    uint32 argb() const {
        static const auto C = 0x100;
        uint32 r = floatToUInt8(alpha());
        r = C * r + floatToUInt8(red());
        r = C * r + floatToUInt8(green());
        r = C * r + floatToUInt8(blue());
        return r;
    }

    void clear() {
        red_ = green_ = blue_ = 0.0f;
        alpha_ = 1.0f;
    }

    static const FColor BLACK;

    void gamma(float f) {
        red_ = powf(red_, f);
        green_ = powf(green_, f);
        blue_ = powf(blue_, f);
    }

    void copy(const FColor& other) { *this = other; }
    void copy(const FColor* other) { copy(*other); }

    bool operator==(const FColor& other) const { return not compare(other); }
    bool operator!=(const FColor& other) const { return compare(other); }
    bool operator>(const FColor& other) const { return compare(other) > 0; }
    bool operator>=(const FColor& other) const { return compare(other) >= 0; }
    bool operator<(const FColor& other) const { return compare(other) < 0; }
    bool operator<=(const FColor& other) const { return compare(other) <= 0; }

    static float round(float x) {
        return roundf(1000.0 * x) / 1000.0;
    }

    static bool near(float x, float y) {
        return ::fcolor::near(round(x), round(y), 0.0005);
    }

    bool isGrey() const {
        return near(red(), green()) and
                near(green(), blue()) and
                near(blue(), red());
    }

    int compare(const FColor& x) const {
        if (this == &x)
            return 0;

        if (not near(red(), x.red())) {
            if (red() < x.red())
                return -1;
            if (red() > x.red())
                return 1;
        }
        if (not near(green(), x.green())) {
            if (green() < x.green())
                return -1;
            if (green() > x.green())
                return 1;
        }
        if (not near(blue(), x.blue())) {
            if (blue() < x.blue())
                return -1;
            if (blue() > x.blue())
                return 1;
        }
        if (alpha() < x.alpha())
            return -1;
        if (alpha() > x.alpha())
            return 1;

        return 0;
    }

    FColor interpolate(
        const FColor& end, float ratio, uint smooth = 0, uint index = 0) const {
        if (smooth) {
            LOG_FIRST_N(ERROR, 1) << "We don't support smooth interpolation yet. ";
        }
        return FColor(red_ + ratio * (end.red_ - red_),
                      green_ + ratio * (end.green_ - green_),
                      blue_ + ratio * (end.blue_ - blue_),
                      alpha_ + ratio * (end.alpha_ - alpha_));
    }

    void scale(float scale) {
        red_ *= scale;
        green_ *= scale;
        blue_ *= scale;
    }

    struct Comparer {
        bool operator()(const FColor& x, const FColor& y) {
            return x.compare(y) < 0;
        }
    };

    // Deprecated.
    void combine(FColor const& x) { return max(x); }

    void min(const FColor& x) {
        red_ = std::min(red_ * alpha_, x.red() * x.alpha());
        green_ = std::min(green_ * alpha_, x.green() * x.alpha());
        blue_ = std::min(blue_ * alpha_, x.blue() * x.alpha());
        alpha_ = 1.0f;
    }

    void max(const FColor& x) {
        red_ = std::max(red_ * alpha_, x.red() * x.alpha());
        green_ = std::max(green_ * alpha_, x.green() * x.alpha());
        blue_ = std::max(blue_ * alpha_, x.blue() * x.alpha());
        alpha_ = 1.0f;
    }

    void add(const FColor& x) {
        red_ = red_ * alpha_ + x.red() * x.alpha();
        green_ = green_ * alpha_ + x.green() * x.alpha();
        blue_ = blue_ * alpha_ + x.blue() * x.alpha();
        alpha_ = 1.0f;
    }

    void subtract(const FColor& x) {
        red_ = red_ * alpha_ - x.red() * x.alpha();
        green_ = green_ * alpha_ - x.green() * x.alpha();
        blue_ = blue_ * alpha_ - x.blue() * x.alpha();
        alpha_ = 1.0f;
    }

    void multiply(const FColor& x) {
        red_ = red_ * alpha_ * x.red() * x.alpha();
        green_ = green_ * alpha_ * x.green() * x.alpha();
        blue_ = blue_ * alpha_ * x.blue() * x.alpha();
        alpha_ = 1.0f;
    }

    void divide(const FColor& x) {
        red_ = red_ * alpha_ / (x.red() * x.alpha());
        green_ = green_ * alpha_ / (x.green() * x.alpha());
        blue_ = blue_ * alpha_ / (x.blue() * x.alpha());
        alpha_ = 1.0f;
    }

    void andC(const FColor& x) {
        *this = FColor(argb() & x.argb());
    }

    void orC(const FColor& x) {
        *this = FColor(argb() | x.argb());
    }

    void xorC(const FColor& x) {
        *this = FColor(argb() ^ x.argb());
    }

    void limit() {
        red_ = std::min(std::max(red_, 1.0f), 0.0f);
        green_ = std::min(std::max(green_, 1.0f), 0.0f);
        blue_ = std::min(std::max(blue_, 1.0f), 0.0f);
        alpha_ = std::min(std::max(alpha_, 1.0f), 0.0f);
    }

    std::string toString() const {
        return rgbToName(*this);
    }

  private:
    float red_ = 0.0;
    float green_ = 0.0;
    float blue_ = 0.0;
    float alpha_ = 1.0;
};

}  // namespace color
}  // namespace fcolor

inline std::ostream & operator<<(
    std::ostream &os, const fcolor::color::FColor& color) {
    return os << color.toString();
}
