#include "tskfunctions.h"

class SqlErrLog : public Log
{
public:
    SqlWrapper *sqlObject;
    sqlite3* sqldb;
    sqlite3_stmt* sqlStatement;
    char* sqlErrMsg;
    int sqlValue;
    int selected;

    SqlErrLog() : Log()
    {
    }

    ~SqlErrLog()
    {

    }

    void log(Channel a_channel, const std::string &a_msg)
    {
        Log::log(a_channel, a_msg);
        // put code to write to sql table here...
        sqlObject = new SqlWrapper(sqlStatement, "1.1");
        sqlObject->PrepareSql("insert into logtable (logchannel, logmessage) VALUES(?, ?);");
        sqlObject->BindValue(1, a_channel);
        sqlObject->BindValue(2, a_msg.c_str());
        sqlObject->StepSql();
        sqlObject->FinalizeSql();
        sqlObject->CloseSql();
    }
};

void TskFunctions::SetupTskFramework()
{
    // container for the framework setup
}

void TskFunctions::InitializeFrameworkProperties()
{

}

void TskFunctions::InitializeFrameworkLog()
{
    frameworkLog = std::auto_ptr<Log>(new SqlErrLog());
    ret = frameworkLog->open(QDir(QCoreApplication::applicationDirPath()).absolutePath().toStdString().c_str()); // modify the logdir path
    TskServices::Instance().setLog(*frameworkLog);
}

void TskFunctions::InitializeFrameworkScheduler()
{

}

void TskFunctions::InitializeFrameworkBlackboard()
{

}

void TskFunctions::InitializeFrameworkDatabase()
{

}
