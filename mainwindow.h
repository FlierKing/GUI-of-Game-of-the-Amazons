#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QLabel>
#include <QPushButton>
#include <QSignalMapper>
#include <QString>
#include <QFile>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();
        QSignalMapper *signalMapper;
        QString outFileName;
        QString path;
        QFile outFile;

    private:
        Ui::MainWindow *ui;
        QPushButton *b;

    public slots:
        void exit();
        void loadGame();
        void newGameFirst();
        void newGameSecond();

        void initChessboard();
        void deleteButtons();
        void continueGame();
        void loadStepInGame(int step);
        void reloadStep();
        void initGame();
        void createLogFile();
        QByteArray getAIResult();
        void achieveStepInGame(int x0, int y0, int x1, int y1, int x2, int y2);
        void achieveAIStep(QByteArray step);
        void endGame();
        bool isGameOver(int color);
        void showResult(int winner);
        void showMessage(QString x, QString y);
};

#endif // MAINWINDOW_H
