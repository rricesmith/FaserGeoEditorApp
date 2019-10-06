# FaserGeoEditorApp
Qt Application for editing and maintaining the Faser GeoModel Sql Database


These are subfiles within larger repository https://gitlab.cern.ch//dcasper/calypso

Requires entire calypso repository to run/compile currently

Replace files found in calypso/DetectorDescription/GeoModel/FaserGeoEditor/src with these files

After following calypso readme to build all files:

cd ../run

source ./setup.sh

bin/qtTest data/geomDB_sqlite b

Use b as third input when using binary sqlite files
Use t as third input when using text file of sql commands

Adding rows/columns adds one after the selected index (if none selected inputs new one at beginning)

Changes to table are only saved if submitted
