#ifndef SLEUTHKIT_H 
#define SLEUTHKIT_H

#include <main/interfaces.h>
#include <sqlite3.h>
#include <tsk/framework/framework.h>
#include <QtPlugin>
#include <QObject>
#include <QtWidgets>
#include <QStringList>
#include <QFileDialog>

class SleuthKitPlugin : public QObject, public SleuthKitInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "wombat.SleuthKitInterface" FILE "sleuthkit.json")
    Q_INTERFACES(SleuthKitInterface)
    
public:
    //SleuthKit Interface Functions
    //SetupImageDatabase() const;

    //QStringList evidenceActions() const;
    //QStringList evidenceActionIcons() const;

    //void addEvidence(int currentCaseID);
    //void remEvidence(int currentCaseID);

private:
    int currentcaseid;

};

#endif // SLEUTHKIT_H
