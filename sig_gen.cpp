#include "sig_gen.h"
using namespace std;
static constexpr double PI = 3.14159265358979323846;
double SineSignal::sample(double tSec) const
{
    const double w = 2.0 * PI * m_freq;
    return m_bias + m_amp * sin(w * tSec + m_phase);
}
double SquareSignal::sample(double tSec) const
{
    const double w = 2.0 * PI * m_freq;
    const double s = sin(w * tSec);
    const double sign = (s >= 0.0) ? 1.0 : -1.0;
    return m_bias + m_amp * sign;
}
double SawtoothSignal::sample(double tSec) const
{
    const double x = m_freq * tSec;
    const double frac = x - floor(x);
    const double saw = 2.0 * frac - 1.0;
    return m_bias + m_amp * saw;
}
