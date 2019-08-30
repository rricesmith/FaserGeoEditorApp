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
    showMaximized();
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


    //Add submenu stuff
    m_subMenu = m_contextMenu->addMenu("Tags");

    QStandardItem* root = m_standardModel->invisibleRootItem();

    setWindowTitle(tr("FaserGeoEditorApp"));
    createActions();
    createStatusBar();

    QSqlTableModel *model = m_hvsNodeTableModel;
    
    for(int i = 0; i < model->rowCount(); i++)
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



    m_treeView->setModel(m_standardModel);
    m_treeView->expandAll();
    m_treeView->resizeColumnToContents(0);
    m_treeView->resizeColumnToContents(1);

    m_treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FaserDbMainWindow::selectionChanged);

    return;
}

bool FaserDbMainWindow::isBranch(QString name)
{
    int index;
    bool value = false;
    for(index = 0; index < m_hvsNodeTableModel->rowCount(); index++)
    {
        if(m_hvsNodeTableModel->record(index).value("NODE_NAME").toString() == name)
        {
            value  = m_hvsNodeTableModel->record(index).value("BRANCH_FLAG").toString().endsWith("1"); 
            break;
        }
    }
    return value;

}

//testing custom context menu
void FaserDbMainWindow::contextMenu(const QPoint &point)
{
    //Add branch/leaf functions if selected is a branch
    if(isBranch(m_currentSelected))
    {
        m_addBranch = new QAction("Add Branch", m_contextMenu);
        m_addLeaf = new QAction("Add Leaf", m_contextMenu);
        m_contextMenu->addAction(m_addBranch);
        m_contextMenu->addAction(m_addLeaf);
        connect(m_addBranch, &QAction::triggered, this, &FaserDbMainWindow::addBranch);
        connect(m_addLeaf, &QAction::triggered, this, &FaserDbMainWindow::addLeaf);
        m_contextMenu->addSeparator();
    }

    //Add create tag action for hvs_node rows only
    QString current = m_currentSelected;    
    if( !current.endsWith("_DATA") && !current.endsWith("_DATA2TAG"))
    {
        m_createTag = new QAction("Create Tag", m_contextMenu);
        m_contextMenu->addAction(m_createTag);     
        connect(m_createTag, &QAction::triggered, this, &FaserDbMainWindow::createTag );
    }

    //Now set current QString to reflect the correspondng data2tag if we are looking at a leaf or data table
    if( !isBranch(current) && !current.endsWith("_DATA2TAG"))
    {
        if(current.endsWith("_DATA"))
        {
            current.replace(QString("_DATA"), QString("_DATA2TAG"));
        }
        else
        {
            current.append("_DATA2TAG");
        }
    }
    //First build a submenu action for each tag id and set its internal tag id data
    vector<QAction *> actions;
    if( current.endsWith("_DATA2TAG") )
    {
        QSqlTableModel model;
        model.setTable(current);
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
        connect(m_subMenu, &QMenu::triggered, this, &FaserDbMainWindow::tagAction);
    }



    //Connect our menu choice to the proper action
    m_contextMenu->exec(m_treeView->viewport()->mapToGlobal(point));
    

    //Delete all actions we made and disconnect them for next use
    if(isBranch(m_currentSelected))
    {
        disconnect(m_contextMenu, &QMenu::triggered, this, &FaserDbMainWindow::addBranch);
        disconnect(m_contextMenu, &QMenu::triggered, this, &FaserDbMainWindow::addLeaf);
        delete m_addBranch;
        delete m_addLeaf;
    }
    if( !current.endsWith("_DATA") && !current.endsWith("_DATA2TAG"))
    {
        disconnect(m_contextMenu, &QMenu::triggered, this, &FaserDbMainWindow::createTag);
        delete m_createTag;
    }
    if( m_currentSelected.endsWith("_DATA2TAG") )
    {
        disconnect(m_subMenu, &QMenu::triggered, this, &FaserDbMainWindow::tagAction);

/*        for(size_t i = 0; i < actions.size(); i++)
        {
            delete actions[i];
        }
        m_subMenu->clear();*/
    }
    for(size_t i = 0; i < actions.size(); i++)
    {
        delete actions[i];
    }
    m_subMenu->clear();
}

void FaserDbMainWindow::selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{   
    if(!selected.isEmpty())
    {
        m_currentSelected = selected.indexes().at(1).data(0).toString();
    }

    if(!selected.isEmpty() 
        && (selected.indexes().at(1).data(0).toString().endsWith("_DATA") 
        || selected.indexes().at(1).data(0).toString().endsWith("_DATA2TAG")))
    {
        m_secondWindow->hide();
        m_secondWindow->clearWindow();
        m_secondWindow->setWindow(selected.indexes().at(1).data(0).toString());
        m_secondWindow->show();
        return;
    }

    //First second finds corresponding record in our table model
    bool found = false;
    int i;
    for( i = 0; i < m_hvsNodeTableModel->rowCount(); i++)
    {
        if( m_hvsNodeTableModel->record(i).value("NODE_NAME").toString() == selected.indexes().at(1).data(0).toString())
        {
            found = true;
            break;
        }
    }
    //And then checks if its a leaf to display data, or a branch to hide data
    if(found && m_hvsNodeTableModel->record(i).value("BRANCH_FLAG").toString().endsWith("0"))
    {
        QString nodename = selected.indexes().at(1).data(0).toString();
        nodename.append("_DATA");
        m_secondWindow->hide();
        m_secondWindow->clearWindow();
        m_secondWindow->setWindow(nodename);
        m_secondWindow->show();
        return;

    }
    //Hide window for branch case to avoid errant behavior (editing tables when wrong row selected in treeview will mess it up)
    else
    {
        m_secondWindow->hide();
        m_secondWindow->clearWindow();
    }
    
    if(deselected.isEmpty())
    { //this code is to supress warning of unused deselected
    }
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

    //Get name of table we are creating
    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("New Table Name"), QLineEdit::Normal, QDir::home().dirName(),&ok);

    if( !ok)
    {   
        cout<<"Invalid name/exited\n";
        return;
    }


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

//Current filler function, is dynamic action function that utilized the data put insode the action
void FaserDbMainWindow::tagAction(QAction *action)
{
    //Get the saved data in the action, which is the tag we are filtering for
    QString input = action->data().toString();

    m_secondWindow->setTagFilter(input);

}

//This function is used to check if a tagId is locked according to HVS_TAG2NODE
bool FaserDbMainWindow::isLocked(QString tagId)
{
    QSqlTableModel tag2node;
    tag2node.setTable("HVS_TAG2NODE");
    tag2node.select();
    
    int i = 0;
    for(; i < tag2node.rowCount(); i++)
    {
        if( tag2node.record(i).value("TAG_ID").toString() == tagId)
        {
            break;
        }
    }
    if(tag2node.record(i).value("TAG_ID").toString() == tagId)
    {
        return tag2node.record(i).value("LOCKED").toBool();
    }
    return false;
}

//Function is used to find associated values, ie given a child tag id find parent tag id
QStringList FaserDbMainWindow::findAssociatedList(QSqlTableModel *table, QString known, QString kvalue, QString search)
{
    QStringList found;
    for(int i = 0; i < table->rowCount(); i++)
    {
        if( table->record(i).value(known).toString() == kvalue)
        {
            found<<table->record(i).value(search).toString();
        }
    }
    return found;
}

//This function returns a qstring of the associated nodeId of the given 
QString FaserDbMainWindow::findAssociated(QSqlTableModel *table, QString known, QString kvalue, QString search)
{
    QString found;
    for(int i = 0; i < table->rowCount(); i++)
    {
        if( table->record(i).value(known).toString() == kvalue)
        {
            found = table->record(i).value(search).toString();
            break;
        }
    }
    return found;
}

QString FaserDbMainWindow::createTag()
{
    //This function only works if used on root, or on a node whose parent has a unlocked tag
    //Get tag we are creating under
    QString nodeGettingTagName = selectedRowName();
    QString nodeGettingTagId = findAssociated( m_hvsNodeTableModel, QString("NODE_NAME"), nodeGettingTagName, QString("NODE_ID"));
    QString newTag;
    QString nullTag;

    QSqlTableModel ltag2ltag;
    ltag2ltag.setTable("HVS_LTAG2LTAG");
    ltag2ltag.select();
    QSqlTableModel tag2node;
    tag2node.setTable("HVS_TAG2NODe");
    tag2node.select();
    QSqlTableModel tag2cache;
    tag2cache.setTable("HVS_TAG2CACHE");
    tag2cache.select();
    bool ok = false;


    int tagAvailable = 0;
    for(int i = 0; i < tag2node.rowCount(); i++)
    {
        tagAvailable = (tag2node.record(i).value("TAG_ID").toString().toInt() > tagAvailable) ? tag2node.record(i).value("TAG_ID").toString().toInt() : tagAvailable;
    }
    tagAvailable++;
    newTag = QString::number(tagAvailable);

    //Case where we are making a root tag
    if(nodeGettingTagName == "FASER")
    {

        QString rootTag = QInputDialog::getItem(this, tr("QInputDialog::getText()"), tr("New Root Tag name:"),
            findAssociatedList( &tag2node, QString("NODE_ID"), nodeGettingTagId, QString("TAG_NAME")), 0, true, &ok);
        if( !ok)
        {
            cout<<"Error in name given/closed window\n";
            return nullTag;
        }

        //Prevent repeated root name
        for(int i = 0; i < tag2cache.rowCount(); i++)
        {
            if(tag2cache.record(i).value(0).toString() == rootTag)
            {
                QMessageBox messageBox;
                messageBox.critical(0, "Error", "Root tag name input is already taken");
                messageBox.setFixedSize(500,200);
                messageBox.exec();
            }
        }

        //Get tag we will make this new one as a replica of
        QStringList replicateList;
        for(int i = 0; i < tag2cache.rowCount(); i++)
        {
            if( !replicateList.contains(tag2cache.record(i).value("ROOTTAG").toString()))
            {
                replicateList.push_back(tag2cache.record(i).value("ROOTTAG").toString());
            }
        }
        QString replicateTag = QInputDialog::getItem(this, tr("QInputDialog::getItem()"), tr("Root tag to initially copy:"), replicateList, 0, false, &ok);
        if( !ok)
        {
            cout<<"Failed to get tag to replicate to/closed window\n";
            return nullTag;
        }

        //Get list of children who will be under our new tag so we put proper children under our new tag
        QStringList childTags = findAssociatedList( &tag2cache, tr("ROOTTAG"), replicateTag, tr("CHILDTAG"));

        //Valid root tag name
        //Make appropriate row inserts in each table
        //Get highest tag value then increment
        //Edit ltag2ltag
        for(int i = 0; i < ltag2ltag.rowCount(); i++)
        {
            if( ltag2ltag.record(i).value("PARENT_NODE").toString() == "0" 
                && ltag2ltag.record(i).value("PARENT_TAG").toString() != newTag
                && childTags.contains(ltag2ltag.record(i).value("CHILD_TAG").toString()))
            {
                QSqlRecord record = ltag2ltag.record(i);
                record.setValue("PARENT_TAG", newTag);

                ltag2ltag.insertRecord(-1, record);
            }
        }
        ltag2ltag.submitAll();

        //Edit tag2node
        for(int i = 0; i < tag2node.rowCount(); i++)
        {
            if(  tag2node.record(i).value("TAG_NAME").toString() == replicateTag)
            {
                QSqlRecord record = tag2node.record(i);
                record.setValue("TAG_NAME", rootTag);
                record.setValue("TAG_ID", newTag);
                record.setNull("TAG_COMMENT");
                record.setValue("LOCKED", QString("0"));
                record.setValue("REPLICATED", QString("0"));
                auto dt = QDateTime::currentMSecsSinceEpoch();
                record.setValue("DATE_CREATED", QString::number(dt));
                record.setNull("DATE_LOCKED");

                tag2node.insertRecord(-1, record);

            }
        }

        //Edit tag2cache
        for(int i = 0; i < tag2cache.rowCount(); i++)
        {
            if(tag2cache.record(i).value("ROOTTAG").toString() == replicateTag)
            {
                QSqlRecord record = tag2cache.record(i);
                record.setValue("ROOTTAG", rootTag);

                tag2cache.insertRecord(-1, record);
            }
        }
        tag2cache.submitAll();


    }

    //Case of adding tag to non root
    //Get list of unlocked parents
    QStringList parentTags = findAssociatedList( &ltag2ltag, QString("CHILD_NODE"), nodeGettingTagId, QString("PARENT_TAG"));
    for(int i = 0; i < parentTags.size(); i++)
    {
        if( !isLocked(parentTags.at(i)))
        {
            parentTags.removeAt(i);
            i = 0;
        }
    }
    //Case of no unlocked parent tags, can't add tag
    if(parentTags.empty())
    {
        QMessageBox messageBox;;
        messageBox.critical(0, "Error", "No unlocked parent tags to choose from");
        messageBox.setFixedSize(500,200);
        messageBox.exec();
        return nullTag;
    }

    //Get parent tag to make under
    QStringList parentTagNames;
    for(int i = 0; i < parentTags.size(); i++)
    {
        parentTagNames.push_back( findAssociated( &tag2node, QString("TAG_ID"), parentTags.at(i), QString("TAG_NAME")));
    }
    QString parentTagName = QInputDialog::getItem(this, tr("QInputDialog::getItem()"), tr("Unlocked tag to set as parent of new tag:"), parentTagNames, 0, false, &ok);
    QString parentTag = findAssociated(&tag2node, QString("TAG_NAME"), parentTagName, QString("TAG_ID"));

    if(!ok)
    { //user exited choosing
        cout<<"Failed to obtain parent tag for new tag\n";
        return nullTag;
    }

    //Now we need to choose an existing tag ours will be a copy of
    QStringList currentTags = findAssociatedList( &tag2node, QString("NODE_ID"), nodeGettingTagId, QString("TAG_ID"));
    QString copyTag = QInputDialog::getItem(this, tr("QInputDialog::getItem()"), tr("Choose tag to initialize as copy of:"), currentTags, 0, false, &ok);
    if( !ok)
    {//user closed window
        cout<<"No tag to copy passed through\n";
        return nullTag;
    }

    //Lastly get name of new tag
    QString newTagName = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("New Tag name:"), QLineEdit::Normal, parentTag, &ok);
    if( !ok || currentTags.contains(newTagName))
    {
        cout<<"Failed to get new tag name/input repeated tag name\n";
        return nullTag;
    }

    //Now we edit the three tables, inserting our new tag
    //Edit ltag2ltag
    for(int i = 0; i < ltag2ltag.rowCount(); i++)
    {
        if(ltag2ltag.record(i).value("PARENT_TAG").toString() == parentTag
            && ltag2ltag.record(i).value("CHILD_TAG").toString() == copyTag)
        {
            ltag2ltag.record(i).setValue("CHILD_TAG", newTag);

        }
        if(ltag2ltag.record(i).value("PARENT_TAG").toString() == copyTag)
        {
            QSqlRecord record = ltag2ltag.record(i);
            record.setValue("PARENT_TAG", newTag);

            ltag2ltag.insertRecord(-1, record);
        }
    }

    //Edit tag2node
    for(int i = 0; i < tag2node.rowCount(); i++)
    {
        if(tag2node.record(i).value("TAG_ID").toString() == copyTag)
        {
            QSqlRecord record = tag2node.record(i);
            record.setNull("TAG_COMMENT");
            record.setValue("LOCKED", QString("0"));
            record.setValue("REPLICATED", QString("0"));
            record.setValue("TAG_NAME", newTagName);
            record.setNull("DATE_LOCKED");
            auto dt = QDateTime::currentMSecsSinceEpoch();
            record.setValue("DATE_CREATED", QString::number(dt));

            tag2node.insertRecord(-1, record);
        }
    }

    //Edit tag2cache
    QString rootTag = findAssociated( &tag2cache, QString("CHILDTAG"), parentTagName, QString("ROOTTAG"));
    for(int i = 0; i < tag2cache.rowCount(); i++)
    {
        if(tag2cache.record(i).value("CHILDTAGID").toString() == copyTag
            && tag2cache.record(i).value("ROOTTAG").toString() == rootTag)
        {
            tag2cache.record(i).setValue("CHILDTAG", newTagName);
            tag2cache.record(i).setValue("CHILDTAGID", newTag);
        }
    }

    ltag2ltag.submitAll();
    tag2node.submitAll();
    tag2cache.submitAll();

    return newTag;
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
    for(int i = 0; i < m_tableModel->rowCount(); i++)
    {
        QString tagId = m_tableModel->record(i).value(0).toString();
        if( m_parentWindow->isLocked(tagId))
        {
            cout<<"Trying to delete date from a locked column\n";
            return;
        }
    }
//    cout<<"Cannot alter table with sqlite, need to implement function that creates a new table with col to remove gone, then rename that table to curren table name\n";
    QString rowname = m_parentWindow->selectedRowName();
    int currentColumnIndex = m_tableView->selectionModel()->currentIndex().column();
    if(currentColumnIndex == 0)
    {
        cout<<"Cannot delete associated TAG_ID column\n";
        return;
    }
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

//Following fucntion is reimplementation of flags function
//Reimplemented to return a non-editable flag for rows where data is locked
Qt::ItemFlags testtest::flags(const QModelIndex &index) const
{
Qt::ItemFlags flags;

//    flags = QAbstractItemModel::flags(index);
    cout<<"hello\n";
    if(index.isValid())
    {
    }
/*    QString tableName = m_parentWindow->selectedRowName();
    int row = m_tableView->selectionModel()->currentIndex().row();
    if( !tableName.endsWith("_DATA2TAG"))
    {
        if( tableName.endsWith("_DATA"))
        {
            tableName.append("2TAG");
        }
        else
        {
            tableName.append("_DATA2TAG");
        }
    }

    //Get current data id we are looking at
    cout<<m_tableModel->record(row).value(0).toString().toStdString()<<endl;
    cout<<row<<endl;

    QSqlTableModel tagTable;
    tagTable.setTable(tableName);
    tagTable.select();

    QString tagId = tagTable.record(row).value(0).toString();

    if( m_parentWindow->isLocked(tagId))
    {
//        flags = Qt::ItemIsSelectable;
        return Qt::ItemIsSelectable;
    }
*/
    return Qt::ItemIsEditable;


}

void FaserDbSecondWindow::click_cell(int row, int column)
{
    cout<<row<<column<<endl;
}


void FaserDbSecondWindow::setTagFilter(QString tagFilter)
{
    //Goal is to hide all currently shown rows with given associated tag in data table
    QString currentNode = m_parentWindow->selectedRowName();
    //Case where we are looking at the tag table
    if(currentNode.endsWith("_DATA2TAG"))
    {
        QSqlTableModel data2tagtable;
        data2tagtable.setTable(currentNode);
        data2tagtable.select();
        for(int i = 0; i < data2tagtable.rowCount(); i++)
        {
            if(data2tagtable.record(i).value(0).toString() != tagFilter)
            {
                m_tableView->hideRow(i);
            }
        }
        return;
    }

    //Case where we are looking at data table
    //Need to get a vector of data id's we will permit
    vector<QString> nodeIds;
    if(currentNode.endsWith("_DATA"))
    {
        currentNode.append("2TAG");
    }
    else
    {
        currentNode.append("_DATA2TAG");
    }
    QSqlTableModel data2TagTable;
    data2TagTable.setTable(currentNode);
    data2TagTable.select();

    for(int i = 0; i < data2TagTable.rowCount(); i++)
    {
        if( data2TagTable.record(i).value(0).toString() == tagFilter)
        {
            nodeIds.push_back(data2TagTable.record(i).value(1).toString());
        }
    }

    //Now match filter data ids with those on
    for(int i = 0; i < m_tableModel->rowCount(); i++)
    {
        m_tableView->hideRow(i);
        for(size_t j = 0; j < nodeIds.size(); j++)
        {
            if(nodeIds[j] == m_tableModel->record(i).value(0).toString())
            {
                m_tableView->showRow(i);
                break;
            }
        }
    }


}


