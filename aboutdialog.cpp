#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#define APP_NAME_VERSION_1  "<html><head/><body><p><span style=\" font-size:16pt; font-weight:600;\">"
#define APP_NAME_VERSION_2  "</span></p></body></html>"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_okCloseButton_clicked()
{
    close();
}

void AboutDialog::setAppVersion(const QString& a_strAppVresion)
{
    m_strAppVersion = APP_NAME_VERSION_1 + a_strAppVresion + APP_NAME_VERSION_2;

    ui->labelTitle1->setText(m_strAppVersion);
}
