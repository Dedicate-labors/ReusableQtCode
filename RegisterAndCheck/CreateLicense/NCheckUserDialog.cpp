#include "NCheckUserDialog.h"
#include "ui_NCheckUserDialog.h"
#include <QMessageBox>
#include<QDebug>


NCheckUserDialog::NCheckUserDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NCheckUserDialog)
{
    ui->setupUi(this);
    SetupUI();
    SetupConnect();
}

NCheckUserDialog::~NCheckUserDialog()
{
    delete ui;
}

void NCheckUserDialog::SetupUI()
{

}

void NCheckUserDialog::SetupConnect()
{
    connect(ui->pushButton_1, &QPushButton::clicked, this, &NCheckUserDialog::actionOK);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &NCheckUserDialog::reject);
}

//65957B03B66B65A2AC507F9361F422D3
// password:Nicole3025932830
void NCheckUserDialog::actionOK()
{
    char pDestP[17] = {0};
    const char* pSourceP = "o5cN9el8i2233030";
    int pIndexTable[] = {3, 8, 2, 0, 6, 5, 11, 15, 9, 1, 4, 14, 10, 7, 12, 13};
    for(int nIndex = 0; nIndex < 16; ++nIndex)
    {
        pDestP[nIndex] = pSourceP[pIndexTable[nIndex]];
    }
    qDebug() << pDestP;
    if(QStringLiteral("%1").arg(pDestP) == ui->lineEdit->text())
    {
        accept();
    }
    else
    {
        QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("非管理人员无法使用许可大师"));
    }
}
