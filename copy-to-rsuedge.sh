NS3_APP_FOLDER=~/workspace/ns-3-allinone/ns-3.28/src/applications
NS3_INT_FOLDER=~/workspace/ns-3-allinone/ns-3.28/src/internet
NS3_FOLDER=~/workspace/ns-3-allinone/ns-3.28
PROJECT_APP_FOLDER=~/workspace/project_ns3/2018_V2X_edge_ns3/rsuedge/applications
PROJECT_INT_FOLDER=~/workspace/project_ns3/2018_V2X_edge_ns3/rsuedge/internet
PROJECT_FOLDER=~/workspace/project_ns3/2018_V2X_edge_ns3/rsuedge

cp $NS3_APP_FOLDER/model/rsu-app.h $PROJECT_APP_FOLDER/model/
cp $NS3_APP_FOLDER/model/rsu-app.cc $PROJECT_APP_FOLDER/model/
cp $NS3_APP_FOLDER/model/rsu-vehicle-app.h $PROJECT_APP_FOLDER/model/
cp $NS3_APP_FOLDER/model/rsu-vehicle-app.cc $PROJECT_APP_FOLDER/model/
cp $NS3_APP_FOLDER/helper/rsu-app-helper.h $PROJECT_APP_FOLDER/helper/
cp $NS3_APP_FOLDER/helper/rsu-app-helper.cc $PROJECT_APP_FOLDER/helper/
cp $NS3_APP_FOLDER/helper/rsu-vehicle-helper.h $PROJECT_APP_FOLDER/helper/
cp $NS3_APP_FOLDER/helper/rsu-vehicle-helper.cc $PROJECT_APP_FOLDER/helper/
cp $NS3_INT_FOLDER/model/rsu-header.h $PROJECT_INT_FOLDER/model/
cp $NS3_INT_FOLDER/model/rsu-header.cc $PROJECT_INT_FOLDER/model/
cp $NS3_FOLDER/scratch/rsu_test_1.cc $PROJECT_FOLDER/scratch/
