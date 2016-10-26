######## SGX SDK Settings ########

SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= SIM
#SGX_MODE ?= HW
SGX_PRELEASE ?= 1

#turn on this bit to enable enclave verify
LSM_VERIFY ?= 0
#turn on this bit to enable profiling
PROFILE ?= 0

ifneq ($(SGX_DEBUG), 1) 
	SGX_PRERELEASE ?= 1
endif
SGX_ARCH ?= x64

ifeq ($(shell getconf LONG_BIT), 32)
	SGX_ARCH := x86
else ifeq ($(findstring -m32, $(CXXFLAGS)), -m32)
	SGX_ARCH := x86
endif

ifeq ($(SGX_ARCH), x86)
	SGX_COMMON_CFLAGS := -m32
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x86/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x86/sgx_edger8r
else
	SGX_COMMON_CFLAGS := -m64
	SGX_LIBRARY_PATH := $(SGX_SDK)/lib64
	SGX_ENCLAVE_SIGNER := $(SGX_SDK)/bin/x64/sgx_sign
	SGX_EDGER8R := $(SGX_SDK)/bin/x64/sgx_edger8r
endif

ifeq ($(SGX_DEBUG), 1)
ifeq ($(SGX_PRERELEASE), 1)
$(error Cannot set SGX_DEBUG and SGX_PRERELEASE at the same time!!)
endif
endif

ifeq ($(SGX_DEBUG), 1)
        SGX_COMMON_CFLAGS += -O0 -g
else
        SGX_COMMON_CFLAGS += -O0
endif


######## App Settings ########

ifneq ($(SGX_MODE), HW)
	Urts_Library_Name := sgx_urts_sim
else
	Urts_Library_Name := sgx_urts
endif

LEVELDB_FILES := App/untrusted/leveldb_entry.cpp \
                 App/untrusted/file_system.cpp \
                 App/untrusted/table.cpp \
                 App/untrusted/format.cpp \
                 App/untrusted/block.cpp \
                 App/untrusted/coding.cpp \
                 App/untrusted/iterator.cpp \
                 App/untrusted/two_level_iterator.cpp \
                 App/untrusted/filter_block.cpp \
                 App/untrusted/options.cpp \
                 App/untrusted/comparator.cpp \
                 App/untrusted/port_posix.cpp \
                 App/untrusted/merger.cpp \
                 App/untrusted/table_builder.cpp \
                 App/untrusted/block_builder.cpp \
                 App/untrusted/crc32c.cpp

LEVELDB_ZC_FILES := App/zerocpy/file_system.cpp \
                    App/zerocpy/my_merge.cpp

LEVELDB_INT_FILES := App/integration/builder.cpp \
                     App/integration/c.cpp \
                     App/integration/db_impl.cpp \
                     App/integration/db_iter.cpp \
                     App/integration/dbformat.cpp \
                     App/integration/dumpfile.cpp \
                     App/integration/filename.cpp \
                     App/integration/log_reader.cpp \
                     App/integration/log_writer.cpp \
                     App/integration/memtable.cpp \
                     App/integration/repair.cpp \
                     App/integration/table_cache.cpp \
                     App/integration/version_edit.cpp \
                     App/integration/version_set.cpp \
                     App/integration/write_batch.cpp \
                     App/integration/block.cpp \
                     App/integration/block_builder.cpp \
                     App/integration/filter_block.cpp \
                     App/integration/format.cpp \
                     App/integration/iterator.cpp \
                     App/integration/merger.cpp \
                     App/integration/table.cpp \
                     App/integration/table_builder.cpp \
                     App/integration/two_level_iterator.cpp \
                     App/integration/arena.cpp \
                     App/integration/bloom.cpp \
                     App/integration/cache.cpp \
                     App/integration/coding.cpp \
                     App/integration/comparator.cpp \
                     App/integration/crc32c.cpp \
                     App/integration/env.cpp \
                     App/integration/env_posix.cpp \
                     App/integration/filter_policy.cpp \
                     App/integration/hash.cpp \
                     App/integration/histogram.cpp \
                     App/integration/logging.cpp \
                     App/integration/options.cpp \
                     App/integration/status.cpp \
                     App/integration/port_posix.cpp \
                     App/integration/db_bench.cpp  \
                     App/integration/testutil.cpp 
#ifeq ($(LDB_INT), 1)
# 	App_Cpp_Files := App/App.cpp $(wildcard App/Edger8rSyntax/*.cpp) $(LEVELDB_FILES) $(LEVELDB_ZC_FILES)
#else
 	App_Cpp_Files := App/App_Int.cpp $(wildcard App/Edger8rSyntax/*.cpp) $(LEVELDB_INT_FILES)
#endif

App_Include_Paths := -IInclude -IApp -I$(SGX_SDK)/include

App_C_Flags := $(SGX_COMMON_CFLAGS) -fPIC -Wno-attributes $(App_Include_Paths)
App_C_Flags += -fno-builtin-memcmp -DOS_LINUX -DLEVELDB_PLATFORM_POSIX -DLEVELDB_ATOMIC_PRESENT
ifeq ($(LSM_VERIFY), 1)
App_C_Flags += -DVERIFY
endif
ifeq ($(PROFILE), 1)
App_C_Flags += -pg
endif

# Three configuration modes - Debug, prerelease, release
#   Debug - Macro DEBUG enabled.
#   Prerelease - Macro NDEBUG and EDEBUG enabled.
#   Release - Macro NDEBUG enabled.
ifeq ($(SGX_DEBUG), 1)
        App_C_Flags += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
        App_C_Flags += -DNDEBUG -DEDEBUG -UDEBUG
else
        App_C_Flags += -DNDEBUG -UEDEBUG -UDEBUG
endif

App_Cpp_Flags := $(App_C_Flags) -std=c++11
App_Link_Flags := $(SGX_COMMON_CFLAGS) -L$(SGX_LIBRARY_PATH) -l$(Urts_Library_Name) -lpthread 
ifeq ($(PROFILE), 1)
App_Link_Flags += -pg
endif

ifneq ($(SGX_MODE), HW)
	App_Link_Flags += -lsgx_uae_service_sim
else
	App_Link_Flags += -lsgx_uae_service
endif

App_Cpp_Objects := $(App_Cpp_Files:.cpp=.o)

App_Name := app

######## Enclave Settings ########

ifneq ($(SGX_MODE), HW)
	Trts_Library_Name := sgx_trts_sim
	Service_Library_Name := sgx_tservice_sim
	Crypto_Library_Name := sgx_tcrypto
else
	Trts_Library_Name := sgx_trts
	Service_Library_Name := sgx_tservice
	Crypto_Library_Name := sgx_tcrypto_opt
endif
Enclave_zc_Files := Enclave/zerocpy/ecall_entry.cpp \
                         Enclave/zerocpy/table.cpp \
                         Enclave/zerocpy/block.cpp \
                         Enclave/zerocpy/coding.cpp \
                         Enclave/zerocpy/format.cpp \
                         Enclave/zerocpy/iterator.cpp \
                         Enclave/zerocpy/two_level_iterator.cpp \
                         Enclave/zerocpy/merger.cpp \
                         Enclave/zerocpy/options.cpp \
                         Enclave/zerocpy/table_builder.cpp \
                         Enclave/zerocpy/block_builder.cpp \
                         Enclave/zerocpy/crc32c.cpp \
                         Enclave/zerocpy/sha3.cpp

#Enclave_Cpp_Files := Enclave/Enclave.cpp $(wildcard Enclave/Edger8rSyntax/*.cpp) Enclave/merge_sort2.cpp Enclave/merge_sort_eextrac.cpp Enclave/merge_sort_1c.cpp $(Enclave_zc_Files)
Enclave_Cpp_Files := Enclave/Enclave.cpp $(wildcard Enclave/Edger8rSyntax/*.cpp) Enclave/merge_sort_eextrac.cpp Enclave/merge_sort_1c.cpp Enclave/sha3.cpp Enclave/front_writer.cpp Enclave/sha1.cpp Enclave/cc.cpp
Enclave_Include_Paths := -IInclude -IEnclave -I$(SGX_SDK)/include -I$(SGX_SDK)/include/tlibc -I$(SGX_SDK)/include/stlport

Enclave_C_Flags := $(SGX_COMMON_CFLAGS) -nostdinc -fvisibility=hidden -fpie -fstack-protector $(Enclave_Include_Paths)
Enclave_Cpp_Flags := $(Enclave_C_Flags) -std=c++03 -nostdinc++
Enclave_Link_Flags := $(SGX_COMMON_CFLAGS) -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) \
	-Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tstdc -lsgx_tstdcxx -l$(Crypto_Library_Name) -l$(Service_Library_Name) -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0 \
	-Wl,--version-script=Enclave/Enclave.lds

Enclave_Cpp_Objects := $(Enclave_Cpp_Files:.cpp=.o)

Enclave_Name := enclave.so
Signed_Enclave_Name := enclave.signed.so
Enclave_Config_File := Enclave/Enclave.config.xml

ifeq ($(SGX_MODE), HW)
ifneq ($(SGX_DEBUG), 1)
ifneq ($(SGX_PRERELEASE), 1)
Build_Mode = HW_RELEASE
endif
endif
endif


.PHONY: all run

ifeq ($(Build_Mode), HW_RELEASE)
all: $(App_Name) $(Enclave_Name)
	@echo "The project has been built in release hardware mode."
	@echo "Please sign the $(Enclave_Name) first with your signing key before you run the $(App_Name) to launch and access the enclave."
	@echo "To sign the enclave use the command:"
	@echo "   $(SGX_ENCLAVE_SIGNER) sign -key <your key> -enclave $(Enclave_Name) -out <$(Signed_Enclave_Name)> -config $(Enclave_Config_File)"
	@echo "You can also sign the enclave using an external signing tool. See User's Guide for more details."
	@echo "To build the project in simulation mode set SGX_MODE=SIM. To build the project in prerelease mode set SGX_PRERELEASE=1 and SGX_MODE=HW."
else
all: $(App_Name) $(Signed_Enclave_Name)
endif

run: all
ifneq ($(Build_Mode), HW_RELEASE)
	@$(CURDIR)/$(App_Name)
	@echo "RUN  =>  $(App_Name) [$(SGX_MODE)|$(SGX_ARCH), OK]"
endif

######## App Objects ########

App/Enclave_u.c: $(SGX_EDGER8R) Enclave/Enclave.edl
	@cd App && $(SGX_EDGER8R) --untrusted ../Enclave/Enclave.edl --search-path ../Enclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

App/Enclave_u.o: App/Enclave_u.c
	@$(CC) $(App_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

App/%.o: App/%.cpp
	@$(CXX) $(App_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

$(App_Name): App/Enclave_u.o $(App_Cpp_Objects)
	@$(CXX) $^ -o $@ $(App_Link_Flags)
	@echo "LINK =>  $@"

######## Enclave Objects ########

Enclave/Enclave_t.c: $(SGX_EDGER8R) Enclave/Enclave.edl
	@cd Enclave && $(SGX_EDGER8R) --trusted ../Enclave/Enclave.edl --search-path ../Enclave --search-path $(SGX_SDK)/include
	@echo "GEN  =>  $@"

Enclave/Enclave_t.o: Enclave/Enclave_t.c
	@$(CC) $(Enclave_C_Flags) -c $< -o $@
	@echo "CC   <=  $<"

Enclave/%.o: Enclave/%.cpp
	@$(CXX) $(Enclave_Cpp_Flags) -c $< -o $@
	@echo "CXX  <=  $<"

$(Enclave_Name): Enclave/Enclave_t.o $(Enclave_Cpp_Objects)
	@$(CXX) $^ -o $@ $(Enclave_Link_Flags)
	@echo "LINK =>  $@"

$(Signed_Enclave_Name): $(Enclave_Name)
	@$(SGX_ENCLAVE_SIGNER) sign -key Enclave/Enclave_private.pem -enclave $(Enclave_Name) -out $@ -config $(Enclave_Config_File)
	@echo "SIGN =>  $@"

.PHONY: clean

clean:
	@rm -f $(App_Name) $(Enclave_Name) $(Signed_Enclave_Name) $(App_Cpp_Objects) App/Enclave_u.* $(Enclave_Cpp_Objects) Enclave/Enclave_t.*
