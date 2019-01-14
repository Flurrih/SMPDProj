#include "Classifiers.h"



Classifiers::Classifiers(Database *main)
{
	this->main = main;
	classNames = main->getClassNames();
}


Classifiers::~Classifiers()
{
}

void fillMatrixWithAvarages(std::map<int, long double> classAvg, boost::numeric::ublas::matrix<long double> &matrix)
{
	for (int i = 0; i < classAvg.size(); i++)
	{
		matrix(i, 0) = classAvg[i];
	}
}

void Classifiers::divideObjectsAsTrainAndTest(double trainPercent)
{
	testObjects.clear();
	trainObjects.clear();
	std::map<std::string, int> objectCount;
	for (int i = 0; i < main->getNoObjects(); i++)
		objectCount[main->getObjects()[i].getClassName()] ++;
	int numberOfTrainA = objectCount[classNames[0]] * trainPercent/100;
	int numberOfTrainB = objectCount[classNames[1]] * trainPercent/100;

	for each (Object ob in main->getObjects())
	{
		if (ob.getClassName() == classNames[0])
		{
			if (numberOfTrainA > 0)
			{
				trainObjects.push_back(ob);
				numberOfTrainA--;
			}
			else
			{
				testObjects.push_back(ob);
			}
		}
		else if (ob.getClassName() == classNames[1])
		{
			if (numberOfTrainB > 0)
			{
				trainObjects.push_back(ob);
				numberOfTrainB--;
			}
			else
			{
				testObjects.push_back(ob);
			}
		}
	}
}

void fillFeatureMatrix(Object obj, boost::numeric::ublas::matrix<long double> &matrix)
{
	for (int ft = 0; ft < obj.getFeatures().size(); ft++)
	{
		matrix(ft, 0) = obj.getFeatures()[ft];
	}
}

long double calculateLengthOfMatrix(boost::numeric::ublas::matrix<long double> matrix)
{
	long double sum = 0;
	for (int i = 0; i < matrix.size1(); i++)
	{
		sum += matrix(i, 0)*matrix(i, 0);
	}

	return sqrt(sum);
}
void Classifiers::NNClasiffier(std::vector<int> cechyDoKlasyf)
{
	int idNajblizszegoSasiada = 0;;
	liczbaAcer = 0;
	lTrafienAcer = 0;
	lTrafienQuercus = 0;
	liczbaQuercus = 0;
	procentTrafienAcer = 0;
	procentTrafienQuercus = 0;


	for (int i = 0; i < testObjects.size(); i++) {
		double najmniejszaOdleglosc = 99999;

		for (int j = 0; j < trainObjects.size(); j++) {
			double odleglosc = 0;

			for (int k = 0; k < cechyDoKlasyf.size(); k++) {
				odleglosc += pow(trainObjects[j].getFeatures()[cechyDoKlasyf[k]] - testObjects[i].getFeatures()[cechyDoKlasyf[k]], 2);
			}

			odleglosc = sqrt(odleglosc);

			if (odleglosc < najmniejszaOdleglosc) {

				najmniejszaOdleglosc = odleglosc;
				idNajblizszegoSasiada = j;
			}
		}

		if (testObjects[i].getClassName().compare("Acer") == 0) {
			liczbaAcer++;
			if (trainObjects[idNajblizszegoSasiada].getClassName().compare("Acer") == 0) {
				lTrafienAcer++;
			}
		}
		else if (testObjects[i].getClassName().compare("Quercus") == 0) {
			liczbaQuercus++;
			if (trainObjects[idNajblizszegoSasiada].getClassName().compare("Quercus") == 0) {
				lTrafienQuercus++;

			}
		}
	}

	if (liczbaAcer != 0)
	{
		procentTrafienAcer = (lTrafienAcer * 100 / liczbaAcer);
	}
	else {
		procentTrafienAcer = 0;
	}
	if (liczbaQuercus != 0) {
		procentTrafienQuercus = (lTrafienQuercus * 100 / liczbaQuercus);
	}
	else {
		procentTrafienQuercus = 0;
	}

}


void Classifiers::NMClasiffier(std::vector<int> cechyDoKlasyf)
{
	std::map<std::string, int> objectCount;
	std::map<std::string, std::map<int, long double>> classAverages;
	
	for (int i = 0; i < trainObjects.size(); i++)
		objectCount[trainObjects[i].getClassName()] ++;

	
	for (int i = 0; i < trainObjects.size(); i++)
	{
		for (int xx = 0; xx < main->getNoFeatures(); xx++)
		{
			long double val = trainObjects[i].getFeatures()[xx] / objectCount[trainObjects[i].getClassName()];
			classAverages[trainObjects[i].getClassName()][xx] += val;
		}
	}

	boost::numeric::ublas::matrix<long double> AClassAvgMatrix(main->getNoFeatures(), 1);
	boost::numeric::ublas::matrix<long double> BClassAvgMatrix(main->getNoFeatures(), 1);
	fillMatrixWithAvarages(classAverages[classNames[0]], AClassAvgMatrix);
	fillMatrixWithAvarages(classAverages[classNames[1]], BClassAvgMatrix);


	for each (Object testOb in testObjects)
	{
		boost::numeric::ublas::matrix<long double> featureMatrix(main->getNoFeatures(), 1);
		fillFeatureMatrix(testOb, featureMatrix);

		boost::numeric::ublas::matrix<long double> Ua_X(main->getNoFeatures(), 1);
		boost::numeric::ublas::matrix<long double> Ub_X(main->getNoFeatures(), 1);
		Ua_X = AClassAvgMatrix - featureMatrix;
		Ub_X = BClassAvgMatrix - featureMatrix;
		long double LengthToA = calculateLengthOfMatrix(Ua_X);
		long double LengthToB = calculateLengthOfMatrix(Ub_X);

		if (LengthToA < LengthToB)
		{//A
			if (testOb.getClassName() == classNames[0])
			{
				APass++;
			}
			else
			{
				AFail++;
			}
		}
		else if (LengthToB < LengthToA)
		{//B
			if (testOb.getClassName() == classNames[1])
			{
				BPass++;
			}
			else
			{
				BFail++;
			}
		}
		else
		{//Draw
			Draw++;
		}

	}
}
