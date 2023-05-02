#include "mainwindow.h"
#include<iostream>
#include "qgridlayout.h"
#include "ui_mainwindow.h"
#include <QDialog>
#include <QListWidgetItem>
#include <QMouseEvent>
#include<QInputDialog>
#include<QDialogButtonBox>
#include<QMessageBox>
#include<QDateEdit>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include<QDateEdit>
#include<QCalendarWidget>
#include<QtWidgets>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    // minimalny rozmiar okna
    setMinimumSize(QSize(800, 600));

    // maksymalny rozmiar okna
    setMaximumSize(QGuiApplication::primaryScreen()->availableSize());

    // Utworzenie połączenia z bazą danych
    QSqlQuery query;
db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("TaskManager.db");

    // Sprawdzenie czy udało się połączyć z bazą danych
    if (!db.open()) {
        qDebug() << "Nie udało się połączyć z bazą danych: " << db.lastError().text();
         return;
    }

    // Utworzenie tabeli "tasks" jeśli jeszcze nie istnieje
    if (!query.exec("CREATE TABLE IF NOT EXISTS tasks (name TEXT, date TEXT)")) {
        qDebug() << "Nie udało się utworzyć tabeli: " << query.lastError().text();
          return;
    }



    // Wczytanie zadań z bazy danych
    loadTasksFromDatabase();
    ui->stackedWidget->setCurrentIndex(ui->stackedWidget->indexOf(ui->pageTasks));
    connect(ui->pushButtonMyTasks, &QPushButton::clicked, this, &MainWindow::on_pushButtonMyTasks_clicked);
    connect(ui->pushButtonFinances, &QPushButton::clicked, this, &MainWindow::on_pushButtonFinances_clicked);
    connect(ui->pushButtonRemove, &QPushButton::clicked, this, &MainWindow::removeTask);
    connect(ui->pushButtonEdit, &QPushButton::clicked, this, &MainWindow::editTask);


    //QSqlDatabase mydb= QSqlDatabase::addDatabase("QSQLITE");
   // mydb.setDatabaseName("");
}

MainWindow::~MainWindow()
{
    saveTasksToDatabase();
    db.close();
    delete ui;
}

void MainWindow::saveTasksToDatabase()
{
    if (!db.open())
    {
          qDebug() << "Cannot open database: " << db.lastError().text();
          return;
    }

    QSqlQuery query;
    query.prepare("CREATE TABLE IF NOT EXISTS tasks (name TEXT, date TEXT)");
    query.exec();

    // Usunięcie wszystkich zadań z tabeli
    query.exec("DELETE FROM tasks");

    for (int i = 0; i < ui->listWidget->count(); i++)
    {
          QString taskString = ui->listWidget->item(i)->text();

          QStringList taskData = taskString.split("(");
          QString taskName = taskData[0].trimmed();
          QString taskDate = taskData[1].replace(")", "").trimmed();

          query.prepare("INSERT INTO tasks (name, date) VALUES (:name, :date)");
          query.bindValue(":name", taskName);
          query.bindValue(":date", taskDate);
          query.exec();
    }

    db.close();
}

void MainWindow::loadTasksFromDatabase()
{
    if (!db.open())
    {
          qDebug() << "Cannot open database: " << db.lastError().text();
          return;
    }

    QSqlQuery query;
    query.prepare("SELECT name, date FROM tasks");
    query.exec();

    ui->listWidget->clear();

    while (query.next())
    {
          QString taskName = query.value(0).toString();
          QString taskDate = query.value(1).toString();

          QString taskString = taskName + " (" + taskDate + ")";
          ui->listWidget->addItem(taskString);
    }

    db.close();
}


void MainWindow::on_pushButtonMyTasks_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageTasks);
}

void MainWindow::on_pushButtonFinances_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageFinances);
}


void MainWindow::on_pushButtonSchedule_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->pageSchedule);
}


void MainWindow::mousePressEvent(QMouseEvent *event)
{
    m_dragPos = event->globalPos() - frameGeometry().topLeft();
    event->accept();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPos() - m_dragPos);
        event->accept();
    }
}

void MainWindow::on_pushButtonAdd_clicked()
{
    // Tworzenie okna dialogowego
    QDialog dialog(this);
    dialog.setWindowTitle("Add Task");

    // Tworzenie widgetów w oknie dialogowym
    QLabel* nameLabel = new QLabel("Task Name:");
    QLineEdit* nameEdit = new QLineEdit(&dialog);
    QLabel* dateLabel = new QLabel("Task Date:");
    QDateEdit* dateEdit = new QDateEdit(&dialog);
    dateEdit->setDate(QDate::currentDate()); // Ustawienie daty domyślnej na dzisiejszą

    // Tworzenie przycisków "OK" i "Cancel"
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                       Qt::Horizontal, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Ułożenie widgetów w oknie dialogowym
    QGridLayout* layout = new QGridLayout(&dialog);
    layout->addWidget(nameLabel, 0, 0);
    layout->addWidget(nameEdit, 0, 1);
    layout->addWidget(dateLabel, 1, 0);
    layout->addWidget(dateEdit, 1, 1);
    layout->addWidget(buttonBox, 2, 0, 1, 2);

    // Wyświetlenie okna dialogowego
    if (dialog.exec() == QDialog::Accepted)
    {
        QString taskName = nameEdit->text();
        QDate taskDate = dateEdit->date();
        QString taskDescription = taskName;

        // Dodanie zadania do listy
        QString taskString = taskName + " (" + taskDate.toString(Qt::ISODate) + ")";
        ui->listWidget->addItem(taskString);
        ui->listWidget->sortItems(Qt::AscendingOrder);
    }
}

void MainWindow::on_pushButtonAddFinance_clicked()
{
    // Tworzenie okna dialogowego
    QDialog dialog(this);
    dialog.setWindowTitle("Add");
    dialog.setFixedSize(210, 180);
    // Tworzenie widgetu z formularzem
    QWidget* formWidget = new QWidget(&dialog);
    QFormLayout* formLayout = new QFormLayout(formWidget);

    // Wybór rodzaju (przychód/wydatek)
    QComboBox* typeComboBox = new QComboBox();
    typeComboBox->addItem("Przychód");
        typeComboBox->addItem("Wydatek");
    formLayout->addRow("Rodzaj", typeComboBox);

    // Wpis tytułu
    QLineEdit* titleLineEdit = new QLineEdit();
    formLayout->addRow("Tytuł", titleLineEdit);

        // Wybór kategorii
        QComboBox* categoryComboBox = new QComboBox();

            // Pętla iterująca po wszystkich elementach listy i dodająca je do comboboxa
            for (const QString& category : categoryList) {
        categoryComboBox->addItem(category);
        }

        // Przycisk "Dodaj kategorię"
        QPushButton* addCategoryButton = new QPushButton("Dodaj kategorię", &dialog);
        connect(addCategoryButton, &QPushButton::clicked, [&](){
            // Tworzenie okna dialogowego do dodawania kategorii
            QDialog categoryDialog(&dialog);
            categoryDialog.setWindowTitle("Dodaj kategorię");
                categoryDialog.setFixedSize(210, 100);

            // Tworzenie widgetu z formularzem
            QWidget* categoryFormWidget = new QWidget(&categoryDialog);
            QFormLayout* categoryFormLayout = new QFormLayout(categoryFormWidget);

            // Wpis nazwy kategorii
            QLineEdit* categoryNameLineEdit = new QLineEdit();
            categoryFormLayout->addRow("Nazwa", categoryNameLineEdit);

            // Przyciski akcji
            QDialogButtonBox* categoryButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &categoryDialog);
            categoryFormLayout->addRow(categoryButtonBox);

            // Obsługa przycisków akcji
            connect(categoryButtonBox, &QDialogButtonBox::accepted, &categoryDialog, &QDialog::accept);

            connect(categoryButtonBox, &QDialogButtonBox::rejected, &categoryDialog, &QDialog::reject);



            // Wyświetlanie okna dialogowego i oczekiwanie na zakończenie
            if (categoryDialog.exec() == QDialog::Accepted)
            {
                // Dodanie nowej kategorii do listy
                QString categoryName = categoryNameLineEdit->text();
                categoryComboBox->addItem(categoryName);
                updateCategoryList(categoryName);
            }
        });

    formLayout->addRow("Kategoria", categoryComboBox);
    formLayout->addRow(addCategoryButton);

    // Wpis kwoty
    QDoubleSpinBox* amountSpinBox = new QDoubleSpinBox();
    amountSpinBox->setRange(0, 1000000);
    amountSpinBox->setMinimum(0.01);
    amountSpinBox->setDecimals(2);
    formLayout->addRow("Kwota", amountSpinBox);

    // Przyciski akcji
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
    formLayout->addRow(buttonBox);
    // Obsługa przycisków akcji
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);




    // Wyświetlanie okna dialogowego i oczekiwanie na zakończenie
    if (dialog.exec() == QDialog::Accepted)
    {

        // Pobranie wartości z formularza
        QString type = typeComboBox->currentText();
        QString title = titleLineEdit->text();
        QString category = categoryComboBox->currentText();
        double amount = amountSpinBox->value();
        QString date = QDate::currentDate().toString("dd.MM.yyyy");

// Aktualizacja bilansu przychodów
if (type == "Przychód") {
    plusBalance += amount;
    balance = plusBalance - minusBalance;
    homeBalance += balance;
    ui->labelPlusBalance->setText(QString::number(plusBalance, 'f', 2) + " zł");
}
else
{
    minusBalance += amount;
    balance = plusBalance - minusBalance;
    homeBalance += balance;
    ui->labelMinusBalance->setText(QString::number(minusBalance, 'f', 2) + " zł");
}

ui->labelBalance->setText(QString::number(balance, 'f', 2) + " zł");

    ui->labelHomeBalance->setText(QString::number(homeBalance, 'f', 2) + " zł");

if (title.isEmpty() || category == "Wybierz kategorię") {
            buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    }
    else {
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

        // Dodanie nowego wiersza do tabeli
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->insertRow(row);
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(type));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(title));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(category));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(amount, 'f', 2)));
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(date));
    }
}

}


void MainWindow::removeTask()
{
    QListWidgetItem* selectedItem = ui->listWidget->currentItem();
    if (selectedItem)
    {
        ui->listWidget->takeItem(ui->listWidget->row(selectedItem));
        delete selectedItem;
    }
}

void MainWindow::editTask()
{
    // Pobierz wybrane zadanie z listy
    QListWidgetItem* currentItem = ui->listWidget->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "Błąd", "Nie wybrano zadania do edycji.");
        return;
    }
    QString currentTaskName = currentItem->text();
    QString currentTaskDate = currentItem->data(Qt::UserRole).toString();

    // Utwórz okno dialogowe do edycji zadania
    QDialog dialog(this);
    dialog.setWindowTitle("Edytuj zadanie");

    // Utwórz pola tekstowe
    QLabel* labelTaskName = new QLabel(&dialog);
    labelTaskName->setText("Nazwa zadania:");
    QLineEdit* lineEditTaskName = new QLineEdit(&dialog);
    lineEditTaskName->setText(currentTaskName);

    QLabel* labelTaskDate = new QLabel(&dialog);
    labelTaskDate->setText("Data zadania:");
    QDateEdit* dateEditTaskDate = new QDateEdit(QDate::currentDate(), &dialog);

    dateEditTaskDate->setCalendarPopup(true);

    // Utwórz przyciski
    QPushButton* buttonOk = new QPushButton(&dialog);
    buttonOk->setText("OK");
    QPushButton* buttonCancel = new QPushButton(&dialog);
    buttonCancel->setText("Anuluj");

    // Ustaw layout
    QGridLayout* layout = new QGridLayout(&dialog);
    layout->addWidget(labelTaskName, 0, 0);
    layout->addWidget(lineEditTaskName, 0, 1);
    layout->addWidget(labelTaskDate, 1, 0);
    layout->addWidget(dateEditTaskDate, 1, 1);
    layout->addWidget(buttonOk, 2, 0);
    layout->addWidget(buttonCancel, 2, 1);
    dialog.setLayout(layout);

    // Połącz przyciski z slotami
    connect(buttonOk, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(buttonCancel, &QPushButton::clicked, &dialog, &QDialog::reject);

    // Wyświetl okno dialogowe i przetwórz wynik
    if (dialog.exec() == QDialog::Accepted) {
        QString newTaskName = lineEditTaskName->text();
        QString newTaskDate = dateEditTaskDate->date().toString("dd/MM/yyyy");

        // Zaktualizuj wybrane zadanie w liście
        currentItem->setText(newTaskName);
        currentItem->setData(Qt::UserRole, newTaskDate);

        // Dodanie zadania do kalendarza

    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // Zapisanie listy zadań do bazy danych przed zamknięciem programu
    saveTasksToDatabase();
    event->accept();
}

void MainWindow::on_pushButtonCategories_clicked()
{
    // Tworzenie okna dialogowego
    QDialog dialog(this);
    dialog.setWindowTitle("Lista kategorii");
    dialog.setFixedSize(200, 150);

    // Tworzenie widgetu z listą kategorii
    QListWidget* categoryListWidget = new QListWidget(&dialog);
    categoryListWidget->addItems(categoryList);

    // Przycisk "OK"
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);

    // Układanie widgetów na formularzu
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->addWidget(categoryListWidget);
    layout->addWidget(buttonBox);

    // Wyświetlanie okna dialogowego i oczekiwanie na zakończenie
    dialog.exec();
}
void MainWindow::updateCategoryList(const QString& categoryName)
{
    if (!categoryList.contains(categoryName)) {
        categoryList.append(categoryName);
    }
}
void MainWindow::on_stackedWidget_currentChanged(int index)
{
    if (ui->stackedWidget->currentWidget()->objectName() == "pageFinances") {


        QPieSeries *series = new QPieSeries();
        series->append("Przychody", plusBalance);
        series->append("Wydatki", minusBalance);

        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Finances");

        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);

        ui->verticalLayout_3->addWidget(chartView,0,0);  // Dodajemy wykres do layoutu}
        }
}

void MainWindow::on_pushButtonEditFinance_clicked()
{
        // Get the currently selected row
        int row = ui->tableWidget->currentRow();
        if (row == -1) {
        return;
        }

        // Retrieve the values from the selected row
        QTableWidgetItem* typeItem = ui->tableWidget->item(row, 0);
        QTableWidgetItem* titleItem = ui->tableWidget->item(row, 1);
        QTableWidgetItem* categoryItem = ui->tableWidget->item(row, 2);
        QTableWidgetItem* amountItem = ui->tableWidget->item(row, 3);

        QString type = typeItem->text();
        QString title = titleItem->text();
        QString category = categoryItem->text();
        double amount = amountItem->text().toDouble();

        // Create the dialog and set its fields to the values of the selected row
        QDialog dialog(this);
        dialog.setWindowTitle("Edit");
        dialog.setFixedSize(210, 180);

        QWidget* formWidget = new QWidget(&dialog);
        QFormLayout* formLayout = new QFormLayout(formWidget);

        QComboBox* typeComboBox = new QComboBox();
        typeComboBox->addItem("Przychód");
            typeComboBox->addItem("Wydatek");
        typeComboBox->setCurrentText(type);
        formLayout->addRow("Rodzaj", typeComboBox);

        QLineEdit* titleLineEdit = new QLineEdit();
        titleLineEdit->setText(title);
        formLayout->addRow("Tytuł", titleLineEdit);

            QComboBox* categoryComboBox = new QComboBox();
        for (const QString& category : categoryList) {
        categoryComboBox->addItem(category);
        }
        categoryComboBox->setCurrentText(category);
        formLayout->addRow("Kategoria", categoryComboBox);

        QDoubleSpinBox* amountSpinBox = new QDoubleSpinBox();
        amountSpinBox->setRange(0, 1000000);
        amountSpinBox->setMinimum(0.01);
        amountSpinBox->setDecimals(2);
        amountSpinBox->setValue(amount);
        formLayout->addRow("Kwota", amountSpinBox);

        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
        formLayout->addRow(buttonBox);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted)
        {
        // Update the selected row with the new values
        QString newType = typeComboBox->currentText();
        QString newTitle = titleLineEdit->text();
        QString newCategory = categoryComboBox->currentText();
        double newAmount = amountSpinBox->value();

        typeItem->setText(newType);
        titleItem->setText(newTitle);
        categoryItem->setText(newCategory);
        amountItem->setText(QString::number(newAmount, 'f', 2));

        // Update the balances
        if (newType == "Przychód") {
        plusBalance = plusBalance - amount + newAmount;
        balance = plusBalance - minusBalance;
        homeBalance = balance + startHomeBalance;
        ui->labelPlusBalance->setText(QString::number(plusBalance, 'f', 2) + " zł");
        } else
        {
        minusBalance = minusBalance - amount + newAmount;
        balance = plusBalance - minusBalance;
        homeBalance = balance + startHomeBalance;
        ui->labelMinusBalance->setText(QString::number(minusBalance, 'f', 2) + " zł");
        }

        ui->labelBalance->setText(QString::number(balance, 'f', 2) + " zł");

            ui->labelHomeBalance->setText(QString::number(homeBalance, 'f', 2) + " zł");

        if (title.isEmpty() || category == "Wybierz kategorię") {
            buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        }
        else {
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);

        // Dodanie nowego wiersza do tabeli
        int row = ui->tableWidget->currentRow();
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(type));
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(title));
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(category));
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(newAmount, 'f', 2)));
        }
}

}

void MainWindow::on_pushButtonStartHomeBalance_clicked()
{
bool ok;
double value = QInputDialog::getDouble(this, tr("Start Home Balance"), tr("Enter the starting balance:"), 0, -1000000, 1000000, 2, &ok);
if (ok) {
        startHomeBalance = value;
        updateStartHomeBalance(value);
        ui->labelHomeBalance->setText(QString::number(startHomeBalance, 'f', 2));
}
}
void MainWindow::updateStartHomeBalance(double value)
{
homeBalance = startHomeBalance;
}



