#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QFile>
#include <QTimer>
#include <QtMultimedia/QAudioSink>
#include <QKeyEvent>
#include "GmeDecoder.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void createMenus();
    void createComponents();
    void initDevice();
    void cleanupPlayback();
    void changeTrack();
protected:
    void keyPressEvent(QKeyEvent*);

private:
    //compounents
    Ui::MainWindow *ui;
    QMenu* fileMenu;
    QAction *actionOpen,*actionClose,*actionExit;
    QMenu* controlMenu;
    QAction *controlPrev,*controlNext,*controlPlayPause;
    QMenu* aboutMenu;
    QAction *aboutInfo;
    QLabel* label_Game;
    QLabel* label_Composer;
    QLabel* label_Company;
    QLabel* label_CurTrack;
    QLabel* label_Tracks;
    QLineEdit* editGame;
    QLineEdit* editComposer;
    QLineEdit* editCompany;
    QGridLayout* layout;
    QWidget* central;
    //
    GmeDecoder gmeDecoder;
    QString filePath;
    bool isPlaying,isOpened;
    int songIndex = 0;
    game_info_t info;
    //Device
    QTimer* m_timer = nullptr;
    QAudioSink* m_audioSink = nullptr;
    QIODevice* m_audioDevice = nullptr;
private slots:
    void onActionOpen();
    void onActionClose();
    void onActionExit();
    void onControlPrev();
    void onControlPrevBy10();
    void onControlNext();
    void onControlNextBy10();
    void onControlPlayPause();
    void getAudioData();
    void onAudioStateChanged(QAudio::State state);
    void onAboutMenu();
};
#endif // MAINWINDOW_H
