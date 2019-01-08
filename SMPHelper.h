#pragma once
#include "list"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include"database.h"

class SMPHelper
{
public:
	SMPHelper();
	~SMPHelper();
	//Wygeneruj macierz srednich dla A i B
	boost::numeric::ublas::matrix<long double> GenerateAvarageMatrixForFeatures
	(std::vector<int> featureComb, std::map<int, long double> avarages, int objCount, int dimension);

	void GenerateXMatrixForFeatures
	(std::vector<int> featureComb, Database database, std::map<std::string, int> objectCount,
		std::vector<std::string> classNames, boost::numeric::ublas::matrix<long double>& MatrixXa, boost::numeric::ublas::matrix<long double>& MatrixXb);

	long double CalculateUa_UbAvaragesLength
	(std::vector<int> featureComb, std::map<int, long double> avaragesA, std::map<int, long double> avaragesB);
};

