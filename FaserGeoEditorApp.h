//#ifndef GEOMODEL_FASERGEOEDITORAPP_H
//#define GEOMODEL_FASERGEOEDITORAPP_H
#include <QStringList>
#include <QApplication>
#include <QSqlDatabase>
#include "FaserDbMainWindow.h"


class FaserGeoEditorApp
{
    public:
        FaserGeoEditorApp(int& argc, char** argv);

        int exec();
        QSqlDatabase& getDatabase() { return m_database; }
    private:
        QApplication m_application;
        QSqlDatabase m_database;
        FaserDbMainWindow m_mainWindow;
};


//#endif