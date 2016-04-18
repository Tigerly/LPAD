ZeroCopyC
===

TODO
---

Enclave/Enclave.edl, to replace `void ocall_reload([out] int key_sizes[1024]` by:

```
    void ocall_eextrac_nextKey([out] int key_sizes[1],
        [out] int value_sizes[1],
        [out] char key[32],
        [out] char value[100],
        int fileIdx);
/*
    void ocall_extrac_reload(
        [out] int length[1],
        int fileIndex);
*/
    void ocall_1c_reload(
        [out] int length[1],
        int fileIndex);
 
```

ecall 
---

In Enclave/Enclave_t.c, replace `_in_input` by `_tmp_input`

```c

static sgx_status_t SGX_CDECL sgx_ecall_foo(void* pms)
{
	ms_ecall_foo_t* ms = SGX_CAST(ms_ecall_foo_t*, pms);
	sgx_status_t status = SGX_SUCCESS;
	int* _tmp_input1 = ms->ms_input1;
	size_t _len_input1 = 10 * sizeof(*_tmp_input1);
	int* _tmp_input2 = ms->ms_input2;
	size_t _len_input2 = 10 * sizeof(*_tmp_input2);
	int* _tmp_output = ms->ms_output;
	size_t _len_output = 20 * sizeof(*_tmp_output);

	CHECK_REF_POINTER(pms, sizeof(ms_ecall_foo_t));
	CHECK_UNIQUE_POINTER(_tmp_input1, _len_input1);
	CHECK_UNIQUE_POINTER(_tmp_input2, _len_input2);
	CHECK_UNIQUE_POINTER(_tmp_output, _len_output);
	ms->ms_retval = ecall_foo(_tmp_input1, _tmp_input2, _tmp_output);
err:
	return status;
}
```
ocall
---

Enclave/Enclave_t.c

```c
if (value) memcpy((void*)value, ms->ms_value, _len_value);
==>
value = ms->ms_value;
```
