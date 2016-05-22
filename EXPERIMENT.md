checklist

options -> block size
107700 = 1024 records buffer
215600 = 2048 records buffer
431300 = 4096 records buffer
862700 = 8192 records buffer


6480 = 1024 records buffer at 1 byte value
15700 = 1024 records buffer at 10 byte value
107700 = 1024 records buffer at 100 byte value
1030000 = 1024 records buffer at 10000 byte value


MERKLE TREE TOGGLE

zero copy
MERKLE tree toggle in Enclave/zerocpy/ecall_entry.cpp

one copy
Enclave/merge_sort_1c.cpp

eextracopy
Enclave/merge_sort_eextrac.cpp


compaction method selection in Enclave/Enclave.cpp

NONCPY toggle in App/App.cpp

BUFFER size selection

zero copy
Enclave/zerocpy/options.cpp

one copy
App/untrusted/options.cpp
App/untrusted/leveldb_entry.cpp   MACRO

extra extra copy
App/untrusted/options.cpp






APP integration test
Adjust Enclave/Enclave.edl and Enclave/Libc.edl
Remove comments in Enclave.cpp





Record value size
Adjust Enclave/Enclave.edl

