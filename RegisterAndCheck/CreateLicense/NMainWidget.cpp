#include "NMainWidget.h"
#include "ui_NMainWidget.h"
#include "NCheckUserDialog.h"
#include "NLicenseFileManager.h"
#include <QMessageBox>
#include <QFileDialog>

NMainWidget::NMainWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NMainWidget)
{
    ui->setupUi(this);
    SetupUI();
    SetupConnect();
}

NMainWidget::~NMainWidget()
{
    delete ui;
}

void NMainWidget::SetupUI()
{
    QDate dt = QDate::currentDate();
    dt = dt.addMonths(6);
    ui->dateEdit->setDate(dt);
}

void NMainWidget::SetupConnect()
{
    connect(ui->pushButton_1, &QPushButton::clicked, this, &NMainWidget::actionMakeLicense);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &NMainWidget::actionCheckLicense);
}

void NMainWidget::actionMakeLicense()
{
    if(ui->lineEdit_1->text().isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("机器码不能为空"));
        return ;
    }

    if(ui->lineEdit_1->text().length() != 32)
    {
        QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("机器码格式有误"));
        return ;
    }

    NCheckUserDialog checkDialog(this);
    if(QDialog::Accepted == checkDialog.exec())
    {
        QString strSavePath = QFileDialog::getExistingDirectory(this, QStringLiteral("保存license文件"), QDir::homePath());
        if(!strSavePath.isEmpty())
        {
            SLicense license;
            license.m_strDevideCode = ui->lineEdit_1->text();
            license.m_strCreateDate = QDate::currentDate().toString(QStringLiteral("yyyyMMdd"));
            license.m_strLicenseDate = ui->checkBox->isChecked() ? QStringLiteral("") : ui->dateEdit->date().toString(QStringLiteral("yyyyMMdd"));
            license.m_strUserInfo = ui->lineEdit_2->text();
            license.m_nLicenseCount = ui->spinBox->value();

            NLicenseFileManager licenseFileManager;
            licenseFileManager.SetLicense(license);
            if(licenseFileManager.WriteLicenseConfig(strSavePath))
            {
                QMessageBox::information(this, QStringLiteral("成功"), QStringLiteral("生成license文件成功"));
            }
            else
            {
                QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("生成license文件失败"));
            }
        }
    }
}

void NMainWidget::actionCheckLicense()
{
    QString strLicenseFile = QFileDialog::getOpenFileName(this, QStringLiteral("license文件"), QDir::homePath());
    if(!strLicenseFile.isEmpty())
    {
        NLicenseFileManager licenseFileManager;
        licenseFileManager.LoadLicenseConfig(strLicenseFile);
        SLicense license = licenseFileManager.GetLicense();

        QString strLicenseInfo = QStringLiteral("DeviceCode: %1\nCreateDate: %2\nLicenseDate: %3\nUserInfo: %4\nLicenseCount: %5")
                .arg(license.m_strDevideCode)
                .arg(license.m_strCreateDate)
                .arg(license.m_strLicenseDate)
                .arg(license.m_strUserInfo)
                .arg(license.m_nLicenseCount);

        ui->textEdit->setText(strLicenseInfo);
    }
}
