#include <iostream>
#include "CoqSet.h"
//#include "SetC.h"
#include <cassert>
#include <set>
#include <chrono>

#include <iostream>


std::set<int> buildLargeSet(int n) {
    std::set<int> s;
	srand((unsigned) time(NULL)); // seed rng

    for (int i=0; i<n; i++) {
        int random = rand();
        s.insert(random);
    }
    std::cout << "Built a set of size " << s.size() << "\n";
    return s;
}


certicoq::set buildLargeCoqSet(int n) {
    certicoq::set s;
	srand((unsigned) time(NULL)); // seed rng

    for (int i=0; i<n; i++) {
	    int random = rand();
        s.add(random);
        //std::cout << "added " << i << "\n";
    }
    std::cout << "Built a set of size " << s.size() << "\n";
    return s;
}


/*
void buildLargeCoqSet(int n) {
    auto s = certicoq::set<int>::makeLargeSet();
    std::cout << "Built a set of size " << s.size() << "\n";
}
*/

/*
void buildLargeCoqList(int n) {
    certicoq::list<int> ls;
    for (int i=0; i<n; i++) {
        ls.cons(i);
        //std::cout << "added " << i << "\n";
    }
}
*/

bool lookupRandCoq(int max, certicoq::set& X) {
	int random = rand() % max;
    //int random = 463;
    //std::cout << "Looking up " << random << "< " << max << "\n";
    assert (random < max);

    value v = X.getValue();
    bool b = X.isMember(random);
    return b;
}
/*
bool lookupRandC(int max, std::set<int>& X) {
	int random = rand() % max;
    //int random = 463;
    //std::cout << "Looking up " << random << "< " << max << "\n";
    assert (random < max);

    bool b = X.elem(random);
    return b;
}
*/
void lookupManyCoq(int m, certicoq::set& X) {
	srand((unsigned) time(NULL));
    //std::cout << "Starting " << m << " lookups\n";
    for (int i=0; i<m; i++) {
	    int random = rand() % (2*m);
        //std::cout << "(m=" << m << ", i=" << i << ") " 
        //          << random << " is a member of the set? ";
        bool b = X.isMember(random);
        //std::cout << b << "\n";
        //std::cout.flush();
    }
    //std::cout << "Done with lookups\n";
}

void lookupManyC(int m, std::set<int>& X) {
	srand((unsigned) time(NULL));
    //std::cout << "Starting lookups\n";
    for (int i=0; i<m; i++) {
	    int random = rand() % (2*m);
        auto it = X.find(random);
    }
    //std::cout << "Done with lookups\n";
}

template <typename Fun, typename... Args>
int timeit(Fun f, Args&... args) {
    auto start = std::chrono::high_resolution_clock::now();
    f(args...);
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>(stop - start);
    return duration.count();
}


void benchmarkCoqLookups(int n, int m) {
    certicoq::set s = buildLargeCoqSet(m);
    int dur = timeit(lookupManyCoq, n, s);
    std::cout << n << " |-> " << dur << "\n";
}



void benchmarkLookups(int n, int m) {
    //std::cout << "Building set of size " << m << "\n";
    /*std::set<int> s;
    for (int i=0; i<m; i++) {
        s.insert(i);
        //std::cout << "added " << i << "\n";
    }*/
    
    std::set<int> s = buildLargeSet(m);
    //std::cout << "Got set " << s <<"\n";
    std::cout << "Got set of size " << s.size() << "\n";
    int dur = timeit(lookupManyC, n, s);
    std::cout << n << " |-> " << dur << "\n";
    //std::cout << "Got set of size " << s.size() << "\n";
}


int foo() {
    certicoq::set Z;
    Z.add(5);
    Z.add(4);
    Z.add(200);
    Z.add(20000);
    return Z.size();
}
void test() {
    certicoq::set X;
    std::cout << "Created X\n";
    std::cout << "set has size: " << X.size() << "\n";

    certicoq::set Y;
    Y.add(-3);

    for (int i=0; i<1000; i++) {
        std::cout << "Adding " << i << "\n";
        X.add(i);
        std::cout << "set has size: " << X.size() << "\n";
    }

    for (int i=-100; i<20000; i++) {
        std::cout << "Checking membership of " << i << ": " << X.isMember(i) << "\n";
    }

    int sz;
    for (int i=0; i<10000; i++) {
        sz = foo();
        //std::cout << "Calling foo, which created a set of size " << foo() << "\n";
    }
    std::cout << "X has size " << X.size() << "\n";
    std::cout << "Y has size " << Y.size() << "\n";
}

int main() {
    
    
    certicoq::initialize_global_thread_info();
    //certicoq::set S = buildLargeCoqSet(1000);

    test();

    int n = 10000; // number of lookups
    int m = 1000; // size of the set
    for (int i=0; i<6; i++) { // prev 6, starting at 1000
        //int dur = timeit(buildLargeSet, n);
        //int dur = timeit(buildLargeCoqSet, n);
        //int dur = timeit(buildLargeCoqList, n);
        //std::cout << n << " |-> " << dur << "\n";

        benchmarkCoqLookups(n, m);
        //benchmarkLookups(n, m);
        n = n * 10;
        //m = m * 10;
    }
    
/*

    print_Coq_Init_Datatypes_bool(b);

    std::cout << "\n";
    return 0;
*/
}