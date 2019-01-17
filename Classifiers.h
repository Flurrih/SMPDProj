#pragma once
#include "database.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include <boost/qvm/mat_operations.hpp>


class Classifiers
{
public:
	Classifiers(Database* main);
	~Classifiers();
	std::vector<std::string> classNames;
	void NMClasiffier(std::vector<int> cechydoklasyfikacji);
	void NMClasiffier(std::vector<int> cechyDoKlasyf, std::vector<Object> trainObjects, std::vector<Object> testObjects);
	void NNClasiffier(std::vector<int> cechydoklasyfikacji);
	void NNClasiffier(std::vector<int> cechyDoKlasyf, std::vector<Object> trainObjects, std::vector<Object> testObjects);
	void kNNClasiffier(std::vector<int> cechydoklasyfikacji, int k);
	void kNNClasiffier(std::vector<int> cechydoklasyfikacji, int k, std::vector<Object> trainObjects, std::vector<Object> testObjects);
	std::vector<Object> testObjects;
	std::vector<Object> trainObjects;
	void divideObjectsAsTrainAndTest(double trainPercent);

	Database* main;

	int APass = 0;
	int AFail = 0;
	int BPass = 0;
	int BFail = 0;
	int Draw = 0;

	int numberOfAcer = 0;
	int numberOfHitsAcer = 0;
	int numberOfHitsQuercus = 0;
	int numberOfQuercus = 0;
	float percentOfHitsAcer = 0;
	float percentOfHitsQuercus = 0;

	
	
};

