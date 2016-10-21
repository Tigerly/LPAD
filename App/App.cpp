#include <stdio.h>
#include <string.h>
#include <assert.h>

# include <unistd.h>
# include <pwd.h>
# define MAX_PATH FILENAME_MAX
# include <time.h>
# include <sys/time.h>
#include "sgx_urts.h"
#include "sgx_status.h"
#include "App.h"
#include "Enclave_u.h"

#define NONCPY 1
int su_prepare(int argc, char* argv[], int *r, long* user_arg);
void su_cleanup();
int su_prepare_zc(int argc, char* argv[], int *r, long* user_arg1, long *user_arg2);
void su_cleanup_zc();
void do_reload(int key_sizes[],int value_sizes[],
		char key[], char value[], int length[], int channel);
void do_flush(int key_sizes[], int value_sizes[], 
		char key[],char value[], int length);
void do_reload_eextrac(int key_size[],int value_size[],
		char key[], char value[], int valid[], int fileIdx);
void do_flush_eextrac(int key_size, int value_size, 
		char key[],char value[]);
void do_reload_1c(int fileIdx);
void do_flush_1c();
int do_read(uint64_t offset, size_t size, int length[1], int fileIdx, char space[100]);
int do_read_nospace(uint64_t offset, size_t size, int length[1], int fileIdx, int isIndex);
int do_file_flush();
int do_append(char space[100],size_t size);
int do_append_nospace(int block_type,size_t size);



int ecall_foo1(int file_count, long arg1, long arg2)
{
	sgx_status_t ret = SGX_ERROR_UNEXPECTED;
	int retval;
	ret = ecall_foo(global_eid, &retval, file_count, arg1, arg2);

	if (ret != SGX_SUCCESS)
		abort();

	int cpuid[4] = {0x1, 0x0, 0x0, 0x0};
	ret = ecall_sgx_cpuid(global_eid, cpuid, 0x0);
	if (ret != SGX_SUCCESS)
		abort();
	return retval;
}
/* Global EID shared by multiple threads */
sgx_enclave_id_t global_eid = 0;

typedef struct _sgx_errlist_t {
	sgx_status_t err;
	const char *msg;
	const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
	{
		SGX_ERROR_UNEXPECTED,
		"Unexpected error occurred.",
		NULL
	},
	{
		SGX_ERROR_INVALID_PARAMETER,
		"Invalid parameter.",
		NULL
	},
	{
		SGX_ERROR_OUT_OF_MEMORY,
		"Out of memory.",
		NULL
	},
	{
		SGX_ERROR_ENCLAVE_LOST,
		"Power transition occurred.",
		"Please refer to the sample \"PowerTransition\" for details."
	},
	{
		SGX_ERROR_INVALID_ENCLAVE,
		"Invalid enclave image.",
		NULL
	},
	{
		SGX_ERROR_INVALID_ENCLAVE_ID,
		"Invalid enclave identification.",
		NULL
	},
	{
		SGX_ERROR_INVALID_SIGNATURE,
		"Invalid enclave signature.",
		NULL
	},
	{
		SGX_ERROR_OUT_OF_EPC,
		"Out of EPC memory.",
		NULL
	},
	{
		SGX_ERROR_NO_DEVICE,
		"Invalid SGX device.",
		"Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
	},
	{
		SGX_ERROR_MEMORY_MAP_CONFLICT,
		"Memory map conflicted.",
		NULL
	},
	{
		SGX_ERROR_INVALID_METADATA,
		"Invalid enclave metadata.",
		NULL
	},
	{
		SGX_ERROR_DEVICE_BUSY,
		"SGX device was busy.",
		NULL
	},
	{
		SGX_ERROR_INVALID_VERSION,
		"Enclave version was invalid.",
		NULL
	},
	{
		SGX_ERROR_INVALID_ATTRIBUTE,
		"Enclave was not authorized.",
		NULL
	},
	{
		SGX_ERROR_ENCLAVE_FILE_ACCESS,
		"Can't open enclave file.",
		NULL
	},
};

/* Check error conditions for loading enclave */
void print_error_message(sgx_status_t ret)
{
	size_t idx = 0;
	size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

	for (idx = 0; idx < ttl; idx++) {
		if(ret == sgx_errlist[idx].err) {
			if(NULL != sgx_errlist[idx].sug)
				printf("Info: %s\n", sgx_errlist[idx].sug);
			printf("Error: %s\n", sgx_errlist[idx].msg);
			break;
		}
	}

	if (idx == ttl)
		printf("Error: Unexpected error occurred.\n");
}

/* Initialize the enclave:
 *   Step 1: retrive the launch token saved by last transaction
 *   Step 2: call sgx_create_enclave to initialize an enclave instance
 *   Step 3: save the launch token if it is updated
 */
int initialize_enclave(void)
{
	char token_path[MAX_PATH] = {'\0'};
	sgx_launch_token_t token = {0};
	sgx_status_t ret = SGX_ERROR_UNEXPECTED;
	int updated = 0;

	/* Step 1: retrive the launch token saved by last transaction */

	/* __GNUC__ */
	/* try to get the token saved in $HOME */
	const char *home_dir = getpwuid(getuid())->pw_dir;

	if (home_dir != NULL && 
			(strlen(home_dir)+strlen("/")+sizeof(TOKEN_FILENAME)+1) <= MAX_PATH) {
		/* compose the token path */
		strncpy(token_path, home_dir, strlen(home_dir));
		strncat(token_path, "/", strlen("/"));
		strncat(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME)+1);
	} else {
		/* if token path is too long or $HOME is NULL */
		strncpy(token_path, TOKEN_FILENAME, sizeof(TOKEN_FILENAME));
	}

	FILE *fp = fopen(token_path, "rb");
	if (fp == NULL && (fp = fopen(token_path, "wb")) == NULL) {
		printf("Warning: Failed to create/open the launch token file \"%s\".\n", token_path);
	}

	if (fp != NULL) {
		/* read the token from saved file */
		size_t read_num = fread(token, 1, sizeof(sgx_launch_token_t), fp);
		if (read_num != 0 && read_num != sizeof(sgx_launch_token_t)) {
			/* if token is invalid, clear the buffer */
			memset(&token, 0x0, sizeof(sgx_launch_token_t));
			printf("Warning: Invalid launch token read from \"%s\".\n", token_path);
		}
	}

	/* Step 2: call sgx_create_enclave to initialize an enclave instance */
	/* Debug Support: set 2nd parameter to 1 */
	ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, &token, &updated, &global_eid, NULL);
	if (ret != SGX_SUCCESS) {
		print_error_message(ret);
		if (fp != NULL) fclose(fp);
		return -1;
	}

	/* Step 3: save the launch token if it is updated */
	/* __GNUC__ */
	if (updated == FALSE || fp == NULL) {
		/* if the token is not updated, or file handler is invalid, do not perform saving */
		if (fp != NULL) fclose(fp);
		return 0;
	}

	/* reopen the file with write capablity */
	fp = freopen(token_path, "wb", fp);
	if (fp == NULL) return 0;
	size_t write_num = fwrite(token, 1, sizeof(sgx_launch_token_t), fp);
	if (write_num != sizeof(sgx_launch_token_t))
		printf("Warning: Failed to save launch token to \"%s\".\n", token_path);
	fclose(fp);
	return 0;
}

/* OCall functions */
void ocall_bar(const char *str)
{
	/* Proxy/Bridge will check the length and null-terminate 
	 * the input string to prevent buffer overflow. 
	 */
	printf("%s", str);
}

void ocall_flush(int key_sizes[],int value_sizes[], 
		char key[], char value[],int length){
	do_flush(key_sizes, value_sizes, key,value,length); 
}

void ocall_reload(int key_sizes[],int value_sizes[],
		char key[], char value[], int length[], int way){
	do_reload(key_sizes,value_sizes,key,value,length,way);
}

/* extra-extra-copy*/
void ocall_eextrac_flush(int key_size,int value_size, 
		char key[], char value[]){
	do_flush_eextrac(key_size, value_size, key,value); 
}

void ocall_eextrac_nextKey(int key_size[],int value_size[],
		char key[], char value[], int valid[], int fileIdx){
	do_reload_eextrac(key_size,value_size,key,value,valid,fileIdx);
}

/* 1-copy*/
void ocall_1c_flush(){
	do_flush_1c(); 
}

void ocall_1c_reload(int fileIdx){
	do_reload_1c(fileIdx);
}

/* no copy*/
int ocall_read(uint64_t offset, size_t size, int length[1], int fileIdx, char space[100]) {
	return  do_read(offset,size,length,fileIdx,space);
}

int ocall_read_nospace(uint64_t offset, size_t size, int length[1], int fileIdx, int isIndex) {
	return do_read_nospace(offset,size,length,fileIdx,isIndex);
}

int ocall_file_flush() {
	return do_file_flush();
}

int ocall_append(char space[100], size_t size) {
	return do_append(space,size);
}

int ocall_append_nospace(int block_type, size_t size) {
	return do_append_nospace(block_type,size);
}
/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
	int i = 3;
	/* Initialize the enclave */
	if(initialize_enclave() < 0){printf("Error enclave and exit\n");return -1;}

	/* Utilize edger8r attributes */
	edger8r_function_attributes();

	/* Utilize trusted libraries */
	int retval;
	int file_count = 0 ;
	long my_arg;

	su_prepare(argc, argv, &file_count,&my_arg);
	retval=ecall_foo1(file_count, my_arg,0);
	su_cleanup();

	/* Destroy the enclave */
	sgx_destroy_enclave(global_eid);
	printf("Info: SampleEnclave successfully returned.\n");
	return 0;
}


