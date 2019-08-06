#include "FaserGeoEditorApp.h"
#include "FaserDbMainWindow.h"

#include <QTreeView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <iostream>
#include <QSqlTableModel>
#include <QTableView>
#include <QSqlRecord>
#include <QtWidgets>
#include <QtSql>
using namespace std;

FaserDbMainWindow::FaserDbMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_treeView(new QTreeView(this))
    , m_standardModel(new QStandardItemModel(this))
{
/*    setCentralWidget(m_treeView);

    QStandardItem* root = m_standardModel->invisibleRootItem();

    QSqlTableModel *m_model = new QSqlTableModel(nullptr, m_database);
    m_model->setTable("HVS_NODE");
    m_model->select();
    
    QList<QStandardItem*> firstRow;
    for( int i = 0; i < m_model->record(0).count(); i++)
    {
        QStandardItem *temp = new QStandardItem(m_model->record(0).value(i).toString());
        firstRow.push_back(temp);
    }
    
    root->appendRow(firstRow);
    QList<QStandardItem*> subRow = prepareRow( m_model->record(2).value(0).toString(), m_model->record(2).value(1).toString(), m_model->record(2).value(2).toString(), m_model->record(2).value(3).toString(), m_model->record(2).value(4).toString() );
    firstRow.first()->appendRow(subRow);*/

//    auto tables = m_database.tables();
//    for (auto it = tables.constBegin(); it != tables.constEnd(); ++it) std::cout << (*it).toLocal8Bit().constData() << std::endl;

/*    QList<QStandardItem*> firstRow = prepareRow("name 1", "leaf 1");
    root->appendRow(firstRow);
    QList<QStandardItem*> secondRow = prepareRow("name 2", "leaf 2");
    root->appendRow(secondRow);
    QList<QStandardItem*> subRow = prepareRow("subName 1", "subLeaf 1");
    firstRow.first()->appendRow(subRow);*/

 //   m_treeView->setModel(m_standardModel);
   // m_treeView->expandAll();
}

// FaserDbMainWindow::~FaserDbMainWindow()
// {
//     if (m_standardModel != nullptr) delete m_standardModel;
//     if (m_treeView != nullptr) delete m_treeView;
// }


QList<QStandardItem*> FaserDbMainWindow::prepareRow(  const QString& name1, const QString& name2, const QString& name3, const QString& name4, const QString& name5)
{
    return {    new QStandardItem(name1),
                new QStandardItem(name2),
                new QStandardItem(name3),
                new QStandardItem(name4),
                new QStandardItem(name5)};
}

/*
QList<QStandardItem*> FaserDbMainWindow::prepareRow(const QString& name, const QString& leaf) const
{
    return { new QStandardItem(name),
             new QStandardItem(leaf)};
}*/

void FaserDbMainWindow::setDatabase( QSqlDatabase *db)
{
    m_database = *db;
    m_hvsNodeTableModel = new QSqlTableModel(nullptr, m_database);
    m_hvsNodeTableModel->setTable("HVS_NODE");
    m_hvsNodeTableModel->select();
    return;
}

void FaserDbMainWindow::initializeWindow()
{
    QDockWidget *secondWid = new QDockWidget(tr("Data"), this);
    secondWid->setAllowedAreas(Qt::RightDockWidgetArea);


    m_secondWindow = new FaserDbSecondWindow(this, secondWid);
    setCentralWidget(m_treeView);

    secondWid->setWidget(m_secondWindow);
    addDockWidget(Qt::RightDockWidgetArea, secondWid);

    QStandardItem* root = m_standardModel->invisibleRootItem();

    setWindowTitle(tr("FaserGeoEditorApp"));
    createActions();
    createStatusBar();

//    connect( m_treeView, &FaserDbMainWindow::doubleClicked, this,  &FaserDbMainWindow::setTable);
    QSqlTableModel *model = m_hvsNodeTableModel;
//    QSqlTableModel *model = m_secondWindow->tablePointer();
/*    m_model = new QSqlTableModel(nullptr, m_database);
    m_model->setTable("HVS_NODE");
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_model->select();

    root->setData( tr("NODE_ID"), 1);
    model->setHeaderData(1, Qt::Horizontal, tr("NODE_NAME"));
    model->setHeaderData(2, Qt::Horizontal, tr("PARENT_ID"));
    model->setHeaderData(3, Qt::Horizontal, tr("BRANCH_FLAG"));
    model->setHeaderData(4, Qt::Horizontal, tr("CODE_COMMENT"));*/
    
    for(int i = 1; i < model->rowCount(); i++)
    {
        if(model->record(i).value("PARENT_ID").toInt() == 0)
        {
            QList<QStandardItem*> topRow;
            for(int j = 0; j < model->record(i).count(); j++)
            {
                QStandardItem *temp = new QStandardItem(model->record(i).value(j).toString());
                topRow.push_back(temp);
            }
            buildChildren(  &topRow, model->record(i).value("NODE_ID").toString());
            root->appendRow(topRow);
        }
    }

//Test code of editing from cached table example
/*    QTableView *view = new QTableView;
    view->setModel(m_standardModel);
    view->resizeColumnsToContents();

    m_submitButton = new QPushButton(tr("Submit"));
    m_submitButton->setDefault(true);
    m_revertButton = new QPushButton(tr("&Revert"));
    m_quitButton = new QPushButton(tr("Quit"));

    m_buttonBox = new QDialogButtonBox(Qt::Vertical);
    m_buttonBox->addButton(m_submitButton, QDialogButtonBox::ActionRole);
    m_buttonBox->addButton(m_revertButton, QDialogButtonBox::ActionRole);
    m_buttonBox->addButton(m_quitButton, QDialogButtonBox::RejectRole);

    connect(m_submitButton, &QPushButton::clicked, this,  &FaserDbMainWindow::submit);
    connect(m_revertButton, &QPushButton::clicked, m_model, &QSqlTableModel::revertAll);
    connect(m_quitButton, &QPushButton::clicked, this, &FaserDbMainWindow::close);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(view);
    mainLayout->addWidget(m_buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Edit Table"));

    view->show();*/
    
//    root->appendRow(firstRow);
//    QList<QStandardItem*> subRow = prepareRow( m_model->record(2).value(0).toString(), m_model->record(2).value(1).toString(), m_model->record(2).value(2).toString(), m_model->record(2).value(3).toString(), m_model->record(2).value(4).toString() );
//    firstRow.first()->appendRow(subRow);

    m_treeView->setModel(m_standardModel);
    m_treeView->expandAll();
    m_treeView->resizeColumnToContents(0);
    m_treeView->resizeColumnToContents(1);
    m_treeView->resizeColumnToContents(2);
    m_treeView->resizeColumnToContents(3);
    m_treeView->resizeColumnToContents(4);

    return;
}

void FaserDbMainWindow::setTable()
{
    m_secondWindow->setTable();
}

void FaserDbMainWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&Menu"));
//unused atm    QToolBar *fileToolBar = addToolBar(tr("Verify"));

    fileMenu->addSeparator();

    QAction *rebuildAct = fileMenu->addAction(tr("&ReBuild"), this, &FaserDbMainWindow::rebuildTree);
    rebuildAct->setStatusTip(tr("Rebuild tree based off of changes"));

    QAction *quitAct = fileMenu->addAction(tr("&Quit"), this, &QWidget::close);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));

    m_viewMenu = menuBar()->addMenu(tr("&View"));

}

QString FaserDbMainWindow::selectedRowName()
{
    int row = m_treeView->currentIndex().row();
    return m_treeView->model()->data( m_treeView->model()->index(row, 1)).toString();
}

void FaserDbMainWindow::rebuildTree()
{
    m_standardModel->clear();
    QStandardItem* root = m_standardModel->invisibleRootItem();

    m_hvsNodeTableModel->select();
    QSqlTableModel *model = m_hvsNodeTableModel;
//    QSqlTableModel *model = m_secondWindow->tablePointer();
    
    for(int i = 1; i < model->rowCount(); i++)
    {
        if(model->record(i).value("PARENT_ID").toInt() == 0)
        {
            QList<QStandardItem*> topRow;
            for(int j = 0; j < model->record(i).count(); j++)
            {
                QStandardItem *temp = new QStandardItem(model->record(i).value(j).toString());
                topRow.push_back(temp);
            }
            buildChildren(  &topRow, model->record(i).value("NODE_ID").toString());
            root->appendRow(topRow);
        }
    }
    
    m_treeView->expandAll();
    m_treeView->resizeColumnToContents(0);
    m_treeView->resizeColumnToContents(1);
    m_treeView->resizeColumnToContents(2);
    m_treeView->resizeColumnToContents(3);
    m_treeView->resizeColumnToContents(4);
    show();
}

void FaserDbMainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void FaserDbMainWindow::buildChildren( QList<QStandardItem*> *PRow, QString parent_id)
{   
    QSqlTableModel *model = m_hvsNodeTableModel;
//    QSqlTableModel *model = m_secondWindow->tablePointer();
    for(int i = 1; i < model->rowCount(); i++)
    {
        if(model->record(i).value("PARENT_ID").toString() == parent_id)
        {  
            QList<QStandardItem*> *newRow = new QList<QStandardItem*>;
            for(int j = 0; j < model->record(i).count(); j++)
            {
                QStandardItem *temp = new QStandardItem(model->record(i).value(j).toString());
                newRow->push_back(temp);
            }

            if(model->record(i).value("BRANCH_FLAG").toInt() == 1)
            {      
                buildChildren( newRow, model->record(i).value("NODE_ID").toString());
            }
            else
            {
                QString datastr = model->record(i).value("NODE_NAME").toString();
                datastr.append("_DATA");
                QString data2tagstr = model->record(i).value("NODE_NAME").toString();
                data2tagstr.append("_DATA2TAG");
                QList<QStandardItem*> dataRow = prepareRow(nullptr, datastr, nullptr, nullptr, nullptr );
                QList<QStandardItem*> data2tagRow = prepareRow(nullptr, data2tagstr, nullptr, nullptr, nullptr);

                newRow->first()->appendRow(dataRow);
                newRow->first()->appendRow(data2tagRow);
                
            }

            PRow->first()->appendRow(*newRow);
        }
    }
    return;
}

QSqlDatabase FaserDbMainWindow::returnDatabase()
{
    return m_database;
}

void FaserDbMainWindow::showAll()
{
    show();
    m_secondWindow->show();
    return;
}

void FaserDbSecondWindow::submit()
{
    m_tableModel->database().transaction();
    if(m_tableModel->submitAll())
    {
        m_tableModel->database().commit();
        m_parentWindow->rebuildTree();
    }
    else
    {
        m_tableModel->database().rollback();
        QMessageBox::warning(this, tr("Edit Table"), tr("The database reported an error: %1").arg(m_tableModel->lastError().text()));
    }
    
}

void FaserDbSecondWindow::addRow()
{
    if(!m_tableModel->insertRows(m_tableView->selectionModel()->currentIndex().row() + 1, 1))
    {
        //This executes only if there was no valid index, and then puts a new row at beginning
        m_tableModel->insertRows(0, 1);
    }
}

void FaserDbSecondWindow::addColumn()
{
    if(!m_tableModel->insertColumn(m_tableView->selectionModel()->currentIndex().column() + 1))
    {
        m_tableModel->insertColumn(0);
    }
}

void FaserDbSecondWindow::removeColumn()
{
    m_tableModel->removeColumns(m_tableView->selectionModel()->currentIndex().column(), 1);
}

void FaserDbSecondWindow::removeRow()
{
    m_tableModel->removeRows(m_tableView->selectionModel()->currentIndex().row(), 1);
}

FaserDbSecondWindow::FaserDbSecondWindow(FaserDbMainWindow *window_parent, QWidget* parent)
    : QWidget(parent)
    , m_tableModel(new QSqlTableModel(nullptr, window_parent->returnDatabase()))
   /* , m_standardModel(new QStandardItemModel(this))*/
{
//    setCentralWidget(m_tableModel);

    m_tableModel->setTable("HVS_NODE");
    m_tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    m_tableModel->select();

    m_tableModel->setHeaderData(0, Qt::Horizontal, tr("NODE_ID"));
    m_tableModel->setHeaderData(1, Qt::Horizontal, tr("NODE_NAME"));
    m_tableModel->setHeaderData(2, Qt::Horizontal, tr("PARENT_ID"));
    m_tableModel->setHeaderData(3, Qt::Horizontal, tr("BRANCH_FLAG"));
    m_tableModel->setHeaderData(4, Qt::Horizontal, tr("CODE_COMMENT"));    

    m_tableView = new QTableView;
    m_tableView->setModel(m_tableModel);
    m_tableView->resizeColumnsToContents();
    m_tableView->setMinimumWidth(500);

    m_submitButton = new QPushButton(tr("Submit"));
    m_submitButton->setDefault(true);
    m_revertButton = new QPushButton(tr("&Revert"));
    m_quitButton = new QPushButton(tr("Quit"));
    m_addRowButton = new QPushButton(tr("&Add Row"));
    m_addColumnButton = new QPushButton(tr("&Add Column"));
    m_removeColumn = new QPushButton(tr("&Remove Column"));
    m_removeRow = new QPushButton(tr("&Remove Row"));

    m_buttonBox = new QDialogButtonBox(Qt::Vertical);
    m_buttonBox->addButton(m_submitButton, QDialogButtonBox::ActionRole);
    m_buttonBox->addButton(m_revertButton, QDialogButtonBox::ActionRole);
    m_buttonBox->addButton(m_addRowButton, QDialogButtonBox::ActionRole);
    m_buttonBox->addButton(m_addColumnButton, QDialogButtonBox::ActionRole);
    m_buttonBox->addButton(m_removeColumn, QDialogButtonBox::ActionRole);
    m_buttonBox->addButton(m_removeRow, QDialogButtonBox::ActionRole);
    m_buttonBox->addButton(m_quitButton, QDialogButtonBox::RejectRole);

    connect(m_submitButton, &QPushButton::clicked, this,  &FaserDbSecondWindow::submit);
    connect(m_revertButton, &QPushButton::clicked, m_tableModel, &QSqlTableModel::revertAll);
    connect(m_addRowButton, &QPushButton::clicked, this,  &FaserDbSecondWindow::addRow);
    connect(m_addColumnButton, &QPushButton::clicked, this,  &FaserDbSecondWindow::addColumn);
    connect(m_removeColumn, &QPushButton::clicked, this,  &FaserDbSecondWindow::removeColumn);
    connect(m_removeRow, &QPushButton::clicked, this, &FaserDbSecondWindow::removeRow);
    connect(m_quitButton, &QPushButton::clicked, this, &FaserDbSecondWindow::close);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addWidget(m_tableView);
    mainLayout->addWidget(m_buttonBox);
    setLayout(mainLayout);

    setWindowTitle(tr("Edit Table"));


    return;
}

/*
QSqlTableModel* FaserDbSecondWindow::tablePointer()
{
    return m_tableModel;
}*/

void FaserDbSecondWindow::setTable()
{
    //Check if table is a _DATA or _DATA2TAG table
//    if( m_parentWindow->)
}

void FaserDbSecondWindow::initializeWindow()
{

}



