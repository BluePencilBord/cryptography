#ifndef PASSWORDRECORD_H
#define PASSWORDRECORD_H

#include <QWidget>

namespace Ui {
class PasswordRecord;
}

class PasswordRecord : public QWidget
{
    Q_OBJECT

public:
    explicit PasswordRecord(QString site, QString login_encrypted, QString password_encrypted, QWidget *parent = nullptr);
    ~PasswordRecord();

private slots:
    void on_pushButtonCopyLogin_clicked();


    void on_pushButtonCopyPassword_clicked();

signals:
    void enterPinSignal(QString toEncryptLogOrPass);

private:
    Ui::PasswordRecord *ui;
    QString pass_encr;
    QString log_encr;
};

#endif // PASSWORDRECORD_H
