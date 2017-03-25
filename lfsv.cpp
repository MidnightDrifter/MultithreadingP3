#include "lfsv.h"

std::vector<int>* MemoryBank::Get()
{
	std::lock_guard<std::mutex> lock(m);
	std::vector<int>* p = slots[0];
	slots.pop_front();
	return p;
}
void MemoryBank::Store(std::vector<int>* p)
{
	std::lock_guard<std::mutex> lock(m);
	slots.push_back(p);
}

int const& LFSV::operator[](int pos) //const
{
	//return (*pdata.load()->first)[pos];



	Data* pdata_new = nullptr, *pdata_old;
	do
	{
		pdata_old = pdata;
		pdata_new = pdata_old;
		++pdata_new->second;

	} while (!((pdata).compare_exchange_weak(pdata_new, pdata_old)));
	int out = pdata_new->first->at(pos);

	do
	{
		pdata_old = pdata;
		pdata_new = pdata_old;
		--pdata_old->second;
	} while (!((pdata).compare_exchange_weak(pdata_new, pdata_old)));

	return out;
}





void LFSV::Insert(int const& v)
{
	//std::vector<int> *pdata_new = nullptr, *pdata_old;
	Data* pdata_new = nullptr, *pdata_old;
	do {
		++counter;

		//delete pdata_new;
		if (pdata_new) {
			//pdata_new->~vector(); //TODO causes mem errors sometimes 
			//mb.Store(pdata_new);

			pdata_new->first->~vector();
			mb.Store(pdata_new->first);
		}

		pdata_old = pdata;
	//	pdata_new = new (mb.Get()) std::vector<int>(*pdata_old);
		pdata_new = new(mb.Get()) Data(*pdata_old);

		//std::vector<int>::iterator b = pdata_new->begin();
		//std::vector<int>::iterator e = pdata_new->end();

		std::vector<int>::iterator b = pdata_new->first->begin();
		std::vector<int>::iterator e = pdata_new->first->end();
		//if (b == e || v >= pdata_new->back()) { pdata_new->push_back(v); } //first in empty or last element
		//else {
		//	for (; b != e; ++b) {
		//		if (*b >= v) {
		//			pdata_new->insert(b, v);
		//			break;
		//		}
		//	}
		//}

		if (b == e || v >= pdata_new->first->back()) { pdata_new->first->push_back(v); } //first in empty or last element
		else {
			for (; b != e; ++b) {
				if (*b >= v) {
					pdata_new->first->insert(b, v);
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

	pdata_old->first->~vector();
	mb.Store(pdata_old->first);


	//        std::lock_guard< std::mutex > write_lock( wr_mutex );
	//        std::vector<int> * pdata_current = pdata;
	//        std::vector<int>::iterator b = pdata_current->begin();
	//        std::vector<int>::iterator e = pdata_current->end();
	//        for ( ; b!=e; ++b ) {
	//            std::cout << *b << ' ';
	//        }
	//        std::cout << "Size " << pdata_current->size() << " after inserting " << v << std::endl;
}
