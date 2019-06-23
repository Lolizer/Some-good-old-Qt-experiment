#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QAction>
#include <QInputDialog>
#include <cstring>
#include <QShortcut>
#include <QSettings>
//#include <Python.h>

QMenu* par = 0;
QAction* delf = 0;
QSettings* settings = new QSettings("Bonch","parolka");

void PyFuncCalling()
{

}

void MainWindow::del()
{
    delete par->menuAction();
    par->removeAction(delf);
    model->setFilter("ID NOT LIKE 0");
    par = 0;
    delf = 0;
}

void MainWindow::slotShortcutCtrlF()
{
    bool bok;
    if(par != 0 || delf != 0)
    {
        delete par->menuAction();
        par->removeAction(delf);
    }
    QString urls = QInputDialog::getText(0,"Поиск по БД","Введите url:",QLineEdit::Normal,"",&bok);
    if(!bok)
    {
        par = 0;
        delf = 0;
        return;
    }
    QMenu* parolka;
    parolka = menuBar()->addMenu("&Ctrl+F");
    model->setFilter("ID NOT LIKE 0 AND url = '" + urls + tr("'"));
    ui->tableView->setModel(model);
    QAction* delfil = new QAction("&Убрать фильтр",this);
    parolka->addAction(delfil);
    connect(delfil,SIGNAL(triggered()),this,SLOT(del()));
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,"Фильтр","Оставить?",QMessageBox::Yes | QMessageBox::No);
    if(reply == QMessageBox::No)
    {
        model->setFilter("ID NOT LIKE 0");
        delete parolka->menuAction();
        parolka->removeAction(delfil);
        delf = 0;
        par = 0;
    }
    else
    {
        delf = delfil;
        par = parolka;
    }
}

void MainWindow::password_changing()
{
    QString bufpass;
    bool bok;
    do{
    bufpass = QInputDialog::getText(0,"Изменение пароля БД","Новый пароль:",QLineEdit::Password,"******",&bok).toUtf8();
    if(!bok)
    {
        return;
    }
    }while(bufpass.isEmpty());
    for(int i = 0; i < bufpass.size(); i++)
    {
        bufpass[i] = QChar(bufpass.at(i).unicode() ^ settings->value("userkey").toInt());
    }
    query->exec("UPDATE Passwords SET password = '" + bufpass + "' WHERE ID = 0;");
    settings->setValue("password",QVariant(bufpass));
}

void beg(QSqlQuery* query,QSqlTableModel* model)
{
    QString res;
    QString buflog;
    query->exec("SELECT COUNT(ID) AS id_count FROM Passwords;");
    query->next();
    QSqlRecord rec = query->record();
    int rows = query->value(rec.indexOf("id_count")).toInt();
    for(int i = 0; i < rows; i++)
    {
        res.resize(model->data(model->index(i,2)).toString().size());
        buflog = model->data(model->index(i,2)).toString().toUtf8();
        qDebug() << buflog;
        for(int j = 0; j < model->data(model->index(i,2)).toString().size(); j++)
        {
            res[j] = QChar(buflog.at(j).unicode() ^ settings->value("userkey").toInt()).unicode();
        }
        model->setData(model->index(i,2),QVariant(res));
    }
    query->clear();
}

void elements(QSqlQuery* query,Ui::MainWindow* ui)
{
    query->exec("SELECT COUNT(ID) AS id_count FROM Passwords WHERE ID NOT LIKE 0;");
    query->next();
    QSqlRecord rec = query->record();
    ui->elems->setText(query->value(rec.indexOf("id_count")).toString());
    query->clear();
}

void MainWindow::author()
{
    QMessageBox myBox;
    QSize picsize(150,300);
    QPixmap myPixmap(100,100);
    myPixmap.load("author.jpg");
    myPixmap = myPixmap.scaled(picsize,Qt::KeepAspectRatio);
    myBox.setIconPixmap(myPixmap);
    myBox.setWindowTitle("Подробнее");
    myBox.setText("Версия: 1.0.2\nАвтор: bonjour'ka\nРелиз: 04.10.2017");
    myBox.setStandardButtons(QMessageBox::Ok);
    myBox.setDefaultButton(QMessageBox::Ok);
    myBox.show();
    myBox.exec();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    int i;
    bool bok;
    QString ukey,skey;
    ui->setupUi(this);
    settings->setValue("userkey",QString("131").toInt());

    QMenu* parolka;
    keyCtrlF = new QShortcut(this);
    keyCtrlF->setKey(Qt::CTRL + Qt::Key_F);
    connect(keyCtrlF, SIGNAL(activated()), this, SLOT(slotShortcutCtrlF()));

    QAction* quit2 = new QAction("&Выход",this);
    QAction* about = new QAction("&О программе",this);
    QAction* pchange = new QAction("&Изменить пароль от БД",this);

    parolka = menuBar()->addMenu("&Меню");
    parolka->addAction(about);
    connect(about,SIGNAL(triggered()),this,SLOT(author()));

    parolka->addAction(pchange);
    connect(pchange,SIGNAL(triggered()),this,SLOT(password_changing()));

    parolka->addAction(quit2);
    connect(quit2,SIGNAL(triggered()),qApp,SLOT(quit()));

    qApp->addLibraryPath(QString("./plugins/"));
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    m_db.setDatabaseName("db");
    ui->tableView->setItemDelegateForColumn(3,new NonEditTableColumnDelegate());
    query = new QSqlQuery(m_db);
    if(!m_db.open())
    {
        QMessageBox::warning(0,"Ошибка","База данных не была открыта");
        exit(EXIT_FAILURE);
    }
    if(!m_db.tables().contains("Passwords"))
    {
        do{
        ukey = QInputDialog::getText(0,"Создайте пароль для вашей базы","Пароль:",QLineEdit::Password,"******",&bok).toUtf8();
        if(!bok)
        {
            exit(EXIT_SUCCESS);
        }
        }while(ukey.isEmpty());
        skey.resize(ukey.size());
        query->exec("CREATE TABLE Passwords(ID INTEGER PRIMARY KEY, url VARCHAR,login VARCHAR, password VARCHAR, comment VARCHAR);");
        query->clear();
        for(i = 0; i < ukey.size(); i++)
        {
            skey[i] = QChar(ukey.at(i).unicode() ^ settings->value("userkey").toInt()).unicode();
        }
        settings->setValue("password",QVariant(skey));
        query->exec(tr("INSERT INTO Passwords(ID,url,login,password,comment) VALUES (0,'NULL','NULL','") + skey + tr("','');"));
        query->clear();
    }
        query->exec("SELECT COUNT(ID) AS id_count FROM Passwords WHERE ID = 0;");
        query->next();
        QSqlRecord rec = query->record();
        if(query->value(rec.indexOf("id_count")).toInt() != 0)
        {
            skey.clear();
            ukey.clear();
            ukey = QInputDialog::getText(0,"Доступ","Пароль:",QLineEdit::Password,"******",&bok).toUtf8();
            skey.resize(ukey.size());
            if(!bok)
                exit(EXIT_SUCCESS);

            for(i = 0; i < ukey.size(); i++)
            {
                skey[i] = QChar(ukey.at(i).unicode() ^ settings->value("userkey").toInt()).unicode();
            }

            query->clear();
            query->exec("SELECT password AS ps FROM Passwords WHERE ID = 0;");
            query->next();
            rec = query->record();
            if(query->value(rec.indexOf("ps")).toString() == "")
            {
                QMessageBox::warning(0,"Ошибка входа","Зафиксирована попытка взлома.Доступ к базе запрещен.Обратитесь к автору программного обеспечения.");
                exit(EXIT_FAILURE);
            }
            if(query->value(rec.indexOf("ps")) != settings->value("password"))
            {
                QMessageBox::warning(0,"Ошибка входа","Зафиксирована попытка взлома или неправильного переноса на новое аппаратное обеспечение.Доступ к базе запрещен.Обратитесь к автору программы.");
                exit(EXIT_FAILURE);
            }
            if(skey != settings->value("password"))
            {
                QMessageBox::warning(0,"Неверный пароль","Доступ к базе запрещен");
                exit(EXIT_FAILURE);
            }
        }
        else exit(EXIT_FAILURE);
        model = new QSqlTableModel(this,m_db);
        model->setTable("Passwords");
        model->setFilter("ID NOT LIKE 0");
        model->select();
        model->setEditStrategy(QSqlTableModel::OnFieldChange);
        beg(query,model);
        model->select();
        ui->tableView->setModel(model);
        ui->tableView->setColumnHidden(0,true);
        elements(query,ui);
}

MainWindow::~MainWindow()
{
    beg(query,model);
    delete ui;
    delete query;
    delete model;
}

void Pix(bool flag)
{
    QMessageBox myBox;
    QSize picsize(100,100);
    QPixmap myPixmap(100,100);
    myPixmap.load("hand.jpg");
    myPixmap = myPixmap.scaled(picsize,Qt::KeepAspectRatio);
    myBox.setIconPixmap(myPixmap);
    flag ? myBox.setText("Все поля должны быть заполнены!"):myBox.setText("Заполните поле \"Site/Resource\"");
    myBox.setWindowTitle("Паролька");
    myBox.setStandardButtons(QMessageBox::Ok);
    myBox.setDefaultButton(QMessageBox::Ok);
    myBox.show();
    myBox.exec();
}

void MainWindow::on_ent_clicked()
{
    int i = 0;
    QString respass,bufpass;
    if(ui->login->text().isEmpty() || ui->password->text().isEmpty() || ui->url->text().isEmpty())
    {
        Pix(true);
        return;
    }
    bufpass = ui->password->text().toUtf8();
    respass.resize(bufpass.size());
    query->clear();
    query->exec("SELECT COUNT(ID) AS id_count FROM Passwords;");
    query->next();
    QSqlRecord rec = query->record();
    int counter = query->value(rec.indexOf("id_count")).toInt();
    query->clear();

    for(i = 0; i < bufpass.size(); i++)
    {
        respass[i] = QChar(bufpass.at(i).unicode() ^ settings->value("userkey").toInt());
    }

    QString buf = tr("INSERT INTO Passwords(ID,url,login,password,comment)VALUES(") + QString().setNum(counter + 1) + tr(",'")+ ui->url->text() + tr("','") + ui->login->text() + tr("','") + respass + tr("','');");
    query->exec(buf);
    model->select();
    ui->tableView->setModel(model);
    ui->tableView->setColumnHidden(0,true);
    elements(query,ui);
    ui->url->setText("");
    ui->login->setText("");
    ui->password->setText("");
    query->clear();
}

void MainWindow::on_del_clicked()
{
    bool bok = 1;
    QString s = ui->url->text();
    if(ui->url->text().isEmpty())
    {
        Pix(false);
        return;
    }
    //этот код позволит получить количество записей из бд в код в числовом значении, взят копипастом выше
    query->exec("SELECT COUNT(ID) AS id_count FROM Passwords;");
    query->next();
    QSqlRecord rec = query->record();
    int counter = query->value(rec.indexOf("id_count")).toInt();
    query->clear();

    query->exec("SELECT COUNT(url) AS url_count FROM Passwords WHERE url = '" + s + "';");//
    query->next();
    QSqlRecord buf = query->record();
    int urls = query->value(buf.indexOf("url_count")).toInt();
    query->clear();

    if(urls != 1)
    {
        if(urls == 0)
        {
            QMessageBox::warning(0,"Неточность","Записей не найдено");
            return;
        }
        model->setFilter("url = '" + s + tr("'"));
        model->select();
        ui->tableView->setModel(model);
        ui->tableView->setColumnHidden(0,true);
        query->clear();
        QString str = QInputDialog::getText(0,"Неточность","Укажите номер удаляемой записи:",QLineEdit::Normal,"",&bok);
        model->removeRow(str.toInt() - 1);
        query->exec("VACUUM Passwords");
        query->clear();
        model->select();
        ui->tableView->setModel(model);
        query->clear();
    }
    else
    {
        query->exec("DELETE FROM Passwords WHERE url = '" + s + tr("';"));
        query->clear();
        query->exec("VACUUM Passwords");
        query->clear();
    }
    if(!bok)
    {
        model->setFilter("ID NOT LIKE 0");
        model->select();
        ui->tableView->setModel(model);
        return;
    }
        int id = 1;

        for(int i = 1; i <= counter; i++)
        {
            query->exec("SELECT COUNT(ID) AS id_count FROM Passwords WHERE ID = " + QString().setNum(i) + tr(";"));
            query->next();
            rec = query->record();
            if(query->value(rec.indexOf("id_count")).toInt() == 0)
            {
                continue;
            }
            query->clear();

            query->exec("UPDATE Passwords SET id = " + QString().setNum(id) + tr(" WHERE ID = ") + QString().setNum(i));

            id++; //индексируем новые айдишники

            query->clear();
        }
    model->setFilter("ID NOT LIKE 0");;
    model->select();
    ui->tableView->setModel(model);
    ui->tableView->setColumnHidden(0,true);
    elements(query,ui);
    ui->url->setText("");
    ui->login->setText("");
    ui->password->setText("");
}

void MainWindow::on_tableView_doubleClicked(const QModelIndex &index)
{
    bool bok;
    int i;
    if(index.column() == 3)
    {
        QString bufpass = ui->tableView->model()->data(index).toString();
        QString pass(bufpass.size());
        for(i = 0; i < bufpass.size(); i++)
        {
            pass[i] = QChar(bufpass.at(i).unicode() ^ settings->value("userkey").toInt());
        }
        QString login = model->data(model->index(index.row(),2)).toString();
        QMessageBox passBox;
        passBox.setWindowTitle("Расшифрованный пароль");
        passBox.setText("Логин: " + login);
        passBox.setInformativeText("Пароль: " + pass);
        passBox.setStandardButtons(QMessageBox::Ok);
        QPushButton* change = passBox.addButton(tr("Изменить пароль"),QMessageBox::ActionRole);
        passBox.setDefaultButton(QMessageBox::Ok);
        passBox.show();
        passBox.exec();
        if(passBox.clickedButton() == change)
        {
            bufpass.clear();
            bufpass = QInputDialog::getText(0,"Изменение пароля","Новый пароль:",QLineEdit::Password,"******",&bok).toUtf8();
            if(!bok)
            {
                return;
            }
            for(i = 0; i < bufpass.size(); i++)
            {
                bufpass[i] = QChar(bufpass.at(i).unicode() ^ settings->value("userkey").toInt());
            }
            model->setData(index,QVariant(bufpass));
        }
    }
}
