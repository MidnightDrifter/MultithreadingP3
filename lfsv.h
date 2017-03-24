#pragma once
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>
#include <vector>
class LFSV
{
public:
	LFSV();
	~LFSV();

	//Requires, bare minimum: Insert method, [] operator
	int operator[](int i) { return 0; }
	void Insert(int i){}
};

