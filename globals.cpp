#include "globals.h"

// Copyright 2013-2019 Pasquale J. Rinaldi, Jr.
// Distrubted under the terms of the GNU General Public License version 2

WombatVariable wombatvariable;
QFile logfile;
QFile viewerfile;
QFile settingsfile;
QFile previewfile;
QFile bookmarkfile;
QDir thumbdir;
QTextEdit* msglog = NULL;
qint64 filesfound = 0;
qint64 fileschecked = 0;
qint64 totalchecked = 0;
qint64 totalcount = 0;
qint64 exportcount = 0;
qint64 dighashcount = 0;
qint64 digimgthumbcount = 0;
qint64 errorcount = 0;
qint64 jumpoffset = 0;
qint64 filejumpoffset = 0;
qint64 orphancount = 1;
qint64 carvedcount = 1;
int ecount = 0;
int thumbsize = 320; // thumbnail size settings variable
int vidcount = 10;
int autosave = 10;
QByteArray reporttimezone;
int mftrecordsize = 1024;
int hashsum = 1;
int idcol = 0;
TreeNodeModel* treenodemodel = NULL;
TreeNode* selectednode = NULL;
InterfaceSignals* isignals = new InterfaceSignals();
QStringList propertylist;
QStringList exportlist;
QStringList digfilelist;
QStringList listeditems;
QStringList existingevidence;
QStringList newevidence;
QString casepath = QDir::homePath(); // case path settings variable
QString reportpath = QDir::homePath(); // report path settings variable
QString exportpath = "";
QString genthmbpath = "";
bool originalpath = true;
QString hexstring = "";
QHash<QString, bool> checkhash;
QHash<QString, QString> imageshash;
QHash<QString, QString> taggedhash; // list of tagged files: [id], tag
QHash<QString, QString> hashlist; // list of file hashes: ids, hash
QMutex mutex;
FilterValues filtervalues;
QList<EvidenceReportData> evidrepdatalist;
