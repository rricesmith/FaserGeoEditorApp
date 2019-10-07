#include <iostream>
#include <QFile>
#include "FaserGeoEditorApp.h"

FaserGeoEditorApp::FaserGeoEditorApp(int& argc, char** argv, string type) 
: m_application(argc, argv), m_database(QSqlDatabase::addDatabase("QSQLITE")), m_mainWindow(nullptr, this)
{ 
    //Need to separate cases of opening a text and binary file
/*    if(type = "t")
    {
        //do something here
    }
    else
    {
        m_application = new QApplication(argc, argv);
        m_database = new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE"));
        m_mainWindow = new FaserDbMainWindow(nullptr, this);
    }*/
    

// db file name should be first argument after command name; retrieve it
    auto arguments = m_application.arguments();
    auto dbName = arguments.at(1);
    std::cout << "Database name: " << dbName.toLocal8Bit().constData() << std::endl;

    bool ok;
    cout<<"type:"<<type<<endl;
    if( strcmp(type.c_str(), "b") == 0)
    {
        m_database.setDatabaseName(dbName);
        ok = m_database.open();
    }
    else
    {
        cout<<"Binary file will initially be saved to current directory as GeoEditorSqlite\n";
        m_database.setDatabaseName("GeoEditorSqlite");
        ok = m_database.open();
    
        if(ExecuteSqlScriptFile( m_database, dbName) == 0)
        {
            cout<<"Exiting...\n";
            return;
        }
    }
    
    std::cout << "Database open status: " << ok << std::endl;
    if(!ok)
    {
        cout<<"Failed to open database, exiting...\n";
        return;
    }
    m_mainWindow.setDatabase(&m_database);
    m_mainWindow.initializeWindow();

//    auto tables = m_database.tables();
//    for (auto it = tables.constBegin(); it != tables.constEnd(); ++it) std::cout << (*it).toLocal8Bit().constData() << std::endl;
}

int FaserGeoEditorApp::exec()
{
    m_mainWindow.show();
    return m_application.exec();
}

int FaserGeoEditorApp::ExecuteSqlScriptFile(QSqlDatabase & db, const QString & fileName)
{
    cout<<"executing\n";
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        cout<<"Error opening file to save to\n";
        return  0;
    }

    QTextStream in(&file);
    QString sql = in.readAll();
    QStringList sqlStatements = sql.split(';', QString::SkipEmptyParts);
    int successCount = 0;
 
    foreach(const QString& statement, sqlStatements)
    {
        if (statement.trimmed() != "")
        {
            QSqlQuery query(db);
            if (query.exec(statement))
                successCount++;
            else
                std::cout << "Failed:" << statement.toStdString() << "\nReason:" << query.lastError().text().toStdString()<<endl;
        }
    }
    return successCount;
}
