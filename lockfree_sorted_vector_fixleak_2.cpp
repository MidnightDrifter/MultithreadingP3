#include <iostream>       // std::cout
#include <atomic>         // std::atomic
#include <thread>         // std::thread
#include <vector>         // std::vector
#include <mutex>          // std::mutex


#include <deque>         // std::deque
#include <chrono>
// accepts pointers to vector<int> and waits for 250 ms to delete them
// ctor starts thread from private WatchingThread
// dtor tells the thread to stop and waits to join
class GarbageRemover {
    std::deque<
        std::pair<
            std::vector<int>*,
            std::chrono::time_point<std::chrono::system_clock>
                 >
             > to_be_deleted;
    std::mutex m;
    std::atomic<bool> stop;
    std::thread worker;
    // run this method once per application
    void WatchingThread() {
        while( !stop ) {
            std::this_thread::sleep_for( std::chrono::milliseconds( 20 ) );
            std::lock_guard<std::mutex> lock( m );
            if ( !to_be_deleted.empty() ) {
                //peek
                std::chrono::duration<double> how_old = std::chrono::system_clock::now() - to_be_deleted[0].second;
                if ( how_old > std::chrono::duration<int, std::milli>(250) ) {
                    std::vector<int>* p = to_be_deleted[0].first;
                    to_be_deleted.pop_front();
                    // delete p;
                    delete [] reinterpret_cast<char*>( p );
                }
            }
        }
        // free the rest 
        for (auto& pt : to_be_deleted) { 
            // delete pt.first;
            delete [] reinterpret_cast<char*>( pt.first );
        }
    }
    public:
        GarbageRemover() : to_be_deleted(), m(), stop(false) {
            worker = std::thread( &GarbageRemover::WatchingThread, this );
        }
        ~GarbageRemover() { worker.join(); }
        // method may be called concurrently
        void Add( std::vector<int>* p ) {
            std::lock_guard<std::mutex> lock( m );
            to_be_deleted.push_back( std::make_pair( p, std::chrono::system_clock::now() ) );
        }
        void Stop() { stop = true; }
};

std::atomic<int> counter(0);

class LFSV {
    std::atomic< std::vector<int>* > pdata;
    GarbageRemover gr;
    public:

    LFSV() : pdata( new std::vector<int> ), gr() {}   

    ~LFSV() { delete pdata.load(); gr.Stop();  }

    void Insert( int const & v ) {
        std::vector<int> *pdata_new = nullptr, *pdata_old;
        do {
            ++counter;
            delete pdata_new;
            pdata_old = pdata;
            pdata_new = new std::vector<int>( *pdata_old );

            std::vector<int>::iterator b = pdata_new->begin();
            std::vector<int>::iterator e = pdata_new->end();
            if ( b==e || v>=pdata_new->back() ) { pdata_new->push_back( v ); } //first in empty or last element
            else {
                for ( ; b!=e; ++b ) {
                    if ( *b >= v ) {
                        pdata_new->insert( b, v );
                        break;
                    }
                }
            }
        } while ( !(this->pdata).compare_exchange_weak( pdata_old, pdata_new  ));
        //fixing memory leaks
        pdata_old->~vector();
        gr.Add( pdata_old );
    }

    int const& operator[] ( int pos ) const {
        return (*pdata)[ pos ];
    }
};

LFSV lfsv;

#include <algorithm>//copy, random_shuffle
#include <ctime>    //std::time (NULL) to seed srand
void insert_range( int b, int e ) {
    int * range = new int [e-b];
    for ( int i=b; i<e; ++i ) {
        range[i-b] = i;
    }
    std::srand( static_cast<unsigned int>(std::time (NULL)) );
    std::random_shuffle( range, range+e-b );
    for ( int i=0; i<e-b; ++i ) {
        lfsv.Insert( range[i] );
    }
    delete [] range;
}

int read_position_0( int how_many_times ) {
    int j = 0;
    for ( int i=0; i<how_many_times; ++i ) {
        if ( lfsv[0] != -1 ) {
            std::cout << "not -1 on iteration " << i << "\n"; // see main - all element are non-negative, so index 0 should always be -1
        }
    }
    return j;
}

// ABA is "solved" by delaying delete by 250ms
// but writer may still delete while reader is reading - uncomment line threads.push_back( std::thread( read_position_0, 1000000000 ) ); below
int main ()
{
    std::vector<std::thread> threads;

    lfsv.Insert( -1 );
    // threads.push_back( std::thread( read_position_0, 100000 ) );
    
    int num_threads = 2;
    int num_per_thread = 10;
    for (int i=0; i<num_threads; ++i) {
        threads.push_back( std::thread( insert_range, i*num_per_thread, (i+1)*num_per_thread ) );
    }
    for (auto& th : threads) th.join();

    for (int i=0; i<num_threads*num_per_thread; ++i) { 
        std::cout << lfsv[i] << ' '; 
        if ( lfsv[i] != i-1 ) {
            std::cout << "Error\n";
            return 1;
        }
    }
    std::cout << "All good\n";
    std::cout << "Counter = " << counter << std::endl;
    std::cout << "Repeats = " << counter-num_threads*num_per_thread << std::endl;

    return 0;
}
