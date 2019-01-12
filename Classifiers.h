#pragma once
#include "database.h"
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/operation.hpp>
#include <boost/qvm/mat_operations.hpp>
#include "mainwindow.h"

class Classifiers
{
public:
	Classifiers(MainWindow* main);
	~Classifiers();
	std::vector<std::string> classNames;
	void NMClasiffier();
	std::vector<Object> testObjects;
	std::vector<Object> trainObjects;
	void divideObjectsAsTrainAndTest(std::vector<Object> allObjects, std::vector<Object>& testObjects, std::vector<Object>& trainObjects, double trainPercent);

	MainWindow* main;

	int APass = 0;
	int AFail = 0;
	int BPass = 0;
	int BFail = 0;
	int Draw = 0;
	
};

