#pragma once
#include <string>
#include <vector>

namespace gfx {

// A "nice" axis: round limits and evenly spaced round tick values.
struct AxisScale {
    double min = 0.0;             // first tick (<= data minimum)
    double max = 1.0;             // last tick  (>= data maximum)
    double step = 0.2;            // distance between ticks (1, 2 or 5 x a power of 10)
    int decimals = 1;             // decimal places needed to print the ticks
    std::vector<double> ticks;    // min, min+step, ..., max
    bool adjusted = false;        // true if the input was NaN/infinite/constant and had to be repaired
};

// Chooses axis limits and ticks for data spanning [dataMin, dataMax].
//  - targetTicks is the desired number of ticks (limited to 2..50); the real
//    number is usually close but not exact.
//  - includeZero forces 0 inside the axis.  Use it for bar charts and
//    histograms, where the bar length must start from zero.
// The data range is always fully inside [min, max].
// Never fails: NaN/infinite/constant input is repaired (adjusted = true).
AxisScale niceScale(double dataMin, double dataMax, int targetTicks = 5, bool includeZero = false);

// Text for one tick.  Uses `decimals` places normally, and scientific notation
// for very large (>= 1e9) or very small (< 1e-4) numbers.
// Never prints "-0".  Returns "" for NaN/infinite.
std::string formatTick(double value, int decimals);

// Labels for all ticks of a scale.
std::vector<std::string> tickLabels(const AxisScale& scale);

}  // namespace gfx
