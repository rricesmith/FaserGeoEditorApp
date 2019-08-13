//#ifndef GEOMODEL_FASERDBMAINWINDOW_H
//#define GEOMODEL_FASERDBMAINWINDOW_H 1
#pragma once
#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QDialog>
#include <QTableView>


// #include <QtGui/QWidget>
using namespace std;

QT_BEGIN_NAMESPACE
class QTreeView; //forward declarations
class QStandardItemModel;
class QStandardItem;
class QPushButton;
class QDialogButtonBox;
QT_END_NAMESPACE


class FaserDbSecondWindow;

class FaserDbMainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QTreeView* m_treeView;
    QStandardItemModel* m_standardModel;
    QSqlDatabase m_database;
/*    QSqlTableModel *m_model;
    QPushButton *m_submitButton;
    QPushButton *m_revertButton;
    QPushButton *m_quitButton;
    QDialogButtonBox *m_buttonBox;*/
    FaserDbSecondWindow *m_secondWindow;
    QSqlTableModel *m_hvsNodeTableModel;
    QMenu *m_viewMenu;
    QMenu *m_contextMenu;
    QAction* m_addBranch;
    QAction* m_addLeaf;
    QString m_currentSelected;

    void createActions();
    void createStatusBar();
    void setTable();
    void addBranch();
    void addLeaf();

//    string obtainNamePopup();

public:

    void initializeWindow();
    FaserDbMainWindow(QWidget *parent = nullptr);
    QList<QStandardItem*> prepareRow(  const QString& name1, const QString& name2, const QString& name3, const QString& name4, const QString& name5);
    void buildChildren( QList<QStandardItem*> *Row, QString parent_id);
    void setDatabase( QSqlDatabase *db);
//    void submit();
    QSqlDatabase returnDatabase();
    void rebuildTree();
    QString selectedRowName();
    void selectionChanged(const QItemSelection&, const QItemSelection&);



    // virtual ~FaserDbMainWindow();
};

/*
class FaserDbPopup : public QWidget
{
    Q_OBJECT
private:

public:
    FaserDbPopup(FaserDbMainWindow *window_parent, QWidget *parent = nullptr);
};*/

class FaserDbSecondWindow : public QWidget
{
    Q_OBJECT
private:
    QSqlTableModel *m_tableModel;
    QPushButton *m_submitButton;
    QPushButton *m_revertButton;
    QPushButton *m_quitButton;
    QPushButton *m_addRowButton;
    QPushButton *m_addColumnButton;
    QPushButton *m_removeColumn;
    QPushButton *m_removeRow;
    QDialogButtonBox *m_buttonBox;
    FaserDbMainWindow *m_parentWindow;
    QTableView *m_tableView;

public:
    FaserDbSecondWindow( FaserDbMainWindow *window_parent, QWidget *parent = nullptr);
    void submit();
//    QSqlTableModel* tablePointer();
    void addRow();
    void addColumn();
    void removeColumn();
    void removeRow();
    void setWindow(QString tableName);
    void clearWindow();

};

// #endif