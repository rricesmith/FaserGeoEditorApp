// main.cpp
#include "FaserGeoEditorApp.h"
#include <iostream>
// #include <QTableView>
// #include <QStringList>
// #include <QSqlDatabase>
// #include <iostream>
// #include "mymodel.h"
//#include <string.h>
//#include <sqlite3.h>
//#include <fstream>

//string textToBinary(string textFileName, string binFileName = std::string(""));

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        std::cout<<"Error: Require two command line inputs only, first path to application, second path to database, third to indicate binary vs text file\n";
        std::cout<<"Example: bin/FaserGeoEditor data/geomDB_sqlite t\n";
        return 0;
    }
    string type;
    string fileName;
    if( strcmp(argv[2], "t") || strcmp(argv[2], "T"))
    {
        type = "t";
//        fileName = textToBinary(argv[1]);
    }
    else if( strcmp(argv[2], "b") || strcmp(argv[2], "B"))
    {
        type = "b";
    }
    else    
    {
        std::cout<<argv[2]<<endl;
        std::cout<<"Error: third argument incorrectly indicating if file is text or binary; only use t or b to indicate\n";
        return 0;
    }
    char *reducedArgv[2];
    reducedArgv[0] = argv[0];
    reducedArgv[1] = argv[1];
    argc = 2;
    
    FaserGeoEditorApp a(argc, reducedArgv, type);
    // QApplication a(argc, argv);

    // QTableView tableView;
    // MyModel myModel;
    // tableView.setModel(&myModel);
    // tableView.show();
    // FaserDbMainWindow w;
    // w.show();
    return a.exec();
}
/*
string textToBinary(string textFileName, string binFileName)
{
	int rc;
	char *zErrMsg;
    sqlite3 *database;

	string sqlFileName = textFileName;
    sqlFileName.append("sql");
	string sqlToRun;
	string singlestring;

	rc = sqlite3_open((sqlFileName).c_str(), &database);
	if(rc)
	{
		cout<<"Failed to open file\n\n";
        return nullptr;
	}
	else
	{
		cout<<"File opened\n\n";
	}
	
	cout<<"Enter name of sql file:";
	cin>>textFileName;
	ifstream openedFile;
	openedFile.open(textFileName.c_str());

	while(!openedFile.eof())
	{
		sqlToRun.clear();
		singlestring = "a";
		while(!openedFile.eof() && *singlestring.rbegin() != ';')
		{
			singlestring.clear();
			getline(openedFile, singlestring);
			sqlToRun = sqlToRun + singlestring;
		}
		rc = sqlite3_exec(database, sqlToRun.c_str(), NULL, 0, &zErrMsg);
		if( rc!=SQLITE_OK)
		{
			cout<<zErrMsg;
			cout<<"\n";
			sqlite3_free(zErrMsg);
		}
		
	}
	cout<<"SQL commands run successfully!\n\n";

	return sqlFileName;
}*/
