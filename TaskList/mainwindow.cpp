#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->boxGrade->hide();
    ui->inTask->hide();

    connect(ui->gradeTask, &QPushButton::clicked, this, &MainWindow::on_gradeTask_clicked);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("myDB.sqlite");

    if (!db.open()) {
        qDebug() << "Не удалось подключиться к базе данных. Попытка создать новую базу данных...";
        db.setDatabaseName("./myDB.sqlite");
        if (!db.open()) {
            qDebug() << "Не удалось создать или подключиться к базе данных.";
            return;
        }

        QSqlQuery query;
        if (!query.exec("CREATE TABLE IF NOT EXISTS task (Name TEXT)")) {
            qDebug() << "Ошибка при создании таблицы 'task':" << query.lastError().text();
            return;
        }

        qDebug() << "База данных успешно создана и подключена.";
    } else {
        qDebug() << "База данных успешно подключена.";
        if (db.tables().contains("task")) {
            qDebug() << "Таблица 'task' существует в базе данных.";
        } else {
            qDebug() << "Таблица 'task' не существует в базе данных. Попытка создать...";
            QSqlQuery query;
            if (!query.exec("CREATE TABLE task (Name TEXT)")) {
                qDebug() << "Ошибка при создании таблицы 'task':" << query.lastError().text();
            } else {
                qDebug() << "Таблица 'task' успешно создана.";
            }
        }
    }

    loadTasks();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_addTask_clicked()
{
    QLineEdit *lineEdit = new QLineEdit(this);
    lineEdit->setStyleSheet(ui->inTask->styleSheet());
    lineEdit->setFixedHeight(ui->inTask->height());
    lineEdit->setAlignment(Qt::AlignCenter);

    QWidget *boxGradeLayout = new QWidget(this);
    QVBoxLayout *boxGradeLayoutInner = new QVBoxLayout();
    QPushButton *button = new QPushButton(this);

    boxGradeLayout->setStyleSheet(ui->boxGrade->styleSheet());
    boxGradeLayout->setLayout(boxGradeLayoutInner);
    boxGradeLayout->setFixedHeight(ui->boxGrade->height());
    boxGradeLayoutInner->addWidget(button);

    button->setFixedHeight(ui->gradeTask->height());
    button->setFixedWidth(ui->gradeTask->width());
    button->setStyleSheet(ui->gradeTask->styleSheet());

    QScrollArea *wScroll = ui->scroll;
    QWidget *scrollWidget = wScroll->widget();

    QVBoxLayout *scrollLayout = qobject_cast<QVBoxLayout*>(scrollWidget->layout());
    scrollLayout->setAlignment(Qt::AlignTop);

    if (!scrollWidget) {
        scrollWidget = new QWidget(wScroll);
        wScroll->setWidget(scrollWidget);
    }

    if (!scrollLayout) {
        scrollLayout = new QVBoxLayout(scrollWidget);
        scrollWidget->setLayout(scrollLayout);
    }

    QHBoxLayout *rowLayout = new QHBoxLayout();
    rowLayout->addWidget(boxGradeLayout);
    rowLayout->addWidget(lineEdit);
    scrollLayout->addLayout(rowLayout);

    connect(button, &QPushButton::clicked, this, [scrollLayout, rowLayout]() {
        QLayoutItem *item;
        while ((item = rowLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        scrollLayout->removeItem(rowLayout);
        delete rowLayout;
    });

    if (scrollLayout->count() > 10) {
        wScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

    wScroll->verticalScrollBar()->setStyleSheet("QScrollBar:vertical { background: #CCCCCC; width: 1px; }"
                                                "QScrollBar::handle:vertical { background: #999999; }"
                                                "QScrollBar::add-line:vertical { background: none; }"
                                                "QScrollBar::sub-line:vertical { background: none; }");
}

void MainWindow::on_showOn_clicked()
{
    if (originalWidgetWidth == 0) {
        originalWidgetWidth = ui->widget->width();
        originalScrollWidth = ui->scroll->width();
    }

    int currentWidth = ui->widget->width();
    int currentHeight = ui->widget->height();

    double percentIncrease = 2.3;

    if (currentWidth == originalWidgetWidth) {
        int newWidth = currentWidth * (1 + percentIncrease);
        ui->widget->resize(newWidth, currentHeight);
        int currentScrollWidth = ui->scroll->width();
        int newScrollWidth = currentScrollWidth + 620;
        ui->scroll->resize(newScrollWidth, ui->scroll->height());
    } else {
        ui->widget->resize(originalWidgetWidth, ui->widget->height());
        ui->scroll->resize(originalScrollWidth, ui->scroll->height());
        originalWidgetWidth = 0;
        originalScrollWidth = 0;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSqlDatabase::database().transaction();

    QSqlQuery deleteQuery;
    if (!deleteQuery.exec("DELETE FROM task")) {
        qDebug() << "Ошибка при удалении старых задач:" << deleteQuery.lastError().text();
    }

    QScrollArea *wScroll = ui->scroll;
    QWidget *scrollWidget = wScroll->widget();
    QVBoxLayout *scrollLayout = qobject_cast<QVBoxLayout*>(scrollWidget->layout());

    if (scrollLayout) {
        for (int i = 0; i < scrollLayout->count(); ++i) {
            QHBoxLayout *rowLayout = qobject_cast<QHBoxLayout*>(scrollLayout->itemAt(i)->layout());
            if (rowLayout) {
                for (int j = 0; j < rowLayout->count(); ++j) {
                    QWidget *widget = rowLayout->itemAt(j)->widget();
                    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widget)) {
                        QString taskName = lineEdit->text();
                        if (!taskName.isEmpty()) {
                            QSqlQuery query;
                            query.prepare("INSERT INTO task (Name) VALUES (:name)");
                            query.bindValue(":name", taskName);
                            if (!query.exec()) {
                                qDebug() << "Ошибка при добавлении задачи:" << query.lastError().text();
                            }
                        }
                    }
                }
            }
        }
    }

    QSqlDatabase::database().commit();
    QMainWindow::closeEvent(event);
}


void MainWindow::loadTasks()
{
    QSqlQuery query("SELECT Name FROM task");
    if (!query.exec()) {
        qDebug() << "Ошибка при выполнении запроса на загрузку задач:" << query.lastError().text();
        return;
    }

    QScrollArea *wScroll = ui->scroll;
    QWidget *scrollWidget = wScroll->widget();

    // Удаляем все существующие элементы из компоновки
    QVBoxLayout *scrollLayout = qobject_cast<QVBoxLayout*>(scrollWidget->layout());
    if (scrollLayout) {
        QLayoutItem *item;
        while ((item = scrollLayout->takeAt(0)) != nullptr) {
            delete item->widget();  // Удаляем виджет
            delete item;            // Удаляем элемент компоновки
        }
    } else {
        scrollLayout = new QVBoxLayout(scrollWidget);
        scrollWidget->setLayout(scrollLayout);
    }

    // Применяем стилизацию к вертикальной полосе прокрутки
    wScroll->verticalScrollBar()->setStyleSheet("QScrollBar:vertical { background: #CCCCCC; width: 1px; }"
                                                "QScrollBar::handle:vertical { background: #999999; }"
                                                "QScrollBar::add-line:vertical { background: none; }"
                                                "QScrollBar::sub-line:vertical { background: none; }");

    while (query.next()) {
        QString taskName = query.value(0).toString();

        QLineEdit *lineEdit = new QLineEdit(this);
        lineEdit->setText(taskName);
        lineEdit->setStyleSheet(ui->inTask->styleSheet());
        lineEdit->setFixedHeight(ui->inTask->height());
        lineEdit->setAlignment(Qt::AlignCenter);

        QWidget *boxGradeLayout = new QWidget(this);
        QVBoxLayout *boxGradeLayoutInner = new QVBoxLayout();
        QPushButton *button = new QPushButton(this);

        boxGradeLayout->setStyleSheet(ui->boxGrade->styleSheet());
        boxGradeLayout->setLayout(boxGradeLayoutInner);
        boxGradeLayout->setFixedHeight(ui->boxGrade->height());
        boxGradeLayoutInner->addWidget(button);

        button->setFixedHeight(ui->gradeTask->height());
        button->setFixedWidth(ui->gradeTask->width());
        button->setStyleSheet(ui->gradeTask->styleSheet());

        QHBoxLayout *rowLayout = new QHBoxLayout();
        rowLayout->addWidget(boxGradeLayout);
        rowLayout->addWidget(lineEdit);
        scrollLayout->addLayout(rowLayout);

        // Лямбда-функция для удаления rowLayout
        connect(button, &QPushButton::clicked, this, [scrollLayout, rowLayout]() {
            // Удаляем все виджеты внутри rowLayout
            QLayoutItem *item;
            while ((item = rowLayout->takeAt(0)) != nullptr) {
                delete item->widget();
                delete item;
            }
            // Удаляем сам rowLayout
            scrollLayout->removeItem(rowLayout);
            delete rowLayout;
        });
    }
}

void MainWindow::on_gradeTask_clicked()
{
    QScrollArea *wScroll = ui->scroll;
    QWidget *scrollWidget = wScroll->widget();
    QVBoxLayout *scrollLayout = qobject_cast<QVBoxLayout*>(scrollWidget->layout());

    if (scrollLayout) {
        for (int i = 0; i < scrollLayout->count(); ++i) {
            QHBoxLayout *rowLayout = qobject_cast<QHBoxLayout*>(scrollLayout->itemAt(i)->layout());
            if (rowLayout) {
                for (int j = 0; j < rowLayout->count(); ++j) {
                    QWidget *widget = rowLayout->itemAt(j)->widget();
                    if (QLineEdit *lineEdit = qobject_cast<QLineEdit*>(widget)) {
                        delete lineEdit;
                    }
                }
            }
        }
    }
}




