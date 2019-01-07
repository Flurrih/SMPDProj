#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <QImage>
#include <QDebug>




MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    FSupdateButtonState();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateDatabaseInfo()
{
    ui->FScomboBox->clear();
    for(unsigned int i=1; i<=database.getNoFeatures(); ++i)
        ui->FScomboBox->addItem(QString::number(i));

    ui->FStextBrowserDatabaseInfo->setText("noClass: " +  QString::number(database.getNoClass()));
    ui->FStextBrowserDatabaseInfo->append("noObjects: "  +  QString::number(database.getNoObjects()));
    ui->FStextBrowserDatabaseInfo->append("noFeatures: "  +  QString::number(database.getNoFeatures()));

}

void MainWindow::FSupdateButtonState(void)
{
    if(database.getNoObjects()==0)
    {
        FSsetButtonState(false);
    }
    else
        FSsetButtonState(true);

}


void MainWindow::FSsetButtonState(bool state)
{
   ui->FScomboBox->setEnabled(state);
   ui->FSpushButtonCompute->setEnabled(state);
   ui->FSpushButtonSaveFile->setEnabled(state);
   ui->FSradioButtonFisher->setEnabled(state);
   ui->FSradioButtonSFS->setEnabled(state);
}

void MainWindow::on_FSpushButtonOpenFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open TextFile"), "", tr("Texts Files (*.txt)"));

    if ( !database.load(fileName.toStdString()) )
        QMessageBox::warning(this, "Warning", "File corrupted !!!");
    else
        QMessageBox::information(this, fileName, "File loaded !!!");

    FSupdateButtonState();
    updateDatabaseInfo();
}

void MainWindow::on_FSpushButtonCompute_clicked()
{
    int dimension = ui->FScomboBox->currentText().toInt();

    std::vector<std::string> classNames = database.getClassNames();

    if( ui->FSradioButtonFisher ->isChecked())
    {
    if (dimension == 1 && database.getNoClass() == 2)
        {
            float FLD = 0, tmp;
            int max_ind = -1;

            //std::map<std::string, int> classNames = database.getClassNames();
            for (uint i = 0; i < database.getNoFeatures(); ++i)
            {
                std::map<std::string, float> classAverages;
                std::map<std::string, float> classStds;

                for (auto const &ob : database.getObjects())
                {
                    classAverages[ob.getClassName()] += ob.getFeatures()[i];
                    classStds[ob.getClassName()] += ob.getFeatures()[i] * ob.getFeatures()[i];
                }

                std::for_each(database.getClassCounters().begin(), database.getClassCounters().end(), [&](const std::pair<std::string, int> &it)
                {
                    classAverages[it.first] /= it.second;
                    classStds[it.first] = std::sqrt(classStds[it.first] / it.second - classAverages[it.first] * classAverages[it.first]);
                }
                );

                tmp = std::abs(classAverages[ database.getClassNames()[0] ] - classAverages[database.getClassNames()[1]]) / (classStds[database.getClassNames()[0]] + classStds[database.getClassNames()[1]]);

                if (tmp > FLD)
                {
                    FLD = tmp;
                    max_ind = i;
                }

              }

            ui->FStextBrowserDatabaseInfo->append("max_ind: "  +  QString::number(max_ind) + " " + QString::number(FLD));
          }

    if (dimension > 1 && database.getNoClass() == 2)
        {
            float FLD = 0, tmp;
            int max_ind = -1;

            //std::map<std::string, int> classNames = database.getClassNames();
            for (uint i = 0; i < database.getNoFeatures(); ++i)
            {
                std::map<std::string, std::map<int, float>> classAverages;
                std::map<std::string, std::map<std::pair<int,int>, int>> classMatrixCov;

                for (int i = 1; i <= 4; i++)
                {
                    for (auto const &ob : database.getObjects())
                    {
                        classAverages[ob.getClassName()][i] += ob.getFeatures()[i]/64;
                    }
                    for (int j = i + 1; j <= 4; j++)
                    {
                        boost::numeric::ublas::matrix<double> Xa (2,64); // macierz z 2 cechami i j
                        boost::numeric::ublas::matrix<double> Ua (2,64); // macierz srednich
                        boost::numeric::ublas::matrix<double> Xb (2,64); // macierz z 2 cechami i j
                        boost::numeric::ublas::matrix<double> Ub (2,64); // macierz srednich
                        boost::numeric::ublas::matrix<double> FUa (2,1);
                        boost::numeric::ublas::matrix<double> FUb (2,1);
                        int objectCounter = 0;
                        for (auto const &ob : database.getObjects())
                        {
                            if(ob.getClassName() == classNames[0])
                            {
                                Xa[0][objectCounter] = ob.getFeatures()[i];
                                Xa[1][objectCounter] = ob.getFeatures()[j];
                            }
                            else if(ob.getClassName() == classNames[1])
                            {
                                Xb[0][objectCounter] = ob.getFeatures()[i];
                                Xb[1][objectCounter] = ob.getFeatures()[j];
                            }

                            Ua[0][objectCounter] = classAverages[classNames[0]][i];
                            Ua[1][objectCounter] = classAverages[classNames[0]][j];
                            Ub[0][objectCounter] = classAverages[classNames[1]][i];
                            Ub[1][objectCounter] = classAverages[classNames[1]][j];

                            FUa[0][0] = classAverages[classNames[0]][i];
                            FUa[1][0] = classAverages[classNames[0]][j];
                            FUb[0][0] = classAverages[classNames[1]][i];
                            FUb[1][0] = classAverages[classNames[1]][j];

                            objectCounter ++;

                        }
                        boost::numeric::ublas::matrix<double> tmp1 (2,64);
                        boost::numeric::ublas::matrix<double> tmp2 (2,64);
                        tmp1 = Xa - Ua;
                        tmp2 = Xb - Ub;
                        classMatrixCov[0][std::make_pair(i, j)] = ((tmp1)*boost::numeric::ublas::trans(tmp1))/64;
                        classMatrixCov[1][std::make_pair(i, j)] = ((tmp2)*boost::numeric::ublas::trans(tmp2))/64;
                        float absAvgSub = std::abs(classMatrixCov[0][std::make_pair(i, j)] - classMatrixCov[1][std::make_pair(i, j)]);
                        //float detCov =  0;
                    }
                }

                boost::numeric::ublas::matrix<double> FUa (2,1);
                boost::numeric::ublas::matrix<double> FUb (2,1);


                //

                for (auto const &ob : database.getObjects())
                {
                    classAverages[ob.getClassName()] += ob.getFeatures()[i];
                    //classStds[ob.getClassName()] += ob.getFeatures()[i] * ob.getFeatures()[i];
                }

                std::for_each(database.getClassCounters().begin(), database.getClassCounters().end(), [&](const std::pair<std::string, int> &it)
                {
                    classAverages[it.first] /= it.second;
                    //classStds[it.first] = std::sqrt(classStds[it.first] / it.second - classAverages[it.first] * classAverages[it.first]);
                }
                );

                tmp = std::abs(classAverages[ database.getClassNames()[0] ] - classAverages[database.getClassNames()[1]]) / (classStds[database.getClassNames()[0]] + classStds[database.getClassNames()[1]]);

                if (tmp > FLD)
                {
                    FLD = tmp;
                    max_ind = i;
                }

              }

            ui->FStextBrowserDatabaseInfo->append("max_ind: "  +  QString::number(max_ind) + " " + QString::number(FLD));
          }
     }
}



void MainWindow::on_FSpushButtonSaveFile_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this,
    tr("Open TextFile"), "D:\\Users\\Krzysiu\\Documents\\Visual Studio 2015\\Projects\\SMPD\\SMPD\\Debug\\", tr("Texts Files (*.txt)"));

        QMessageBox::information(this, "My File", fileName);
        database.save(fileName.toStdString());
}

void MainWindow::on_PpushButtonSelectFolder_clicked()
{
}

void MainWindow::on_CpushButtonOpenFile_clicked()
{

}

void MainWindow::on_CpushButtonSaveFile_clicked()
{

}

void MainWindow::on_CpushButtonTrain_clicked()
{

}

void MainWindow::on_CpushButtonExecute_clicked()
{

}
