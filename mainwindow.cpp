#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
#include "sig_gen.h"
#include <QComboBox>
#include <QDial>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton>
#include <cmath>
using namespace std;
static double degToRad(double deg)
{
    return deg * 3.14159265358979323846 / 180.0;
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    stPlot();
    stUiControls();
    m_signal.reset(new SineSignal());
    apParamsToSig();
    rsPlotData();
    m_timer.setTimerType(Qt::PreciseTimer);
    connect(&m_timer, &QTimer::timeout, this, &MainWindow::onTick);
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::stPlot()
{
    ui->customPlot->addGraph();
    ui->customPlot->graph(0)->setPen(QPen(Qt::green, 2));
    ui->customPlot->xAxis->setLabel("t [s]");
    ui->customPlot->yAxis->setLabel("y");
    ui->customPlot->xAxis->setRange(0, m_windowSec);
    ui->customPlot->yAxis->setRange(-2, 2);
    ui->customPlot->setInteraction(QCP::iRangeDrag, false);
    ui->customPlot->setInteraction(QCP::iRangeZoom, false);
}

void MainWindow::stUiControls()
{
    auto* root = new QWidget(ui->centralwidget);
    auto* h = new QHBoxLayout(root);
    h->setContentsMargins(8, 8, 8, 8);
    h->setSpacing(10);
    auto* panelBox = new QGroupBox("Sterowanie", root);
    panelBox->setMinimumWidth(260);
    auto* panel = new QVBoxLayout(panelBox);
    panel->addWidget(new QLabel("Typ sygnału:", panelBox));
    m_waveformCombo = new QComboBox(panelBox);
    m_waveformCombo->addItem("Sinusoidalny");
    m_waveformCombo->addItem("Prostokątny");
    m_waveformCombo->addItem("Piłokształtny");
    panel->addWidget(m_waveformCombo);
    connect(m_waveformCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onWvfChgd);

    auto makeDialBlock = [&](const QString& title, int minV, int maxV, int value, QDial*& dialOut, QLabel*& labelOut) {
        auto* box = new QGroupBox(title, panelBox);
        auto* v = new QVBoxLayout(box);
        dialOut = new QDial(box);
        dialOut->setRange(minV, maxV);
        dialOut->setNotchesVisible(true);
        dialOut->setValue(value);
        labelOut = new QLabel(box);
        labelOut->setAlignment(Qt::AlignCenter);
        v->addWidget(dialOut, 0, Qt::AlignCenter);
        v->addWidget(labelOut);
        panel->addWidget(box);
    };
    makeDialBlock("Amplituda (A)", 0, 50, 10, m_dialAmp, m_lblAmp);
    connect(m_dialAmp, &QDial::valueChanged, this, &MainWindow::onAmpChgd);
    makeDialBlock("Bias (B)", 0, 100, 50, m_dialBias, m_lblBias);
    connect(m_dialBias, &QDial::valueChanged, this, &MainWindow::onBsChgd);
    makeDialBlock("Częstotliwość (5–10 Hz)", 50, 100, 50, m_dialFreq, m_lblFreq);
    connect(m_dialFreq, &QDial::valueChanged, this, &MainWindow::onFqChgd);
    makeDialBlock("Faza (φ) [°] (tylko sinus)", 0, 360, 0, m_dialPhase, m_lblPhase);
    connect(m_dialPhase, &QDial::valueChanged, this, &MainWindow::onPsChgd);
    auto* btnRow = new QHBoxLayout();
    m_btnStart = new QPushButton("Start", panelBox);
    m_btnStop  = new QPushButton("Stop", panelBox);
    btnRow->addWidget(m_btnStart);
    btnRow->addWidget(m_btnStop);
    panel->addLayout(btnRow);
    connect(m_btnStart, &QPushButton::clicked, this, &MainWindow::onStart);
    connect(m_btnStop,  &QPushButton::clicked, this, &MainWindow::onStop);
    panel->addStretch(1);
    ui->customPlot->setParent(root);
    h->addWidget(panelBox, 0);
    h->addWidget(ui->customPlot, 1);
    auto* cwLayout = new QVBoxLayout(ui->centralwidget);
    cwLayout->setContentsMargins(0, 0, 0, 0);
    cwLayout->addWidget(root);
    updLbl();
}

double MainWindow::mapAmp(int v) const
{
    return v / 10.0; // 0..5.0
}

double MainWindow::mapBias(int v) const
{
    return (v - 50) / 10.0; // -5..+5
}

double MainWindow::mapFreq(int v) const
{
    return v / 10.0; // 50..100 => 5..10 Hz
}

double MainWindow::mapPhaseDeg(int v) const
{
    return static_cast<double>(v);
}

void MainWindow::apParamsToSig()
{
    if (!m_signal){
        return;
    }
    m_amp = mapAmp(m_dialAmp->value());
    m_bias = mapBias(m_dialBias->value());
    m_freq = mapFreq(m_dialFreq->value());
    m_phaseRad = degToRad(mapPhaseDeg(m_dialPhase->value()));
    m_signal->setAmplitude(m_amp);
    m_signal->setBias(m_bias);
    m_signal->setFrequency(m_freq);
    if (auto* s = dynamic_cast<SineSignal*>(m_signal.get())) {
        s->setPhaseRad(m_phaseRad);
    }
}

void MainWindow::updLbl()
{
    m_lblAmp->setText(QString("A = %1").arg(mapAmp(m_dialAmp->value()), 0, 'f', 2));
    m_lblBias->setText(QString("B = %1").arg(mapBias(m_dialBias->value()), 0, 'f', 2));
    m_lblFreq->setText(QString("f = %1 Hz").arg(mapFreq(m_dialFreq->value()), 0, 'f', 2));
    m_lblPhase->setText(QString("φ = %1°").arg(m_dialPhase->value()));
}

void MainWindow::rsPlotData()
{
    ui->customPlot->graph(0)->data()->clear();
    ui->customPlot->xAxis->setRange(0, m_windowSec);
    ui->customPlot->replot();
}

void MainWindow::onWvfChgd(int idx)
{
    switch (idx) {
    case 0: m_signal.reset(new SineSignal()); break;
    case 1: m_signal.reset(new SquareSignal()); break;
    case 2: m_signal.reset(new SawtoothSignal()); break;
    default: m_signal.reset(new SineSignal()); break;
    }
    const bool isSine = (idx == 0);
    m_dialPhase->setEnabled(isSine);

    apParamsToSig();
    rsPlotData();
    if (m_running) {
        m_elapsed.restart();
    }
}

void MainWindow::onAmpChgd(int)
{
    apParamsToSig();
    updLbl();
}

void MainWindow::onBsChgd(int)
{
    apParamsToSig();
    updLbl();
}

void MainWindow::onFqChgd(int)
{
    apParamsToSig();
    updLbl();
}

void MainWindow::onPsChgd(int)
{
    apParamsToSig();
    updLbl();
}

void MainWindow::onStart()
{
    if (m_running) return;
    m_running = true;
    m_elapsed.restart();
    const int intervalMs = std::max(1, static_cast<int>(m_dt * 1000.0));
    m_timer.start(intervalMs);
}

void MainWindow::onStop()
{
    if (!m_running) return;
    m_running = false;
    m_timer.stop();
}

void MainWindow::onTick()
{
    if (!m_signal){
        return;
    }
    const double t = m_elapsed.elapsed() / 1000.0;
    const double y = m_signal->sample(t);
    ui->customPlot->graph(0)->addData(t, y);
    const double ymin = (m_bias - std::abs(m_amp)) - 0.2;
    const double ymax = (m_bias + std::abs(m_amp)) + 0.2;
    ui->customPlot->yAxis->setRange(ymin, ymax);
    if (t < m_fillHalfSec) {
        ui->customPlot->xAxis->setRange(0.0, m_windowSec);
    }
    else {
        ui->customPlot->xAxis->setRange(t, m_windowSec, Qt::AlignRight);
    }
    ui->customPlot->replot(QCustomPlot::rpQueuedReplot);
}
