# readbin
A repository to test reading binary files for DUNE

Note: it requires that:

*  https://github.com/jmitrevs/daqdataformats/tree/fpga-usage
*  https://github.com/jmitrevs/detdataformats/tree/fpga-usage

be checked out at the same level. These branches comment out the throwing of exceptions, which is not supported in HLS, and certain C++17 features that nominally should be supported but do not seem to be.
