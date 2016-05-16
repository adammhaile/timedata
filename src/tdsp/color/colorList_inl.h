#pragma once

#include <tdsp/color/colorList.h>
#include <tdsp/base/make.h>
#include <tdsp/base/math_inl.h>
#include <tdsp/color/names_inl.h>
#include <tdsp/signal/slice.h>

namespace tdsp {

inline
ColorList duplicate(ColorList const& v, size_t count) {
    ColorList result;
    for (size_t i = 0; i < count; ++i)
        result.insert(result.end(), v.begin(), v.end());
    return result;
}

inline
void reverse(ColorList& v) {
    std::reverse(v.begin(), v.end());
}

inline std::string toString(ColorList const& colors, Base base) {
    std::string result = "(";
    for (auto& c : colors) {
        if (result.size() > 1)
            result += ", ";
        auto s = colorToString(c, base);
        result += "'("[isdigit(s[0])];
        result += s;
        result += "')"[isdigit(s[0])];
    }
    result += ")";
    return result;
}

inline
ColorList sliceVector(
        ColorList const& in, int begin, int end, int step) {
    // TODO: is this used?
    auto slice = make<Slice>(begin, end, step);
    ColorList out;
    forEach(slice, [&](int j) { out.push_back(in[j]); });
    return out;
}

inline
bool sliceIntoVector(ColorList const& in, ColorList& out,
                     int begin, int end, int step) {
    auto slice = make<Slice>(begin, end, step);
    auto size = slice.size();

    if (in.size() == size) {
        auto i = in.begin();
        forEach(slice, [&](int j) { out[j] = *(i++); });
        return true;
    }

    if (step != 1)
        return false;

    auto ob = out.begin() + begin;
    if (in.size() < size) {
        // Shrink!  Copy the input, then erase the remains.
        std::copy(in.begin(), in.end(), ob);
        out.erase(ob + in.size(), ob + size);
    } else {
        // Grow!  Copy the first segment, then insert the second.
        std::copy(in.begin(), in.begin() + size, ob);
        out.insert(ob + slice.begin, in.begin() + size, in.end());
    }

    return true;
}

template <typename Function>
void forEachColorComponent(ColorList& colors, Function f) {
    // TODO: WRONG!
    for (auto& color : colors)
        for (auto& c : color)
            f(c);
}

template <typename Function>
void forEachColorComponent(ColorList const& in, ColorList& out, Function f) {
    if (out.size() < in.size())
        out.resize(in.size());

    for (size_t i = 0; i < in.size(); ++i)
        for (size_t j = 0; j < in[i].size(); ++j)
            f(in[i][j], out[i][j]);
}

inline void absColor(ColorList& out) {
    forEachColorComponent(out, [](float& x) { x = std::abs(x); });
}

inline void negateColor(ColorList& out) {
    forEachColorComponent(out, [](float& x) { x = -x; });
}

inline void invertColor(ColorList& out) {
    forEachColorComponent(out, [](float& x) { x = (x > 0 ? 1.0 : -1.0) - x; });
}

////////////////////////////////////////////////////////////////////////////////

inline void addInto(float f, ColorList& out) {
    forEachColorComponent(out, [=](float& x) { x += f; });
}
inline void addInto(ColorList const& in, ColorList& out) {
    forEachColorComponent(in, out, [](float i, float& o) { o += i; });
}

inline void divideInto(float f, ColorList& out) {
    forEachColorComponent(out, [=](float& x) { x /= f; });
}
inline void divideInto(ColorList const& in, ColorList& out) {
    forEachColorComponent(in, out, [](float i, float& o) { o /= i; });
}

inline void multiplyInto(float f, ColorList& out) {
    forEachColorComponent(out, [=](float& x) { x *= f; });
}
inline void multiplyInto(ColorList const& in, ColorList& out) {
    forEachColorComponent(in, out, [](float i, float& o) { o *= i; });
}

inline void powInto(float f, ColorList& out) {
    forEachColorComponent(out, [=](float& x) { x = pow(x, f); });
}
inline void powInto(ColorList const& in, ColorList& out) {
    forEachColorComponent(in, out, [](float i, float& o) { o = pow(o, i); });
}

inline void rdivideInto(float f, ColorList& out) {
    forEachColorComponent(out, [=](float& x) { x = f / x; });
}
inline void rdivideInto(ColorList const& in, ColorList& out) {
    // TODO: PROBLEM HERE!
    forEachColorComponent(in, out, [](float i, float& o) { o = i = o; });
}

inline void rpowInto(float f, ColorList& out) {
    forEachColorComponent(out, [=](float& x) { x = pow(f, x); });
}
inline void rpowInto(ColorList const& in, ColorList& out) {
    forEachColorComponent(in, out, [](float i, float& o) { o = pow(o, i); });
}

inline void rsubtractInto(float f, ColorList& out) {
    forEachColorComponent(out, [=](float& x) { x = f - x; });
}
inline void rsubtractInto(ColorList const& in, ColorList& out) {
    forEachColorComponent(in, out, [](float i, float& o) { o = i - o; });
}

inline void subtractInto(float f, ColorList& out) {
    forEachColorComponent(out, [=](float& x) { x -= f; });
}
inline void subtractInto(ColorList const& in, ColorList& out) {
    forEachColorComponent(in, out, [](float i, float& o) { o -= i; });
}

inline void minInto(float f, ColorList& out) {
    forEachColorComponent(out, [=](float& x) { x = std::min(f, x); });
}
inline void minInto(ColorList const& in, ColorList& out) {
    forEachColorComponent(in, out,
                          [](float i, float& o) { o = std::min(i, o); });
}

inline void maxInto(float f, ColorList& out) {
    forEachColorComponent(out, [=](float& x) { x = std::max(f, x); });
}
inline void maxInto(ColorList const& in, ColorList& out) {
    forEachColorComponent(in, out,
                          [](float i, float& o) { o = std::max(i, o); });
}

} // tdsp
