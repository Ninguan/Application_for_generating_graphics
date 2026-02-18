#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QElapsedTimer>
#include <QTimer>
#include <memory>
using namespace std;
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE
class QComboBox;
class QDial;
class QLabel;
class QPushButton;
class QCustomPlot;
class SignalGenerator;
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void onWvfChgd(int idx);
    void onAmpChgd(int v);
    void onBsChgd(int v);
    void onFqChgd(int v);
    void onPsChgd(int v);
    void onStart();
    void onStop();
    void onTick();
private:
    void stPlot();
    void stUiControls();
    void apParamsToSig();
    void updLbl();
    void rsPlotData();
    double mapAmp(int v) const;
    double mapBias(int v) const;
    double mapFreq(int v) const;
    double mapPhaseDeg(int v) const;
private:
    Ui::MainWindow *ui;
    QComboBox*m_waveformCombo = nullptr;
    QDial*m_dialAmp = nullptr;
    QDial*m_dialBias = nullptr;
    QDial*m_dialFreq = nullptr;
    QDial*m_dialPhase = nullptr;
    QLabel*m_lblAmp = nullptr;
    QLabel*m_lblBias = nullptr;
    QLabel*m_lblFreq = nullptr;
    QLabel*m_lblPhase = nullptr;
    QPushButton*m_btnStart = nullptr;
    QPushButton*m_btnStop  = nullptr;
    QTimer m_timer;
    QElapsedTimer m_elapsed;
    double m_dt = 0.002;
    bool m_running = false;
    const double m_windowSec = 2.0;
    const double m_fillHalfSec = m_windowSec / 2.0;
    unique_ptr<SignalGenerator> m_signal;
    double m_amp = 1.0;
    double m_bias = 0.0;
    double m_freq = 5.0;
    double m_phaseRad = 0.0;
};

#endif // MAINWINDOW_H
