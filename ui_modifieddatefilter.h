/********************************************************************************
** Form generated from reading UI file 'modifieddatefilter.ui'
**
** Created by: Qt User Interface Compiler version 5.4.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MODIFIEDDATEFILTER_H
#define UI_MODIFIEDDATEFILTER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ModifiedDateFilter
{
public:
    QGridLayout *gridLayout;
    QCheckBox *morecheckBox;
    QCheckBox *lesscheckBox;
    QDateTimeEdit *moredateTimeEdit;
    QDateTimeEdit *lessdateTimeEdit;
    QPushButton *pushButton;

    void setupUi(QWidget *ModifiedDateFilter)
    {
        if (ModifiedDateFilter->objectName().isEmpty())
            ModifiedDateFilter->setObjectName(QStringLiteral("ModifiedDateFilter"));
        ModifiedDateFilter->setWindowModality(Qt::ApplicationModal);
        ModifiedDateFilter->resize(346, 105);
        QFont font;
        font.setPointSize(8);
        ModifiedDateFilter->setFont(font);
        ModifiedDateFilter->setAutoFillBackground(true);
        gridLayout = new QGridLayout(ModifiedDateFilter);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setHorizontalSpacing(0);
        morecheckBox = new QCheckBox(ModifiedDateFilter);
        morecheckBox->setObjectName(QStringLiteral("morecheckBox"));

        gridLayout->addWidget(morecheckBox, 0, 0, 1, 1);

        lesscheckBox = new QCheckBox(ModifiedDateFilter);
        lesscheckBox->setObjectName(QStringLiteral("lesscheckBox"));

        gridLayout->addWidget(lesscheckBox, 2, 0, 1, 1);

        moredateTimeEdit = new QDateTimeEdit(ModifiedDateFilter);
        moredateTimeEdit->setObjectName(QStringLiteral("moredateTimeEdit"));
        moredateTimeEdit->setCalendarPopup(true);
        moredateTimeEdit->setTimeSpec(Qt::UTC);

        gridLayout->addWidget(moredateTimeEdit, 0, 1, 1, 1);

        lessdateTimeEdit = new QDateTimeEdit(ModifiedDateFilter);
        lessdateTimeEdit->setObjectName(QStringLiteral("lessdateTimeEdit"));
        lessdateTimeEdit->setCalendarPopup(true);
        lessdateTimeEdit->setTimeSpec(Qt::UTC);

        gridLayout->addWidget(lessdateTimeEdit, 2, 1, 1, 1);

        pushButton = new QPushButton(ModifiedDateFilter);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setDefault(true);
        pushButton->setFlat(true);

        gridLayout->addWidget(pushButton, 3, 1, 1, 1);


        retranslateUi(ModifiedDateFilter);
        QObject::connect(lesscheckBox, SIGNAL(toggled(bool)), lessdateTimeEdit, SLOT(setEnabled(bool)));
        QObject::connect(morecheckBox, SIGNAL(toggled(bool)), moredateTimeEdit, SLOT(setEnabled(bool)));

        QMetaObject::connectSlotsByName(ModifiedDateFilter);
    } // setupUi

    void retranslateUi(QWidget *ModifiedDateFilter)
    {
        ModifiedDateFilter->setWindowTitle(QApplication::translate("ModifiedDateFilter", "Filter", 0));
        morecheckBox->setText(QApplication::translate("ModifiedDateFilter", "Show Items with date >", 0));
        lesscheckBox->setText(QApplication::translate("ModifiedDateFilter", "Show Items with date <", 0));
        moredateTimeEdit->setDisplayFormat(QApplication::translate("ModifiedDateFilter", "MM/dd/yyyy HH:mm:ss", 0));
        lessdateTimeEdit->setDisplayFormat(QApplication::translate("ModifiedDateFilter", "MM/dd/yyyy HH:mm:ss", 0));
        pushButton->setText(QApplication::translate("ModifiedDateFilter", "Apply", 0));
    } // retranslateUi

};

namespace Ui {
    class ModifiedDateFilter: public Ui_ModifiedDateFilter {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MODIFIEDDATEFILTER_H
