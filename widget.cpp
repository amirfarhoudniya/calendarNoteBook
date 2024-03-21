#include "widget.h"
#include "ui_widget.h"
#include<QMainWindow>
#include <QTextStream>
#include <QMessageBox>
#include <QTextCharFormat>

//dataBase init
QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");



Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //set icon to window
    this->setWindowIcon(QIcon(":/icons/icons/calendarNotebook.pro"));

    //set pointingHand cursor to radioButtons and pushButtons
    ui->save_pushButton->setCursor(Qt::PointingHandCursor);
    ui->priority_high->setCursor(Qt::PointingHandCursor);
    ui->priority_medium->setCursor(Qt::PointingHandCursor);
    ui->priority_low->setCursor(Qt::PointingHandCursor);

    //connect the calendar to update note function .. so when the date seleced ,the update note func would call .
    connect(ui->calendarWidget, &QCalendarWidget::selectionChanged, this, &Widget::updateNotes);

    //calendar format editing
    QTextCharFormat regularDayFormat;
    regularDayFormat.setForeground(Qt::black);
    ui->calendarWidget->setWeekdayTextFormat(Qt::Saturday, regularDayFormat);
    ui->calendarWidget->setWeekdayTextFormat(Qt::Sunday, regularDayFormat);
    ui->calendarWidget->setFirstDayOfWeek(Qt::Saturday);

    // Set text format for Fridays
    QTextCharFormat holidayFormat;
    holidayFormat.setForeground(Qt::red);
     ui->calendarWidget->setWeekdayTextFormat(Qt::Friday, holidayFormat);

    // Set the layout direction to right-to-left
     ui->calendarWidget->setLayoutDirection(Qt::RightToLeft);

    //set color to priority options
    ui->priority_low->setStyleSheet("color: green");
    ui->priority_medium->setStyleSheet("color:blue");
    ui->priority_high->setStyleSheet("color:red");

    // Get the current year
    int currentYear = QDate::currentDate().year();

    // Loop through each month of the year
    for (int month = 1; month <= 12; ++month) {
        // Set the current month of the calendar widget
        ui->calendarWidget->setCurrentPage(currentYear, month);

        // Loop through each day of the current month
        for (int day = 1; day <= QDate(currentYear, month, 1).daysInMonth(); ++day) {
            // Set the selected date to the current day
            ui->calendarWidget->setSelectedDate(QDate(currentYear, month, day));

            // Call the updateNotes() function for the current day
            updateNotes();
        }
    }

    // Set the selected date to today's date
    ui->calendarWidget->setSelectedDate(QDate::currentDate());

    // Call the updateNotes() function for the selected date
    updateNotes();

}

Widget::~Widget()
{
    delete ui;
}

void Widget::updateNotes() {

    ui->textBox->clear();
    db.setDatabaseName("db.db");
    db.open();
    QSqlQuery query ;
    QTextCharFormat format;


    //Find the selected date's note
    query.prepare("SELECT detail , priority FROM notes WHERE day = :day AND month = :month AND year = :year");
    query.bindValue(":day",ui->calendarWidget->selectedDate().day());
    query.bindValue(":month" , ui->calendarWidget->selectedDate().month());
    query.bindValue(":year" , ui->calendarWidget->selectedDate().year());

    if(query.exec() && query.next()){
        //find the detail of selected date and set to plainText
        QSqlRecord record = query.record();
        ui->textBox->setPlainText(record.value("detail").toString());

       //set color to date by priority level
        int priority = record.value("priority").toInt();
        if (priority == 1) {
            QBrush redBrush(Qt::red);
            format.setBackground(redBrush);
            ui->calendarWidget->setDateTextFormat(ui->calendarWidget->selectedDate(), format);
        }else if (priority == 2) {
            QBrush blueBrush(Qt::blue);
            format.setBackground(blueBrush);
            ui->calendarWidget->setDateTextFormat(ui->calendarWidget->selectedDate(), format);
        }else if (priority == 3) {
            QBrush greenBrush(Qt::green);
            format.setBackground(greenBrush);
            ui->calendarWidget->setDateTextFormat(ui->calendarWidget->selectedDate(), format);
        }
    }

}


void Widget::on_save_pushButton_clicked()
{
    db.setDatabaseName("db.db");
    db.open();

    //get the date and Data
    int day = ui->calendarWidget->selectedDate().day();
    int month = ui->calendarWidget->selectedDate().month();
    int year = ui->calendarWidget->selectedDate().year();
    int priority ;
    if ( ui->priority_high->isChecked()) {
        priority = 1 ;
    }else if ( ui->priority_medium->isChecked()) {
        priority = 2;
    }else {
        priority = 3;
    }
    QString txt = ui->textBox->toPlainText();

    //delete the last notes in selected date
    QSqlQuery query ;
    query.prepare("DELETE FROM notes WHERE year = :year AND month = :month AND day= :day");
    query.addBindValue(year);
    query.addBindValue(month);
    query.addBindValue(day);
    query.exec();

    //add or update the selected date's note
    query.prepare("INSERT INTO notes (year , month , day, detail , priority) VALUES (? , ? , ? , ? , ?)");
    query.addBindValue(year);
    query.addBindValue(month);
    query.addBindValue(day);
    query.addBindValue(txt);
    query.addBindValue(priority);
    query.exec();

    updateNotes();
}

