#pragma once
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <iostream>
#include <vector>
#include <deque>


//typedef std::pair<std::vector<int>*, unsigned int> Data;
struct Data
{
	std::vector<int>* data;
	int count;

	Data(std::vector<int>* d) : data(d), count(1) {}
	Data() : data(NULL), count(1) {}


	void decrementCounter() { --count; }
	void incrementCounter() { ++count; }
};


//std::atomic<int> counter(0);

class MemoryBank {
	std::deque< std::vector<int>* > slots;
	std::mutex m;
public:
	MemoryBank() : slots(60000) {
		for (int i = 0; i<60000; ++i) {
			slots[i] = reinterpret_cast<std::vector<int>*>(new char[sizeof(std::vector<int>)]);
		}
	}
	std::vector<int>* Get()
	{
		std::lock_guard<std::mutex> lock(m);
		std::vector<int>* p = slots[0];
		slots.pop_front();
		return p;
	}


	void Store(std::vector<int>* p)
	{
		std::lock_guard<std::mutex> lock(m);
		slots.push_back(p);
	}


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
	std::atomic<Data> pdata;
	std::mutex lockMutex;
	//Data* pdata;
	//std::mutex wr_mutex;
public:

	LFSV() : mb(),  pdata(new(mb.Get()) std::vector<int>), lockMutex() {
		//std::cout << "Is lockfree " << pdata.is_lock_free() << std::endl;

		//pdata = new Data();
		//pdata.load()->data = new(mb.Get()) std::vector<int>;

		//std::vector<int>* test = pdata.load().data;
	//	test->push_back(1);
	//	test->erase(test->begin());
	//	test = NULL;
		//test->erase(test->front());
		//std::cout << "Testing constructor, should get 1:  " << test->at(0) << std::endl;

		//*(pdata.load()->data) = std::vector<int>();


	

		
		//Ask about this constructor bit, not sure if the vector is created by the above 'new' call or not
		//pdata.load()->count = 1;
		//pdata.load()->data = new std::vector<int>();
	}

	~LFSV() {
		//delete pdata.load(); 
		//(pdata.load().count) = (pdata.load().count) - 1;
		pdata.load().decrementCounter();
		if (pdata.load().count <= 0)
		{
			pdata.load().data->~vector();
			mb.Store(pdata.load().data);   //Double check this too, not sure what the mb.Store( ~~~ ) does
			//delete pdata.load();
		}
	}

	void Insert(int const & v)
	{
		//std::vector<int> *pdata_new = nullptr, *pdata_old;
		Data  pdata_new,pdata_old;
		do {
			//	++counter;

			//delete pdata_new;
		//	if (pdata_new.data) {
				//pdata_new->~vector(); //TODO causes mem errors sometimes 
				//mb.Store(pdata_new);

		//		pdata_new.data->~vector();
		//		mb.Store(pdata_new.data);
		//	}
			//std::vector<int>* t = pdata.load().data;
			//pdata.incrementCounter();
			
			pdata_old = pdata;
			//	pdata_new = new (mb.Get()) std::vector<int>(*pdata_old);
			//++pdata_old.count;
			pdata_new.data = new(mb.Get()) std::vector<int>(*(pdata_old.data));
			//pdata_new.incrementCounter();
			//std::vector<int>::iterator b = pdata_new->begin();
			//std::vector<int>::iterator e = pdata_new->end();

			std::vector<int>::iterator b = pdata_new.data->begin();
			std::vector<int>::iterator e = pdata_new.data->end();
			//if (b == e || v >= pdata_new->back()) { pdata_new->push_back(v); } //data in empty or last element
			//else {
			//	for (; b != e; ++b) {
			//		if (*b >= v) {
			//			pdata_new->insert(b, v);
			//			break;
			//		}
			//	}
			//}

			if (b == e || v >= pdata_new.data->back()) { pdata_new.data->push_back(v); } //data in empty or last element
			else {
				for (; b != e; ++b) {
					if (*b >= v) {
						pdata_new.data->insert(b, v);
						break;
					}
				}
			}
			//            std::lock_guard< std::mutex > write_lock( wr_mutex );
			//            std::cout << "insert " << v << "(attempt " << counter << ")" << std::endl;
		} while (!(this->pdata).compare_exchange_weak(pdata_old, pdata_new));
		// if we use a simple "delete pdata_old" here, crash is almost guaranteed
		// the cause of the problem is ABA
		// using MemoryBank KIND OF solves it (for demo purposes only!)
		// it uses deque to store "std::vector<int>*", adds in the back, removes from front
		// this way there is some time before we get the same address back, so we home 
		// we will never see the same address again (ABA) in one call to Insert

		//pdata_old->~vector(); //TODO causes mem errors sometimes 
		//mb.Store(pdata_old);
		//std::vector<int>* temp = this->pdata.load().data;
		//std::cout << temp->at(0) << ", array size, " << temp->size() << std::endl;
		

	//	pdata_old.decrementCounter();
	//	if (pdata_old.count == 0)
	//	{
	//		pdata_old.data->~vector();
	//		mb.Store(pdata_old.data);
	//	}
//		pdata_old.decrementCounter();

			pdata_old.data->~vector();
			mb.Store(pdata_old.data);

	//	--pdata_old.count;
	//	if (pdata_old.count == 0)
	//	{
	//		pdata_old.data->~vector();
	//		mb.Store(pdata_old.data);
			//delete pdata_old;
	//	}



		//        std::lock_guard< std::mutex > write_lock( wr_mutex );
		//        std::vector<int> * pdata_current = pdata;
		//        std::vector<int>::iterator b = pdata_current->begin();
		//        std::vector<int>::iterator e = pdata_current->end();
		//        for ( ; b!=e; ++b ) {
		//            std::cout << *b << ' ';
		//        }
		//        std::cout << "Size " << pdata_current->size() << " after inserting " << v << std::endl;
	}
//
//


	//void Insert(int const& v)
	//{

	//	Data pdata_old, pdata_new;
	//	std::vector<int>* last = NULL;
	//	
	//	
	//	//Maybe move this into the loop?
	//
	//	pdata_old.data = pdata.load().data;
	//	if (last != pdata_old.data)
	//	{
	//		//delete pdata_new.data;
	//		pdata_new.data->~vector();
	//		mb.Store(pdata_new.data);
	//		pdata_new.data = new(mb.Get()) std::vector<int>(*pdata_old.data);
	//		last = pdata_old.data;

	//	}

	//	do
	//	{
	//		pdata_old.data = pdata.load().data;
	//		if (last != pdata_old.data)
	//		{
	//			//delete pdata_new.data;
	//			pdata_new.data->~vector();
	//			mb.Store(pdata_new.data);
	//			pdata_new.data = new(mb.Get()) std::vector<int>(*pdata_old.data);
	//			last = pdata_old.data;

	//		}


	//		            std::vector<int>::iterator b = pdata_new.data->begin();
	//		            std::vector<int>::iterator e = pdata_new.data->end();
	//		            if ( b==e || v>=pdata_new.data->back() ) { pdata_new.data->push_back( v ); } //first in empty or last element
	//		            else {
	//		                for ( ; b!=e; ++b ) {
	//		                    if ( *b >= v ) {
	//		                        pdata_new.data->insert( b, v );
	//		                        break;
	//		                    }
	//		                }
	//		            }
	//		//            std::lock_guard< std::mutex > write_lock( wr_mutex );
	//		//            std::cout << "insert " << v << "(attempt " << counter << ")" << std::endl;
	//		       

	//	} while (!(pdata.compare_exchange_weak(pdata_old, pdata_new)));

	//	pdata_old.data->~vector();
	//	mb.Store(pdata_old.data);


	//}



	int const& operator[] (int pos)
	{
		//return (*pdata.load()->data)[pos];



		Data pdata_new, pdata_old;
	//	std::vector<int>* temp = pdata.load().data;
		//std::cout << temp->at(0) << "v2 " << std::endl;
		//Test that pdata is still legitimate
	//	std::vector<int>* test = pdata.load()->data;
	//	int bob = test->at(0);

		pdata_new = pdata;
		++pdata_new.count;
		do
		{
			pdata_old = pdata;
			pdata_new = pdata_old;
			

		} while (!((pdata).compare_exchange_weak(pdata_new, pdata_old)));
		//int out = pdata_new->data->at(pos);
		
		--pdata_old.count;
		do
		{
			pdata_old = pdata;
			pdata_new = pdata_old;
			
		} while (!((pdata).compare_exchange_weak(pdata_new, pdata_old)));

		return pdata_new.data->at(pos);
	}
	////const;
	////Requires, bare minimum: Insert method, [] operator
	////int operator[](int i) { return 0; }
	////void Insert(int i){}



//
//
//
//void Insert(int const& v)
//{
//	Data old, fresh;
//	old.count = 1;
//	fresh.data = 0;
//	fresh.count = 1;
//	std::vector<int>* last = 0;
//
//
//	do
//	{
//		old.data = pdata.load().data;
//		if (last != old.data)
//		{
//			//delete fresh.data;
//
//
//			fresh.data = new(mb.Get()) std::vector<int>(*old.data);
//
//			//fresh.data->insert(v);
//
//			std::vector<int>::iterator b = fresh.data->begin();
//			std::vector<int>::iterator e = fresh.data->end();
//			if (b == e || v >= fresh.data->back()) { fresh.data->push_back(v); } //first in empty or last element
//			else {
//				for (; b != e; ++b) {
//					if (*b >= v) {
//						fresh.data->insert(b, v);
//						break;
//					}
//				}
//			}
//
//
//			last = old.data;
//
//			fresh.data->~vector();
//			mb.Store(fresh.data);
//		
//		}
//	} while (!(pdata).compare_exchange_weak(old, fresh));
//	//delete old.data;
//	old.data->~vector();
//	mb.Store(old.data);
//
//
//
//}
//
//int const& operator[](int pos)
//{
//	Data old, fresh;
//	do
//	{
//		old = pdata;
//		fresh = old;
//		fresh.count++;
//		
//	} while (!(pdata).compare_exchange_weak(old, fresh));
//
//	const int& out= fresh.data->at(pos);
//
//	do
//	{
//		old = pdata;
//		fresh = old;
//		fresh.count--;
//	} while (!(pdata).compare_exchange_weak(old, fresh));
//
//	return out;
//}



//
//
//
////Well, lock-full imiplementation seems to work without incurring the wrath of the timeouts?
//
//
//void Insert(const int& v)
//{
//	std::lock_guard<std::mutex> lock(lockMutex);
//				std::vector<int>::iterator b = pdata.load().data->begin();
//				std::vector<int>::iterator e = pdata.load().data->end();
//				if (b == e || v >= pdata.load().data->back()) { pdata.load().data->push_back(v); } //first in empty or last element
//				else {
//					for (; b != e; ++b) {
//						if (*b >= v) {
//							pdata.load().data->insert(b, v);
//							break;
//						}
//					}
//				}
//}
//
//
//const int& operator[](int pos)
//{
//	std::lock_guard<std::mutex> lock(lockMutex);
//	return pdata.load().data->at(pos);
//}

};

