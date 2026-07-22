#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QMediaDevices>
#include <QDebug>

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 480

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_audioSink(nullptr)
    , m_audioDevice(nullptr)
    , m_timer(nullptr)
    , isPlaying(false)
    , isOpened(false)
    , songIndex(0)
{
    ui->setupUi(this);
    setWindowTitle("BeefPlayer");
    setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    createMenus();
    createComponents();
    initDevice();


}

MainWindow::~MainWindow()
{
    // 停止所有活动
    cleanupPlayback();
    delete ui;
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu("File");
    actionOpen = new QAction("Open(file)", this);
    actionClose = new QAction("Close(file)", this);
    actionExit = new QAction("Exit", this);
    fileMenu->addAction(actionOpen);
    fileMenu->addAction(actionClose);
    fileMenu->addAction(actionExit);

    controlMenu = menuBar()->addMenu("Control");
    controlPrev = new QAction("Previous", this);
    controlNext = new QAction("Next", this);
    controlPlayPause = new QAction("Play/Pause", this);
    controlMenu->addAction(controlPrev);
    controlMenu->addAction(controlNext);
    controlMenu->addAction(controlPlayPause);
    // 为 Control 菜单的 Action 添加快捷键
    //controlPrev->setShortcut(QKeySequence(Qt::Key_Left));
    //controlNext->setShortcut(QKeySequence(Qt::Key_Right));
    //controlPlayPause->setShortcut(QKeySequence(Qt::Key_Space));
    aboutMenu = menuBar()->addMenu("About");
}

void MainWindow::createComponents()
{
    central = new QWidget(this);
    setCentralWidget(central);
    layout = new QGridLayout(central);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(10);

    label_Game = new QLabel(tr("Game:"), central);
    editGame = new QLineEdit("%1", central);
    label_Composer = new QLabel("Composer:", central);
    editComposer = new QLineEdit("%2", central);
    label_Company = new QLabel("Co.Ltd:", central);
    editCompany = new QLineEdit("%3", central);
    label_Tracks = new QLabel("__/__", central);

    editGame->setReadOnly(true);
    editComposer->setReadOnly(true);
    editCompany->setReadOnly(true);
    // 让所有 QLineEdit 不抢焦点
    editGame->setFocusPolicy(Qt::NoFocus);
    editComposer->setFocusPolicy(Qt::NoFocus);
    editCompany->setFocusPolicy(Qt::NoFocus);

    // 设置布局
    layout->addWidget(label_Game, 0, 0);
    layout->addWidget(editGame, 0, 1);
    layout->addWidget(label_Composer, 1, 0);
    layout->addWidget(editComposer, 1, 1);
    layout->addWidget(label_Company, 2, 0);
    layout->addWidget(editCompany, 2, 1);
    layout->addWidget(label_Tracks, 3, 0);

    connect(actionOpen, &QAction::triggered, this, &MainWindow::onActionOpen);
    connect(actionClose, &QAction::triggered, this, &MainWindow::onActionClose);
    connect(actionExit, &QAction::triggered, this, &MainWindow::onActionExit);
    connect(controlPlayPause, &QAction::triggered, this, &MainWindow::onControlPlayPause);
    connect(controlPrev, &QAction::triggered, this, &MainWindow::onControlPrev);
    connect(controlNext, &QAction::triggered, this, &MainWindow::onControlNext);
}

void MainWindow::initDevice()
{
    // 如果已存在，先清理
    cleanupPlayback();

    // 创建定时器
    m_timer = new QTimer(this);
    int delay = BYTES_PER_FRAME * 1000 / SAMPLE_RATE / 2;
    m_timer->setInterval(delay);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::getAudioData);

    // 设置音频格式
    QAudioFormat format;
    format.setSampleRate(SAMPLE_RATE);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);

    m_audioSink = new QAudioSink(format, this);
    if (!m_audioSink) {
        QMessageBox::critical(this, "Critical error!", "Audio sink unknown!");
        return;
    }

    connect(m_audioSink, &QAudioSink::stateChanged,
            this, &MainWindow::onAudioStateChanged);

    // 启动音频设备
    //m_audioDevice = m_audioSink->start();
}

void MainWindow::cleanupPlayback()
{
    // 停止定时器
    if (m_timer) {
        m_timer->stop();
    }

    // 停止音频设备
    if (m_audioSink) {
        m_audioSink->stop();
    }

    // 重置解码器（如果已打开）
    if (isOpened) {
        // 假设 gmeDecoder 有相应的清理方法
        // 如果没有，确保它不会持有外部资源
        isOpened = false;
        isPlaying = false;
    }

    // 重置 UI 显示
    editGame->setText("%1");
    editComposer->setText("%2");
    editCompany->setText("%3");
    label_Tracks->setText("__/__");
}

void MainWindow::changeTrack()
{
    gmeDecoder.SetTrack(songIndex);
    label_Tracks->setText(QString("%1/%2")
                              .arg(songIndex + 1)
                              .arg(gmeDecoder.GetTrackCount()));
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "Key pressed:" << event->key();
    switch(event->key())
    {
    case Qt::Key_Left:
        onControlPrev();
        qDebug() << "Left key pressed.";
        break;
    case Qt::Key_Right:
        onControlNext();
        qDebug() << "Right key pressed.";
        break;
    case Qt::Key_Space:
        onControlPlayPause();
        qDebug() << "Space pressed.";
        break;
    }
}

void MainWindow::onActionOpen()
{
    // 如果已有文件打开，先清理
    if(isOpened)
    {
        cleanupPlayback();
    }
    if (!isOpened)
    {
        QString filePath = QFileDialog::getOpenFileName(
            this,
            "Select a file",
            "/home/kewen/Music",
            "NSF File(*.nsf);;GBS File(*.gbs)"
            );

        if (filePath.isEmpty()) {
            return;  // 用户取消了选择
        }

        // 加载文件
        QByteArray ba = filePath.toLatin1();
        char* filename = ba.data();

        if (!gmeDecoder.LoadFile(filename)) {
            QMessageBox::warning(this, "Warning", "Open file error.");
            return;
        }

        // 更新状态
        isOpened = true;
        isPlaying = false;  // 先设为 false，等 start 后再设 true

        // 更新 UI
        info = gmeDecoder.GetGameInfo();
        editGame->setText(QString("%1").arg(info.game));
        editComposer->setText(QString("%1").arg(info.author));
        editCompany->setText(QString("%1").arg(info.copyright));
        label_Tracks->setText(QString("%1/%2")
                                  .arg(songIndex + 1)
                                  .arg(gmeDecoder.GetTrackCount()));

        // 设置曲目并开始播放
        gmeDecoder.SetTrack(songIndex);
        m_audioDevice = m_audioSink->start();
        m_timer->start();
        isPlaying = true;
    }
}

void MainWindow::onActionClose()
{
    if (isOpened) {
        cleanupPlayback();
    }
}

void MainWindow::onActionExit()
{
    exit(0);
}

void MainWindow::onControlPrev()
{
    if(!isOpened)return;
    if(!isPlaying)return;
    if(songIndex > 0)
    {
        songIndex--;
    }
    changeTrack();
}

void MainWindow::onControlNext()
{
    if(!isOpened)return;
    if(!isPlaying)return;
    if(songIndex < gmeDecoder.GetTrackCount() - 1)
    {
        songIndex++;
    }
    changeTrack();
}

void MainWindow::onControlPlayPause()
{
    //should be opened.
    if(isOpened)
    {
        if(isPlaying)
        {
            m_timer->stop();
            m_audioSink->suspend();
            isPlaying = false;
        }
        else
        {
            m_timer->start();
            m_audioSink->resume();
            isPlaying = true;
        }
    }
}

void MainWindow::getAudioData()
{
    // 安全检查
    if (!isOpened || !isPlaying || !m_audioSink || !m_audioDevice) {
        return;
    }

    // 检查音频设备是否还有可用空间
    qint64 freeBytes = m_audioSink->bytesFree();
    if (freeBytes <= 0) {
        return;
    }

    // 获取 PCM 数据
    short* pcm = gmeDecoder.OutputData();
    if (!pcm) {
        return;  // 没有数据可读取
    }

    // 转换为 QByteArray
    int byteCount = BYTES_PER_FRAME * sizeof(short);
    QByteArray audioData(reinterpret_cast<const char*>(pcm), byteCount);

    // 写入音频设备
    qint64 written = m_audioDevice->write(audioData);
    if (written != byteCount) {
        qDebug() << "Written " << written << " bytes, expected " << byteCount;
    }
}

void MainWindow::onAudioStateChanged(QAudio::State state)
{
    qDebug() << "Audio state:" << state << "Error:" << m_audioSink->error();

    // 处理音频状态变化
    if (state == QAudio::IdleState && isPlaying) {
        // 如果正在播放但进入了 IdleState，说明数据不足
        // 检查是否播放完成
        if (isOpened) {
            // 可能已经播放到末尾，尝试继续或停止
            qDebug() << "IdleState detected, checking if playback finished...";
        }
    } else if (state == QAudio::StoppedState) {
        if (m_audioSink->error() != QAudio::NoError) {
            qDebug() << "Audio error:" << m_audioSink->error();
        }
    }
}
