#include "passwordrecord.h"
#include "ui_passwordrecord.h"

PasswordRecord::PasswordRecord(QString site, QString login_encrypted, QString password_encrypted, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PasswordRecord)
{
    this->pass_encr = password_encrypted;
    this->log_encr = login_encrypted;

    ui->setupUi(this);

    ui->labelSiteName->setText(site);

    QPixmap pix("C:\\Users\\bruh\\Documents\\PassManager\\img\\keys.png");
    int w = ui->labelIcon->width();
    int h = ui->labelIcon->height();

    ui->labelIcon->setPixmap(pix.scaled(w,h, Qt::KeepAspectRatio));
}

PasswordRecord::~PasswordRecord()
{
    delete ui;
}

void PasswordRecord::on_pushButtonCopyLogin_clicked()
{
    emit enterPinSignal(log_encr);
}


void PasswordRecord::on_pushButtonCopyPassword_clicked()
{
    emit enterPinSignal(pass_encr);
}

