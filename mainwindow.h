#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <fstream>
#include <QCheckBox>
#include <QFileDialog>
#include <QtCore>
#include <QtGui>
#include <QMessageBox>
#include "Classifiers.h"


#include"database.h"
#include "SMPHelper.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    bool loadFile(const std::string &fileName);
    void updateDatabaseInfo();
    void saveFile(const std::string &fileName);

    void FSupdateButtonState(void);
    void FSsetButtonState(bool state);



private slots:
    void on_FSpushButtonOpenFile_clicked();

    void on_FSpushButtonCompute_clicked();

    void on_FSpushButtonSaveFile_clicked();

    void on_PpushButtonSelectFolder_clicked();


    void on_CpushButtonOpenFile_clicked();

    void on_CpushButtonSaveFile_clicked();

    void on_CpushButtonTrain_clicked();

    void on_CpushButtonExecute_clicked();

	void on_CpushButtonBoostrap_clicked();
		
	void on_CpushButtonBoostrapR_clicked();

	public:
    Ui::MainWindow *ui;
	Classifiers  *classifiers;

	SMPHelper* SMPDHelper;
	Database database;
	std::vector<std::vector<Object>> bootstrapTrainObjectVectors;
	std::vector<std::vector<Object>> bootstrapTestObjectVectors;
private:
     
};

#endif // MAINWINDOW_H
