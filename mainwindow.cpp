#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTime>
#include <QDebug>
#include <QLabel>
#include <QString>
#include <QProcess>
#include <QMessageBox>
#include <QTextStream>
#include <QPushButton>
#include <QFileDialog>
#include <QSignalMapper>
#include <QPropertyAnimation>

#include <windows.h>

#define GRIDSIZE 8
#define GRID_BLACK 1
#define GRID_WHITE -1
#define OBSTACLE 2
#define LENGTHOFIMG 59

#include <iostream>
#include <string>
using namespace std;

pair<int,int> cooBlack[4], cooWhite[4];
int gridInfo[8][8];
int currentColor = GRID_WHITE, currentTurnTo = GRID_BLACK;
QPushButton *gridBlack[4], *gridWhite[4], *obstacle[GRIDSIZE][GRIDSIZE], *blank[GRIDSIZE][GRIDSIZE];
int nextProcess[6];

bool inMap(int x, int y)
{
    return x >= 0 && x < GRIDSIZE && y >= 0 && y < GRIDSIZE;
}

bool Process(int x0, int y0, int x1, int y1, int x2, int y2, int color, bool checkOnly = true)
{
    qDebug() << x0 << y0 << x1 << y1 << x2 << y2 << color << checkOnly;
    if (!inMap(x0,y0)||(x1 != -1 && !inMap(x1,y1))||(x2 != -1 &&!inMap(x2,y2)))
        return false;
    if ((x1 != -1 && x0 == x1 && y0 == y1) || (x2 != -1 && x1 == x2 && y1 == y2))
        return false;
    if (gridInfo[x0][y0] != color)
        return false;
    if (x1 != -1)
    {
        int dx = x1 - x0, dy = y1 - y0;
        if (dx && dy && abs(dx) != abs(dy))
            return false;
        if (dx) dx /= abs(dx);if (dy) dy /= abs(dy);
        for (int tx = x0, ty = y0; tx != x1 || ty != y1; tx += dx, ty += dy)
        {
            if (gridInfo[tx + dx][ty + dy] != 0)
                return false;
        }
    }

    if (x2 != -1)
    {
        int dx = x2 - x1, dy = y2 - y1;
        if (dx && dy  && abs(dx) != abs(dy))
            return false;
        if (dx) dx /= abs(dx);if (dy) dy /= abs(dy);
        for (int tx = x1, ty = y1; tx != x2 || ty != y2; tx += dx, ty += dy)
        {
            if (x0 != tx + dx && y0 != ty + dy && gridInfo[tx + dx][ty + dy] != 0)
                return false;
        }
        if (!checkOnly)
        {
            gridInfo[x0][y0] = 0;
            gridInfo[x1][y1] = color;
            gridInfo[x2][y2] = OBSTACLE;
        }
    }
    return true;
}

void MainWindow::exit()
{
    //what you want to do
    this->close();
}

void MainWindow::deleteButtons()
{
    this->ui->loadGame->hide();
    this->ui->newGameFirst->hide();
    this->ui->newGameSecond->hide();
    this->ui->exitGame->hide();
    this->ui->textTitle->hide();
}

inline int cal(int x)
{
    return 62 * x;
}

void timeWait(int x)
{
    QTime t;
    t.start();
    while(t.elapsed()<x)
        QCoreApplication::processEvents();
}

void MainWindow::initGame()
{
    this->ui->stepShowSelf->setText("");
    this->ui->stepShowCom->setText("");
    this->ui->stepShowSelf->show();
    this->ui->stepShowCom->show();
    int cnt = 0;
    for (int i = 0; i < GRIDSIZE; i++)
        for (int j = 0; j < GRIDSIZE; j++)
            if (gridInfo[i][j] == GRID_BLACK)
            {
                cooBlack[cnt] = make_pair(i, j);
                gridBlack[cnt++]->setGeometry(cal(i), cal(j), LENGTHOFIMG, LENGTHOFIMG);
                signalMapper->setMapping(gridBlack[i], cooBlack[i].first * GRIDSIZE + cooBlack[i].second);

            }
    cnt = 0;
    for (int i = 0; i < GRIDSIZE; i++)
        for (int j = 0; j < GRIDSIZE; j++)
            if (gridInfo[i][j] == GRID_WHITE)
            {
                cooWhite[cnt] = make_pair(i, j);
                gridWhite[cnt++]->setGeometry(cal(i), cal(j), LENGTHOFIMG, LENGTHOFIMG);
                signalMapper->setMapping(gridWhite[i], cooWhite[i].first * GRIDSIZE + cooWhite[i].second);
            }
    for (int i = 0; i < 4; i++)
    {
        gridBlack[i]->show();
        gridWhite[i]->show();
    }
    for (int i = 0; i < GRIDSIZE; i++)
        for (int j = 0; j < GRIDSIZE; j++)
        {
            if (gridInfo[i][j] == 0)
            {
                blank[i][j]->show();
                obstacle[i][j]->hide();
            }
            else if (gridInfo[i][j] == 2)
            {
                obstacle[i][j]->show();
                blank[i][j]->hide();
            }
        }
}

QByteArray MainWindow::getAIResult()
{
    QString dir_str = "./source/ai.exe";
    QDir dir;
    if (!dir.exists(dir_str))
    {
        QMessageBox message;
        message.warning(this, "警告", "没有AI程序");
        this->close();
    }

    QProcess *po = new QProcess(this);
    QString resStorage = QString("%1/source/tmp.txt").arg(this->path);
    QString program = this->path + "/source/ai.exe";
    QStringList argv;
    argv.append(this->outFileName);
    argv.append(resStorage);
    po->start(program, argv);

    timeWait(1100);

    QFile AIResult(resStorage);
    AIResult.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray res = AIResult.readLine();
    AIResult.close();
    return res;
}

void MainWindow::reloadStep()
{
    QString stepContent = "";
    if (currentColor == GRID_BLACK)
        stepContent = "黑";
    else
        stepContent = "白";
    for (int i = 0; i < 6; i++)
        if (nextProcess[i] == -1) break;
        else stepContent += " " + QString::number(nextProcess[i]);
    this->ui->stepShowSelf->setText(stepContent);
}

void MainWindow::createLogFile()
{
    QDateTime time = QDateTime::currentDateTime();
    QString dateTime = time.toString("yyyy-MM-dd-hh-mm-ss");
    this->outFileName= this->path + QString("/chesslog/chesslog-%1.txt").arg(dateTime);
    this->outFile.setFileName(this->outFileName);
    if(!this->outFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox message;
        message.warning(this, "警告", "无法创建记录文件");
        this->close();
    }
    outFile.write("1\n");
    outFile.write("-1 -1 -1 -1 -1 -1\n");
    outFile.close();
}

QByteArray intToByte(int i)
{
    QByteArray s = "";
    if (i == 0) return QByteArray("0");
    while (i)
    {
        s = char(i % 10 + '0') + s;
        i /= 10;
    }
    return s;
}

void MainWindow::achieveStepInGame(int x0, int y0, int x1, int y1, int x2, int y2)
{
    if (!Process(x0, y0, x1, y1, x2, y2, currentTurnTo, false))
    {
        QMessageBox message;
        message.warning(this, "警告", "未知错误：棋步出错");
        this->close();
    }
    blank[x0][y0]->show();
    for (int i = 0; i < 4; i++)
        if (currentTurnTo == GRID_BLACK)
        {
            if (cooBlack[i].first == x0 && cooBlack[i].second == y0)
            {
                cooBlack[i].first = x1;
                cooBlack[i].second = y1;
                gridBlack[i]->setGeometry(cal(x1), cal(y1), LENGTHOFIMG, LENGTHOFIMG);
                signalMapper->setMapping(gridBlack[i], cooBlack[i].first * GRIDSIZE + cooBlack[i].second);
                break;
            }
        }
        else/*currentTurnTo == GRID_WHITE)*/
        {
            if (cooWhite[i].first == x0 && cooWhite[i].second == y0)
            {
                cooWhite[i].first = x1;
                cooWhite[i].second = y1;
                gridWhite[i]->setGeometry(cal(x1), cal(y1), LENGTHOFIMG, LENGTHOFIMG);
                signalMapper->setMapping(gridWhite[i], cooWhite[i].first * GRIDSIZE + cooWhite[i].second);
                break;
            }
        }
    blank[x1][y1]->hide();
    timeWait(100);
    blank[x2][y2]->hide();
    obstacle[x2][y2]->show();

    this->outFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray firstLine = outFile.readLine();
    int n = 0;
    QByteArray f[70];
    bool firstHand = false;
    char *s = firstLine.data();
    for (int i = 0; s[i] != '\r' && s[i] != '\n'; i++)
        n = n * 10 + s[i] - '0';
    for (int i = 1; i <= 2 * n - 1; i++)
    {
        QByteArray line = outFile.readLine();
        if (i == 1 && line == QByteArray("-1 -1 -1 -1 -1 -1\n"))
        {
            firstHand = true;
            continue;
        }

        f[i] = line;
    }
    outFile.close();

    outFile.open(QIODevice::WriteOnly | QIODevice::Text);
    if (!firstHand) n++;
    outFile.write(intToByte(n) + "\n");
    if (!firstHand) outFile.write("-1 -1 -1 -1 -1 -1\n");
    for (int i = 1; i <= 2 * n - 1; i++)
    {
        if (i == 1 && firstHand) continue;
        outFile.write(f[i]);
    }
    outFile.write(intToByte(x0) + " " + intToByte(y0) + " " + intToByte(x1) + " " + intToByte(y1) + " " + intToByte(x2) + " " + intToByte(y2) + '\n');
    outFile.close();

    if (isGameOver(-currentTurnTo)) return;
}

void MainWindow::achieveAIStep(QByteArray step)
{
    char *s = step.data();
    int a[6], cnt = 0;
    for (int i = 0;;i++)
    {
        if (s[i] == ' ' || s[i] == '\r' || s[i] == '\n')
        {
            cnt++;
            if (s[i] == '\r' || s[i] == '\n') break;
        }
        a[cnt] = s[i] - '0';
    }
    QString comColor = (currentColor == GRID_BLACK ? "白 " : "黑 ");
    this->ui->stepShowCom->setText(comColor + QString::number(a[0]) + " " + QString::number(a[1]) + " " + QString::number(a[2])
            + " " + QString::number(a[3]) + " " + QString::number(a[4]) + " " + QString::number(a[5]));
    this->achieveStepInGame(a[0], a[1], a[2], a[3], a[4], a[5]);
}

void resetChessboard()
{
    for (int i = 0; i < GRIDSIZE; i++)
        for (int j = 0; j < GRIDSIZE; j++)
        gridInfo[i][j] = 0;
    gridInfo[0][(GRIDSIZE - 1) / 3] = gridInfo[(GRIDSIZE - 1) / 3][0]
            = gridInfo[GRIDSIZE - 1 - ((GRIDSIZE - 1) / 3)][0]
            = gridInfo[GRIDSIZE - 1][(GRIDSIZE - 1) / 3] = GRID_BLACK;
    gridInfo[0][GRIDSIZE - 1 - ((GRIDSIZE - 1) / 3)] = gridInfo[(GRIDSIZE - 1) / 3][GRIDSIZE - 1]
            = gridInfo[GRIDSIZE - 1 - ((GRIDSIZE - 1) / 3)][GRIDSIZE - 1]
            = gridInfo[GRIDSIZE - 1][GRIDSIZE - 1 - ((GRIDSIZE - 1) / 3)] = GRID_WHITE;
    cooBlack[0] = make_pair(0, (GRIDSIZE - 1) / 3);
    cooBlack[1] = make_pair((GRIDSIZE - 1) / 3, 0);
    cooBlack[2] = make_pair(GRIDSIZE - 1 - ((GRIDSIZE - 1) / 3), 0);
    cooBlack[3] = make_pair(GRIDSIZE - 1, (GRIDSIZE - 1) / 3);
    cooWhite[0] = make_pair(0, GRIDSIZE - 1 - ((GRIDSIZE - 1) / 3));
    cooWhite[1] = make_pair((GRIDSIZE - 1) / 3, GRIDSIZE - 1);
    cooWhite[2] = make_pair(GRIDSIZE - 1 - ((GRIDSIZE - 1) / 3), GRIDSIZE - 1);
    cooWhite[3] = make_pair(GRIDSIZE - 1, GRIDSIZE - 1 - ((GRIDSIZE - 1) / 3));
}

void MainWindow::showMessage(QString x, QString y)
{
    QMessageBox message;
    message.warning(this, x, y);
}

void MainWindow::showResult(int winner)
{
    if (currentTurnTo == GRID_BLACK)
        this->showMessage("游戏结束", "黑方胜利");
    else
        this->showMessage("游戏结束", "白方胜利");
    this->endGame();
    return;
}

bool MainWindow::isGameOver(int color)
{
    const int wx[] = {0, 1, 0, -1, 1, 1, -1, -1};
    const int wy[] = {1, 0, -1, 0, 1, -1, 1, -1};
    for (int i = 0; i < GRIDSIZE; i++)
        for (int j = 0; j < GRIDSIZE; j++)
            if (gridInfo[i][j] == color)
            {
                for (int k = 0; k < 8; k++)
                {
                    int nx = i + wx[k], ny = j + wy[k];
                    if (inMap(nx, ny) && gridInfo[nx][ny] == 0)
                        return false;
                }
            }
    showResult(-color);
    return true;
}

void MainWindow::endGame()
{
    currentColor = GRID_WHITE, currentTurnTo = GRID_BLACK;
    for (int i = 0; i < GRIDSIZE; i++)
        for (int j = 0; j < GRIDSIZE; j++)
        {
            blank[i][j]->hide();
            obstacle[i][j]->hide();
        }
    for (int i = 0; i < 4; i++)
    {
        gridBlack[i]->hide();
        gridWhite[i]->hide();
    }
    resetChessboard();
    for (int i = 0; i < 4; i++)
    {
        signalMapper->setMapping(gridBlack[i], cooBlack[i].first * GRIDSIZE + cooBlack[i].second);
        signalMapper->setMapping(gridWhite[i], cooWhite[i].first * GRIDSIZE + cooWhite[i].second);
    }
    this->ui->stepShowSelf->hide();
    this->ui->stepShowCom->hide();
    this->ui->exitGame->show();
    this->ui->newGameFirst->show();
    this->ui->newGameSecond->show();
    this->ui->loadGame->show();
    this->ui->textTitle->show();
    this->setStyleSheet("");
}

void MainWindow::loadStepInGame(int step)
{
    int x = step / GRIDSIZE, y = step % GRIDSIZE, st;
    if (nextProcess[0] == -1) st = 0;
    else if (nextProcess[2] == -1) st = 2;
    else /*nextProcess[4] == -1*/ st = 4;
    nextProcess[st] = x;
    nextProcess[st + 1] = y;
    this->reloadStep();
    if (Process(nextProcess[0], nextProcess[1], nextProcess[2], nextProcess[3], nextProcess[4], nextProcess[5], currentColor))
    {
        if (st == 4)
        {
            this->achieveStepInGame(nextProcess[0], nextProcess[1], nextProcess[2], nextProcess[3], nextProcess[4], nextProcess[5]);
            currentTurnTo = -currentTurnTo;
            QByteArray step = this->getAIResult();
            this->achieveAIStep(step);
            currentTurnTo = -currentTurnTo;

            for (int i = 0; i < 6; i++)
                nextProcess[i] = -1;
        }
    }
    else
    {
        for (int i = 0; i < 6; i++)
            nextProcess[i] = -1;
        this->reloadStep();
    }
}

void MainWindow::continueGame()
{
    this->setStyleSheet("background-color:rgb(180, 180, 180)");
    this->deleteButtons();
    this->initGame();
}

void MainWindow::newGameFirst()
{
    this->createLogFile();
    currentColor = GRID_BLACK;
    currentTurnTo = GRID_BLACK;
    this->continueGame();
}

void MainWindow::newGameSecond()
{
    this->createLogFile();
    currentColor = GRID_WHITE;
    currentTurnTo = GRID_BLACK;

    QByteArray res = this->getAIResult();
    qDebug() << res;
    this->achieveAIStep(res);
    currentTurnTo = -currentTurnTo;

    this->continueGame();
}

void MainWindow::loadGame()
{
    this->createLogFile();
  //  this->outFile.open(QIODevice::WriteOnly | QIODevice::Text);
    currentColor = GRID_WHITE;
    currentTurnTo = GRID_WHITE;
    QFileDialog *fileDialog = new QFileDialog(this);
    fileDialog->setWindowTitle(tr("打开对局记录"));
    fileDialog->setDirectory(this->path + "/chesslog");
    fileDialog->setNameFilter(tr("TXT(*.txt)"));
    fileDialog->setViewMode(QFileDialog::Detail);
    QStringList fileName;
    if (fileDialog->exec())
        fileName = fileDialog->selectedFiles();
    if (fileName.isEmpty())
    {
        QMessageBox message;
        message.warning(this, "警告", "打开文件失败");
        return;
    }
    QFile inFile(fileName[0]);
    if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        this->showMessage("警告", "打开文件失败");
        return;
    }
    if (inFile.atEnd())
    {
        this->showMessage("警告", "文件为空");
        return;
    }
    int v = 0, n;
    char *s = inFile.readLine().data();
  //  outFile.write(s);
    for (int i = 0;; i++)
    {
        if (s[i] == '\n' || s[i] == '\r')
        {
            n = v;
            break;
        }
        else if (s[i] >= '0' && s[i] <= '9') v = v * 10 + s[i] - '0';
        else
        {
            this->showMessage("警告", "文件格式错误：未知字符");
            inFile.close();
            return;
        }
    }
    this->setStyleSheet("background-color:rgb(180, 180, 180)");
    this->deleteButtons();
    this->initGame();
    int a[6], cnt;
    for (int i = 1; i <= 2 * n - 1; i++)
    {
        QByteArray line = inFile.readLine();
        if (i == 1)
        {
            if(line == QByteArray("-1 -1 -1 -1 -1 -1\n"))
            {
                currentColor = GRID_BLACK;
                currentTurnTo = GRID_BLACK;
                continue;
            }
            else
            {
                currentColor = GRID_WHITE;
                currentTurnTo = GRID_BLACK;
            }
        }

        v = cnt = 0;
        s = line.data();
    //    outFile.write(s);
        for (int j = 0;; j++)
        {
            if (s[j] == ' ' || s[j] == '\r' || s[j] == '\n')
            {
                if (cnt == 6)
                {
                    endGame();
                    inFile.close();
                    this->showMessage("警告", "文件格式错误：单行数字数量过多");
                    return;
                }
                a[cnt++] = v;
                if (s[j] == '\r' || s[j] == '\n') break;
            }
            else if (s[j] >= '0' && s[j] <= '9')
                v = s[j] - '0';
            else
            {
                endGame();
                inFile.close();
                this->showMessage("警告", "文件格式错误：未知字符");
                return;
            }
        }
        if (cnt < 6)
        {
            endGame();
            inFile.close();
            this->showMessage("警告", "文件格式错误：单行数字数量过少");
            return;
        }
        if (!Process(a[0], a[1], a[2], a[3], a[4], a[5],
                    (((i % 2 == 0 && currentColor == GRID_BLACK) || (i % 2 == 1 && currentColor == GRID_WHITE)) ? GRID_BLACK : GRID_WHITE)))
        {
            endGame();
            inFile.close();
            this->showMessage("警告", "文件格式错误：行棋错误");
            return;
        }
        else
        {
            achieveStepInGame(a[0], a[1], a[2], a[3], a[4], a[5]);
            currentTurnTo = -currentTurnTo;
        }
    }
  //  outFile.close();
//    this->continueGame();
}

void MainWindow::initChessboard()
{
    resetChessboard();
    for (int i = 0; i < 4; i++)
    {
        gridBlack[i] = new QPushButton(this);
        gridBlack[i]->setStyleSheet("background-image: url(:/img/grid_black);");
        gridBlack[i]->hide();
        connect(gridBlack[i], SIGNAL(clicked()), this->signalMapper, SLOT(map()));
        signalMapper->setMapping(gridBlack[i], cooBlack[i].first * GRIDSIZE + cooBlack[i].second);

        gridWhite[i] = new QPushButton(this);
        gridWhite[i]->setStyleSheet("background-image: url(:/img/grid_white);");
        gridWhite[i]->hide();
        signalMapper->setMapping(gridWhite[i], cooWhite[i].first * GRIDSIZE + cooWhite[i].second);
        connect(gridWhite[i], SIGNAL(clicked()), this->signalMapper, SLOT(map()));
    }

    for (int i = 0; i < GRIDSIZE; i++)
        for (int j = 0; j < GRIDSIZE; j++)
        {
            obstacle[i][j] = new QPushButton(this);
            obstacle[i][j]->setStyleSheet("background-image: url(:/img/obstacle);");
            obstacle[i][j]->setGeometry(cal(i), cal(j), LENGTHOFIMG, LENGTHOFIMG);
            obstacle[i][j]->hide();

            blank[i][j] = new QPushButton(this);
            blank[i][j]->setStyleSheet("background-image: url(:/img/blank);");
            blank[i][j]->setGeometry(cal(i), cal(j), LENGTHOFIMG, LENGTHOFIMG);
            blank[i][j]->hide();
            connect(blank[i][j], SIGNAL(clicked()), this->signalMapper, SLOT(map()));
            signalMapper->setMapping(blank[i][j], i * GRIDSIZE + j);
        }
    connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(loadStepInGame(int)));
    QString dir_str = this->path + "/chesslog";
    QDir dir;
    if (!dir.exists(dir_str))
    {
        bool res = dir.mkpath(dir_str);
        if (res == false)
        {
            this->showMessage("警告", "无法创建记录文件夹");
            this->close();
        }
    }
}

MainWindow::MainWindow(QWidget *parent) :
    ui(new Ui::MainWindow)
    {
        this->path=QDir::currentPath();
        this->signalMapper = new QSignalMapper(this);
        this->setWindowFlags(Qt::WindowStaysOnTopHint);
        ui->setupUi(this);
        this->initChessboard();
        ui->stepShowSelf->show();
        ui->stepShowCom->show();
        for (int i = 0; i < 6; i++)
            nextProcess[i] = -1;
        connect(ui->exitGame, SIGNAL(clicked()), this, SLOT(exit()));
        connect(ui->loadGame, SIGNAL(clicked()), this, SLOT(loadGame()));
        connect(ui->newGameFirst, SIGNAL(clicked()), this, SLOT(newGameFirst()));
        connect(ui->newGameSecond, SIGNAL(clicked()), this, SLOT(newGameSecond()));
    }

MainWindow::~MainWindow()
{
    delete ui;
}

#undef GRIDSIZE
#undef GRID_BLACK
#undef GRID_WHITE
#undef LENGTHOFIMG
