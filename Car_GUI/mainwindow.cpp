#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    instance = this;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setLabel2Visible(bool on)
{
    ui->re_trai->setVisible(on);
}
