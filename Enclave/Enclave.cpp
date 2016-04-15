#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include <string.h>
#include <sgx_cpuid.h>

#include "sgx_trts.h"
#include "Enclave.h"
#include "Enclave_t.h"  /* bar*/



/* 
 * printf: 
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void bar1(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_bar(buf);
}

/* ecall_foo:
 *   Uses malloc/free to allocate/free trusted memory.
 */
int ecall_foo(int i)
{
    void *ptr = malloc(100);
    assert(ptr != NULL);
    memset(ptr, 0x0, 100);
    free(ptr);

const char* pointer = "haha\n";
bar1(pointer);

    return i+1;
}

/* ecall_sgx_cpuid:
 *   Uses sgx_cpuid to get CPU features and types.
 */
void ecall_sgx_cpuid(int cpuinfo[4], int leaf)
{
    sgx_status_t ret = sgx_cpuid(cpuinfo, leaf);
    if (ret != SGX_SUCCESS)
        abort();
}