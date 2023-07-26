#ifndef NCHECKUSERDIALOG_H
#define NCHECKUSERDIALOG_H

#include <QDialog>

namespace Ui {
class NCheckUserDialog;
}

class NCheckUserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NCheckUserDialog(QWidget *parent = 0);
    ~NCheckUserDialog();

private:
    void SetupUI();
    void SetupConnect();

private slots:
    void actionOK();

private:
    Ui::NCheckUserDialog *ui;
};

#endif // NCHECKUSERDIALOG_H
