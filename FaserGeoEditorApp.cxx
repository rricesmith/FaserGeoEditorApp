#include <iostream>
#include "FaserGeoEditorApp.h"

FaserGeoEditorApp::FaserGeoEditorApp(int& argc, char** argv) 
: m_application(argc, argv), m_database(QSqlDatabase::addDatabase("QSQLITE")), m_mainWindow()
{ 
// db file name should be first argument after command name; retrieve it
    auto arguments = m_application.arguments();
    auto dbName = arguments.at(1);
    std::cout << "Database name: " << dbName.toLocal8Bit().constData() << std::endl;

    m_database.setDatabaseName(dbName);
    bool ok = m_database.open();
    std::cout << "Database open status: " << ok << std::endl;
    m_mainWindow.setDatabase(&m_database);
    m_mainWindow.initializeWindow();

//    auto tables = m_database.tables();
//    for (auto it = tables.constBegin(); it != tables.constEnd(); ++it) std::cout << (*it).toLocal8Bit().constData() << std::endl;
}

int FaserGeoEditorApp::exec()
{
    m_mainWindow.showAll();
    return m_application.exec();
}