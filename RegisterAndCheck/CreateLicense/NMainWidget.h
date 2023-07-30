#ifndef NMAINWIDGET_H
#define NMAINWIDGET_H

#include <QWidget>

namespace Ui {
class NMainWidget;
}

class NMainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NMainWidget(QWidget *parent = 0);
    ~NMainWidget();

private:
    void SetupUI();
    void SetupConnect();

private slots:
    void actionMakeLicense();
    void actionCheckLicense();


private:
    Ui::NMainWidget *ui;
};

#endif // NMAINWIDGET_H
