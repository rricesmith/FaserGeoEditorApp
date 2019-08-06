# FaserGeoEditorApp
Qt Application for editing and maintaining the Faser GeoModel Sql Database


Is subfile withing larger repository https://gitlab.cern.ch//dcasper/calypso

Requires entire calypso repository to run/compile currently

After following calypso readme to build all files:

cd ../run

source ./setup.sh

bin/qtTest data/geomDB_sqlite

Adding rows/columns adds one after the selected index (if none selected inputs new one at beginning)

Changes to table are only saved if submitted
