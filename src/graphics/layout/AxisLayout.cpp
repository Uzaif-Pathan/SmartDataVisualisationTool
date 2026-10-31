#include "graphics/layout/AxisLayout.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace gfx {

AxisScale niceScale(double dataMin, double dataMax, int targetTicks, bool includeZero) {
    AxisScale s;
    double lo = dataMin;
    double hi = dataMax;

    if (!std::isfinite(lo) || !std::isfinite(hi)) {
        lo = 0.0;
        hi = 1.0;
        s.adjusted = true;
    }
    if (lo > hi) std::swap(lo, hi);

    if (includeZero) {
        lo = std::min(lo, 0.0);
        hi = std::max(hi, 0.0);
    }

    if (lo == hi) {                      // a single value: give the axis some width
        s.adjusted = true;
        if (lo == 0.0) {
            if (includeZero) {
                hi = 1.0;                // all zeros with a zero baseline: 0..1
            } else {
                lo = -1.0;
                hi = 1.0;
            }
        } else {
            const double delta = std::fabs(lo) * 0.1;
            lo -= delta;
            hi += delta;
        }
    }

    if (!(hi - lo > 0.0) || !std::isfinite(hi - lo)) {   // absurd range: start over
        lo = 0.0;
        hi = 1.0;
        s.adjusted = true;
    }

    targetTicks = std::max(2, std::min(50, targetTicks));

    // Step size: the raw step is rounded to 1, 2, 5 or 10 times a power of ten.
    const double rawStep = (hi - lo) / (targetTicks - 1);
    const int exponent = static_cast<int>(std::floor(std::log10(rawStep)));
    const double magnitude = std::pow(10.0, exponent);
    const double residual = rawStep / magnitude;
    double factor = 10.0;
    if (residual < 1.5) factor = 1.0;
    else if (residual < 3.0) factor = 2.0;
    else if (residual < 7.0) factor = 5.0;
    const double step = factor * magnitude;

    if (!std::isfinite(step) || !(step > 0.0)) {
        AxisScale fallback = niceScale(0.0, 1.0, targetTicks, false);
        fallback.adjusted = true;
        return fallback;
    }
    const int stepExponent = exponent + (factor == 10.0 ? 1 : 0);

    // The tiny 1e-9 tolerance stops rounding noise from adding a useless
    // extra tick (for example 0.3 / 0.1 = 2.9999999999999996).
    const double first = std::floor(lo / step + 1e-9);
    const double last = std::ceil(hi / step - 1e-9);
    long long count = std::llround(last - first);
    if (count < 1) count = 1;
    if (count > 1000) count = 1000;

    s.decimals = std::max(0, std::min(15, -stepExponent));
    const double p = std::pow(10.0, s.decimals);
    for (long long k = 0; k <= count; ++k) {
        double v = (first + static_cast<double>(k)) * step;
        if (s.decimals > 0) v = std::round(v * p) / p;   // remove 0.6000000000000001-style noise
        if (v == 0.0) v = 0.0;                            // turns -0.0 into 0.0
        s.ticks.push_back(v);
    }
    s.step = step;
    s.min = s.ticks.front();
    s.max = s.ticks.back();
    return s;
}

std::string formatTick(double value, int decimals) {
    if (!std::isfinite(value)) return std::string();
    decimals = std::max(0, std::min(15, decimals));

    const double a = std::fabs(value);
    char buf[64];
    if (a != 0.0 && (a >= 1e9 || a < 1e-4)) std::snprintf(buf, sizeof buf, "%.3g", value);
    else std::snprintf(buf, sizeof buf, "%.*f", decimals, value);

    std::string s(buf);
    if (!s.empty() && s[0] == '-') {          // "-0", "-0.0" ... -> "0", "0.0"
        bool allZero = true;
        for (std::size_t i = 1; i < s.size(); ++i) {
            if (s[i] != '0' && s[i] != '.') {
                allZero = false;
                break;
            }
        }
        if (allZero) s.erase(0, 1);
    }
    return s;
}

std::vector<std::string> tickLabels(const AxisScale& scale) {
    std::vector<std::string> labels;
    labels.reserve(scale.ticks.size());
    for (double t : scale.ticks) labels.push_back(formatTick(t, scale.decimals));
    return labels;
}

}  // namespace gfx
