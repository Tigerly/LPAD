execution flow
----

```
 App.cpp  |---------------------------------------------------------------->+      |Enclave.cpp
          | Libc.cpp |                                                 | Libc.cpp  |            
          |          | edl      |                             | edl    |           |            
          |          |          | sgx_urts |sgx_trts/svc/crypt|        |           |            
          |          |          |          |                  |        |           |            
                                           |
       app/lib.so                          |              enclave.so     
                                           |
-------------------------------------------+--------------------------------------------------
main()    |          |          |          |                  |        |           |  
          |foo1   |          |          |                  |        |           |  
          |          |ec_foo |          |                  |        |           |  
          |          |          |sgx_ec(1,oc_t_E,ms)          |        |           |  
          |          |          |          |enc_entry{}       |        |           |
          |          |          |          |             do_ec|        |           |
          |          |          |          |                  |<g_ecall_table[1]>  |  
          |          |          |          |                  |sgx_ec_foo(ms)   |  
          |          |          |          |                  |        |ec_foo-----+
          |          |          |          |enc_entry{EEXIT}  |        |           |  |
ret       |ret       |ret       |ret       |                  |        |           |  |
-------------------------------------------+------------------------------------------v---ocall
          |          |          |          |                  |        |           |bar1 
          |          |          |          |                  |oc_bar  |           |
          |          |          |          |   sgx_ocloc()->ms|        |           |
          |          |          |          |      sgx_oc(0,ms)|        |           |
          |          |          |          |do_oc{EEXIT}      |        |           |
          |          |<oc_t_E[0]>          |                  |        |           |
          |          |E_oc_bar(ms)         |                  |        |           |
oc_bar    |          |          |          |                  |        |           |
```

* `ms` is a memory segment on host memory used for passing context for ocall/ecall

ecall/ocall control workflow
---

```
main()@App/App.cpp
  +-->initialize_enclave()@App/App.cpp
  |     +-->sgx_create_enclave()@libsgx_urts.so
  |
  +-->edger8r_function_attributes()@App/Edger8rSyntax/Functions.cpp
  |     +-->ecall_function_calling_convs(eid)@App/Enclave_u.c(Enclave/Enclave.edl)
  |       ...ecall...
  |
  |         @Enclave:sgx_ecall_function_calling_convs(pms)@Enclave/Enclave_t.c(Enclave/Edger8rSyntax/Functions.edl)
  |            +-->ecall_function_calling_convs()@Enclave/Edger8rSyntax/Functions.cpp
  |         
  +-->ecall_libc_functions()@App/TrustedLibrary/Libc.cpp,@App/App.h
        +-->ecall_malloc_free(eid)@App/Enclave_u.c(Libc.edl by ecall_malloc_free())
          +-->sgx_ecall(eid,27,...)@sgxsdk/lib64/libsgx_urts.so,include/sgx_edger8r.h
            ...g_ecall_table[27==>sgx_ecall_malloc_free]@Enclave/Enclave_t.c,Enclave.edl

              @enclave.so:sgx_ecall_malloc_free(pms)@Enclave/Enclave_t.c
                +-->ecall_malloc_free()@Enclave/TrustedLibrary/Libc.cpp
---------------------------------ocall----------------------------------------------------------
                  +-->malloc()@enclave.so,/opt/intel/sgxsdk/lib64/libsgx_trts.a
                  +-->printf(const char *,...)@Enclave/Enclave.cpp
                    +-->
                    ...ocall_print_string([in, string] const char *str)@Enclave/Enclave.edl

                    @app:ocall_print_string(const char *)@App/App.cpp
```

about ecall-lib `int ecall_malloc_free(int i)`:
---

* del: `public int ecall_malloc_free(int i);`
    * `Enclave/TrustedLibrary/Libc.edl,Enclave_u.h`
* def: `int ecall_malloc_free(int i)`
    * Enclave/TrustedLibrary/Libc.cpp
* inv: `ret = ecall_malloc_free(global_eid, &retval, i);`
    * App/TrustedLibrary/Libc.cpp

about ecall-app `int ecall_libc_functions(int i)`
---

* def: `void ecall_libc_functions(int i)`
    * ./App/TrustedLibrary/Libc.cpp:
* inv: `ecall_libc_functions(i);`
    * ./App/App.cpp:    
* del: `void ecall_libc_functions(int);`
    * ./App/App.h:

about ocall-app `printf`
---

* def: `void printf(const char *fmt, ...)`
    * ./Enclave/Enclave.cpp
* inv: `printf(pointer);`
    * ./Enclave/TrustedLibrary/Libc.cpp
* del: `void printf(const char *fmt, ...);`
    * ./Enclave/Enclave.h

about ocall-lib `ocall_print_string(buf)`
---

* def: `void ocall_print_string(const char *str)`
    * ./App/App.cpp
* inv: `ocall_print_string(buf);`
    * ./Enclave/Enclave.cpp
* del: `void ocall_print_string([in, string] const char *str);`
    * ./Enclave/Enclave.edl

? TEE: Proof of execution?
