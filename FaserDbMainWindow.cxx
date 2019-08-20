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
#include <QVariant>
#include <QSqlField>
#include <vector>

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
    //Build second window
    QDockWidget *secondWid = new QDockWidget(tr("Data"), this);
    secondWid->setAllowedAreas(Qt::RightDockWidgetArea);
    m_secondWindow = new FaserDbSecondWindow(this, secondWid);
    secondWid->setWidget(m_secondWindow);
    addDockWidget(Qt::RightDockWidgetArea, secondWid);

    setCentralWidget(m_treeView);

    //This code sets the right click menu for the tree
    m_contextMenu = new QMenu(m_treeView);
    m_treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(m_treeView, &QWidget::customContextMenuRequested, this, &FaserDbMainWindow::contextMenu);
    m_addBranch = new QAction("Add Branch", m_contextMenu);
    m_addLeaf = new QAction("Add Leaf", m_contextMenu);
    m_contextMenu->addAction(m_addBranch);
    m_contextMenu->addAction(m_addLeaf);
    connect(m_addBranch, &QAction::triggered, this, &FaserDbMainWindow::addBranch);
    connect(m_addLeaf, &QAction::triggered, this, &FaserDbMainWindow::addLeaf);
    m_contextMenu->addSeparator();
    //Add submenu stuff
    m_subMenu = m_contextMenu->addMenu("Tags");
/*    m_testTag1 = new QAction("Tag stuff", m_subMenu);
    m_testTag2 = new QAction("more tag?", m_subMenu);
    m_subMenu->addAction(m_testTag1);
    m_subMenu->addAction(m_testTag2);
    connect(m_testTag1, &QAction::triggered, this, &FaserDbMainWindow::testTag1);
    connect(m_testTag2, &QAction::triggered, this, &FaserDbMainWindow::testTag2);*/

    QStandardItem* root = m_standardModel->invisibleRootItem();

    setWindowTitle(tr("FaserGeoEditorApp"));
    createActions();
    createStatusBar();

/*    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FaserDbMainWindow::selectionChanged);*/
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
            for(int j = 0; j < 2; j++)
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

//    m_treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FaserDbMainWindow::selectionChanged);

    return;
}

//testing custom context menu
void FaserDbMainWindow::contextMenu(const QPoint &point)
{
    //First build a submenu action for each tag id and set its internal tag id data
    vector<QAction *> actions;
    if( m_currentSelected.endsWith("_DATA2TAG") )
    {
        QSqlTableModel model;
        model.setTable(m_currentSelected);
        model.select();
        vector<QString> tag_ids;
        for(int i = 0; i < model.rowCount(); i++)
        {   
            bool found = false;
            for(size_t j = 0; j < tag_ids.size(); j++)
            {
                if( model.record(i).value(0).toString() == tag_ids[j])
                {
                    found = true;
                    break;
                }
            }
            if( !found)
            {
                tag_ids.push_back(model.record(i).value(0).toString());
            }
        }
        for(size_t i = 0; i < tag_ids.size(); i++)
        {
            QAction * tempAct = new QAction(tag_ids[i], m_subMenu);
            actions.push_back(tempAct);
            tempAct->setData(tag_ids[i]);
            m_subMenu->addAction(tempAct);
        }
/*        m_testTag1 = new QAction("Tag stuff", m_subMenu);
        m_testTag2 = new QAction("more tag?", m_subMenu);
        m_testTag1->setData(tr("tag1\n"));
        m_testTag2->setData(tr("tag2\n"));
        m_subMenu->addAction(m_testTag1);
        m_subMenu->addAction(m_testTag2);*/

//        connect(m_testTag1, &QAction::triggered, this, &FaserDbMainWindow::testTag1);
//        connect(m_testTag2, &QAction::triggered, this, &FaserDbMainWindow::testTag2);
        connect(m_subMenu, &QMenu::triggered, this, &FaserDbMainWindow::testTagFunc);
    }

    //Call context menu for every case
    m_contextMenu->exec(m_treeView->viewport()->mapToGlobal(point));
    
    //Delete all actions we made and disconnect them for next use
    if( m_currentSelected.endsWith("_DATA2TAG") )
    {
        disconnect(m_subMenu, &QMenu::triggered, this, &FaserDbMainWindow::testTagFunc);

        for(size_t i = 0; i < actions.size(); i++)
        {
            delete actions[i];
        }
        m_subMenu->clear();
    }
}

void FaserDbMainWindow::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{   
    if(!selected.isEmpty())
    {
        m_currentSelected = selected.indexes().at(1).data(0).toString();
    }
    if(!selected.isEmpty() && (selected.indexes().at(1).data(0).toString().endsWith("_DATA") || selected.indexes().at(1).data(0).toString().endsWith("_DATA2TAG")))
    {
        m_secondWindow->hide();
        m_secondWindow->clearWindow();
        m_secondWindow->setWindow(selected.indexes().at(1).data(0).toString());
        m_secondWindow->show();
/*        for(int i = 0; i < 4; i++) at(i), i gives column num, data(j), j idk
        {
            string changedindex = selected.indexes().at(1).data(i).toString().toStdString();
            cout<<changedindex<<endl;
        }*/
    }
    if(deselected.isEmpty())
    {}
}

void FaserDbMainWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&Menu"));
//unused atm    QToolBar *fileToolBar = addToolBar(tr("Verify"));

    fileMenu->addSeparator();

    QAction *rebuildAct = fileMenu->addAction(tr("&ReBuild"), this, &FaserDbMainWindow::rebuildTree);
    rebuildAct->setStatusTip(tr("Rebuild tree based off of changes"));

    QAction *verifyAct = fileMenu->addAction(tr("&Verify Database"), this, &FaserDbMainWindow::verifyDatabase);
    verifyAct->setStatusTip(tr("Verify database adheres to standards"));

    QAction *quitAct = fileMenu->addAction(tr("&Quit"), this, &QWidget::close);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));

    m_viewMenu = menuBar()->addMenu(tr("&View"));

}

QString FaserDbMainWindow::selectedRowName()
{
//    int row = m_treeView->currentIndex().row();
//    return m_treeView->model()->data( m_treeView->model()->index(row, 1)).toString();
    return m_currentSelected;
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
            for(int j = 0; j < 2; j++)
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
    show();
}

bool FaserDbMainWindow::verifyDatabase()
{
    //First get a query of all tables in database
    QSqlTableModel masterTable;
    masterTable.setTable("sqlite_master");
    masterTable.select();
    bool nodeExists = false;
    bool ltag2ltagExists = false;
    bool tag2nodeExists = false;
    bool tag2cacheExists = false;
    vector<QString> masterTables;
    for(int i = 0; i < masterTable.rowCount(); i++)
    {
        QString tableName = masterTable.record(i).value(1).toString();
        nodeExists = nodeExists || (tableName.toStdString() == "HVS_NODE") ? true : false;
        ltag2ltagExists = nodeExists || (tableName.toStdString() == "HVS_LTAG2LTAG") ? true : false;
        tag2nodeExists = nodeExists || (tableName.toStdString() == "HVS_TAG2NODE") ? true : false;
        tag2cacheExists = nodeExists || (tableName.toStdString() == "HVS_TAG2CACHE") ? true : false;
        if(masterTable.record(i).value("type").toString() == "table")
        {
            masterTables.push_back(tableName);        
        }
    }
    if( !(ltag2ltagExists && nodeExists && tag2nodeExists && tag2cacheExists))
    {
        if(!nodeExists)
        {
            m_errors.push_back("HVS_NODE does not exist");
        }
        if(!ltag2ltagExists)
        {
            m_errors.push_back("HVS_LTAG2LTAG does not exist");
        }
        if(!tag2nodeExists)
        {
            m_errors.push_back("HVS_TAG2NODE does not exist");
        }
        if(!tag2cacheExists)
        {
            m_errors.push_back("HVS_TAG2CACHE does not exist");
        }
        printErrors();
        return false;
    }

    //Now set tables for hvs_node and hvs_tag2node
    QSqlTableModel hvsNodeTable;
    hvsNodeTable.setTable("HVS_NODE");
    hvsNodeTable.select();

    //First we will verify all node id's in hvs_node are unique
    vector<QString> node_ids;
    for(auto i = 0; i < hvsNodeTable.rowCount(); i++)
    {
        bool matched = false;
        for(size_t j = 0; j < node_ids.size() && !node_ids.empty() && !matched; j++)
        {
            if( hvsNodeTable.record(i).value("NODE_ID").toString() == node_ids[j])
            {
                string err = "Repeated NODE_ID in row ";
                err += hvsNodeTable.record(i).value("NODE_NAME").toString().toStdString();
                err += " of value ";
                err += hvsNodeTable.record(i).value("NODE_ID").toString().toStdString();
                matched = true;
                m_errors.push_back(err);
            }
        }
        if(!matched)
        {
            node_ids.push_back(hvsNodeTable.record(i).value("NODE_ID").toString());
        }
    }

    //Next we will verify all tag id's are unique, and that all node_ids are also in HVS_NODE
    vector<QString> tag_ids;
    QSqlTableModel hvsTag2NodeTable;
    hvsTag2NodeTable.setTable("HVS_TAG2NODE");
    hvsTag2NodeTable.select();

    for(int i = 0; i < hvsTag2NodeTable.rowCount(); i++)
    {
        QString curNodeId = hvsTag2NodeTable.record(i).value("NODE_ID").toString();
        //Case where node id is in tag2node but not hvs_node
        if( std::find(node_ids.begin(), node_ids.end(), curNodeId) == node_ids.end())
        {
            string err = "Node ID in HVS_TAG2NODE not found in HVS_NODE of ";
            err += curNodeId.toStdString();
            m_errors.push_back(err);
        }

        //Next part ensures all tag ids are unique
        QString curTagId = hvsTag2NodeTable.record(i).value("TAG_ID").toString();
        if( std::find(tag_ids.begin(), tag_ids.end(), curTagId) != tag_ids.end())
        {
            string err = "Repeated Tag ID in HVS_TAG2NODE of ";
            err += curTagId.toStdString();
            m_errors.push_back(err);
        }
        else
        {
            tag_ids.push_back(curTagId);
        }
    }
    //Now checking that all nodes in hvs_node are in hvs_tag2node
    for(size_t i = 0; i < node_ids.size(); i++)
    {
        bool found = false;
        for( int j = 0; j < hvsTag2NodeTable.rowCount() && !found; j++)
        {
            found = found || (node_ids[i] == hvsTag2NodeTable.record(j).value("NODE_ID").toString()) ? true : false;
        }
        if(!found)
        {
            string err = "Node ID in HVS_NODE not in HVS_TAG2NODE of ";
            err += node_ids[i].toStdString();
            m_errors.push_back(err);
        }
    }

    //Now we need to verify that HVS_NODE is built properly



    if(m_errors.empty())
    {
        return true;
    }
    else
    {
        printErrors();
        return false;
    }
    
}

void FaserDbMainWindow::printErrors()
{
    if(m_errors.empty())
    {
        cout<<"No m_errors found\n\n";
        return;
    }
    while(!m_errors.empty())
    {
        cout<<m_errors.back()<<endl;
        m_errors.pop_back();
    }
    return;

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
            for(int j = 0; j < 2; j++)
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

/*old popup
string FaserDbMainWindow::obtainNamePopup()
{
    FaserDbPopup newpopup(this);
    return "test";
}*/

void FaserDbMainWindow::addBranch()
{
    //Ensures you can't add branch to a leaf
    if( selectedRowName().endsWith("_DATA") || selectedRowName().endsWith("_DATA2TAG"))
    {
        cout<<"Cannot add branch to a non-hvs row\n";
        return;
    }
    int modelRow = 0;
    while( m_hvsNodeTableModel->record(modelRow).value("NODE_NAME") != selectedRowName())
    {
        modelRow++;
    }
    if(m_hvsNodeTableModel->record(modelRow).value("BRANCH_FLAG").toInt() == 0)
    {
        cout<<"Cannot add branch to a leaf node\n";
        return;
    }


    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("New Table Name"), QLineEdit::Normal, QDir::home().dirName(),&ok);
 
    int pint = m_hvsNodeTableModel->record(modelRow).value("NODE_ID").toInt();
    pint = pint * 10;
    int i = 0;
    while( i < m_hvsNodeTableModel->rowCount())
    {  
        if(pint == m_hvsNodeTableModel->record(i).value("NODE_ID").toInt())
        {
            pint++;
            i = -1;
        }
        i++;
    }

    int row = m_hvsNodeTableModel->rowCount();

    if( ok && !text.isEmpty())
    {   

        m_hvsNodeTableModel->insertRow( row);
        m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 0), pint);
        m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 1), text);
        m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 2), m_hvsNodeTableModel->record(modelRow).value("NODE_ID"));
        m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 3), 1);        
        
        m_hvsNodeTableModel->database().transaction();
        m_hvsNodeTableModel->submitAll();

        rebuildTree();
    }
    else
    {
        cout<<"Need valid table name\n";
    }
    
}

void FaserDbMainWindow::addLeaf()
{
    //First ensure we are adding to a proper leaf node
    if( selectedRowName().endsWith("_DATA") || selectedRowName().endsWith("_DATA2TAG"))
    {
        cout<<"Cannot add leaf to a non-hvs row\n";
        return;
    }
    int modelRow = 0;
    while( m_hvsNodeTableModel->record(modelRow).value("NODE_NAME") != selectedRowName())
    {
        modelRow++;
    }
    if(m_hvsNodeTableModel->record(modelRow).value("BRANCH_FLAG").toInt() == 0)
    {
        cout<<"Cannot add leaf to a leaf node\n";
        return;
    }

    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("New Table Name"), QLineEdit::Normal, QDir::home().dirName(),&ok);

    int pint = m_hvsNodeTableModel->record(modelRow).value("NODE_ID").toInt();
    pint = pint * 10;
    int i = 0;
    while( i < m_hvsNodeTableModel->rowCount())
    {  
        if(pint == m_hvsNodeTableModel->record(i).value("NODE_ID").toInt())
        {
            pint++;
            i = -1;
        }
        i++;
    }

    int row = m_hvsNodeTableModel->rowCount();

    if( ok && !text.isEmpty())
    {   
        //This inserts new row inside of of HVS_NODE
        m_hvsNodeTableModel->insertRow( row);
        m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 0), pint);
        m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 1), text);
        m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 2), m_hvsNodeTableModel->record(modelRow).value("NODE_ID"));
        m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 3), 0);        
        
        //This will create the _DATA and _DATA2TAG tables in the database
        QString dataname = text;
        QString tagname = text;
        dataname.append("_DATA");
        tagname.append("_DATA2TAG");

        QString dataquery;
        QString data2tagquery;
        QString dataidstring = text;
        dataidstring.append("_DATA_ID");
        QString tagidstring = text;
        tagidstring.append("_TAG_ID");
        dataquery = QString("CREATE TABLE IF NOT EXISTS %1 (%2 SLONGLONG UNIQUE)").arg(dataname).arg(dataidstring);
        data2tagquery = QString("CREATE TABLE IF NOT EXISTS %1(%2 SLONGLONG, %3 SLONGLONG)").arg(tagname).arg(tagidstring).arg(dataidstring);

        QSqlQuery querydata(m_database);
        querydata.prepare(dataquery);
        QSqlQuery querydata2tag(m_database);
        querydata2tag.prepare(data2tagquery);

        if( !querydata.exec())
        {
            qDebug() << querydata.lastError().text(); //error in building table
        }
        if( !querydata2tag.exec())
        {
            qDebug() << querydata2tag.lastError().text(); //error in building table
        }

        m_hvsNodeTableModel->database().transaction();
        m_hvsNodeTableModel->submitAll();

        rebuildTree();
    }
    else
    {
        cout<<"Invalid table name\n";
    }
    
}

void FaserDbMainWindow::testTagFunc(QAction *action)
{
    QString input = action->data().toString();
    cout<<input.toStdString()<<endl;
}



/*
FaserDbPopup::FaserDbPopup(FaserDbMainWindow *window_parent, QWidget *parent)
    :QWidget(parent)
{
    QLineEdit *editor = new QLineEdit(this);
    show();

}*/

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
    bool oktext;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("New Table Name"), QLineEdit::Normal, QDir::home().dirName(),&oktext);

    bool oktype;
    QStringList types;
    types <<tr("Integer") <<tr("Text")<<tr("Double")<<tr("SLongLong");
    QString type = QInputDialog::getItem(this, tr("QInputDialog::getItem()"), tr("Data Type:"), types, 0, true, &oktype);

    //Checks if we are inserting after a certain column, otherwise adds at beginning
/*    if(!m_tableModel->insertColumn(m_tableView->selectionModel()->currentIndex().column() + 1))
    {
        m_tableModel->insertColumn(0);
        m_tableModel->setHeaderData(0, Qt::Horizontal, text);
    }
    else
    {
        m_tableModel->setHeaderData(m_tableView->selectionModel()->currentIndex().column() + 1, Qt::Horizontal, text);
    }*/
    QString addcolstr = "ALTER TABLE ";
    addcolstr.append(m_parentWindow->selectedRowName());
    addcolstr.append(" ADD COLUMN ");
    addcolstr.append(text);
    addcolstr.append(" ");
    addcolstr.append(type);

    QSqlQuery addcolquery(m_parentWindow->returnDatabase());
    addcolquery.prepare(addcolstr);
    if(!addcolquery.exec())
    {
        qDebug() <<addcolquery.lastError().text();
    }

    QString tableName = m_parentWindow->selectedRowName();
    clearWindow();
    setWindow(tableName);    
    
}

void FaserDbSecondWindow::removeColumn()
{
//    cout<<"Cannot alter table with sqlite, need to implement function that creates a new table with col to remove gone, then rename that table to curren table name\n";
    QString rowname = m_parentWindow->selectedRowName();
    int currentColumnIndex = m_tableView->selectionModel()->currentIndex().column();
/*    QString remcolstr = "ALTER TABLE ";
    remcolstr.append(m_parentWindow->selectedRowName());
    remcolstr.append(" DROP COLUMN ");
    int currentcol = m_tableView->selectionModel()->currentIndex().column();
    remcolstr.append(m_tableModel->headerData(currentcol, Qt::Horizontal).toString());
    cout<<"Cannot alter table with sqlite, need to implement function that creates a new table with col to remove gone, then rename that table to curren table name\n";

    QSqlQuery remcolquery(m_parentWindow->returnDatabase());
    remcolquery.prepare(remcolstr);
    if(!remcolquery.exec())
    {
        qDebug() <<remcolquery.lastError().text();
    }*/
    
    //Following code is test of remaking table with column removed
    //This code begins the query string to make a temp table with the column removed
    QString buildTempStr = "CREATE TABLE temp_table(";
    QString insertTempStr = "INSERT INTO temp_table SELECT ";
    QString dropMainStr = "DROP TABLE ";
    dropMainStr.append(rowname);
    QString alterTempStr = "ALTER TABLE temp_table RENAME TO ";
    alterTempStr.append(rowname);


    //Following code gets names and data types of the database we are removing a column from
    QSqlDatabase db = m_parentWindow->returnDatabase();
    QString infoQryStr;
    infoQryStr = QString("PRAGMA table_info(%1)").arg(rowname);
    QSqlQuery tableInfoQuery(db);
    tableInfoQuery.prepare(infoQryStr);
    tableInfoQuery.exec();

    bool prevadded = false;
    while(tableInfoQuery.next())
    {
        QString name = tableInfoQuery.value("name").toString();
        QString type = tableInfoQuery.value("type").toString();
        if( name != m_tableModel->headerData(currentColumnIndex, Qt::Horizontal).toString())
        {
            if(prevadded)
            {
                buildTempStr.append(",");
                insertTempStr.append(",");
            }
            insertTempStr.append(name);
            buildTempStr.append(name);
            buildTempStr.append(" ");
            buildTempStr.append(type);
            prevadded = true;
        }        
    }
    buildTempStr.append(")");
    insertTempStr.append(" FROM ");
    insertTempStr.append(rowname);

    QSqlQuery buildTempQuery(buildTempStr, db);
    QSqlQuery insertTempQuery(insertTempStr, db);
    QSqlQuery dropMainQuery(dropMainStr, db);
    QSqlQuery alterTempQuery(alterTempStr, db);

    clearWindow();
    setWindow(rowname);

}

void FaserDbSecondWindow::removeRow()
{
    m_tableModel->removeRows(m_tableView->selectionModel()->currentIndex().row(), 1);
}

FaserDbSecondWindow::FaserDbSecondWindow(FaserDbMainWindow *window_parent, QWidget* parent)
    : QWidget(parent)
    , m_tableModel(new QSqlRelationalTableModel(nullptr, window_parent->returnDatabase()))
   /* , m_standardModel(new QStandardItemModel(this))*/
{
//    setCentralWidget(m_tableModel);
    m_tableView = nullptr;
    m_parentWindow = window_parent;

/*    m_tableModel->setTable("HVS_NODE");
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

    setWindowTitle(tr("Edit Table"));*/


    return;
}

/*
QSqlTableModel* FaserDbSecondWindow::tablePointer()
{
    return m_tableModel;
}*/

void FaserDbSecondWindow::setWindow(QString tableName)
{
    if(!(tableName.endsWith("_DATA") || tableName.endsWith("_DATA2TAG")))
    {
        return;
    }
    m_tableModel->setTable(tableName);
    m_tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    if( tableName.endsWith("_DATA2TAG") )
    {
        m_tableModel->setRelation(0, QSqlRelation("HVS_TAG2NODE", "TAG_ID", "TAG_NAME"));
    }

    m_tableModel->select();

/*    m_tableModel->setHeaderData(0, Qt::Horizontal, tr("NODE_ID"));
    m_tableModel->setHeaderData(1, Qt::Horizontal, tr("NODE_NAME"));
    m_tableModel->setHeaderData(2, Qt::Horizontal, tr("PARENT_ID"));
    m_tableModel->setHeaderData(3, Qt::Horizontal, tr("BRANCH_FLAG"));
    m_tableModel->setHeaderData(4, Qt::Horizontal, tr("CODE_COMMENT"));    */

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

    setWindowTitle(tableName);


}

void FaserDbSecondWindow::clearWindow()
{
    if(m_tableView != nullptr)
    {
        delete layout();
        delete m_submitButton;
        delete m_revertButton;
        delete m_quitButton;
        delete m_addRowButton;
        delete m_addColumnButton;
        delete m_removeColumn;
        delete m_removeRow;
        delete m_buttonBox;
        delete m_tableView;
        m_tableView = nullptr;
    }
}


