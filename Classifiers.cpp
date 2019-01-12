#include "Classifiers.h"



Classifiers::Classifiers(MainWindow *main)
{
	this->main = main;
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

void Classifiers::divideObjectsAsTrainAndTest(std::vector<Object> allObjects, std::vector<Object>& testObjects, std::vector<Object>& trainObjects, double trainPercent)
{
	std::map<std::string, int> objectCount;
	for (int i = 0; i < allObjects.size(); i++)
		objectCount[allObjects[i].getClassName()] ++;
	int numberOfTrainA = objectCount[classNames[0]] * trainPercent;
	int numberOfTrainB = objectCount[classNames[1]] * trainPercent;

	for each (Object ob in allObjects)
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

void Classifiers::NMClasiffier()
{
	std::map<std::string, int> objectCount;
	std::map<std::string, std::map<int, long double>> classAverages;
	classNames = main->database.getClassNames();


	divideObjectsAsTrainAndTest(main->database.getObjects(), testObjects, trainObjects, 0.8);

	for (int i = 0; i < trainObjects.size(); i++)
		objectCount[trainObjects[i].getClassName()] ++;

	
	for (int i = 0; i < trainObjects.size(); i++)
	{
		for (int xx = 0; xx < main->database.getNoFeatures(); xx++)
		{
			long double val = trainObjects[i].getFeatures()[xx] / objectCount[trainObjects[i].getClassName()];
			classAverages[trainObjects[i].getClassName()][xx] += val;
		}
	}

	boost::numeric::ublas::matrix<long double> AClassAvgMatrix(main->database.getNoFeatures(), 1);
	boost::numeric::ublas::matrix<long double> BClassAvgMatrix(main->database.getNoFeatures(), 1);
	fillMatrixWithAvarages(classAverages[classNames[0]], AClassAvgMatrix);
	fillMatrixWithAvarages(classAverages[classNames[1]], BClassAvgMatrix);


	for each (Object testOb in testObjects)
	{
		boost::numeric::ublas::matrix<long double> featureMatrix(main->database.getNoFeatures(), 1);
		fillFeatureMatrix(testOb, featureMatrix);

		boost::numeric::ublas::matrix<long double> Ua_X(main->database.getNoFeatures(), 1);
		boost::numeric::ublas::matrix<long double> Ub_X(main->database.getNoFeatures(), 1);
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
