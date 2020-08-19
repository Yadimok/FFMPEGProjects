#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>

#include "minirecorder.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private:
    Ui::MainWindow *ui;

    MiniRecorder *kMiniRecorder;

    QPointer<QPushButton> pbStartRecording;
    QPointer<QPushButton> pbStopRecording;
    QPointer<QVBoxLayout> vboxLayout;
    QPointer<QGroupBox> groupBox;
    QPointer<QHBoxLayout> hboxLayout;
    QPointer<QLabel> labelFileName;
    QPointer<QLineEdit> leFileName;
    QPointer<QLabel> labelWidth;
    QPointer<QLabel> labelHeight;
    QPointer<QLabel> labelFrameRate;
    QPointer<QLineEdit> leWidth;
    QPointer<QLineEdit> leHeight;
    QPointer<QLineEdit> leFrameRate;

    QPointer<QLabel> labelSaveFileName;
    QPointer<QLabel> labelRecordingTime;
    QPointer<QLabel> labelTotalBytes;
    QPointer<QLabel> labelCurrentDateTime;

    int mWidth, mHeight;

    enum {
        SIZE_SCREEN_WIDTH = 16383,
        SIZE_SCREEN_HEIGHT = 16383,
        SIZE_SCREEN_FREQUENCY = 200
    };

private slots:
    void slotStartRecording();
    void slotStopRecording();
    void slotMessage(QString text_message);
    void slotLogger(const AVFormatContext *fmt_ctx, const AVPacket *pkt, uint64_t bytes, int64_t datetime);
};
#endif // MAINWINDOW_H
