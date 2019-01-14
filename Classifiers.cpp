#include "Classifiers.h"



Classifiers::Classifiers(Database *main)
{
	this->main = main;
	classNames = main->getClassNames();
}


Classifiers::~Classifiers()
{
}

void fillMatrixWithAvarages(std::map<int, long double> classAvg, boost::numeric::ublas::matrix<long double> &matrix, std::vector<int> features)
{
	for (int i = 0; i < matrix.size1(); i++)
	{
		matrix(i, 0) = classAvg[features[i]];
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

void fillFeatureMatrix(Object obj, boost::numeric::ublas::matrix<long double> &matrix, std::vector<int> features)
{
	for (int ft = 0; ft < matrix.size1(); ft++)
	{
		matrix(ft, 0) = obj.getFeatures()[features[ft]];
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
	int idNearestNeighbor = 0;
	numberOfAcer = 0;
	numberOfHitsAcer = 0;
	numberOfHitsQuercus = 0;
	numberOfQuercus = 0;
	percentOfHitsAcer = 0;
	percentOfHitsQuercus = 0;
	


	for (int i = 0; i < testObjects.size(); i++) {
		double minDistance = 99999;

		for (int j = 0; j < trainObjects.size(); j++) {
			double distance = 0;

			for (int k = 0; k < cechyDoKlasyf.size(); k++) {
				distance += pow(trainObjects[j].getFeatures()[cechyDoKlasyf[k]] - testObjects[i].getFeatures()[cechyDoKlasyf[k]], 2);
			}

			distance = sqrt(distance);

			if (distance < minDistance) {

				minDistance = distance;
				idNearestNeighbor = j;
			}
		}

		if (testObjects[i].getClassName().compare("Acer") == 0) {
			numberOfAcer++;
			if (trainObjects[idNearestNeighbor].getClassName().compare("Acer") == 0) {
				numberOfHitsAcer++;
			}
		}
		else if (testObjects[i].getClassName().compare("Quercus") == 0) {
			numberOfQuercus++;
			if (trainObjects[idNearestNeighbor].getClassName().compare("Quercus") == 0) {
				numberOfHitsQuercus++;

			}
		}
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


void Classifiers::NMClasiffier(std::vector<int> cechyDoKlasyf)
{
	std::map<std::string, int> objectCount;
	std::map<std::string, std::map<int, long double>> classAverages;
	
	for (int i = 0; i < trainObjects.size(); i++)
		objectCount[trainObjects[i].getClassName()] ++;

	
	for (int i = 0; i < trainObjects.size(); i++)
	{
		//for (int xx = 0; xx < main->getNoFeatures(); xx++)
		for (size_t xx = 0; xx < cechyDoKlasyf.size(); xx++)
		{
			long double val = trainObjects[i].getFeatures()[cechyDoKlasyf[xx]] / objectCount[trainObjects[i].getClassName()];
			classAverages[trainObjects[i].getClassName()][cechyDoKlasyf[xx]] += val;
		}
	}

	boost::numeric::ublas::matrix<long double> AClassAvgMatrix(cechyDoKlasyf.size(), 1);
	boost::numeric::ublas::matrix<long double> BClassAvgMatrix(cechyDoKlasyf.size(), 1);
	fillMatrixWithAvarages(classAverages[classNames[0]], AClassAvgMatrix, cechyDoKlasyf);
	fillMatrixWithAvarages(classAverages[classNames[1]], BClassAvgMatrix, cechyDoKlasyf);


	for each (Object testOb in testObjects)
	{
		boost::numeric::ublas::matrix<long double> featureMatrix(cechyDoKlasyf.size(), 1);
		fillFeatureMatrix(testOb, featureMatrix, cechyDoKlasyf);

		boost::numeric::ublas::matrix<long double> Ua_X(cechyDoKlasyf.size(), 1);
		boost::numeric::ublas::matrix<long double> Ub_X(cechyDoKlasyf.size(), 1);
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
