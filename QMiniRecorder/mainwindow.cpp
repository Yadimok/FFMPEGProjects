#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle(tr("QMiniRecorder"));

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenResolution = screen->geometry();
    mWidth = screenResolution.width();
    mHeight = screenResolution.height();

    kMiniRecorder = new MiniRecorder();
    QThread *thread = new QThread();

    labelWidth = new QLabel;
    labelWidth->setText(tr("Set width, pels: <b>[%1 ... %2]</b>").arg(0).arg(mWidth));

    labelHeight = new QLabel;
    labelHeight->setText(tr("Set height, pels: <b>[%1 ... %2]</b>").arg(0).arg(mHeight));

    labelFrameRate = new QLabel;
    labelFrameRate->setText(tr("Set frame rate: <b>[%1 ... %2]</b>").arg(0).arg(SIZE_SCREEN_FREQUENCY));

    leWidth = new QLineEdit;
    leWidth->setValidator(new QIntValidator(0, mWidth, this));
    leWidth->setText("1920");

    leHeight = new QLineEdit;
    leHeight->setValidator(new QIntValidator(0, mHeight, this));
    leHeight->setText("1080");

    leFrameRate = new QLineEdit;
    leFrameRate->setValidator(new QIntValidator(0, SIZE_SCREEN_FREQUENCY, this));
    leFrameRate->setText("40");

    labelFileName = new QLabel;
    labelFileName->setText(tr("Input file name for save (without suffix <b>'.mp4'</b>):"));

    leFileName = new QLineEdit;
    leFileName->setText(QDir::currentPath() + "/");

    labelSaveFileName = new QLabel;
    labelRecordingTime = new QLabel;
    labelTotalBytes = new QLabel;
    labelCurrentDateTime = new QLabel;

    pbStartRecording = new QPushButton;
    pbStartRecording->setText(tr("Start recording"));
    pbStartRecording->setIcon(QIcon(":/play_500x500.png"));
    pbStartRecording->setEnabled(true);

    pbStopRecording = new QPushButton;
    pbStopRecording->setText(tr("Stop recording"));
    pbStopRecording->setIcon(QIcon(":/stop_500x500.png"));
    pbStopRecording->setEnabled(false);

    QPointer<QGridLayout> gridLayout = new QGridLayout;
    gridLayout->addWidget(labelWidth, 0, 0, 1, 1);
    gridLayout->addWidget(leWidth, 1, 0, 1, 1);
    gridLayout->addWidget(labelHeight, 0, 1, 1, 1);
    gridLayout->addWidget(leHeight, 1, 1, 1, 1);
    gridLayout->addWidget(labelFrameRate, 0, 2, 1, 1);
    gridLayout->addWidget(leFrameRate, 1, 2, 1, 1);
    QPointer<QWidget> tmpWidget = new QWidget;
    tmpWidget->setLayout(gridLayout);

    vboxLayout = new QVBoxLayout;
    vboxLayout->addWidget(labelFileName);
    vboxLayout->addStretch(1);
    vboxLayout->addWidget(leFileName);
    vboxLayout->addStretch(1);
    vboxLayout->addWidget(labelSaveFileName);
    vboxLayout->addStretch(1);
    vboxLayout->addWidget(labelRecordingTime);
    vboxLayout->addWidget(labelTotalBytes);
    vboxLayout->addWidget(labelCurrentDateTime);
    vboxLayout->addStretch(1);
    vboxLayout->addWidget(tmpWidget);
    vboxLayout->addStretch(1);
    vboxLayout->addWidget(pbStartRecording);
    vboxLayout->addStretch(5);
    vboxLayout->addWidget(pbStopRecording);

    groupBox = new QGroupBox;
    groupBox->setLayout(vboxLayout);

    hboxLayout = new QHBoxLayout;
    hboxLayout->addWidget(groupBox);

    centralWidget()->setLayout(hboxLayout);

    connect(pbStartRecording, SIGNAL(clicked()), this, SLOT(slotStartRecording()));
    connect(pbStopRecording, SIGNAL(clicked()), this, SLOT(slotStopRecording()));
    connect(kMiniRecorder, SIGNAL(signalMessage(QString)), this, SLOT(slotMessage(QString)));
    connect(kMiniRecorder, SIGNAL(signalLogger(const AVFormatContext *, const AVPacket *, uint64_t, int64_t)),
            this, SLOT(slotLogger(const AVFormatContext *, const AVPacket *, uint64_t, int64_t)));

    connect(thread, SIGNAL(finished()), kMiniRecorder, SLOT(deleteLater()));
    connect(this, SIGNAL(destroyed()), thread, SLOT(quit()));
    connect(kMiniRecorder, SIGNAL(destroyed()), thread, SLOT(quit()));
    connect(kMiniRecorder, SIGNAL(destroyed()), kMiniRecorder, SLOT(deleteLater()));
    thread->start();
}

MainWindow::~MainWindow()
{
    delete ui;

    delete kMiniRecorder;
}

void MainWindow::slotStartRecording()
{
    if (leFileName->text().isEmpty()) {

        QMessageBox::critical(this, tr("LineEdit"),
                              tr("Input file name."),
                              QMessageBox::Ok);
        return;
    }

    if (leWidth->text().isEmpty()) {
        QMessageBox::critical(this, tr("LineEdit"),
                              tr("Set Width."),
                              QMessageBox::Ok);
        return;
    }

    if (leHeight->text().isEmpty()) {
        QMessageBox::critical(this, tr("LineEdit"),
                              tr("Set height."),
                              QMessageBox::Ok);
        return;
    }

    if (leFrameRate->text().isEmpty()) {
        QMessageBox::critical(this, tr("LineEdit"),
                              tr("Set frame rate."),
                              QMessageBox::Ok);
        return;
    }


    if ((leWidth->text().toInt() < 0) || (leWidth->text().toInt() > mWidth)) {
        QMessageBox::critical(this, tr("LineEdit"),
                              tr("The value '<b>Width</b>' must be between %1 and %2.").arg(0).arg(mWidth),
                              QMessageBox::Ok);
        return;
    }

    if ((leHeight->text().toInt() < 0) || (leHeight->text().toInt() > mHeight)) {
        QMessageBox::critical(this, tr("LineEdit"),
                              tr("The value '<b>Height</b>' must be betweeen %1 and %2.").arg(0).arg(mHeight),
                              QMessageBox::Ok);
        return;
    }

    if ((leFrameRate->text().toInt() < 0) || (leFrameRate->text().toInt() > SIZE_SCREEN_FREQUENCY)) {
        QMessageBox::critical(this, tr("LineEdit"),
                              tr("The value '<b>FrameRate</b>' must be between %1 and %2.").arg(0).arg(SIZE_SCREEN_FREQUENCY),
                              QMessageBox::Ok);
        return;
    }

    pbStartRecording->setDisabled(true);
    pbStopRecording->setEnabled(true);

    const QString prefix = ".mp4";
    QString fileName = leFileName->text();
    fileName += prefix;

    labelSaveFileName->setText(fileName);

    int width_ = leWidth->text().toInt();
    int height_ = leHeight->text().toInt();
    int framerate_ = leFrameRate->text().toInt();

    kMiniRecorder->Init(width_, height_, framerate_);
    kMiniRecorder->OpenDesktop();
    kMiniRecorder->InitOutputFile(fileName);
    kMiniRecorder->RecordDesktop();
}

void MainWindow::slotStopRecording()
{
    pbStartRecording->setEnabled(true);
    pbStopRecording->setDisabled(true);

    kMiniRecorder->CloseDesktop();
}

void MainWindow::slotMessage(QString text_message)
{
    QMessageBox msgBox;
    msgBox.setText(text_message);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
}

void MainWindow::slotLogger(const AVFormatContext *fmt_ctx, const AVPacket *pkt, uint64_t bytes, int64_t datetime)
{
    char buf1[AV_TS_MAX_STRING_SIZE] = {0};
    char buf2[AV_TS_MAX_STRING_SIZE] = {0};

    int64_t now0;
    time_t now_secs;
    char timestamp[80] = {0};
    struct tm tmpbuf;

    now0 = datetime;
    now_secs = now0 / 1000000;
    strftime(timestamp, sizeof(timestamp), "%d/%m/%Y %H:%M:%S", localtime_r(&now_secs, &tmpbuf));

    AVRational *time_base = &fmt_ctx->streams[pkt->stream_index]->time_base;

    if (pkt->pts == AV_NOPTS_VALUE) {
        snprintf(buf1, AV_TS_MAX_STRING_SIZE, "NOPTS");
    } else {
        snprintf(buf1, AV_TS_MAX_STRING_SIZE, "%.6g", av_q2d(*time_base) * pkt->pts);
    }

    if (pkt->dts == AV_NOPTS_VALUE) {
        snprintf(buf2, AV_TS_MAX_STRING_SIZE, "NOPTS");
    } else {
        snprintf(buf2, AV_TS_MAX_STRING_SIZE, "%.6g", av_q2d(*time_base) * pkt->dts);
    }

    labelRecordingTime->setText(tr("<b>Recording time</b> (dts): %1").arg(buf2));
    labelTotalBytes->setText(tr("<b>Total bytes</b>: %1 B").arg(bytes));
    labelCurrentDateTime->setText(tr("<b>Current data</b>: %1").arg(timestamp));
}
