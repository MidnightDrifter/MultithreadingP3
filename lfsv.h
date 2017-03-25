#pragma once
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>
#include <vector>
#include <deque>


typedef std::pair<std::vector<int>*, unsigned int> Data;



std::atomic<int> counter(0);

class MemoryBank {
	std::deque< std::vector<int>* > slots;
	std::mutex m;
public:
	MemoryBank() : slots(6000) {
		for (int i = 0; i<6000; ++i) {
			slots[i] = reinterpret_cast<std::vector<int>*>(new char[sizeof(std::vector<int>)]);
		}
	}
	std::vector<int>* Get();
	void Store(std::vector<int>* p);
	~MemoryBank() {
		for (auto & el : slots) { delete[] reinterpret_cast<char*>(el); }
	}
};


class LFSV
{
//public:
	//LFSV();
	//~LFSV();

	MemoryBank mb;
	std::atomic<Data*> pdata;
	std::mutex wr_mutex;
public:

	LFSV() : mb(), pdata(new (mb.Get()) Data), wr_mutex() {
		//std::cout << "Is lockfree " << pdata.is_lock_free() << std::endl;

		//Ask about this constructor bit, not sure if the vector is created by the above 'new' call or not
		pdata.load()->second = 1;
		//pdata.load()->first = new std::vector<int>();
	}

	~LFSV() {
		//delete pdata.load(); 
		--(pdata.load()->second);
		if (pdata.load()->second == 0)
		{
			pdata.load()->first->~vector();
			mb.Store(pdata.load()->first);   //Double check this too, not sure what the mb.Store( ~~~ ) does
		}
	}

	void Insert(int const & v);
	int const& operator[] (int pos); //const;
	//Requires, bare minimum: Insert method, [] operator
	//int operator[](int i) { return 0; }
	//void Insert(int i){}
};

