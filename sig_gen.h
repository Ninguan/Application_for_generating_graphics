#ifndef SIG_GEN_H
#define SIG_GEN_H
#include <cmath>
class SignalGenerator
{
public:
    virtual ~SignalGenerator() = default;
    void setAmplitude(double a){m_amp = a;}
    void setBias(double b){m_bias = b;}
    void setFrequency(double f){m_freq = f;}
    double amplitude() const{return m_amp;}
    double bias() const{return m_bias; }
    double frequency() const {return m_freq;}
    virtual double sample(double tSec) const = 0;
protected:
    double m_amp = 1.0;
    double m_bias = 0.0;
    double m_freq = 5.0;
};
class SineSignal : public SignalGenerator
{
public:
    void setPhaseRad(double p) { m_phase = p; }
    double phaseRad() const    { return m_phase; }
    double sample(double tSec) const override;
private:
    double m_phase = 0.0;
};
class SquareSignal : public SignalGenerator
{
public:
    double sample(double tSec) const override;
};
class SawtoothSignal : public SignalGenerator
{
public:
    double sample(double tSec) const override;
};
#endif //SIG_GEN_H
