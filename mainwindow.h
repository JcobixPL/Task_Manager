
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "QtSql/qsqldatabase.h"
#include <QMainWindow>
#include<QTabWidget>
#include<QTabBar>
#include<QtSql/QSql>
#include<QDebug>
#include<QFileInfo>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include<QtCharts>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE



class MainWindow : public QMainWindow

{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void saveTasksToFile();
    void loadTasksFromFile();
public slots:
    void saveTasksToDatabase();
    void loadTasksFromDatabase();
private slots:
    void on_pushButtonMyTasks_clicked();

    void on_pushButtonSchedule_clicked();

    void on_pushButtonFinances_clicked();

    void on_pushButtonAdd_clicked();


    void removeTask();

    void editTask();

    void on_pushButtonAddFinance_clicked();


    void on_pushButtonCategories_clicked();

    void updateCategoryList(const QString& categoryName);

    void on_stackedWidget_currentChanged(int index);

    void on_pushButtonEditFinance_clicked();

    void on_pushButtonStartHomeBalance_clicked();
    void updateStartHomeBalance(double value);
    void on_pushButtonHelp_clicked();

protected:
    void closeEvent(QCloseEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;



private:
    Ui::MainWindow *ui;
   QSqlDatabase db;
    int state;
   QStringList categoryList;
    double plusBalance = 0.0;
    double minusBalance = 0.0;
    double balance = 0.0;
    double startHomeBalance = 0.0;
    double homeBalance = 0.0;

    QChartView *chartView;
    QPieSeries *series;


QPoint m_dragPos;
};

#endif // MAINWINDOW_H
