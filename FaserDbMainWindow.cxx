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
#include <QtGui>

using namespace std;

FaserDbMainWindow::FaserDbMainWindow(QWidget* parent, FaserGeoEditorApp *parentApp)
    : QMainWindow(parent)
    , m_treeView(new QTreeView(this))
    , m_standardModel(new QStandardItemModel(this))
{
    m_editorApp = parentApp;
    showMaximized();
}


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
    m_tag2node = new QSqlTableModel(nullptr, m_database);
    m_tag2node->setTable("HVS_TAG2NODE");
    m_tag2node->select();
    m_tagcache = new QSqlTableModel(nullptr, m_database);
    m_tagcache->setTable("HVS_TAGCACHE");
    m_tagcache->select();
    m_ltag2ltag = new QSqlTableModel(nullptr, m_database);
    m_ltag2ltag->setTable("HVS_LTAG2LTAG");
    m_ltag2ltag->select();
    return;
}

void FaserDbMainWindow::initializeWindow()
{
    //Build second window and third window
    QDockWidget *secondWid = new QDockWidget(tr("Data"), this);
    QDockWidget *thirdWid = new QDockWidget(tr("Root Tags"), this);
    secondWid->setAllowedAreas(Qt::RightDockWidgetArea);
    thirdWid->setAllowedAreas(Qt::LeftDockWidgetArea);
    m_secondWindow = new FaserDbSecondWindow(this, secondWid);
    m_listWidget = new QListWidget(this);
//    m_thirdWindow = new FaserDbMainWindow(this, thirdWid);
    secondWid->setWidget(m_secondWindow);
    thirdWid->setWidget(m_listWidget);
    addDockWidget(Qt::RightDockWidgetArea, secondWid);
    addDockWidget(Qt::LeftDockWidgetArea, thirdWid);
    m_listWidget->setSelectionMode(QAbstractItemView::SingleSelection);

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
        if(model->record(i).value("NODE_ID").toInt() == 0)
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
    connect(m_listWidget, &QListWidget::currentRowChanged, this, &FaserDbMainWindow::tagRowChanged);

    //Build the list widget
    buildListWidget();


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
    /*
    Improvements to this sections code:
    The end disconnect functions disconnect all
    Can remove all other disconnects and create one QAction pointer vector
    That we iterate through at the end to delete
                                */

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

    //Add create tag and lock action for hvs_node rows only
    QString current = m_currentSelected;    
    if( !current.endsWith("_DATA") && !current.endsWith("_DATA2TAG"))
    {
        if( current == "FASER")
        {
            m_createTag = new QAction("Create Root Tag", m_contextMenu);
            connect(m_createTag, &QAction::triggered, this, &FaserDbMainWindow::createRootTag);
        }
        else
        {
            m_createTag = new QAction("Create Tag", m_contextMenu);
            connect(m_createTag, &QAction::triggered, this, &FaserDbMainWindow::createTag );
        }
        m_lockTag = new QAction("Lock Tag", m_contextMenu);
        m_contextMenu->addAction(m_createTag);     
        m_contextMenu->addAction(m_lockTag);
        //Following is a lambda function, lets us pass lockTag function pointer that has arguments to connect(using default arguments)
        connect(m_lockTag, &QAction::triggered, this, [this]()
        {
            lockTag();
        });
    }

    //If we are looking at root tag, create set root action
    //Obsolete feature, replaced by left root tag list
/*    if( current == QString("FASER"))
    {
        m_setRoot = new QAction("Set Root Tag", m_contextMenu);
        m_contextMenu->addAction(m_setRoot);
        connect(m_setRoot, &QAction::triggered, this, &FaserDbMainWindow::setRoot);
    }*/

    //Now make QString to reflect the correspondng data2tag if we are looking at a leaf or data table
    QString tagCurrent;
    if( !isBranch(current) && !current.endsWith("_DATA2TAG"))
    {
        tagCurrent = current;
        if(current.endsWith("_DATA"))
        {
            tagCurrent.replace(QString("_DATA"), QString("_DATA2TAG"));
        }
        else
        {
            tagCurrent.append("_DATA2TAG");
        }
    }
    else if (current.endsWith("_DATA2TAG"))
    {
        tagCurrent = current;
    }
    
    //Build a submenu action for each tag id and set its internal tag id data
    vector<QAction *> tagActions;
    vector<QString> tag_ids;

    //Build list leaf nodes
    if( !tagCurrent.isNull())
    {
        QSqlTableModel model(nullptr, m_database);
        model.setTable(tagCurrent);
        model.select();
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
    }
    //Build list for branch nodes -- currently i dont believe we want this functionallity
    //Can add back in later, but would need to update setTagFilter's functionality
/*    else
    {
        for(int i = 0; i < m_tagcache.rowCount(); i++)
        {
            if(m_tagcache->record(i).value("CHILDNODE").toString() == current)
            {
                tag_ids.push_back(m_tagcache->record(i).value("CHILDTAGID").toString());
            }
        }
    }*/

    for(size_t i = 0; i < tag_ids.size(); i++)
    {
        QAction * tempAct = new QAction(tag_ids[i], m_subMenu);
        tagActions.push_back(tempAct);
        tempAct->setData(tag_ids[i]);
        m_subMenu->addAction(tempAct);
    }
    if( !tagCurrent.isNull())
    {
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
/*    if( current == QString("FASER"))
    {
        disconnect(m_contextMenu, &QMenu::triggered, this, &FaserDbMainWindow::setRoot);
        delete m_setRoot;
    }*/
    if( !current.endsWith("_DATA") && !current.endsWith("_DATA2TAG"))
    {
//        disconnect(m_contextMenu, &QMenu::triggered, this, &FaserDbMainWindow::createTag);
        delete m_createTag;
        delete m_lockTag;
    }
    if( !tagCurrent.isNull() )
    {
        disconnect(m_subMenu, &QMenu::triggered, this, &FaserDbMainWindow::tagAction);

/*        for(size_t i = 0; i < actions.size(); i++)
        {
            delete actions[i];
        }
        m_subMenu->clear();*/
    }
    for(size_t i = 0; i < tagActions.size(); i++)
    {
        delete tagActions[i];
    }
    disconnect(m_contextMenu, 0,0,0);
    disconnect(m_subMenu, 0,0,0);
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

    QAction *rootAct = fileMenu->addAction(tr("&Create new root"), this, &FaserDbMainWindow::createRootTag);
    rootAct->setStatusTip(tr("Creates a new root tag for a new database version"));

    QAction *quitAct = fileMenu->addAction(tr("&Quit"), this, &QWidget::close);
    quitAct->setShortcuts(QKeySequence::Quit);
    quitAct->setStatusTip(tr("Quit the application"));


}

QString FaserDbMainWindow::selectedRowName()
{
    return m_currentSelected;
}

void FaserDbMainWindow::rebuildTree()
{
    m_standardModel->clear();
    QStandardItem* root = m_standardModel->invisibleRootItem();

    m_hvsNodeTableModel->select();
    QSqlTableModel *model = m_hvsNodeTableModel;
    
    for(int i = 0; i < model->rowCount(); i++)
    {
        if(model->record(i).value("NODE_ID").toInt() == 0)
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
    bool tagcacheExists = false;
    vector<string> masterTables;
    //Build a list of all tables in the database
    for(int i = 0; i < masterTable.rowCount(); i++)
    {
        QString tableName = masterTable.record(i).value(1).toString();
        nodeExists = nodeExists || (tableName.toStdString() == "HVS_NODE") ? true : false;
        ltag2ltagExists = nodeExists || (tableName.toStdString() == "HVS_LTAG2LTAG") ? true : false;
        tag2nodeExists = nodeExists || (tableName.toStdString() == "HVS_TAG2NODE") ? true : false;
        tagcacheExists = nodeExists || (tableName.toStdString() == "HVS_tagcache") ? true : false;
        if(masterTable.record(i).value("type").toString() == "table")
        {
            masterTables.push_back(tableName.toStdString());        
        }
    }
    //Check that main tables exist
    if( !(ltag2ltagExists && nodeExists && tag2nodeExists && tagcacheExists))
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
        if(!tagcacheExists)
        {
            m_errors.push_back("HVS_tagcache does not exist");
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
    //Ensure a _DATA and _DATA2TAG table for each leaf
    //Check no hanging children or extraneous tables

    //First check branch/leaf structure in hvs_node
    vector<int> parent_ids;
    for(int i = 0; i < hvsNodeTable.rowCount(); i++)
    {
        if( hvsNodeTable.record(i).value("BRANCH_FLAG").toString() == "1")
        {
            parent_ids.push_back(hvsNodeTable.record(i).value("NODE_ID").toString().toInt());
        }
    }
    for(int i = 0; i < hvsNodeTable.rowCount(); i++)
    {
        QString nodeName = hvsNodeTable.record(i).value("NODE_NAME").toString();
        QString parentId = hvsNodeTable.record(i).value("PARENT_ID").toString();
        QString nodeId = hvsNodeTable.record(i).value("NODE_ID").toString();
        if( nodeId != "0" && std::find(parent_ids.begin(), parent_ids.end(), parentId.toInt()) == parent_ids.end())
        {
            string err = "Parent of ";
            err += hvsNodeTable.record(i).value("NODE_NAME").toString().toStdString();
            err += "is not a branch";
            m_errors.push_back(err);
        }

        //Now do different logic on branch or leaves
        //Branches first, ensure there are no _DATA or _DATA2TAG tables for it
        if(hvsNodeTable.record(i).value("BRANCH_FLAG").toString() == "1")
        {
            string check_data = hvsNodeTable.record(i).value("NODE_NAME").toString().toStdString();
            check_data += "_DATA";
            transform(check_data.begin(), check_data.end(), check_data.begin(), ::toupper);
            string check_data2tag = hvsNodeTable.record(i).value("NODE_NAME").toString().toStdString();
            check_data2tag += "_DATA2TAG";
            transform(check_data2tag.begin(), check_data2tag.end(), check_data2tag.begin(), ::toupper);
            if( std::find(masterTables.begin(), masterTables.end(), check_data) != masterTables.end())
            {
                string err = "Found _DATA table for branch node in HVS_NODE : ";
                err += check_data;
                m_errors.push_back(err);
            }
            if( std::find(masterTables.begin(), masterTables.end(), check_data2tag) != masterTables.end())
            {
                string err = "Found _DATA2TAG table for branch node in HVS_NODE : ";
                err += check_data2tag;
                m_errors.push_back(err);
            }
        }   
        else //Checking leaf
        {
            string check_data = hvsNodeTable.record(i).value("NODE_NAME").toString().toStdString();
            check_data += "_DATA";
            transform(check_data.begin(), check_data.end(), check_data.begin(), ::toupper);
            string check_data2tag = hvsNodeTable.record(i).value("NODE_NAME").toString().toStdString();
            check_data2tag += "_DATA2TAG";
            transform(check_data2tag.begin(), check_data2tag.end(), check_data2tag.begin(), ::toupper);

            vector<int> leafDataIds;
            vector<int> leafTagIds;

            auto it = std::find(masterTables.begin(), masterTables.end(), check_data);
            if( it != masterTables.end())
            {
                //Remove the found table from master list vector if found
                masterTables.erase( it);
                //Also build list of data id's
                QSqlTableModel leaf;
                leaf.setTable(QString::fromStdString(check_data));
                leaf.select();
                for(int i = 0; i < leaf.rowCount(); i++)
                {
                    int dataid = leaf.record(i).value(0).toString().toInt();
                    if( std::find(leafDataIds.begin(), leafDataIds.end(), dataid) == leafDataIds.end())
                    {
                        leafDataIds.push_back(dataid);
                    }
                    else
                    {
                        string err = "Repeated data tag in ";
                        err += check_data;
                        err += " of ";
                        err += std::to_string(dataid);
                        m_errors.push_back(err);
                    }
                }
            }
            else
            {
                string err = "Missing _DATA table ";
                err += check_data;
                m_errors.push_back(err);
            }

            it = std::find(masterTables.begin(), masterTables.end(), check_data2tag);
            if( it != masterTables.end())
            {
                //Erase from list of master tables
                masterTables.erase( it);
                //Also check that ever data id in _data has a tag
                //And that every tag is associated with this table in tag2node
                QSqlTableModel leaf;
                leaf.setTable(QString::fromStdString(check_data2tag));
                leaf.select();

                for(int i = 0; i < leaf.rowCount(); i++)
                {
                    int data = leaf.record(i).value(1).toString().toInt();
                    int tag = leaf.record(i).value(0).toString().toInt();
                    if(std::find(leafDataIds.begin(), leafDataIds.end(), data) == leafDataIds.end())
                    {
                        string err = "No reference to data id ";
                        err += std::to_string(data);
                        err += " in ";
                        err += check_data2tag;
                        m_errors.push_back(err);
                    }
                    if(std::find(leafTagIds.begin(), leafTagIds.end(), tag) != leafTagIds.end())
                    {
                        leafTagIds.push_back(tag);
                    }
                }

                for(size_t i = 0; i < leafTagIds.size(); i++)
                {
                    if( nodeId != findAssociated( &hvsTag2NodeTable, QString("TAG_ID"), QString(leafTagIds[i]), QString("NODE_ID")))
                    {
                        string err = "Tag of ";
                        err += std::to_string(leafTagIds[i]);
                        err += "not associated with the node ";
                        err += nodeName.toStdString();
                        m_errors.push_back(err);
                    }
                }

            }
            else
            {
                string err = "Missing _DATA2TAG table ";
                err += check_data2tag;
                m_errors.push_back(err);
            }
            
            
        }
         

    }
    


    if(m_errors.empty())
    {
        return true;
    }
    else
    {
        string err = "Errors were found in database verification - check terminal for log";
        errorMessage(err);
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
    
    QSqlTableModel tagcache;
    tagcache.setTable("HVS_TAGCACHE");
    tagcache.select();
//    QSqlTableModel *model = m_secondWindow->tablePointer();

    if(!m_rootDisplayTag.isNull())
    {
        for(int j = 0; j < m_tagcache->rowCount(); j++)
        {
            if(m_tagcache->record(j).value("ROOTTAG").toString() == m_rootDisplayTag
                && m_tagcache->record(j).value("CHILDNODE").toString() == PRow->at(1)->data(Qt::DisplayRole).toString())
            {
                if( isLocked(m_tagcache->record(j).value("CHILDTAGID").toString()))
                {
                    PRow->at(0)->setBackground(Qt::red);
                }
                else
                {
                    PRow->at(0)->setBackground(Qt::green);
                }
                break;
            }
        }
    }
    else
    {
        PRow->at(0)->setBackground(QColor(Qt::white));
    }

    for(int i = 0; i < model->rowCount(); i++)
    {
        //Add new row if it is a child and part of our current root tag system if we have one
        if(model->record(i).value("PARENT_ID").toString() == parent_id && model->record(i).value("NODE_ID").toInt() != 0
            && (m_rootDisplayTag.isNull() || 
            findAssociatedList( &tagcache, QString("ROOTTAG"), m_rootDisplayTag, QString("CHILDNODE")).contains(model->record(i).value("NODE_NAME").toString())))
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
            if(!m_rootDisplayTag.isNull())
                {
                    QSqlTableModel tagcache;
                    tagcache.setTable("HVS_TAGCACHE");
                    tagcache.select();
                    for(int j = 0; j < tagcache.rowCount(); j++)
                    {
                        if(tagcache.record(j).value("ROOTTAG").toString() == m_rootDisplayTag
                            && tagcache.record(j).value("CHILDNODE").toString() == newRow->at(1)->data(Qt::DisplayRole).toString())
                        {
                            if( isLocked(tagcache.record(j).value("CHILDTAGID").toString()))
                            {
                                newRow->at(0)->setBackground(Qt::red);
                            }
                            else
                            {
                                newRow->at(0)->setBackground(Qt::green);
                            }
                            break;
                        }
                    }
                }
                else
                {
                    newRow->at(0)->setBackground(Qt::white);
                }              
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

    //Can't add branch to a locked table
    if( !m_rootDisplayTag.isNull())
    {
        for(int i = 0; i < m_tagcache->rowCount(); i++)
        {
            if(m_tagcache->record(i).value("ROOTTAG").toString() == m_rootDisplayTag
                && m_tagcache->record(i).value("CHILDNODE").toString() == selectedRowName())
            {
                if(isLocked(m_tagcache->record(i).value("CHILDTAGID").toString()))
                {
                    errorMessage("Cannot add branch to a locked node");
                    return;
                }
                else
                {
                    break;
                }
            }
        }
    }

    //Get name of table we are creating
    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("New Table Name"), QLineEdit::Normal, selectedRowName(),&ok);
    if(!findAssociated(m_hvsNodeTableModel, QString("NODE_NAME"), text, QString("NODE_ID")).isNull())
    {
        errorMessage("Cannot create new branch with same name as existing branch");
        return;
    }


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

    //Need to add new branch into related tag tables with a new tag
/*    QSqlTableModel tag2node(nullptr, m_database);
    QSqlTableModel ltag2ltag(nullptr, m_database);
    QSqlTableModel tagcache(nullptr, m_database);
    tag2node.setTable("HVS_TAG2NODE");
    ltag2ltag.setTable("HVS_LTAG2LTAG");
    tagcache.setTable("HVS_TAGCACHE");
    tag2node.select();
    ltag2ltag.select();
    tagcache.select();*/

    QString currentNodeName = selectedRowName();
    QString currentNodeId = findAssociated(m_hvsNodeTableModel, QString("NODE_NAME"), currentNodeName, QString("NODE_ID"));
    
    QString parentTagName;
    QString parentTag;
    //Need to get unlocked parent tag to make new node under
    //Get from user if root tag isn't set
    if(m_rootDisplayTag.isNull())
    {
        QStringList parentTags = findAssociatedList(m_tag2node, QString("NODE_ID"), currentNodeId, QString("TAG_NAME"));
        for(int i = 0; i < parentTags.size(); i++)
        {
            if( isLocked(parentTags.at(i)))
            {
                parentTags.removeAt(i);
                i--;
            }
        }
        if(parentTags.isEmpty())
        {
            errorMessage("No unlocked parent tag to choose from");
            return;
        }
        else if (parentTags.size() == 1)
        {
            parentTagName = parentTags.at(0);
        }
        else
        {
            parentTagName = QInputDialog::getItem(this, tr("QInputDialog::getItem()"), tr("Choose parent tag for branch:"), parentTags, 0, false, &ok);
            if(!ok)
            {
                cout<<"Failed to get parent tag for branch\n";
                return;
            }
        }
        parentTag = findAssociated(m_tag2node, QString("TAG_NAME"), parentTagName, QString("TAG_ID"));
        
    }
    else //Case where we have a root tag, use child tag associated with current
    {
        for(int i = 0; i < m_tagcache->rowCount(); i++)
        {
            if(m_tagcache->record(i).value("ROOTTAG").toString() == m_rootDisplayTag
                && m_tagcache->record(i).value("CHILDNODE").toString() == currentNodeName)
            {
                parentTag = m_tagcache->record(i).value("CHILDTAGID").toString();
                break;
            }
        }
    }
    if(parentTag.isNull())
    {
        errorMessage("Unable to get tag associated with table we are adding a branch to");
        return;
    }

    //Make changes to tag tables
    //Get new tag number
    int tagAvailable = 0;
    QString newTag;
    for(int i = 0; i < m_tag2node->rowCount(); i++)
    {
        tagAvailable = (m_tag2node->record(i).value("TAG_ID").toString().toInt() > tagAvailable) ? m_tag2node->record(i).value("TAG_ID").toString().toInt() : tagAvailable;
    }
    tagAvailable++;
    newTag = QString::number(tagAvailable);

    //Get new tag name
    QString newTagName;
    for(int i = 0; i < m_tag2node->rowCount(); i++)
    {
        if( m_tag2node->record(i).value("TAG_ID").toString() == parentTag)
        {
            newTagName = m_tag2node->record(i).value("TAG_NAME").toString();
            newTagName.replace(findAssociated(m_tagcache, tr("CHILDTAGID"), parentTag, tr("CHILDNODE")), text);
        }
    }

    //Add to ltag2ltag
    QSqlRecord record = m_ltag2ltag->record(0);
    record.setValue("PARENT_NODE", findAssociated(m_tag2node, tr("TAG_ID"), parentTag, tr("NODE_ID")));
    record.setValue("PARENT_TAG", parentTag);
    record.setValue("CHILD_NODE", QString::number(pint));
    record.setValue("CHILD_TAG", newTag);
    m_ltag2ltag->insertRecord(-1, record);
    record.clear();
    if(!( m_ltag2ltag->database().transaction() && m_ltag2ltag->submitAll() && m_ltag2ltag->database().commit()))
    {
        cout<<"Error editing ltag2ltag when inserting branch\n Error: ";
        cout<<m_ltag2ltag->database().lastError().text().toStdString()<<endl;
    }

    //Add to tag2node
    record = m_tag2node->record(0);
    record.setValue("NODE_ID", QString::number(pint));
    record.setValue("TAG_NAME", newTagName);
    record.setValue("TAG_ID", newTag);
    record.setNull("TAG_COMMENT");
    record.setValue("LOCKED", tr("0"));
    record.setValue("REPLICATED", tr("0"));
    auto dt = QDateTime::currentMSecsSinceEpoch();
    record.setValue("DATE_CREATED", QString::number(dt));
    record.setNull("DATE_LOCKED");
    m_tag2node->insertRecord(-1, record);
    record.clear();
    if(!(m_tag2node->database().transaction() && m_tag2node->submitAll() && m_tag2node->database().commit()))
    {
        cout<<"Error editing tag2node when inserting branch\n Error: ";
        cout<<m_tag2node->database().lastError().text().toStdString()<<endl;
    }

    //Add to tagcache
    record = m_tagcache->record(0);
    record.setValue("ROOTTAG", findAssociated( m_tagcache, tr("CHILDTAGID"), parentTag, tr("ROOTTAG")));
    record.setValue("CHILDNODE", text);
    record.setValue("CHILDTAG", newTagName);
    record.setValue("CHILDTAGID", newTag);
    m_tagcache->insertRecord(-1, record);
    if(!m_tagcache->database().transaction() || !m_tagcache->submitAll() || !m_tagcache->database().commit())
    {
        cout<<"Error editing tagcache when inserting branch\n Error: ";
        cout<<m_tagcache->database().lastError().text().toStdString()<<endl;
    }


    //Also add into hvs_node
    m_hvsNodeTableModel->insertRow( row);
    m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 0), pint);
    m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 1), text);
    m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 2), m_hvsNodeTableModel->record(modelRow).value("NODE_ID"));
    m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 3), 1);        
        
    if(!(m_hvsNodeTableModel->database().transaction() && m_hvsNodeTableModel->submitAll() && m_hvsNodeTableModel->database().commit()))
    {
        cout<<"Error editing HVS_NODE when inserting branch\nError: ";
        cout<<m_hvsNodeTableModel->database().lastError().text().toStdString()<<endl;
    }

    rebuildTree();



    
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

    //Can't add branch to a locked table
    if( !m_rootDisplayTag.isNull())
    {
        for(int i = 0; i < m_tagcache->rowCount(); i++)
        {
            if(m_tagcache->record(i).value("ROOTTAG").toString() == m_rootDisplayTag
                && m_tagcache->record(i).value("CHILDNODE").toString() == selectedRowName())
            {
                if(isLocked(m_tagcache->record(i).value("CHILDTAGID").toString()))
                {
                    errorMessage("Cannot add leaf to a locked node");
                    return;
                }
                else
                {
                    break;
                }
            }
        }
    }

    //Gets name of new table
    bool ok;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("New Table Name"), QLineEdit::Normal, selectedRowName(),&ok);
    if(!ok)
    {
        cout<<"Failed to get name for new leaf\n";
        return;
    }
    if(!findAssociated(m_hvsNodeTableModel, QString("NODE_NAME"), text, QString("NODE_ID")).isNull())
    {
        errorMessage("Leaf name input that is already in use");
        return;
    }

    //Get a new node_id for our leaf
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
  
    //This inserts new row inside of of HVS_NODE
    m_hvsNodeTableModel->insertRow( row);
    m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 0), pint);
    m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 1), text);
    m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 2), m_hvsNodeTableModel->record(modelRow).value("NODE_ID"));
    m_hvsNodeTableModel->setData(m_hvsNodeTableModel->index(row, 3), 0);        
    if(!(m_hvsNodeTableModel->database().transaction() && m_hvsNodeTableModel->submitAll() && m_hvsNodeTableModel->database().commit()))
    {
        cout<<"Error editing HVS_NODE when inserting leaf\nError: ";
        cout<<m_hvsNodeTableModel->database().lastError().text().toStdString()<<endl;
    }

    //Also edit tag tables
    QString parentTagName;
    QString parentTag;
    QString currentNodeName = selectedRowName();
    QString currentNodeId = findAssociated(m_hvsNodeTableModel, QString("NODE_NAME"), currentNodeName, QString("NODE_ID"));

    //Need to get unlocked parent tag to make new node under
    //Get from user if root tag isn't set
    if(m_rootDisplayTag.isNull())
    {
        QStringList parentTags = findAssociatedList(m_tag2node, QString("NODE_ID"), currentNodeId, QString("TAG_NAME"));
        for(int i = 0; i < parentTags.size(); i++)
        {
            if( isLocked(parentTags.at(i)))
            {
                parentTags.removeAt(i);
                i--;
            }
        }
        if(parentTags.isEmpty())
        {
            errorMessage("No unlocked parent tag to choose from");
            return;
        }
        else if (parentTags.size() == 1)
        {
            parentTagName = parentTags.at(0);
        }
        else
        {
            parentTagName = QInputDialog::getItem(this, tr("QInputDialog::getItem()"), tr("Choose parent tag for branch:"), parentTags, 0, false, &ok);
            if(!ok)
            {
                cout<<"Failed to get parent tag for leaf\n";
                return;
            }
        }
        parentTag = findAssociated(m_tag2node, QString("TAG_NAME"), parentTagName, QString("TAG_ID"));
                
    }
    else //Case where we have a root tag, use chold tag associated with current
    {
        for(int i = 0; i < m_tagcache->rowCount(); i++)
        {
            if(m_tagcache->record(i).value("ROOTTAG").toString() == m_rootDisplayTag
                && m_tagcache->record(i).value("CHILDNODE").toString() == currentNodeName)
            {
                parentTag = m_tagcache->record(i).value("CHILDTAGID").toString();
                break;
            }
        }
    }
    if(parentTag.isNull())
    {
        errorMessage("Unable to get tag associated with table we are adding a leaf to");
        return;
    }

    //Make changes to tag tables
    //Get new tag number
    int tagAvailable = 0;
    QString newTag;
    for(int i = 0; i < m_tag2node->rowCount(); i++)
    {
        tagAvailable = (m_tag2node->record(i).value("TAG_ID").toString().toInt() > tagAvailable) ? m_tag2node->record(i).value("TAG_ID").toString().toInt() : tagAvailable;
    }
    tagAvailable++;
    newTag = QString::number(tagAvailable);

    //Get new tag name
    QString newTagName;
    for(int i = 0; i < m_tag2node->rowCount(); i++)
    {
        if( m_tag2node->record(i).value("TAG_ID").toString() == parentTag)
        {
            newTagName = m_tag2node->record(i).value("TAG_NAME").toString();
            newTagName.replace(findAssociated(m_tagcache, tr("CHILDTAGID"), parentTag, tr("CHILDNODE")), text);
        }
    }

    //Add to ltag2ltag
    QSqlRecord record = m_ltag2ltag->record(0);
    record.setValue("PARENT_NODE", findAssociated(m_tag2node, tr("TAG_ID"), parentTag, tr("NODE_ID")));
    record.setValue("PARENT_TAG", parentTag);
    record.setValue("CHILD_NODE", QString::number(pint));
    record.setValue("CHILD_TAG", newTag);
    m_ltag2ltag->insertRecord(-1, record);
    record.clear();
    if(!( m_ltag2ltag->database().transaction() && m_ltag2ltag->submitAll() && m_ltag2ltag->database().commit()))
    {
        cout<<"Error editing ltag2ltag when inserting leaf\n Error: ";
        cout<<m_ltag2ltag->database().lastError().text().toStdString()<<endl;
    }

    //Add to tag2node
    record = m_tag2node->record(0);
    record.setValue("NODE_ID", QString::number(pint));
    record.setValue("TAG_NAME", newTagName);
    record.setValue("TAG_ID", newTag);
    record.setNull("TAG_COMMENT");
    record.setValue("LOCKED", tr("0"));
    record.setValue("REPLICATED", tr("0"));
    auto dt = QDateTime::currentMSecsSinceEpoch();
    record.setValue("DATE_CREATED", QString::number(dt));
    record.setNull("DATE_LOCKED");
    m_tag2node->insertRecord(-1, record);
    record.clear();
    if(!(m_tag2node->database().transaction() && m_tag2node->submitAll() && m_tag2node->database().commit()))
    {
        cout<<"Error editing tag2node when inserting leaf\n Error: ";
        cout<<m_tag2node->database().lastError().text().toStdString()<<endl;
    }

    //Add to tagcache
    record = m_tagcache->record(0);
    record.setValue("ROOTTAG", findAssociated( m_tagcache, tr("CHILDTAGID"), parentTag, tr("ROOTTAG")));
    record.setValue("CHILDNODE", text);
    record.setValue("CHILDTAG", newTagName);
    record.setValue("CHILDTAGID", newTag);
    m_tagcache->insertRecord(-1, record);
    if(!m_tagcache->database().transaction() || !m_tagcache->submitAll() || !m_tagcache->database().commit())
    {
        cout<<"Error editing tagcache when inserting leaf\n Error: ";
        cout<<m_tagcache->database().lastError().text().toStdString()<<endl;
    }

    //This will create the _DATA and _DATA2TAG tables in the database
    QString dataname = text.toUpper();
    QString tagname = text.toUpper();
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
    if(!querydata.prepare(dataquery))
    {
        cout<<"prepare fail\n";
    }
    QSqlQuery querydata2tag(m_database);
    if(!querydata2tag.prepare(data2tagquery))
    {
        cout<<"prepare fail\n";
    }
    if( !querydata.exec())
    {
        cout<<querydata.lastError().text().toStdString(); //error in building table
    }
    if( !querydata2tag.exec())
    {
        cout<<querydata2tag.lastError().text().toStdString(); //error in building table
    }


    //Create a first row for each _DATA and _DATA2TAG 
    QSqlQuery dataRow;
    QSqlQuery data2TagRow;
    dataquery = QString("INSERT INTO %1 VALUES (?)").arg(dataname);
    data2tagquery = QString("INSERT INTO %1 VALUES(?, ?)").arg(tagname);
    dataRow.prepare(dataquery);
    data2TagRow.prepare(data2tagquery);
    dataRow.bindValue(0, 1);
    data2TagRow.bindValue(0, tagAvailable);
    data2TagRow.bindValue(1, 1);
    if(!dataRow.exec())
    {
        cout<<dataRow.lastError().text().toStdString();
    }
    if(!data2TagRow.exec())
    {
        cout<<data2TagRow.lastError().text().toStdString();
    }



    rebuildTree();
    
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
    QSqlTableModel tag2node(nullptr, m_database);
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

/* Old function to change roots from right click menu on tree, replaced by list on left
void FaserDbMainWindow::setRoot()
{
    QSqlTableModel tag2node(nullptr, m_database);
    tag2node.setTable("HVS_TAG2NODE");
    tag2node.select();

    bool ok;
    QStringList rootTags = findAssociatedList( &tag2node, QString("NODE_ID"), QString("0"), QString("TAG_NAME"));
    rootTags.push_back(tr("Clear root tag"));
    QString rootTag = QInputDialog::getItem(this, tr("QInputDialog::getText()"), tr("Set root tag:"), rootTags, 0, false, &ok);
    if(!ok)
    {
        cout<<"Failed to get root tag to set\n";
        return;
    }
    if( rootTag == rootTags.last())
    {
        QString nullstr;
        m_rootDisplayTag = nullstr;
        rebuildTree();
        return;
    }
    m_rootDisplayTag = rootTag;

    rebuildTree();
}*/

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

QString FaserDbMainWindow::createRootTag()
{
    //This function is only called in case where we right clicked or selected from menu
    //Creates a new root tag, similar to making a new version of the database

    QString nodeGettingTagName = QString("FASER");
    QString nodeGettingTagId = QString("0");
    QString newTag;
    QString nullTag;

    bool ok = false;


    int tagAvailable = 0;
    for(int i = 0; i < m_tag2node->rowCount(); i++)
    {
        tagAvailable = (m_tag2node->record(i).value("TAG_ID").toString().toInt() > tagAvailable) ? m_tag2node->record(i).value("TAG_ID").toString().toInt() : tagAvailable;
    }
    tagAvailable++;
    newTag = QString::number(tagAvailable);

    QString rootTag = QInputDialog::getItem(this, tr("QInputDialog::getText()"), tr("New Root Tag name:"),
        findAssociatedList( m_tag2node, QString("NODE_ID"), nodeGettingTagId, QString("TAG_NAME")), 0, true, &ok);
    if( !ok)
    {
        cout<<"Error in name given/closed window\n";
        return nullTag;
    }

    //Prevent repeated root name
    for(int i = 0; i < m_tagcache->rowCount(); i++)
    {
        if(m_tagcache->record(i).value(0).toString() == rootTag)
        {
            errorMessage("Root tag name input is already taken");
            return nullTag;
        }
    }

    //Get tag we will make this new one as a replica of
    QStringList replicateList;
    for(int i = 0; i < m_tagcache->rowCount(); i++)
    {
        if( !replicateList.contains(m_tagcache->record(i).value("ROOTTAG").toString()))
        {
            replicateList.push_back(m_tagcache->record(i).value("ROOTTAG").toString());
        }
    }
    QString replicateTagName = QInputDialog::getItem(this, tr("QInputDialog::getItem()"), tr("Root tag to initially copy:"), replicateList, 0, false, &ok);
    if( !ok)
    {
        cout<<"Failed to get tag to replicate to/closed window\n";
        return nullTag;
    }

    QString replicateTagId = findAssociated(m_tag2node, QString("TAG_NAME"), replicateTagName, QString("TAG_ID"));

    //Get list of children who will be under our new tag so we put proper children under our new tag
    QStringList childTags = findAssociatedList( m_tagcache, tr("ROOTTAG"), replicateTagName, tr("CHILDTAG"));

    //Valid root tag name
    //Make appropriate row inserts in each table
    //Get highest tag value then increment
    //Edit ltag2ltag
    for(int i = 0; i < m_ltag2ltag->rowCount(); i++)
    {
        if( m_ltag2ltag->record(i).value("PARENT_NODE").toString() == "0" 
            && m_ltag2ltag->record(i).value("PARENT_TAG").toString() == replicateTagId)
        {
            QSqlRecord record = m_ltag2ltag->record(i);
            record.setValue("PARENT_TAG", newTag);

            m_ltag2ltag->insertRecord(-1, record);
        }
    }
    submitChanges(m_ltag2ltag);
/*        if(!(m_ltag2ltag->database().transaction() && m_ltag2ltag->submitAll() && m_ltag2ltag->database().commit()))
    {
        cout<<"Error editing ltag2ltag when inserting new root tag\n Error: ";
        cout<<m_ltag2ltag->database().lastError().text().toStdString()<<endl;
    }*/

    //Edit tag2node
    if(!( m_tag2node->database().transaction()))
    {
        cout<<"Error starting transaction : "<<m_tag2node->database().lastError().text().toStdString()<<endl;
    }
    for(int i = 0; i < m_tag2node->rowCount(); i++)
    {
        if(  m_tag2node->record(i).value("NODE_ID").toString() == "0")
        {
/*                QSqlRecord record = m_tag2node->record(i);
            record.setValue("TAG_NAME", rootTag);
            record.setValue("TAG_ID", newTag);
            record.setNull("TAG_COMMENT");
            record.setValue("LOCKED", QString("0"));
            record.setValue("REPLICATED", QString("0"));
            auto dt = QDateTime::currentMSecsSinceEpoch();
            record.setValue("DATE_CREATED", QString::number(dt));
            record.setNull("DATE_LOCKED");

            if(!m_tag2node->insertRecord(-1, record))
            {
                cout<<"failed to insert tag2node record\n";
                cout<<m_tag2node->lastError().text().toStdString()<<endl;
            }
            int row = m_tag2node->rowCount();
            if(!m_tag2node->insertRow(row))
            {
                cout<<"failed to insert row\n";
            }             
            m_tag2node->record(row).setValue("TAG_NAME", rootTag);
            m_tag2node->record(row).setValue("TAG_ID", newTag);
            m_tag2node->record(row).setNull("TAG_COMMENT");
            m_tag2node->record(row).setValue("LOCKED", QString("0"));
            m_tag2node->record(row).setValue("REPLICATED", QString("0"));
            auto dt = QDateTime::currentMSecsSinceEpoch();
            m_tag2node->record(row).setValue("DATE_CREATED", QString::number(dt));
            m_tag2node->record(row).setNull("DATE_LOCKED");*/

            QSqlQuery query;
            query.prepare("INSERT INTO HVS_TAG2NODE (NODE_ID, TAG_NAME, TAG_ID, TAG_COMMENT, LOCKED, REPLICATED, DATE_CREATED, DATE_LOCKED, SUPPORTED)"
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
            query.bindValue(0, "0");
            query.bindValue(1, rootTag);
            query.bindValue(2, newTag);
            query.bindValue(3, QString());
            query.bindValue(4, "0");
            query.bindValue(5, "0");
            auto dt = QDateTime::currentMSecsSinceEpoch();
            query.bindValue(6, QString::number(dt));
            query.bindValue(7, QString());
            query.bindValue(8, m_tag2node->record(i).value("SUPPORTED").toString());
            if(!query.exec())
            {
                cout<<query.lastError().text().toStdString()<<endl;
            }
            m_tag2node->select();

            break;
        }
    }
//        submitChanges(m_tag2node);
    if(!(/*m_tag2node->database().transaction() &&*/ m_tag2node->submitAll() && m_tag2node->database().commit()))
    {
        cout<<"Error editing tag2node when inserting new root tag\n Error: ";
        cout<<m_tag2node->database().lastError().text().toStdString()<<endl;
    }

    //Edit tagcache
    m_tagcache->select();
    for(int i = 0; i < m_tagcache->rowCount(); i++)
    {
        if(m_tagcache->record(i).value("ROOTTAG").toString() == replicateTagName)
        {
            QSqlRecord record = m_tagcache->record(i);
            record.setValue("ROOTTAG", rootTag);
            if(m_tagcache->record(i).value("CHILDNODE").toString() == QString("FASER"))
            {
                record.setValue("CHILDTAG", rootTag);
                record.setValue("CHILDTAGID", newTag);
            }

            m_tagcache->insertRecord(-1, record);
        }
    }
    submitChanges(m_tagcache);
/*        if(!( m_tagcache->database().transaction() && m_tagcache->submitAll() && m_tagcache->database().commit()))
    {
        cout<<"Error editing tagcache when inserting new root tag\n Error: ";
        cout<<m_tagcache->database().lastError().text().toStdString()<<endl;
    }*/
    buildListWidget();
    m_rootDisplayTag = rootTag;
    m_listWidget->setCurrentRow(m_listWidget->count()-1);
    return newTag;

}

QString FaserDbMainWindow::createTag()
{
    //This function only works if used on root, or on a node whose parent has a unlocked tag
    //Get tag we are creating under
    QString nodeGettingTagName = selectedRowName();
    QString nodeGettingTagId = findAssociated( m_hvsNodeTableModel, QString("NODE_NAME"), nodeGettingTagName, QString("NODE_ID"));
    QString newTag;
    QString nullTag;

    bool ok = false;


    int tagAvailable = 0;
    for(int i = 0; i < m_tag2node->rowCount(); i++)
    {
        tagAvailable = (m_tag2node->record(i).value("TAG_ID").toString().toInt() > tagAvailable) ? m_tag2node->record(i).value("TAG_ID").toString().toInt() : tagAvailable;
    }
    tagAvailable++;
    newTag = QString::number(tagAvailable);



    //Case of adding tag to non root
    //Get list of unlocked parents
    QStringList parentTags = findAssociatedList( m_ltag2ltag, QString("CHILD_NODE"), nodeGettingTagId, QString("PARENT_TAG"));
    for(int i = 0; i < parentTags.size(); i++)
    {
        if( isLocked(parentTags.at(i)))
        {
            parentTags.removeAt(i);
            i = 0;
        }
    }
    //Case of no unlocked parent tags, can't add tag
    if(parentTags.empty())
    {
        errorMessage("No unlocked parent tags to choose from");
        return nullTag;
    }

    //Get parent tag to make under
    QStringList parentTagNames;
    for(int i = 0; i < parentTags.size(); i++)
    {
        parentTagNames.push_back( findAssociated( m_tag2node, QString("TAG_ID"), parentTags.at(i), QString("TAG_NAME")));
    }
    QString parentTagName = QInputDialog::getItem(this, tr("QInputDialog::getItem()"), tr("Unlocked tag to set as parent of new tag:"), parentTagNames, 0, false, &ok);
    QString parentTag = findAssociated(m_tag2node, QString("TAG_NAME"), parentTagName, QString("TAG_ID"));

    if(!ok)
    { //user exited choosing
        cout<<"Failed to obtain parent tag for new tag\n";
        return nullTag;
    }

    //Now we need to choose an existing tag ours will be a copy of
    QStringList currentTags = findAssociatedList( m_tag2node, QString("NODE_ID"), nodeGettingTagId, QString("TAG_NAME"));
    QString copyTagName = QInputDialog::getItem(this, tr("QInputDialog::getItem()"), tr("Choose tag to initialize as copy of:"), currentTags, 0, false, &ok);
    if( !ok)
    {//user closed window
        cout<<"No tag to copy passed through\n";
        return nullTag;
    }
    QString copyTag = findAssociated(m_tag2node, QString("TAG_NAME"), copyTagName, QString("TAG_ID"));
    QString parentName = findAssociated(m_tagcache, QString("CHILDTAG"), parentTagName, QString("CHILDNODE"));

    //Lastly get name of new tag
    QString temp = parentTagName;
    QString newTagName = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("New Tag name:"), QLineEdit::Normal, temp.replace(parentName, nodeGettingTagName), &ok);
    if( !ok || currentTags.contains(newTagName))
    {
        cout<<"Failed to get new tag name/input repeated tag name\n";
        return nullTag;
    }

    //Now we edit the three tables, inserting our new tag
    //Edit ltag2ltag
    if(!m_ltag2ltag->database().transaction())
    {
        cout<<"failed transaction : "<<m_ltag2ltag->database().lastError().text().toStdString()<<endl;
    }
    for(int i = 0; i < m_ltag2ltag->rowCount(); i++)
    {
        if(m_ltag2ltag->record(i).value("PARENT_TAG").toString() == parentTag
            && m_ltag2ltag->record(i).value("CHILD_TAG").toString() == copyTag)
        {
            m_ltag2ltag->record(i).setValue("CHILD_TAG", newTag);

        }
        if(m_ltag2ltag->record(i).value("PARENT_TAG").toString() == copyTag)
        {
            QSqlRecord record = m_ltag2ltag->record(i);
            record.setValue("PARENT_TAG", newTag);

            m_ltag2ltag->insertRecord(-1, record);
        }
    }
//    submitChanges(m_ltag2ltag);
    if(!( /*m_ltag2ltag->database().transaction() &&*/ m_ltag2ltag->submitAll() && m_ltag2ltag->database().commit()))
    {
        cout<<"Error editing ltag2ltag when inserting tag\n Error: ";
        cout<<m_ltag2ltag->database().lastError().text().toStdString()<<endl;
    }

    //Edit tag2node
    if(!( m_tag2node->database().transaction()))
    {
        cout<<"Error starting transaction : "<<m_tag2node->database().lastError().text().toStdString()<<endl;
    }
    
    for(int i = 0; i < m_tag2node->rowCount(); i++)
    {
        if(m_tag2node->record(i).value("TAG_ID").toString() == copyTag)
        {
/*            QSqlRecord record = m_tag2node->record(i);
            record.setNull("TAG_COMMENT");
            record.setValue("LOCKED", QString("0"));
            record.setValue("REPLICATED", QString("0"));
            record.setValue("TAG_NAME", newTagName);
            record.setValue("TAG_ID", newTag);
            record.setNull("DATE_LOCKED");
            auto dt = QDateTime::currentMSecsSinceEpoch();
            record.setValue("DATE_CREATED", QString::number(dt));

            if(!m_tag2node->insertRecord(-1, record))
            {
                cout<<"failed to insert tag2node record?\n";
                cout<<m_tag2node->lastError().text().toStdString()<<endl;
            }*/

            QSqlQuery query;
            query.prepare("INSERT INTO HVS_TAG2NODE (NODE_ID, TAG_NAME, TAG_ID, TAG_COMMENT, LOCKED, REPLICATED, DATE_CREATED, DATE_LOCKED, SUPPORTED)"
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
            query.bindValue(0, m_tag2node->record(i).value("NODE_ID").toString());
            query.bindValue(1, newTagName);
            query.bindValue(2, newTag);
            query.bindValue(3, QString());
            query.bindValue(4, "0");
            query.bindValue(5, "0");
            auto dt = QDateTime::currentMSecsSinceEpoch();
            query.bindValue(6, QString::number(dt));
            query.bindValue(7, QString());
            query.bindValue(8, m_tag2node->record(i).value("SUPPORTED").toString());
            if(!query.exec())
            {
                cout<<query.lastError().text().toStdString()<<endl;
            }
            m_tag2node->select();

            break;
        }
    }
//    submitChanges(m_tag2node);
    if(!( /*m_tag2node->database().transaction() &&*/ m_tag2node->submitAll() && m_tag2node->database().commit()))
    {
        cout<<"Error editing tag2node when inserting tag\n Error: ";
        cout<<m_tag2node->database().lastError().text().toStdString()<<endl;
    }

    //Edit tagcache
    QString rootTag = findAssociated( m_tagcache, QString("CHILDTAG"), parentTagName, QString("ROOTTAG"));
    if(!m_tagcache->database().transaction())
    {
        cout<<"Error starting tagcache transaction : "<<m_tagcache->database().lastError().text().toStdString()<<endl;
    }
    for(int i = 0; i < m_tagcache->rowCount(); i++)
    {
        if(m_tagcache->record(i).value("CHILDTAGID").toString() == copyTag
            && m_tagcache->record(i).value("ROOTTAG").toString() == rootTag)
        {
            QSqlRecord record = m_tagcache->record(i);
            record.setValue("CHILDTAG", newTagName);
            record.setValue("CHILDTAGID", newTag);

            m_tagcache->setRecord(i, record);
            break;
        }
    }
//    submitChanges(m_tagcache);
    if(!( /*m_tagcache->database().transaction() &&*/ m_tagcache->submitAll() && m_tagcache->database().commit()))
    {
        cout<<"Error editing tagcache when inserting tag\n Error: ";
        cout<<m_tagcache->database().lastError().text().toStdString()<<endl;
    }

    //If we are working on a leaf node, also need to add new rows into correponding _DATA2TAG tables
    QString name = selectedRowName();
    name.append("_DATA2TAG");
    QSqlTableModel data2tag;
    data2tag.setTable(name);
    data2tag.select();

    for(int i = 0; i <data2tag.rowCount(); i++)
    {
        if(data2tag.record(i).value(0).toString() == copyTag)
        {
            QSqlRecord record = data2tag.record(i);
            record.setValue(0, newTag);

            data2tag.insertRecord(-1, record);
        }
    }
    submitChanges(&data2tag);



    rebuildTree();
    return newTag;
}

//Function locks a tag. If the tag is a parent it recursively locks all  children tags too
void FaserDbMainWindow::lockTag(QString toLock)
{
    bool original = false;
    //If function was called by action menu need to get tag to lock
    if( toLock.isNull())
    {
        original = true;
        QString currentRow = m_currentSelected;
        if(currentRow.endsWith("_DATA") || currentRow.endsWith("_DATA2TAG"))
        {
            cout<<"Tried to lock tag at _DATA or _DATA2TAG level\n";
            return;
        }

        //Get list of unlocked tags associated with current table
        //We want to show the user the names of tags, not tags themself
        QStringList tagNames = findAssociatedList(m_tag2node, QString("NODE_ID"), findAssociated(m_hvsNodeTableModel, QString("NODE_NAME"), currentRow, QString("NODE_ID")), QString("TAG_NAME"));
        for(int i = 0; i < tagNames.size(); i++)
        {
            if( isLocked(findAssociated(m_tag2node, QString("TAG_NAME"), tagNames.at(i), QString("LOCKED"))))
            {
                tagNames.removeAt(i);
                i--;
            }
        }
        //We want to show the user the names of tags, not tags themself
        bool ok;
        QString toLockName = QInputDialog::getItem(this, tr("QInputDialog::getItem()"), tr("Choose tag to lock:"), tagNames, 0, false, &ok);
        if(!ok)
        {
            cout<<"Failed to get tag to lock from user\n";
            return;
        }
        toLock = findAssociated(m_tag2node, QString("TAG_NAME"), toLockName, QString("TAG_ID"));
    }

    //Lock tag and recursively call for each child tag 
    for(int i = 0; i < m_tag2node->rowCount(); i++)
    {
        if(m_tag2node->record(i).value("TAG_ID").toString() == toLock)
        {
            if(!m_tag2node->database().transaction())
            {
                cout<<"Error starting transaction in locking tag : "<<m_tag2node->database().lastError().text().toStdString()<<endl;
            }
            QModelIndex index = m_tag2node->index(i, 4);
            m_tag2node->setData(index, QString("1"));
            index = m_tag2node->index(i, 7);
            auto dt = QDateTime::currentMSecsSinceEpoch();
            m_tag2node->setData(index, QString::number(dt));
/*            submitChanges(m_tag2node);
            m_tag2node->database().transaction();
            m_tag2node->submitAll();
            m_tag2node->database().commit();*/
            if(!( m_tag2node->submitAll() &&  m_tag2node->database().commit()))
            {
                cout<<"Error locking tags : "<<m_tag2node->database().lastError().text().toStdString()<<endl;
            }
                
            QStringList toLockList = findAssociatedList(m_ltag2ltag, QString("PARENT_TAG"), toLock, QString("CHILD_TAG"));
            for(int i = 0; i < toLockList.size(); i++)
            {
                lockTag(toLockList.at(i));
            }
            break;
        }
    }

    if(original)
    {

        if(!(m_tag2node->database().transaction() && m_tag2node->submitAll() && m_tag2node->database().commit()))
        {
            cout<<"Error editing tag2node when inserting locking tags tag\n Error: ";
            cout<<m_tag2node->database().lastError().text().toStdString()<<endl;
        }
        m_secondWindow->clearWindow();
        rebuildTree();
        m_secondWindow->setWindow(m_currentSelected);
    }
    return;
}

//Function commits changes to database;
bool FaserDbMainWindow::submitChanges(QSqlTableModel *table)
{
    if(! (table->database().transaction() && table->submitAll() && table->database().commit()))
    {
        cout<<"Error submitting changes to table "<<table->tableName().toStdString()<<endl;
        cout<<table->lastError().text().toStdString()<<endl;
        cout<<table->database().lastError().text().toStdString()<<endl;
        return false;
    }
    table->select();
    return true;
}


void FaserDbMainWindow::errorMessage(string message)
{

//    QMessageBox messageBox;
    QMessageBox::critical(0, "Error", QString::fromStdString(message));
//    messageBox.setFixedSize(500,200);
//    messageBox.exec();
}

void FaserDbMainWindow::buildListWidget()
{
    m_listWidget->clearSelection();
    m_listWidget->clearFocus();
    m_listWidget->clear();
    new QListWidgetItem(QString("No root tag set"), m_listWidget);
//    QSQlTableModel model;
//    model.setTable("HVS_TAG2NODE");
//    model.select();
    for(int i = 0; i < m_tag2node->rowCount(); i++)
    {
        if(m_tag2node->record(i).value("NODE_ID").toString() == "0")
        {
            new QListWidgetItem(m_tag2node->record(i).value("TAG_NAME").toString(), m_listWidget);
        }
    }
}

void FaserDbMainWindow::tagRowChanged(int currentRow)
{
    if(currentRow <= 0)
    {
        QString nullstr;
        m_rootDisplayTag = nullstr;
        m_secondWindow->clearWindow();
        rebuildTree();
        return;
    }
    else
    {
        m_rootDisplayTag = m_listWidget->item(currentRow)->text();
        m_secondWindow->clearWindow();
        rebuildTree();
        return;
    }

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
//This function should only be called when we edit _DATA or _DATA2TAG, so no need to rebuild
//        m_parentWindow->rebuildTree();
    }
    else
    {
        m_tableModel->database().rollback();
        QMessageBox::warning(this, tr("Edit Table"), tr("The database reported an error: %1").arg(m_tableModel->lastError().text()));
    }
    
}

void FaserDbSecondWindow::addRow()
{
/*    QSqlRecord record;
    record = m_tableModel->record(0);
    if(!m_parentWindow->m_rootDisplayTag.isNull())
    {
        for(int i = 0; i < m_parentWindow->m_tagcache->rowCount(); i++)
        {
            if(m_parentWindow->m_tagcache->record(i).value("ROOTTAG").toString() == m_parentWindow->m_rootDisplayTag
                && m_parentWindow->m_tagcache->record(i).value("CHILDNODE").toString() == m_parentWindow->m_rootDisplayTag)
        }
        record.setValue(0, )
    }*/

    //Cant add row if its to a locked table
    if(!m_parentWindow->m_rootDisplayTag.isNull())
    {
        QString nodeName = m_parentWindow->selectedRowName();
        if(nodeName.endsWith("_DATA"))
        {
            nodeName.remove("_DATA");
        }
        else
        {
            nodeName.remove("_DATA2TAG");
        }
        for(int i = 0; i < m_parentWindow->m_tagcache->rowCount(); i++)
        {
            if(m_parentWindow->m_tagcache->record(i).value("ROOTTAG").toString() == m_parentWindow->m_rootDisplayTag
                && m_parentWindow->m_tagcache->record(i).value("CHILDNODE").toString() == nodeName)
            {
                if(m_parentWindow->isLocked(m_parentWindow->m_tagcache->record(i).value("CHILDTAGID").toString()))
                {
                    m_parentWindow->errorMessage("Table is currently associated with locked tag");
                    return;
                }
                else
                {
                    break;
                }
            }
        }
    }
    if(m_tableView->selectionModel()->currentIndex().isValid())
    {
        m_tableModel->insertRows(m_tableView->selectionModel()->currentIndex().row() + 1, 1);
    }
    else
    {
        //This executes only if there was no valid index, and then puts a new row at the end
        m_tableModel->insertRows(m_tableModel->rowCount(), 1);
    }
}

void FaserDbSecondWindow::addColumn()
{
    //Cant add column if any of the associated tags are locked
    QString nodeName = m_parentWindow->selectedRowName();
    if(nodeName.endsWith("_DATA"))
    {
        nodeName.remove("_DATA");
    }
    else
    {
        m_parentWindow->errorMessage("Cannot add columns to data2tag tables");
        return;
    }
    for(int i = 0; i < m_parentWindow->m_tagcache->rowCount(); i++)
    {
        if( m_parentWindow->m_tagcache->record(i).value("CHILDNODE").toString() == nodeName)
        {
            if(m_parentWindow->isLocked(m_parentWindow->m_tagcache->record(i).value("CHILDTAGID").toString()))
            {
                m_parentWindow->errorMessage("Cannot add a column to a table associated with a locked tag");
                return;
            }
            else
            {
                break;
            }
        }
    }

    bool oktext;
    QString text = QInputDialog::getText(this, tr("QInputDialog::getText()"), tr("New Column Name"), QLineEdit::Normal, QString(),&oktext);

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
            cout<<"Trying to delete data from a locked column\n";
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
    m_sqlTableModel->removeRows(m_tableView->selectionModel()->currentIndex().row(), 1);
//    m_tableModel->removeRows(m_tableView->selectionModel()->currentIndex().row(), 1);
//Hide row/reshow if revert later
//    m_tableView->hideRow(m_tableView->selectionModel()->currentIndex().row());
}

FaserDbSecondWindow::FaserDbSecondWindow(FaserDbMainWindow *window_parent, QWidget* parent)
    : QWidget(parent)
    , m_tableModel(new FaserDbRelTableModel(nullptr, window_parent->returnDatabase(), this))
    , m_tagTable(new QSqlTableModel(nullptr, window_parent->returnDatabase() ))
    , m_sqlTableModel(new QSqlTableModel(nullptr, window_parent->returnDatabase()))
   /* , m_standardModel(new QStandardItemModel(this))*/
{
//    setCentralWidget(m_tableModel);
    m_tableView = nullptr;
    m_parentWindow = window_parent;

    return;
}


void FaserDbSecondWindow::setWindow(QString tableName)
{
    if(!(tableName.endsWith("_DATA") || tableName.endsWith("_DATA2TAG")))
    {
        return;
    }
    m_tableModel->setTable(tableName);
    m_tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

    m_sqlTableModel->setTable(tableName);
    m_sqlTableModel->select();

    if( tableName.endsWith("_DATA2TAG") )
    {
        m_tagTable->setTable(tableName);
        m_tagTable->select();
        QSqlQuery query;
        query.prepare("CREATE TABLE IF NOT EXISTS temp_tags (TAG_ID SLONGLONG UNIQUE, TAG_NAME TEXT UNIQUE)");
        query.exec();

        QSqlTableModel model;
        model.setTable("temp_tags");
        model.select();

        QString nameless = tableName;
        nameless.remove("_DATA2TAG");
        for(int i = 0; i < m_parentWindow->m_tagcache->rowCount(); i++)
        {
            if(m_parentWindow->m_tagcache->record(i).value("CHILDNODE").toString() == nameless )
//                && (m_parentWindow->m_rootDisplayTag.isNull() || m_parentWindow->m_tagcache->record(i).value("ROOTTAG").toString() == m_parentWindow->m_rootDisplayTag))
            {
                QSqlRecord record = model.record();
                record.setValue("TAG_ID", m_parentWindow->m_tagcache->record(i).value("CHILDTAGID").toString());
                record.setValue("TAG_NAME", m_parentWindow->m_tagcache->record(i).value("CHILDTAG").toString());

                if(!model.insertRecord(-1, record))
                {
                    cout<<"inserting record failed for temp tags\n";
                }
            }
        }

        

        m_tableModel->setRelation(0, QSqlRelation("temp_tags", "TAG_ID", "TAG_NAME"));
    }
    else
    {
        QString name = tableName;
        name.append("2TAG");
        m_tagTable->setTable(name);
        m_tagTable->select();
    }

    m_tableModel->select();

    m_tableView = new QTableView;
    m_tableView->setModel(m_tableModel);
    m_tableView->resizeColumnsToContents();
    m_tableView->setMinimumWidth(500);

    m_tableView->setItemDelegate(new QSqlRelationalDelegate(m_tableView));

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

    //Code to deal with when we have  a tag filter set
    if( !m_parentWindow->m_rootDisplayTag.isNull())
    {
        QSqlTableModel tagcache(nullptr, m_parentWindow->returnDatabase());
        tagcache.setTable("HVS_TAGCACHE");
        tagcache.select();
        QString nodeName = m_parentWindow->selectedRowName();
        if(nodeName.endsWith("_DATA"))
        {
            nodeName.remove("_DATA");
        }
        else
        {
            nodeName.remove("_DATA2TAG");
        }
        
        //Need to find the tag for our current table under given root tag
        for(int i = 0; i < tagcache.rowCount(); i++)
        {
            if(tagcache.record(i).value("ROOTTAG").toString() == m_parentWindow->m_rootDisplayTag
            && tagcache.record(i).value("CHILDNODE").toString() == nodeName)
            {
                setTagFilter(tagcache.record(i).value("CHILDTAGID").toString());
                break;
            }
        }
    }
}

void FaserDbSecondWindow::clearWindow()
{
    if(m_tableView != nullptr)
    {
        QSqlQuery query;
        query.prepare("DROP TABLE IF EXISTS temp_tags");
        if(!query.exec())
        {
            cout<<"failed clear window drop table\n";
        }
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
    //Old code for when we didnt use temp tags relational table, automatically filters now for _DATA2TAG tables
        for(int i = 0; i < m_tagTable->rowCount(); i++)
        {
            if(m_tagTable->record(i).value(0).toString() != tagFilter)
            {
                m_tableView->hideRow(i);
            }
        }
        return;
    }

    //Case where we are looking at data table
    //Need to get a vector of data id's we will permit
    vector<QString> nodeIds;

    for(int i = 0; i < m_tagTable->rowCount(); i++)
    {
        if( m_tagTable->record(i).value(0).toString() == tagFilter)
        {
            nodeIds.push_back(m_tagTable->record(i).value(1).toString());
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

//Destructor to delete temp tags table used for our relational table model
FaserDbSecondWindow::~FaserDbSecondWindow()
{
    clearWindow();
}

FaserDbRelTableModel::FaserDbRelTableModel(QObject *parent = nullptr, QSqlDatabase db = QSqlDatabase(), FaserDbSecondWindow *secondWin = nullptr)
{
    m_secondWindow = secondWin;
    (void)parent;
    (void)db;
}

//Following function override recolors rows based off of if they are associated with locked rows
//Red = locked, yellow = doubley locked, green = unlocked
QVariant FaserDbRelTableModel::data(const QModelIndex &idx, int role) const
{
    QVariant var = QSqlRelationalTableModel::data(idx, role);

    if( role == Qt::BackgroundRole)
    {
        //Case where this is a data2tag table
        FaserDbMainWindow *main = m_secondWindow->m_parentWindow;
        if(this->tableName().endsWith("_DATA2TAG"))
        {
            QString curtag = main->findAssociated(main->m_tagcache, QString("CHILDTAG"), this->record(idx.row()).value(0).toString(), QString("CHILDTAGID"));
            return (main->isLocked(curtag)) ? QVariant(QColor(Qt::red)) : QVariant(QColor(Qt::green));
        }
        else //Case where it is a data table
        {
            //Need a list of associated tags with current data id
            QString curDatId = this->record(idx.row()).value(0).toString().toUpper();
            QString dataName = this->tableName().replace("_DATA", "_DATA_ID").toUpper();
            QString tagName = this->tableName().replace("_DATA", "_TAG_ID").toUpper();
            QStringList tags = main->findAssociatedList(m_secondWindow->m_tagTable, dataName, curDatId, tagName);
            if(tags.size() == 0)
            {
                return QVariant(QColor(Qt::white));
            }
            else
            {
                if( tags.size() == 1)
                {
                    return (main->isLocked(tags.at(0))) ? QVariant(QColor(Qt::red)) : QVariant(QColor(Qt::green));
                }
                int numNotLocked = 0;
                for(int i = 0; i < tags.size(); i++)
                {
                    if(main->isLocked(tags.at(0)))
                    {
                        return QVariant(QColor(Qt::red));
                    }
                    numNotLocked++;
                }
                if(numNotLocked == 1)
                {
                    return QVariant(QColor(Qt::green));
                }
                else
                {
                    return QVariant(QColor(Qt::yellow));
                }

            }
        }
    }

    return var;
}

//x
//Following fucntion is reimplementation of flags function
//Reimplemented to return a non-editable flag for rows where data is locked
Qt::ItemFlags FaserDbRelTableModel::flags(const QModelIndex &index) const
{
Qt::ItemFlags flags;

    flags = QAbstractItemModel::flags(index);
    int row = index.row();
    QString tagId;

    //If tag table
    if(m_secondWindow->m_sqlTableModel->tableName().endsWith("_DATA2TAG"))
    {
        tagId = m_secondWindow->m_parentWindow->findAssociated(m_secondWindow->m_parentWindow->m_tag2node, QString("TAG_NAME"), 
                record(row).value(0).toString(), QString("TAG_ID"));
    }
    else //If data, want associated tag number
    {
        if( !record(row).value(0).toString().isNull())
        {
//            tagId = m_secondWindow->m_parentWindow->findAssociated(m_secondWindow->m_parentWindow->m_tagTable, QString("TAG_NAME"), 
//                record(row).value(0).toString(), QString("TAG_ID"));
            for(int i = 0; i < m_secondWindow->m_tagTable->rowCount(); i++)
            {
                if(m_secondWindow->m_tagTable->record(i).value(1).toString() == record(row).value(0).toString())
                {
                    tagId = m_secondWindow->m_tagTable->record(i).value(0).toString();
                    if(m_secondWindow->m_parentWindow->isLocked(tagId))
                    {
                        flags = flags & ~Qt::ItemIsEditable;
                        return flags;
                    }
                }
            }
        }
    }


    if( !m_secondWindow->m_parentWindow->isLocked(tagId) || tagId.isNull())
    {
        flags |= Qt::ItemIsEditable;
    }
    return flags;


}




