// main.cpp
#include "FaserGeoEditorApp.h"
#include <iostream>
// #include <QTableView>
// #include <QStringList>
// #include <QSqlDatabase>
// #include <iostream>
// #include "mymodel.h"

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        std::cout<<"Error: Require two command line inputs only, first path to application, second path to database\n";
        std::cout<<"Example: bin/FaserGeoEditor data/geomDB_sqlite\n";
        return 0;
    }
    FaserGeoEditorApp a(argc, argv);
    // QApplication a(argc, argv);

    // QTableView tableView;
    // MyModel myModel;
    // tableView.setModel(&myModel);
    // tableView.show();
    // FaserDbMainWindow w;
    // w.show();
    return a.exec();
}
