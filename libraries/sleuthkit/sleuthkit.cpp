#include "sleuthkit.h"

void SleuthKitPlugin::SetupSystemProperties(QString settingsPath, QString configFilePath)
{
    QString tmpPath = settingsPath;
    tmpPath += "/";
    tmpPath += configFilePath;
    QFile tmpFile(tmpPath);
    fprintf(stderr, "TmpPath: %s\n", tmpPath.toStdString().c_str());
    if(!tmpFile.exists()) // if tsk-config.xml does not exist, create and write it here
    {
        if(tmpFile.open(QFile::WriteOnly | QFile::Text))
        {   
            QXmlStreamWriter xml(&tmpFile);
            xml.setAutoFormatting(true);
            xml.writeStartDocument();
            xml.writeStartElement("TSK_FRAMEWORK_CONFIG");
            xml.writeStartElement("CONFIG_DIR");
            xml.writeCharacters(settingsPath);
            xml.writeEndElement();
            xml.writeEndElement();
            xml.writeEndDocument();
        }
        else
        {
            fprintf(stderr, "Could not open file to write\n");
        }
        tmpFile.close();
    }
    try
    {
        systemproperties = new TskSystemPropertiesImpl();
        systemproperties->initialize(tmpPath.toStdString());
        TskServices::Instance().setSystemProperties(*systemproperties);
        fprintf(stderr, "Configuration File Loading was successful!\n");
    }
    catch(TskException &ex)
    {
        fprintf(stderr, "Loading Config File config file: %s\n", ex.message().c_str());
    }
    try
    {
        SetSystemProperty(TskSystemProperties::OUT_DIR, settingsPath.toStdString());
    }
    catch(TskException &ex)
    {
        fprintf(stderr, "Setting out dir failed: %s\n", ex.message().c_str());
    }
}
void SleuthKitPlugin::SetupSystemLog(QString dataPath, QString logFilePath)
{
    QString tmpPath = dataPath;
    tmpPath += logFilePath;
    try
    {
        log = std::auto_ptr<Log>(new Log());
        log->open(tmpPath.toStdString().c_str());
        TskServices::Instance().setLog(*log);
        fprintf(stderr, "Loading Log File was successful!\n");
    }
    catch(TskException &ex)
    {
        fprintf(stderr, "Loading Log File: %s\n", ex.message().c_str());
    }
}
void SleuthKitPlugin::SetupImageDatabase(QString imgDBPath, QString evidenceFilePath)
{
    QString evidenceFileName = evidenceFilePath.split("/").last();
    evidenceFileName += ".db";

    try
    {
        imgdb = std::auto_ptr<TskImgDB>(new TskImgDBSqlite(imgDBPath.toStdString().c_str(), evidenceFileName.toStdString().c_str()));
        if(imgdb->initialize() != 0)
            fprintf(stderr, "Error initializing db\n");
        else
        {
            fprintf(stderr, "DB was Initialized Successfully!\n");
        }
        TskServices::Instance().setImgDB(*imgdb);
        fprintf(stderr, "Loading ImageDB was Successful!\n");
    }
    catch(TskException &ex)
    {
        fprintf(stderr, "Loading ImageDB: %s\n", ex.message().c_str());
    }
}
void SleuthKitPlugin::SetupSystemBlackboard()
{
    try
    {
        TskServices::Instance().setBlackboard((TskBlackboard &)TskDBBlackboard::instance());
        fprintf(stderr, "Loading Blackboard was successful!\n");
    }
    catch(TskException &ex)
    {
        fprintf(stderr, "Loading Blackboard: %s\n", ex.message().c_str());
    }
}
void SleuthKitPlugin::SetupSystemSchedulerQueue()
{
    try
    {
        TskServices::Instance().setScheduler(scheduler);
        fprintf(stderr, "Loading Scheduler was successful!\n");
    }
    catch(TskException &ex)
    {
        fprintf(stderr, "Loading Scheduler: %s\n", ex.message().c_str());
    }
}
void SleuthKitPlugin::SetupSystemFileManager()
{
    try
    {
        TskServices::Instance().setFileManager(TskFileManagerImpl::instance());
        fprintf(stderr, "Loading File Manager was successful!\n");
    }
    catch(TskException &ex)
    {
        fprintf(stderr, "Loading FileManager: %s\n", ex.message().c_str());
    }
}
void SleuthKitPlugin::OpenEvidence(QString evidencePath)
{
    TskImageFileTsk imagefiletsk;
    try
    {
        imagefiletsk.open(evidencePath.toStdString());
        TskServices::Instance().setImageFile(imagefiletsk);
        fprintf(stderr, "Opening Image File was successful!\n");
    }
    catch(TskException &ex)
    {
        fprintf(stderr, "Opening Evidence: %s\n", ex.message().c_str());
    }
    try
    {
        imagefiletsk.extractFiles();
        fprintf(stderr, "Extracting Evidence was successful\n");
    }
    catch(TskException &ex)
    {
        fprintf(stderr, "Extracting Evidence: %s\n", ex.message().c_str());
    }
}

QStandardItemModel* SleuthKitPlugin::GetCurrentImageDirectoryTree()
{
    std::vector<uint64_t> fileidVector;
    std::vector<TskFileRecord> fileRecordVector;
    fileidVector = imgdb->getFileIds();
    TskFileRecord tmpRecord;
    // implement custom qstandarditem which has a fileid field
    //QStandardItem *tmpItem;
    SleuthFileItem *tmpItem2;
    int ret;
    uint64_t tmpId;
    QStandardItemModel *model = new QStandardItemModel();
    QStandardItem *rootNode = model->invisibleRootItem();
    //QList<QStandardItem*> itemList;
    QList<SleuthFileItem*> sleuthList;
    foreach(tmpId, fileidVector)
    {
        ret = imgdb->getFileRecord(tmpId, tmpRecord);
        fprintf(stderr, "FileId: %d\n", tmpId);
        fileRecordVector.push_back(tmpRecord);
        fprintf(stderr, "File Id: %d - FileName: %s - Parent FileId: %d - dirtype: %d\n", tmpRecord.fileId, tmpRecord.name.c_str(), tmpRecord.parentFileId, tmpRecord.dirType);
    }
    fprintf(stderr, "record vector size: %d\n", fileRecordVector.size());
    for(int i=0; i < (int)fileRecordVector.size(); i++)
    {
        //itemList.append(new QStandardItem(QString(fileRecordVector[i].name.c_str())));
        sleuthList.append(new SleuthFileItem(QString(fileRecordVector[i].name.c_str()), (int)fileRecordVector[i].fileId));
    }
    for(int i = 0; i < (int)fileRecordVector.size(); i++)
    {
        //tmpItem = ((QStandardItem*)itemList[i]);
        tmpItem2 = ((SleuthFileItem*)sleuthList[i]);

        if(((TskFileRecord)fileRecordVector[i]).dirType == 3)
        {
            tmpItem2->setIcon(QIcon(":/basic/treefolder"));
        }
        else
        {
            tmpItem2->setIcon(QIcon(":/basic/treefile"));
        }
        if(((TskFileRecord)fileRecordVector[i]).parentFileId == 1)
        {
            rootNode->appendRow(tmpItem2);
        }
    }
    for(int i=0; i < (int)fileRecordVector.size(); i++)
    {
        tmpRecord = fileRecordVector[i];
        //tmpItem = itemList[i];
        tmpItem2 = sleuthList[i];
        if(tmpRecord.parentFileId > 1)
        {
            fprintf(stderr, "itemList[%d]->appendrow(itemList[%d])\n", tmpRecord.parentFileId-1, i);
            fprintf(stderr, "sleuthListId %d\n", sleuthList[i]->GetSleuthFileID());
            //((QStandardItem*)itemList[tmpRecord.parentFileId-1])->appendRow(tmpItem);
            ((SleuthFileItem*)sleuthList[tmpRecord.parentFileId-1])->appendRow(tmpItem2);
        }
    }
    return model;
}
char* SleuthKitPlugin::GetFileContents(SleuthFileItem* fileItem)
{
    TskImageFileTsk tmpfile;
    //tmpfile = TskServices::Instance().getImageFile();
    //std::auto_ptr<TskImageFile> tmpFile = TskServices::Instance().getImageFile();
    char* buffer;
    //QString
    //int tmpId = imgdb->getFileIds(" where name = ? limit 1;")
    // get fileid using filename.
    // openfile(fileid)
    // readfile(fileid)
    // closefile(fileid)
    // return buffe
    return "";
}
