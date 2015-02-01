#ifndef VIEWERMANAGER_H
#define VIEWERMANAGAER_H

#include "wombatinclude.h"
#include "globals.h"
#include "ui_viewermanager.h"

class ViewerModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ViewerModel(const QStringList extlist, QObject* parent = 0) : QAbstractListModel(parent), externalviewers(extlist)
    {
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        if(parent.row() >= externalviewers.count())
            return 0;
        return externalviewers.count();
    };

    QVariant data(const QModelIndex& index, int role) const
    {
        if(!index.isValid())
            return QVariant();
        if(index.row() >= externalviewers.count())
            return QVariant();
        if(role == Qt::DisplayRole)
            return externalviewers.at(index.row());
        else
            return QVariant();
    };
private:
    QStringList externalviewers;
};

namespace Ui
{
    class ViewerManager;
}

class ViewerManager : public QDialog
{
    Q_OBJECT

public:
    ViewerManager(QWidget* parent = 0);
    ~ViewerManager();

private slots:
    void HideClicked();
    void ShowBrowser();
    void AddViewer();
    void RemoveSelected();
    void SelectionChanged(const QItemSelection &newitem, const QItemSelection &previtem);
signals:
    void HideManagerWindow(bool checkstate);

protected:
    void closeEvent(QCloseEvent* event);
private:
    Ui::ViewerManager* ui;
    QString fileviewerpath;
    ViewerModel* viewmodel; 
    QStringList externlist;
};

Q_DECLARE_METATYPE(ViewerManager*);

#endif // VIDEOMANAGER_H