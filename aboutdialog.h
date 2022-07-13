#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog();

    void setAppVersion(const QString& a_strAppVresion);

private slots:
    void on_okCloseButton_clicked();

private:
    Ui::AboutDialog *ui;

    QString m_strAppVersion;
};

#endif // ABOUTDIALOG_H
