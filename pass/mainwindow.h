#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QShortcut>

namespace Ui {
class MainWindow;
}


class NonEditTableColumnDelegate : public QItemDelegate //копипаст делегата чтобы заблочить столбец
{
    Q_OBJECT
public:
    NonEditTableColumnDelegate(QObject * parent = 0) : QItemDelegate(parent) {}
    virtual QWidget * createEditor ( QWidget *, const QStyleOptionViewItem &,
                                     const QModelIndex &) const
    {
        return 0;
    }
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    friend void elements(QSqlTableModel* query,Ui::MainWindow* ui);
    friend void beg(QSqlQuery* query,QSqlTableModel* model);
    ~MainWindow();

private slots:
    void on_ent_clicked();

    void on_del_clicked();

    void author();

    void on_tableView_doubleClicked(const QModelIndex &index);

    void password_changing();

    void slotShortcutCtrlF();

    void del();

private:
    Ui::MainWindow *ui;
    QSqlDatabase m_db;
    QSqlQuery* query;
    QSqlTableModel* model;
    QShortcut *keyCtrlF;
};

#endif // MAINWINDOW_H
