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
	void NNClasiffier(std::vector<int> cechydoklasyfikacji);
	std::vector<Object> testObjects;
	std::vector<Object> trainObjects;
	void divideObjectsAsTrainAndTest(double trainPercent);

	Database* main;

	int APass = 0;
	int AFail = 0;
	int BPass = 0;
	int BFail = 0;
	int Draw = 0;

	int liczbaAcer = 0;
	int lTrafienAcer = 0;
	int lTrafienQuercus = 0;
	int liczbaQuercus = 0;
	int procentTrafienAcer = 0;
	int procentTrafienQuercus = 0;
	
};

