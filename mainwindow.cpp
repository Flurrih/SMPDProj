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
#include <boost/numeric/ublas/io.hpp>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <ctime>

typedef boost::numeric::ublas::matrix<long double> doubleMatrix;
namespace bnu = boost::numeric::ublas;

float TotalHits = 0;
std::vector<int> bestCombination;
std::vector<int> featuresForClassification;




float determinant2x2(bnu::matrix<long double> mat)
{
	float x = 0;
	x = mat(0, 0) * mat(1, 1) - mat(1, 0) * mat(0, 1);
	return x;
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	QString nnString = "NN";

	ui->setupUi(this);
	FSupdateButtonState();
	time_t now = time(0);
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
	classifiers = new Classifiers(&database);
	//ui->CcomboBoxClassifiers->addItems({ QString::QString("NN") , QString::QString("NM") });

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

void printMatrix(boost::numeric::ublas::matrix<long double> m, int x, int y, std::string name)
{
	std::ofstream outputFile("output/" + name + ".txt");
	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			outputFile << m(i, j) << "	";
		}
		outputFile << std::endl;
	}
	outputFile.close();
}


bool containsValue(std::vector<int> vec, int val)
{
	for each (int var in vec)
	{
		if (var == val)
			return true;
	}
	return false;
}

void MainWindow::on_FSpushButtonCompute_clicked()
{
	int dimension = ui->FScomboBox->currentText().toInt();
	std::vector<std::string> classNames = database.getClassNames();

	SMPDHelper = new SMPHelper();
	int currentFeature = 0;

	std::map<std::string, int> objectCount;

	if (ui->FSradioButtonFisher->isChecked())
	{
		if (dimension == 1 && database.getNoClass() == 2)
		{
			long double FLD = 0, tmp;
			int max_ind = -1;

			//std::map<std::string, int> classNames = database.getClassNames();
			for (uint i = 0; i < database.getNoFeatures(); ++i)
			{
				std::map<std::string, long double> classAverages;
				std::map<std::string, long double> classStds;

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

					featuresForClassification.clear();
					featuresForClassification.push_back(max_ind);
				}

			}

			ui->FStextBrowserDatabaseInfo->append("max_ind: " + QString::number(max_ind) + " " + QString::number((double)FLD));
		}
		else if (dimension > 1 && database.getNoClass() == 2)
		{
			long double FLD = 0, tmpFLD;
			int max_ind = -1;

			std::vector<int> tmpComb;
			std::map<std::string, std::map<int, long double>> classAverages;

			for (int i = 0; i < database.getObjects().size(); i++)//auto const &ob : database.getObjects()
				objectCount[database.getObjects()[i].getClassName()] ++;

			//Stworz liste srednich dla A i B
			for (int i = 0; i < database.getObjects().size(); i++)//auto const &ob : database.getObjects()
			{
				for (int xx = 0; xx < database.getNoFeatures(); xx++)
				{
					long double val = database.getObjects()[i].getFeatures()[xx] / objectCount[database.getObjects()[i].getClassName()];
					classAverages[database.getObjects()[i].getClassName()][xx] += val;
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
				boost::numeric::ublas::matrix<long double> Ua(dimension, objectCount[classNames[0]]);
				boost::numeric::ublas::matrix<long double> Ub(dimension, objectCount[classNames[1]]);

				Ua = SMPDHelper->GenerateAvarageMatrixForFeatures
				(tmpComb, classAverages[classNames[0]], objectCount[classNames[0]], dimension);
				Ub = SMPDHelper->GenerateAvarageMatrixForFeatures
				(tmpComb, classAverages[classNames[1]], objectCount[classNames[1]], dimension);



				//Znajdz Xa i Xb
				boost::numeric::ublas::matrix<long double> Xa(dimension, objectCount[classNames[0]]);
				boost::numeric::ublas::matrix<long double> Xb(dimension, objectCount[classNames[1]]);

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
							Xb(i, countB) = database.getObjects()[j].getFeatures()[tmpComb[i]];
							countB++;
						}
					}
				}

				//Sa i Sb
				boost::numeric::ublas::matrix<long double> Sa(dimension, dimension);
				boost::numeric::ublas::matrix<long double> Sb(dimension, dimension);

				boost::numeric::ublas::matrix<long double> Xa_Ua(dimension, objectCount[classNames[0]]);
				boost::numeric::ublas::matrix<long double> Xb_Ub(dimension, objectCount[classNames[1]]);

				Xa_Ua = Xa - Ua;
				Xb_Ub = Xb - Ub;

				boost::numeric::ublas::matrix<long double> Xa_UaTrans(objectCount[classNames[0]], dimension);
				boost::numeric::ublas::matrix<long double> Xb_UbTrans(objectCount[classNames[1]], dimension);
				Xa_UaTrans = boost::numeric::ublas::trans(Xa_Ua);
				Xb_UbTrans = boost::numeric::ublas::trans(Xb_Ub);

				boost::numeric::ublas::matrix<long double> Xa_UaXa_UaTrans(dimension, dimension);
				boost::numeric::ublas::matrix<long double> Xb_UbXb_UbTrans(dimension, dimension);

				Xa_UaXa_UaTrans = boost::numeric::ublas::prod(Xa_Ua, Xa_UaTrans);
				Xb_UbXb_UbTrans = boost::numeric::ublas::prod(Xb_Ub, Xb_UbTrans);

				long double t1 = (long double)objectCount[classNames[0]];
				long double t2 = (long double)objectCount[classNames[1]];

				Sa = Xa_UaXa_UaTrans * ((long double)1.0 / t1);
				Sb = Xb_UbXb_UbTrans * ((long double)1.0 / t2);

				//det
				boost::numeric::ublas::matrix< long double > sumSaSb(dimension, dimension);
				sumSaSb = Sa + Sb;
				//distance Ua - Ub
				printMatrix(sumSaSb, dimension, dimension, "sumSa");
				//F
				long double x1 = SMPDHelper->CalculateUa_UbAvaragesLength(tmpComb, classAverages[classNames[0]], classAverages[classNames[1]]);
				long double x2 = determinant<long double>(sumSaSb);

				tmpFLD = x1 / x2;

				if (tmpFLD > FLD)
				{
					FLD = tmpFLD;
					bestCombination = tmpComb;


					printMatrix(Sa, dimension, dimension, "Sa");
					printMatrix(Sb, dimension, dimension, "Sb");
					printMatrix(Xa_UaTrans, objectCount[classNames[0]], dimension, "Xa_UaTrans");
					printMatrix(Xb_UbTrans, objectCount[classNames[1]], dimension, "Xb_UbTrans");
					printMatrix(Xa_Ua, dimension, objectCount[classNames[0]], "Xa_Ua");
					printMatrix(Xb_Ub, dimension, objectCount[classNames[1]], "Xb_Ub");
					printMatrix(Ua, dimension, objectCount[classNames[0]], "Ua");
					printMatrix(Ub, dimension, objectCount[classNames[1]], "Ub");
					printMatrix(Xa, dimension, objectCount[classNames[0]], "Xa");
					printMatrix(Xb, dimension, objectCount[classNames[1]], "Xb");
					printMatrix(Xa_UaXa_UaTrans, dimension, dimension, "Xa_UaXa_UaTrans");
					printMatrix(Xb_UbXb_UbTrans, dimension, dimension, "Xb_UbXb_UbTrans");
				}

			} while (std::prev_permutation(bitmask.begin(), bitmask.end()));

			featuresForClassification = bestCombination;

			ui->FStextBrowserDatabaseInfo->append("FLD: " + QString::number((double)FLD));
			for (int i = 0; i < dimension; i++)
			{
				ui->FStextBrowserDatabaseInfo->append(QString::number(bestCombination[i]));
			}
		}
	}
	else if (ui->FSradioButtonSFS->isChecked())
	{

		// C1
		std::vector<int> sfsCombination, bestSfs;
		long double FLD = 0, tmpFLD, maxFLD = 0;
		int max_ind = -1;
		std::vector<int> tmpComb;
		std::map<std::string, std::map<int, long double>> classAverages;

		//std::map<std::string, int> classNames = database.getClassNames();
		for (uint i = 0; i < database.getNoFeatures(); ++i)
		{
			std::map<std::string, long double> classAverages1;
			std::map<std::string, long double> classStds;

			for (auto const &ob : database.getObjects())
			{
				classAverages1[ob.getClassName()] += ob.getFeatures()[i];
				classStds[ob.getClassName()] += ob.getFeatures()[i] * ob.getFeatures()[i];
			}

			std::for_each(database.getClassCounters().begin(), database.getClassCounters().end(), [&](const std::pair<std::string, int> &it)
			{
				classAverages1[it.first] /= it.second;
				classStds[it.first] = std::sqrt(classStds[it.first] / it.second - classAverages1[it.first] * classAverages1[it.first]);
			}
			);

			tmpFLD = std::abs(classAverages1[database.getClassNames()[0]] - classAverages1[database.getClassNames()[1]]) / (classStds[database.getClassNames()[0]] + classStds[database.getClassNames()[1]]);

			if (tmpFLD > FLD)
			{
				FLD = tmpFLD;
				max_ind = i;
				featuresForClassification.clear();
				featuresForClassification.push_back(max_ind);
			}
		}
		maxFLD = FLD;
		FLD = 0;
		bestSfs.push_back(max_ind);

		for (int i = 0; i < database.getObjects().size(); i++)//auto const &ob : database.getObjects()
			objectCount[database.getObjects()[i].getClassName()] ++; //liczymy ilo�� obiekt�w

		//Stworz liste srednich dla A i B
		for (int i = 0; i < database.getObjects().size(); i++)//auto const &ob : database.getObjects()
		{
			for (int xx = 0; xx < database.getNoFeatures(); xx++)
			{
				long double val = database.getObjects()[i].getFeatures()[xx] / objectCount[database.getObjects()[i].getClassName()];
				classAverages[database.getObjects()[i].getClassName()][xx] += val;
			}
		}

		for (int currentDimenstion = 1; currentDimenstion < dimension; currentDimenstion++)
		{
			for (int c = 0; c < database.getNoFeatures(); c++)
			{
				tmpComb = bestSfs;
				if (!containsValue(tmpComb, c)) //sprawdzamy czy cechy si� nie powtarzaj�
					tmpComb.push_back(c);
				else
					continue;
				///// Obliczenie fishera

				//Wygeneruj macierz srednich dla A i B do obliczen
				boost::numeric::ublas::matrix<long double> Ua(currentDimenstion + 1, objectCount[classNames[0]]);
				boost::numeric::ublas::matrix<long double> Ub(currentDimenstion + 1, objectCount[classNames[1]]);

				Ua = SMPDHelper->GenerateAvarageMatrixForFeatures
				(tmpComb, classAverages[classNames[0]], objectCount[classNames[0]], currentDimenstion + 1);
				Ub = SMPDHelper->GenerateAvarageMatrixForFeatures
				(tmpComb, classAverages[classNames[1]], objectCount[classNames[1]], currentDimenstion + 1);



				//Znajdz Xa i Xb
				boost::numeric::ublas::matrix<long double> Xa(currentDimenstion + 1, objectCount[classNames[0]]);
				boost::numeric::ublas::matrix<long double> Xb(currentDimenstion + 1, objectCount[classNames[1]]);

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
							Xb(i, countB) = database.getObjects()[j].getFeatures()[tmpComb[i]];
							countB++;
						}
					}
				}

				//Sa i Sb
				boost::numeric::ublas::matrix<long double> Sa(currentDimenstion + 1, currentDimenstion + 1);
				boost::numeric::ublas::matrix<long double> Sb(currentDimenstion + 1, currentDimenstion + 1);


				boost::numeric::ublas::matrix<long double> Xa_Ua(currentDimenstion + 1, objectCount[classNames[0]]);
				boost::numeric::ublas::matrix<long double> Xb_Ub(currentDimenstion + 1, objectCount[classNames[1]]);


				Xa_Ua = Xa - Ua;
				Xb_Ub = Xb - Ub;



				boost::numeric::ublas::matrix<long double> Xb_UbTrans(objectCount[classNames[1]], currentDimenstion + 1);
				boost::numeric::ublas::matrix<long double> Xa_UaTrans(objectCount[classNames[0]], currentDimenstion + 1);
				Xa_UaTrans = boost::numeric::ublas::trans(Xa_Ua);
				Xb_UbTrans = boost::numeric::ublas::trans(Xb_Ub);

				boost::numeric::ublas::matrix<long double> Xa_UaXa_UaTrans(currentDimenstion + 1, currentDimenstion + 1);
				boost::numeric::ublas::matrix<long double> Xb_UbXb_UbTrans(currentDimenstion + 1, currentDimenstion + 1);

				Xa_UaXa_UaTrans = boost::numeric::ublas::prod(Xa_Ua, Xa_UaTrans);
				Xb_UbXb_UbTrans = boost::numeric::ublas::prod(Xb_Ub, Xb_UbTrans);

				long double t1 = (long double)objectCount[classNames[0]];
				long double t2 = (long double)objectCount[classNames[1]];

				Sa = Xa_UaXa_UaTrans * ((long double)1.0 / t1);
				Sb = Xb_UbXb_UbTrans * ((long double)1.0 / t2);

				//det
				boost::numeric::ublas::matrix< long double > sumSaSb(currentDimenstion + 1, currentDimenstion + 1);
				sumSaSb = Sa + Sb;
				//distance Ua - Ub

				//F
				long double x1 = SMPDHelper->CalculateUa_UbAvaragesLength(tmpComb, classAverages[classNames[0]], classAverages[classNames[1]]);
				long double x2 = determinant<long double>(sumSaSb);

				tmpFLD = x1 / x2;

				if (tmpFLD > FLD)
				{
					FLD = tmpFLD;
					sfsCombination = tmpComb;
					maxFLD = FLD;
					//printMatrix(Sa, currentDimenstion, currentDimenstion, "Sa");
					//printMatrix(Sb, currentDimenstion, currentDimenstion, "Sb");
					//printMatrix(Xa_UaTrans, objectCount[classNames[0]], currentDimenstion, "Xa_UaTrans");
					//printMatrix(Xb_UbTrans, objectCount[classNames[1]], currentDimenstion, "Xb_UbTrans");
					//printMatrix(Xa_Ua, currentDimenstion, objectCount[classNames[0]], "Xa_Ua");
					//printMatrix(Xb_Ub, currentDimenstion, objectCount[classNames[1]], "Xb_Ub");
					//printMatrix(Ua, currentDimenstion, objectCount[classNames[0]], "Ua");
					//printMatrix(Ub, currentDimenstion, objectCount[classNames[1]], "Ub");
					//printMatrix(Xa, currentDimenstion, objectCount[classNames[0]], "Xa");
					//printMatrix(Xb, currentDimenstion, objectCount[classNames[1]], "Xb");
					//printMatrix(Xa_UaXa_UaTrans, currentDimenstion, currentDimenstion, "Xa_UaXa_UaTrans");
					//printMatrix(Xb_UbXb_UbTrans, currentDimenstion, currentDimenstion, "Xb_UbXb_UbTrans");
				}
			}
			bestSfs = sfsCombination;
			featuresForClassification = bestSfs;
			FLD = 0;
		}
		ui->FStextBrowserDatabaseInfo->append("FLD: " + QString::number((double)maxFLD));
		for (int i = 0; i < bestSfs.size(); i++)
		{
			ui->FStextBrowserDatabaseInfo->append(QString::number(bestSfs[i]));
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
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open TextFile"), "", tr("Texts Files (*.txt)"));

	if (!database.load(fileName.toStdString()))
		QMessageBox::warning(this, "Warning", "File corrupted !!!");
	else
		QMessageBox::information(this, fileName, "File loaded !!!");

	FSupdateButtonState();
	updateDatabaseInfo();
}

void MainWindow::on_CpushButtonSaveFile_clicked()
{
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Open TextFile"), "D:\\Users\\Krzysiu\\Documents\\Visual Studio 2015\\Projects\\SMPD\\SMPD\\Debug\\", tr("Texts Files (*.txt)"));

	QMessageBox::information(this, "My File", fileName);
	database.save(fileName.toStdString());
}

void MainWindow::on_CpushButtonTrain_clicked()

{

	std::vector<Object> testObjects;
	std::vector<Object> trainObjects;
	int procToTrainingBase = ui->CplainTextEditTrainingPart->toPlainText().toInt();
	classifiers->divideObjectsAsTrainAndTest(procToTrainingBase);
	
	ui->CtextBrowser->append("Trening zakonczony");
	
}

void MainWindow::on_CpushButtonBoostrap_clicked()
{
	std::vector<Object> tmp0;
	std::vector<Object> tmp1;
	std::vector<Object> tmp2;
	bootstrapTestObjectVectors.push_back(tmp0);
	bootstrapTestObjectVectors.push_back(tmp1);
	bootstrapTestObjectVectors.push_back(tmp2);

	std::vector<Object> tmp00;
	std::vector<Object> tmp11;
	std::vector<Object> tmp22;
	bootstrapTrainObjectVectors.push_back(tmp00);
	bootstrapTrainObjectVectors.push_back(tmp11);
	bootstrapTrainObjectVectors.push_back(tmp22);
	TotalHits = 0;
	for (int i = 0; i < ui->ComboBoxBoostrapK->currentText().toInt(); i++)
	{
		int randomNumber = 0;
		std::vector<int> randomNumbers;
		bool notRepeated = true;
		//classifiers->testObjects.clear();
		//classifiers-> trainObjects.clear();
		if (bootstrapTestObjectVectors[i].size() <= 0)
		{
			for (int j = 0; j < database.getNoObjects(); j++)
			{
				randomNumber = (rand() % (int)(database.getNoObjects()));
				//classifiers->trainObjects.push_back(database.getObjects()[randomNumber]);
				bootstrapTrainObjectVectors[i].push_back(database.getObjects()[randomNumber]);
				randomNumbers.push_back(randomNumber);
			}

			for (int k = 0; k < database.getNoObjects(); k++)
			{
				notRepeated = true;
				for (int j = 0; j < randomNumbers.size(); j++)
				{
					if (randomNumbers[j] == k)
					{
						notRepeated = false;
					}
				}
				if (notRepeated) {
					//classifiers->testObjects.push_back(database.getObjects()[k]);
					bootstrapTestObjectVectors[i].push_back(database.getObjects()[k]);
				}
			}
		}

		//on_CpushButtonExecute_clicked();

		int numberOfAcer = 0;
		int numberOfHitsAcer = 0;
		float percentOfHitsAcer = 0;

		int numberOfQuercus = 0;
		int numberOfHitsQuercus = 0;
		float percentOfHitsQuercus = 0;

		float percentOfHits = 0;
		double distance = 0;
		double minDistance = 99999;
		int idNearestNeighbor = -1;

		if (ui->CcomboBoxClassifiers->currentText() == "NN")
		{

			ui->CtextBrowser->append("-------------------");
			ui->CtextBrowser->append("Klasyfikator NN");

			classifiers->NNClasiffier(featuresForClassification, bootstrapTrainObjectVectors[i], bootstrapTestObjectVectors[i]);

			percentOfHits = ((classifiers->numberOfHitsAcer + classifiers->numberOfHitsQuercus) * 100) / (classifiers->numberOfAcer + classifiers->numberOfQuercus);
			//ui->CtextBrowser->append("liczba Acer:" + QString::number(classifiers->numberOfAcer) + "   liczba trafien dla Acer:" + QString::number(classifiers->numberOfHitsAcer));
			//ui->CtextBrowser->append("liczba Quercus:" + QString::number(classifiers->numberOfQuercus) + "   liczba trafien dla Quercus:" + QString::number(classifiers->numberOfHitsQuercus));
			ui->CtextBrowser->append("Procent tafien dla Acer: " + QString::number(classifiers->percentOfHitsAcer) + "%");
			ui->CtextBrowser->append("Procent tafien dla Quercus: " + QString::number(classifiers->percentOfHitsQuercus) + "%");
			ui->CtextBrowser->append("Procent poprawnie zakfalifikowanych probek: " + QString::number(percentOfHits) + "%");
			TotalHits += percentOfHits;
			ui->CtextBrowser->append("-------------------");
		}



		if (ui->CcomboBoxClassifiers->currentText() == "NM") {
			ui->CtextBrowser->append("-------------------");
			ui->CtextBrowser->append("NM");

			classifiers->NMClasiffier(featuresForClassification, bootstrapTrainObjectVectors[i], bootstrapTestObjectVectors[i]);

			percentOfHits = ((classifiers->numberOfHitsAcer + classifiers->numberOfHitsQuercus) * 100) / (classifiers->numberOfAcer + classifiers->numberOfQuercus);
			//ui->CtextBrowser->append("liczba Acer:" + QString::number(classifiers->numberOfAcer) + "   liczba trafien dla Acer:" + QString::number(classifiers->numberOfHitsAcer));
			//ui->CtextBrowser->append("liczba Quercus:" + QString::number(classifiers->numberOfQuercus) + "   liczba trafien dla Quercus:" + QString::number(classifiers->numberOfHitsQuercus));
			ui->CtextBrowser->append("Procent tafien dla Acer: " + QString::number(classifiers->percentOfHitsAcer) + "%");
			ui->CtextBrowser->append("Procent tafien dla Quercus: " + QString::number(classifiers->percentOfHitsQuercus) + "%");
			ui->CtextBrowser->append("Procent poprawnie zakfalifikowanych probek: " + QString::number(percentOfHits) + "%");
			TotalHits += percentOfHits;
			ui->CtextBrowser->append("-------------------");
		}

		if (ui->CcomboBoxClassifiers->currentText() == "KNN") {
			ui->CtextBrowser->append("-------------------");
			ui->CtextBrowser->append("Klasyfikator kNN");

			classifiers->kNNClasiffier(featuresForClassification, ui->CcomboBoxK->currentText().toInt(), bootstrapTrainObjectVectors[i], bootstrapTestObjectVectors[i]);

			percentOfHits = ((classifiers->numberOfHitsAcer + classifiers->numberOfHitsQuercus) * 100) / (classifiers->numberOfAcer + classifiers->numberOfQuercus);
			//ui->CtextBrowser->append("liczba Acer:" + QString::number(classifiers->numberOfAcer) + "   liczba trafien dla Acer:" + QString::number(classifiers->numberOfHitsAcer));
			//ui->CtextBrowser->append("liczba Quercus:" + QString::number(classifiers->numberOfQuercus) + "   liczba trafien dla Quercus:" + QString::number(classifiers->numberOfHitsQuercus));
			ui->CtextBrowser->append("Procent tafien dla Acer: " + QString::number(classifiers->percentOfHitsAcer) + "%");
			ui->CtextBrowser->append("Procent tafien dla Quercus: " + QString::number(classifiers->percentOfHitsQuercus) + "%");
			ui->CtextBrowser->append("Procent poprawnie zakfalifikowanych probek: " + QString::number(percentOfHits) + "%");
			TotalHits += percentOfHits;
			ui->CtextBrowser->append("-------------------");
		}
		if (numberOfAcer != 0)
		{
			percentOfHitsAcer = (numberOfHitsAcer * 100 / numberOfAcer);
		}
		else {
			percentOfHitsAcer = 0;
		}
		if (numberOfQuercus != 0) {
			percentOfHitsQuercus = (numberOfHitsQuercus * 100 / numberOfQuercus);
		}
		else {
			percentOfHitsQuercus = 0;
		}

	}
	//featuresForClassification.clear();
	ui->CtextBrowser->append(" ++++++++++++++++++++ ");
	ui->CtextBrowser->append("Srednia dla Boostrap ");
	ui->CtextBrowser->append("dla klasyfikatora " + ui->CcomboBoxClassifiers->currentText());
	ui->CtextBrowser->append(QString::number(TotalHits / ui->ComboBoxBoostrapK->currentText().toUInt()) + "%");

}

void MainWindow::on_CpushButtonBoostrapR_clicked()
{
	bootstrapTestObjectVectors.clear();
	bootstrapTrainObjectVectors.clear();

	std::vector<Object> tmp0;
	std::vector<Object> tmp1;
	std::vector<Object> tmp2;
	bootstrapTestObjectVectors.push_back(tmp0);
	bootstrapTestObjectVectors.push_back(tmp1);
	bootstrapTestObjectVectors.push_back(tmp2);

	std::vector<Object> tmp00;
	std::vector<Object> tmp11;
	std::vector<Object> tmp22;
	bootstrapTrainObjectVectors.push_back(tmp00);
	bootstrapTrainObjectVectors.push_back(tmp11);
	bootstrapTrainObjectVectors.push_back(tmp22);

	ui->CtextBrowser->append(" RESET ");
}

void MainWindow::on_CpushButtonExecute_clicked()
{
	int numberOfAcer = 0;
	int numberOfHitsAcer = 0;
	float percentOfHitsAcer = 0;

	int numberOfQuercus = 0;
	int numberOfHitsQuercus = 0;
	float percentOfHitsQuercus = 0;

	float percentOfHits = 0;
	double distance = 0;
	double minDistance = 99999;
	int idNearestNeighbor = -1;



	if (ui->CcomboBoxClassifiers->currentText() == "NN")
	{

		ui->CtextBrowser->append("-------------------");
		ui->CtextBrowser->append("Klasyfikator NN");

		classifiers->NNClasiffier(featuresForClassification);
		
		percentOfHits = ((classifiers->numberOfHitsAcer + classifiers->numberOfHitsQuercus) * 100) / (classifiers->numberOfAcer + classifiers->numberOfQuercus);
		//ui->CtextBrowser->append("liczba Acer:" + QString::number(classifiers->numberOfAcer) + "   liczba trafien dla Acer:" + QString::number(classifiers->numberOfHitsAcer));
		//ui->CtextBrowser->append("liczba Quercus:" + QString::number(classifiers->numberOfQuercus) + "   liczba trafien dla Quercus:" + QString::number(classifiers->numberOfHitsQuercus));
		ui->CtextBrowser->append("Procent tafien dla Acer: " + QString::number(classifiers->percentOfHitsAcer) + "%");
		ui->CtextBrowser->append("Procent tafien dla Quercus: " + QString::number(classifiers->percentOfHitsQuercus) + "%");
		ui->CtextBrowser->append("Procent poprawnie zakfalifikowanych probek: " + QString::number(percentOfHits) + "%");
		TotalHits += percentOfHits;
		ui->CtextBrowser->append("-------------------");
	}



	if (ui->CcomboBoxClassifiers->currentText() == "NM") {
		ui->CtextBrowser->append("-------------------");
		ui->CtextBrowser->append("NM");

		classifiers->NMClasiffier(featuresForClassification);

		percentOfHits = ((classifiers->numberOfHitsAcer + classifiers->numberOfHitsQuercus) * 100) / (classifiers->numberOfAcer + classifiers->numberOfQuercus);
		//ui->CtextBrowser->append("liczba Acer:" + QString::number(classifiers->numberOfAcer) + "   liczba trafien dla Acer:" + QString::number(classifiers->numberOfHitsAcer));
		//ui->CtextBrowser->append("liczba Quercus:" + QString::number(classifiers->numberOfQuercus) + "   liczba trafien dla Quercus:" + QString::number(classifiers->numberOfHitsQuercus));
		ui->CtextBrowser->append("Procent tafien dla Acer: " + QString::number(classifiers->percentOfHitsAcer) + "%");
		ui->CtextBrowser->append("Procent tafien dla Quercus: " + QString::number(classifiers->percentOfHitsQuercus) + "%");
		ui->CtextBrowser->append("Procent poprawnie zakfalifikowanych probek: " + QString::number(percentOfHits) + "%");
		TotalHits += percentOfHits;
		ui->CtextBrowser->append("-------------------");
	}

	if (ui->CcomboBoxClassifiers->currentText() == "KNN") {
		ui->CtextBrowser->append("-------------------");
		ui->CtextBrowser->append("Klasyfikator kNN");

		classifiers->kNNClasiffier(featuresForClassification, ui->CcomboBoxK->currentText().toInt());

		percentOfHits = ((classifiers->numberOfHitsAcer + classifiers->numberOfHitsQuercus) * 100) / (classifiers->numberOfAcer + classifiers->numberOfQuercus);
		//ui->CtextBrowser->append("liczba Acer:" + QString::number(classifiers->numberOfAcer) + "   liczba trafien dla Acer:" + QString::number(classifiers->numberOfHitsAcer));
		//ui->CtextBrowser->append("liczba Quercus:" + QString::number(classifiers->numberOfQuercus) + "   liczba trafien dla Quercus:" + QString::number(classifiers->numberOfHitsQuercus));
		ui->CtextBrowser->append("Procent tafien dla Acer: " + QString::number(classifiers->percentOfHitsAcer) + "%");
		ui->CtextBrowser->append("Procent tafien dla Quercus: " + QString::number(classifiers->percentOfHitsQuercus) + "%");
		ui->CtextBrowser->append("Procent poprawnie zakfalifikowanych probek: " + QString::number(percentOfHits) + "%");
		TotalHits += percentOfHits;
		ui->CtextBrowser->append("-------------------");
	}
	if (numberOfAcer != 0)
	{
		percentOfHitsAcer = (numberOfHitsAcer * 100 / numberOfAcer);
	}
	else {
		percentOfHitsAcer = 0;
	}
	if (numberOfQuercus != 0) {
		percentOfHitsQuercus = (numberOfHitsQuercus * 100 / numberOfQuercus);
	}
	else {
		percentOfHitsQuercus = 0;
	}
}
