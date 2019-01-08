#include "SMPHelper.h"

#include <algorithm>
#include <math.h> 


SMPHelper::SMPHelper()
{
}


SMPHelper::~SMPHelper()
{
}

boost::numeric::ublas::matrix<long double> SMPHelper::GenerateAvarageMatrixForFeatures
(std::vector<int> featureComb, std::map<int, long double> avarages, int objCount, int dimension)
{
	boost::numeric::ublas::matrix<long double> U(dimension, objCount);

	for (int i = 0; i < dimension; i++)
	{
		for (int j = 0; j < objCount; j++)
		{
			U(i, j) = avarages[featureComb[i]]; // zmiana nazwy featureComb
		}
	}

	return U;
}

void SMPHelper::GenerateXMatrixForFeatures
(std::vector<int> featureComb, Database database, std::map<std::string, int> objectCount, std::vector<std::string> classNames,
	boost::numeric::ublas::matrix<long double>& MatrixXa, boost::numeric::ublas::matrix<long double>& MatrixXb)
{
	//boost::numeric::ublas::matrix<long double> Xa(featureComb.size(), objectCount[classNames[0]]);
	//boost::numeric::ublas::matrix<long double> Xb(featureComb.size(), objectCount[classNames[1]]);
	//int countA = 0;
	//int countB = 0;
	//for (int i = 0; i < featureComb.size(); i++)
	//{
	//	for (int j = 0; j < database.getObjects().size; j++)
	//	{
	//		if (database.getObjects()[j].getClassName() == classNames[0])
	//		{
	//			Xa(i, countA) = database.getObjects()[j].getFeatures[featureComb[i]];
	//			countA++;
	//		}
	//		else if (database.getObjects()[j].getClassName() == classNames[1])
	//		{
	//			Xa(i, countB) = database.getObjects()[j].getFeatures[featureComb[i]];
	//			countB++;
	//		}
	//	}
	//}

	//MatrixXa = Xa;
	//MatrixXb = Xb;
}

//boost::numeric::ublas::matrix<long double> SMPHelper::GenerateXMatrixForFeatures
//			(std::vector<int> featureComb, Database database, int objCount, int dimension, std::string className)
//{
//	//Zmienic metode dla 2 klas jednoczesnie
//	boost::numeric::ublas::matrix<long double> X(dimension, objCount);
//	int counter = 0;
//	for (int i = 0; i < dimension; i++)
//	{
//		for (int j = 0; j < objCount;)
//		{
//			if (database.getObjects()[counter].getClassName() == className)
//			{
//				X(i, j) = database.getObjects()[counter].getFeatures()[featureComb[i]];
//				j++;
//			}
//			else
//			{
//				counter++;
//			}
//
//		}
//	}
//
//	return X;
//}

long double SMPHelper::CalculateUa_UbAvaragesLength(std::vector<int> featureComb, std::map<int, long double> avaragesA, std::map<int, long double> avaragesB)
{
	long double substractionUa_Ub = 0;
	for each (int combElement in featureComb)
	{
		substractionUa_Ub += (avaragesA[combElement] - avaragesB[combElement])*(avaragesA[combElement] - avaragesB[combElement]);
	}
	return sqrt(substractionUa_Ub);
}