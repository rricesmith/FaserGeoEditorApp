//#ifndef GEOMODEL_FASERDBMAINWINDOW_H
//#define GEOMODEL_FASERDBMAINWINDOW_H 1
#pragma once
#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QDialog>
#include <QTableView>
#include <QtSql>
#include <QListWidget>


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
class FaserDbRelTableModel;

class FaserDbMainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QTreeView *m_treeView;
    QStandardItemModel *m_standardModel;
    QSqlDatabase m_database;
/*    QSqlTableModel *m_model;
    QPushButton *m_submitButton;
    QPushButton *m_revertButton;
    QPushButton *m_quitButton;
    QDialogButtonBox *m_buttonBox;*/
    FaserDbSecondWindow *m_secondWindow;
    QSqlTableModel *m_hvsNodeTableModel;
    QMenu *m_contextMenu;
    QMenu *m_subMenu;
    QAction *m_addBranch;
    QAction *m_addLeaf;
    QAction *m_createTag;
//    QAction *m_setRoot;
    QAction *m_lockTag;
    QString m_currentSelected;
    vector<string> m_errors;
    QListWidget *m_listWidget;

    void printErrors();
    void createActions();
    void createStatusBar();
    void setTable();
    void addBranch();
    void addLeaf();
    void tagAction(QAction *action);
    bool isBranch(QString name);
    void setRoot();

    void contextMenu(const QPoint &point);

//    string obtainNamePopup();

public:
    QSqlTableModel *m_tag2node;
    QSqlTableModel *m_tagcache;
    QSqlTableModel *m_ltag2ltag;

    QString m_rootDisplayTag;
    void initializeWindow();
    FaserDbMainWindow(QWidget *parent = nullptr);
    QList<QStandardItem*> prepareRow(  const QString& name1, const QString& name2, const QString& name3, const QString& name4, const QString& name5);
    void buildChildren( QList<QStandardItem*> *Row, QString parent_id);
    void setDatabase( QSqlDatabase *db);
//    void submit();
    QSqlDatabase returnDatabase();
    void rebuildTree();
    void buildListWidget();
    bool verifyDatabase();
    QString selectedRowName();
    void selectionChanged(const QItemSelection&, const QItemSelection&);
    void tagRowChanged(int currentRow);
    bool isLocked(QString tagId);
    QStringList findAssociatedList(QSqlTableModel *table, QString known, QString kvalue, QString search);
    QString findAssociated(QSqlTableModel *table, QString known, QString kvalue, QString search);
    QString createTag();
    QString createRootTag();
    void lockTag(QString toLock = QString());

    void errorMessage(string message);

    bool submitChanges(QSqlTableModel *table);



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
//    QSqlRelationalTableModel *m_tableModel;
    FaserDbRelTableModel *m_tableModel;

//    QSqlTableModel *m_tableModel; previously used, replaced with relational
    QPushButton *m_submitButton;
    QPushButton *m_revertButton;
    QPushButton *m_quitButton;
    QPushButton *m_addRowButton;
    QPushButton *m_addColumnButton;
    QPushButton *m_removeColumn;
    QPushButton *m_removeRow;
    QDialogButtonBox *m_buttonBox;

    QString m_rootTag;


public:
    QTableView *m_tableView;
    QSqlTableModel *m_tagTable;
    QSqlTableModel *m_sqlTableModel;
    FaserDbMainWindow *m_parentWindow;
    FaserDbSecondWindow( FaserDbMainWindow *window_parent, QWidget *parent = nullptr);
    ~FaserDbSecondWindow();
    void submit();
//    QSqlTableModel* tablePointer();
    void addRow();
    void addColumn();
    void removeColumn();
    void removeRow();
    void setWindow(QString tableName);
    void clearWindow();
    void setTagFilter(QString tagFilter);

    void click_cell(int row, int column);

//    Qt::ItemFlags flags(const QModelIndex &index) const;

};


class FaserDbRelTableModel : public QSqlRelationalTableModel
{
    private:
    FaserDbSecondWindow *m_secondWindow;
    public:
    FaserDbRelTableModel(QObject *parent, QSqlDatabase db, FaserDbSecondWindow *secondWin);
    QVariant data(const QModelIndex &idx, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
};


//Qt::ItemFlags QAbstractItemModel::flags(const QModelIndex &index) const;

// #endif