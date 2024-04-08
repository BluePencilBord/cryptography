#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "passwordrecord.h"

#include <openssl/evp.h>


#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QPixmap>
#include <QLineEdit>
#include <QBuffer>
#include <QCryptographicHash>
#include <QClipboard>
#include <QTimer>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QObject::connect(ui->lineEditSearch, &QLineEdit::textEdited, this, &MainWindow::filterListItems);

    ui->incorrectPasswordLabel->setVisible(false);

    ui->lineEditPassword->setFocus();


}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::readJSON(unsigned char *key)
{
    QFile jsonFile("C:\\Users\\bruh\\Documents\\PassManager\\json\\cridentials_encrypted.json");

    if(!jsonFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось открыть файл: " + jsonFile.errorString());
        return false;
    }

    QByteArray hexEncryptedBytes = jsonFile.readAll();

    QByteArray encryptedBytes = QByteArray::fromHex(hexEncryptedBytes);

    QByteArray decryptedBytes;

    int ret_code = MainWindow::decryptQByteArray(encryptedBytes, decryptedBytes, key);

    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(decryptedBytes, &error);

    qDebug() << error.errorString();

    qDebug() << decryptedBytes;

    QJsonObject jsonObj = jsonDoc.object();

    this->jsonArr = jsonObj["cridentials"].toArray();

    jsonFile.close();

    return !ret_code;
}

void MainWindow::filterListItems(const QString &searchStrings)
{
    ui->listWidget->clear();

    for (int i = 0; i != jsonArr.size(); ++i)
    {
        QJsonObject jsonItem = jsonArr[i].toObject();

        if ((searchStrings == "") || jsonItem["site"].toString().toLower().contains(searchStrings.toLower()))
        {
            QListWidgetItem *newItem = new QListWidgetItem();
            PasswordRecord *itemWidget = new PasswordRecord(jsonItem["site"].toString(), jsonItem["login"].toString(), jsonItem["password"].toString());

            QObject::connect(itemWidget, &PasswordRecord::enterPinSignal, this, &MainWindow::on_enterPinSignal);

            ui->listWidget->addItem(newItem);
            ui->listWidget->setItemWidget(newItem, itemWidget);

            newItem->setSizeHint(itemWidget->sizeHint());
        }
    }
}

//password  4747
//key = 822a8e0aad6a68a99ee9db27651f1f6115414b7a772984b3f6609246ffbe3ef5

int MainWindow::decryptQByteArray(const QByteArray& encryptedBytes, QByteArray& decryptedBytes, unsigned char *key)
{
    QByteArray iv_hex("00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f");
    QByteArray iv_ba = QByteArray::fromHex(iv_hex);

    unsigned char iv[16] = {0};
    memcpy(iv, iv_ba.data(), 16);

    EVP_CIPHER_CTX *ctx;
    ctx = EVP_CIPHER_CTX_new();
    if (!EVP_DecryptInit_ex2(ctx, EVP_aes_256_cbc(), key, iv, NULL)) {
        qDebug() << "Error";
        /* Error */
        EVP_CIPHER_CTX_free(ctx);
        return 0;
    }
    qDebug() << "NoError";

#define BUF_LEN 256
    unsigned char encrypted_buf[BUF_LEN] = {0}, decrypted_buf[BUF_LEN] = {0};
    int encr_len, decr_len;

    QDataStream encrypted_stream(encryptedBytes);

    decryptedBytes.clear();
    QBuffer decryptedBuffer(&decryptedBytes);
    decryptedBuffer.open(QIODevice::ReadWrite);


    encr_len = encrypted_stream.readRawData(reinterpret_cast<char*>(encrypted_buf), BUF_LEN);
    while(encr_len > 0){

        if (!EVP_DecryptUpdate(ctx, decrypted_buf, &decr_len, encrypted_buf, encr_len)) {
            /* Error */
            qDebug() << "Error";
            EVP_CIPHER_CTX_free(ctx);
            return 0;
        }

        decryptedBuffer.write(reinterpret_cast<char*>(decrypted_buf), decr_len);
        encr_len = encrypted_stream.readRawData(reinterpret_cast<char*>(encrypted_buf), BUF_LEN);
    }

    int tmplen;
    if (!EVP_DecryptFinal_ex(ctx, decrypted_buf, &tmplen)) {
        /* Error */
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    // qDebug() << "***EVP_DecryptFinal_ex " << reinterpret_cast<char*>(decrypted_buf);
    decryptedBuffer.write(reinterpret_cast<char*>(decrypted_buf), tmplen);
    EVP_CIPHER_CTX_free(ctx);

    decryptedBuffer.close();
    return 0;
}

void MainWindow::on_enterPinSignal(QString toEncryptLogOrPass) {
    ui->stackedWidget->setCurrentIndex(0);
    this->toEncryptLogOrPass = toEncryptLogOrPass;
}

void MainWindow::on_pushButtonAuth_clicked()
{
    QByteArray hash = QCryptographicHash::hash(ui->lineEditPassword->text().toUtf8(), QCryptographicHash::Sha256);

    unsigned char hash_key[32] = {0};
    memcpy(hash_key, hash.data(), 32);

    qDebug() << "***isAuthenticated -> " << isAuthenticated;


    if (!isAuthenticated){

        isAuthenticated = readJSON(hash_key);

        if (isAuthenticated)
        {
            ui->stackedWidget->setCurrentIndex(1);
            filterListItems("");
        } else {
            showIncorrectPasswordLabel();
        }
    } else {
        if (readJSON(hash_key)) {
            ui->lineEditPassword->setText("");
            ui->stackedWidget->setCurrentIndex(1);
            qDebug() << "readJson";

            QByteArray decryptedBytes;

            int ret_code = MainWindow::decryptQByteArray(QByteArray::fromHex(toEncryptLogOrPass.toLatin1()),
                                                         decryptedBytes, hash_key);


            QClipboard *clipboard = QApplication::clipboard();
            clipboard->setText(decryptedBytes);
        } else {
            showIncorrectPasswordLabel();
        }
    }

    ui->lineEditPassword->setText("");

}

void MainWindow::showIncorrectPasswordLabel() {
    ui->incorrectPasswordLabel->setVisible(true);

    QTimer::singleShot(2000, [this]() {
        ui->incorrectPasswordLabel->setVisible(false);
    });
}
