sim run
---

```
make app
# cc, link
make enclave.so
# cc Enclave code
# link with sdk lib
# sign
./app
```

real run
---

```
make SGX_MODE=HW SGX_PRERELEASE=1
#or 
# make SGX_MODE=HW
# /opt/intel/sgxsdk/bin/x64/sgx_sign sign -key Enclave/Enclave_private.pem -enclave enclave.so -out enclave.signed.so -config Enclave/Enclave.config.xml 

./app
```

# LPAD
LPAD implementation on LevelDB
