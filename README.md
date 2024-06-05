# certicoq-set-library

The file `MSetImplementations.v` uses CertiCoq to compile certain operations from the MSet library backed by red-black trees. It has been tested with Coq 8.17 and CertiCoq compiled from the master branch of the [CertiCoq github](https://github.com/CertiCoq/certicoq).

The file `CoqSet.cpp` then wraps those operations into a nicely accessible C++ class, which can be called like any set library, as illustrated in the `main` function in `SetBenchmarks.cpp`.

To compile and run the library, edit the `PATH_TO_CERTICOQ` variable in `Makefile` and then call `make`. This will call Coq and compile the C and C++ files, and produce an executable named `SetBenchmarks`.