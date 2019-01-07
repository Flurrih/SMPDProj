#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include <boost/qvm/mat_operations.hpp>
#include <cstdlib>
#include <QImage>
#include <QDebug>
#include <SMPHelper.h>
#include "matrixutil.hpp"

typedef boost::numeric::ublas::matrix<double> doubleMatrix;
namespace bnu = boost::numeric::ublas;

float determinant2x2(bnu::matrix<double> mat)
{
	float x = 0;
	x = mat(0, 0) * mat(1, 1) - mat(1, 0) * mat(0, 1);
	return x;
}

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
	for (unsigned int i = 1; i <= database.getNoFeatures(); ++i)
		ui->FScomboBox->addItem(QString::number(i));

	ui->FStextBrowserDatabaseInfo->setText("noClass: " + QString::number(database.getNoClass()));
	ui->FStextBrowserDatabaseInfo->append("noObjects: " + QString::number(database.getNoObjects()));
	ui->FStextBrowserDatabaseInfo->append("noFeatures: " + QString::number(database.getNoFeatures()));

}

void MainWindow::FSupdateButtonState(void)
{
	if (database.getNoObjects() == 0)
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

	if (!database.load(fileName.toStdString()))
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
	SMPDHelper = new SMPHelper();

	if (ui->FSradioButtonFisher->isChecked())
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

				tmp = std::abs(classAverages[database.getClassNames()[0]] - classAverages[database.getClassNames()[1]]) / (classStds[database.getClassNames()[0]] + classStds[database.getClassNames()[1]]);

				if (tmp > FLD)
				{
					FLD = tmp;
					max_ind = i;
				}

			}

			ui->FStextBrowserDatabaseInfo->append("max_ind: " + QString::number(max_ind) + " " + QString::number(FLD));
		}
		else if (dimension > 1 && database.getNoClass() == 2)
		{
			float FLD = 0, tmpFLD;
			int max_ind = -1;
			std::vector<int> bestCombination;
			std::vector<int> tmpComb;
			std::map<std::string, std::map<int, double>> classAverages;



			//Stworz liste resdnich dla A i B
			for (int i = 0; i < database.getObjects().size(); i++)//auto const &ob : database.getObjects()
			{

				objectCount[database.getObjects()[i].getClassName()] ++;
				for (int xx = 0; xx < database.getNoFeatures(); xx++)
				{
					float val = database.getObjects()[i].getFeatures()[xx] / dimension;
					classAverages[database.getObjects()[i].getClassName()][i] += val;
				}
			}


			std::string bitmask(dimension, 1); // K leading 1's
			bitmask.resize(database.getNoFeatures(), 0); // N-K trailing 0's
			do {
				tmpComb.clear();
				for (int i = 0; i < database.getNoFeatures(); ++i) // [0..N-1] integers
				{
					if (bitmask[i])
					{
						// Building combinations
						tmpComb.push_back(i);
					}
				}
				///// Obliczenie fishera

				//Wygeneruj macierz srednich dla A i B do obliczen
				boost::numeric::ublas::matrix<double> Ua(dimension, objectCount[classNames[0]]);
				boost::numeric::ublas::matrix<double> Ub(dimension, objectCount[classNames[1]]);

				Ua = SMPDHelper->GenerateAvarageMatrixForFeatures
				(tmpComb, classAverages[classNames[0]], objectCount[classNames[0]], dimension);
				Ub = SMPDHelper->GenerateAvarageMatrixForFeatures
				(tmpComb, classAverages[classNames[1]], objectCount[classNames[1]], dimension);

				//Znajdz Xa i Xb
				boost::numeric::ublas::matrix<double> Xa(dimension, objectCount[classNames[0]]);
				boost::numeric::ublas::matrix<double> Xb(dimension, objectCount[classNames[1]]);/*
				Xa = SMPDHelper->GenerateXMatrixForFeatures
								(tmpComb, database, objectCount[classNames[0]], dimension, classNames[0]);
				Xb = SMPDHelper->GenerateXMatrixForFeatures
								(tmpComb, database, objectCount[classNames[1]], dimension, classNames[1]);*/

								//SMPDHelper->GenerateXMatrixForFeatures(tmpComb, database, objectCount, classNames, &Xa, &Xb);

				int countA = 0;
				int countB = 0;
				for (int i = 0; i < tmpComb.size(); i++)
				{
					countA = 0;
					countB = 0;
					for (int j = 0; j < database.getObjects().size(); j++)
					{
						if (database.getObjects()[j].getClassName() == classNames[0])
						{
							Xa(i, countA) = database.getObjects()[j].getFeatures()[tmpComb[i]];
							countA++;
						}
						else if (database.getObjects()[j].getClassName() == classNames[1])
						{
							Xa(i, countB) = database.getObjects()[j].getFeatures()[tmpComb[i]];
							countB++;
						}
					}
				}
				//Sa i Sb
				boost::numeric::ublas::matrix<double> Sa(dimension, dimension);
				boost::numeric::ublas::matrix<double> Sb(dimension, dimension);
				//boost::numeric::ublas::prod(macierzQuercus, boost::numeric::ublas::trans(macierzQuercus));

				//boost::numeric::ublas::matrix<double> Xa_Ua(dimension, objectCount[classNames[0]]);
				//boost::numeric::ublas::matrix<double> Xb_Ub(dimension, objectCount[classNames[1]]);

				//Xa_Ua = Xa - Ua;
				//Xb_Ub = Xb - Ub;
/*
				boost::numeric::ublas::matrix<double> Xa_UaTrans(objectCount[classNames[0]], dimension);
				boost::numeric::ublas::matrix<double> Xb_UbTrans(dimension, objectCount[classNames[1]]);*/
				//UaTrans = boost::numeric::ublas::trans(Ua);
				//UbTrans = boost::numeric::ublas::trans(Ub);(1/ objectCount[classNames[0]])

				Sa = (1 / objectCount[classNames[0]]) * boost::numeric::ublas::prod(Xa - Ua, boost::numeric::ublas::trans(Xa - Ua));
				Sb = (1 / objectCount[classNames[1]]) * boost::numeric::ublas::prod(Xb - Ub, boost::numeric::ublas::trans(Xb - Ub));
				//det
				boost::numeric::ublas::matrix< double > sumSaSb = Sa + Sb;
				//odleglosc Ua - Ub

				//F

				tmpFLD = SMPDHelper->CalculateUa_UbAvaragesLength(tmpComb, classAverages[classNames[0]], classAverages[classNames[1]])
					/ determinant<double>(sumSaSb);

				if (tmpFLD > FLD)
				{
					FLD = tmpFLD;
					bestCombination = tmpComb;
				}


			} while (std::prev_permutation(bitmask.begin(), bitmask.end()));

			ui->FStextBrowserDatabaseInfo->append("FLD: " + QString::number(FLD));
			for (int i = 0; i < dimension; i++)
			{
				ui->FStextBrowserDatabaseInfo->append(QString::number(bestCombination[i]));
			}
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
