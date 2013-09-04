/********************************************************************************
** Form generated from reading UI file 'wombatforensics.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WOMBATFORENSICS_H
#define UI_WOMBATFORENSICS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WombatForensics
{
public:
    QAction *actionNew_Case;
    QAction *actionOpen_Case;
    QAction *actionView_Progress;
    QAction *actionExit;
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QSplitter *splitter;
    QTabWidget *fileViewTabWidget;
    QTabWidget *fileInfoTabWidget;
    QMenuBar *mainMenubar;
    QMenu *menuFile;
    QMenu *menuEvidence;
    QMenu *menuSettings;
    QStatusBar *mainStatusbar;
    QToolBar *mainToolBar;

    void setupUi(QMainWindow *WombatForensics)
    {
        if (WombatForensics->objectName().isEmpty())
            WombatForensics->setObjectName(QStringLiteral("WombatForensics"));
        WombatForensics->resize(800, 600);
        actionNew_Case = new QAction(WombatForensics);
        actionNew_Case->setObjectName(QStringLiteral("actionNew_Case"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/newcase"), QSize(), QIcon::Normal, QIcon::Off);
        actionNew_Case->setIcon(icon);
        actionOpen_Case = new QAction(WombatForensics);
        actionOpen_Case->setObjectName(QStringLiteral("actionOpen_Case"));
        QIcon icon1;
        icon1.addFile(QStringLiteral(":/opencase"), QSize(), QIcon::Normal, QIcon::Off);
        actionOpen_Case->setIcon(icon1);
        actionView_Progress = new QAction(WombatForensics);
        actionView_Progress->setObjectName(QStringLiteral("actionView_Progress"));
        actionView_Progress->setCheckable(true);
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/progresswindow/window"), QSize(), QIcon::Normal, QIcon::Off);
        actionView_Progress->setIcon(icon2);
        actionExit = new QAction(WombatForensics);
        actionExit->setObjectName(QStringLiteral("actionExit"));
        centralwidget = new QWidget(WombatForensics);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Vertical);
        fileViewTabWidget = new QTabWidget(splitter);
        fileViewTabWidget->setObjectName(QStringLiteral("fileViewTabWidget"));
        splitter->addWidget(fileViewTabWidget);
        fileInfoTabWidget = new QTabWidget(splitter);
        fileInfoTabWidget->setObjectName(QStringLiteral("fileInfoTabWidget"));
        splitter->addWidget(fileInfoTabWidget);

        verticalLayout->addWidget(splitter);

        WombatForensics->setCentralWidget(centralwidget);
        mainMenubar = new QMenuBar(WombatForensics);
        mainMenubar->setObjectName(QStringLiteral("mainMenubar"));
        mainMenubar->setGeometry(QRect(0, 0, 800, 19));
        mainMenubar->setAcceptDrops(true);
        menuFile = new QMenu(mainMenubar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        menuEvidence = new QMenu(mainMenubar);
        menuEvidence->setObjectName(QStringLiteral("menuEvidence"));
        menuSettings = new QMenu(mainMenubar);
        menuSettings->setObjectName(QStringLiteral("menuSettings"));
        WombatForensics->setMenuBar(mainMenubar);
        mainStatusbar = new QStatusBar(WombatForensics);
        mainStatusbar->setObjectName(QStringLiteral("mainStatusbar"));
        WombatForensics->setStatusBar(mainStatusbar);
        mainToolBar = new QToolBar(WombatForensics);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        mainToolBar->setMovable(false);
        mainToolBar->setAllowedAreas(Qt::TopToolBarArea);
        mainToolBar->setFloatable(false);
        WombatForensics->addToolBar(Qt::TopToolBarArea, mainToolBar);

        mainMenubar->addAction(menuFile->menuAction());
        mainMenubar->addAction(menuEvidence->menuAction());
        mainMenubar->addAction(menuSettings->menuAction());
        menuFile->addAction(actionNew_Case);
        menuFile->addAction(actionOpen_Case);
        menuFile->addSeparator();
        menuFile->addAction(actionView_Progress);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        mainToolBar->addAction(actionNew_Case);
        mainToolBar->addAction(actionOpen_Case);
        mainToolBar->addSeparator();
        mainToolBar->addAction(actionView_Progress);
        mainToolBar->addSeparator();

        retranslateUi(WombatForensics);
        QObject::connect(actionExit, SIGNAL(triggered()), WombatForensics, SLOT(close()));

        fileInfoTabWidget->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(WombatForensics);
    } // setupUi

    void retranslateUi(QMainWindow *WombatForensics)
    {
        WombatForensics->setWindowTitle(QApplication::translate("WombatForensics", "WombatForensics", 0));
        actionNew_Case->setText(QApplication::translate("WombatForensics", "New Case", 0));
        actionOpen_Case->setText(QApplication::translate("WombatForensics", "Open Case", 0));
        actionView_Progress->setText(QApplication::translate("WombatForensics", "View Progress...", 0));
        actionExit->setText(QApplication::translate("WombatForensics", "Exit", 0));
        menuFile->setTitle(QApplication::translate("WombatForensics", "File", 0));
        menuEvidence->setTitle(QApplication::translate("WombatForensics", "Evidence", 0));
        menuSettings->setTitle(QApplication::translate("WombatForensics", "Settings", 0));
        mainToolBar->setWindowTitle(QApplication::translate("WombatForensics", "toolBar", 0));
    } // retranslateUi

};

namespace Ui {
    class WombatForensics: public Ui_WombatForensics {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WOMBATFORENSICS_H
