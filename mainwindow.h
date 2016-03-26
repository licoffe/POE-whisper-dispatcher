#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QFileDialog>
#include <QLabel>
#include <QDebug>
#include <QTimer>
#include <QDir>
#include <regex>
#include <fstream>
#include <thread>
#include <iostream>
#include <sstream>
#include <cstdlib>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit                 MainWindow( QWidget *parent = 0 );
    ~MainWindow();

private:
    Ui::MainWindow           *ui;
    QFileDialog*             file_dialog;
    QLabel*                  statusMessage;
    QString                  client_path;
    QString                  poe_path;
    std::thread              parser_thread;
    bool                     stop_parser_thread;
    int                      last_line_count;
    QTimer*                  interval;
    void                     parse_file( const std::string file_path, 
    const int start_line);
    int                      count_lines( const std::string file_path );

public slots:
    void                     start();
    void                     stop();
    QString                  browse_to_client_file();
    void                     parse();
};

#endif // MAINWINDOW_H
