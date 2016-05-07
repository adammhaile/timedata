#pragma once

#include <tdsp/color/colorList.h>
#include <tdsp/base/make.h>
#include <tdsp/base/math_inl.h>
#include <tdsp/color/names_inl.h>
#include <tdsp/color/slice.h>

namespace tdsp {

template <typename T>
std::vector<T> duplicate(std::vector<T> const& v, size_t count) {
    std::vector<T> result;
    for (auto i = 0; i < count; ++i)
        result.insert(result.end(), v.begin(), v.end());
    return result;
}

template <typename T>
void reverse(std::vector<T>& v) {
    std::reverse(v.begin(), v.end());
}

inline std::string toString(ColorList const& colors) {
    std::string result = "(";
    for (auto& c : colors) {
        if (result.size() > 1)
            result += ", ";
        auto s = toString(c);
        result += "'("[isdigit(s[0])];
        result += s;
        result += "')"[isdigit(s[0])];
    }
    result += ")";
    return result;
}

template <typename T>
std::vector<T> sliceVector(
        std::vector<T> const& in, int begin, int end, int step) {
    auto slice = make<Slice>(begin, end, step);
    std::vector<T> out;
    forEach(slice, [&](int j) { out.push_back(in[j]); });
    return out;
}

template <typename T>
bool sliceIntoVector(std::vector<T> const& in, std::vector<T>& out,
                     int begin, int end, int step) {
    auto slice = make<Slice>(begin, end, step);

    if (slice.size() == in.size()) {
        auto i = in.begin();
        forEach(slice, [&](int j) { out[j] = *(i++); });
        return true;
    }

    if (step != 1)
        return false;

    auto beginOut = out.begin() + begin;
    if (slice.size() > in.size()) {
        // Shrink!  Copy the input, then erase the remains.
        std::copy(in.begin(), in.end(), beginOut);
        out.erase(beginOut + in.size(), beginOut + slice.size());
    } else {
        // Grow!  Copy the first segment, then insert the second.
        auto split = in.begin() + slice.size();
        std::copy(in.begin(), split, beginOut);
        out.insert(beginOut + slice.begin, split, in.end());
    }

    return true;
}

template <typename Function>
void forEachColorComponent(ColorList& colors, Function f) {
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
    forEachColorComponent(out, [](float& x) { x = 1.0 - x; });
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