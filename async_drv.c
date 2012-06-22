#include <stdlib.h>
#include "erl_driver.h"
#include "ei.h"

#define PUT       'p'
#define PROCESS   'r'

typedef struct _async_drv_t {
	ErlDrvPort port;
} async_drv_t;

typedef struct _process_data_t {
	char command;
	char dbkey[256];
	char value[256];
} process_data_t;

static ErlDrvData start(ErlDrvPort port, char* cmd);
static void stop(ErlDrvData handle);
static void process(void *process_data);
static void process_async(ErlDrvData handle, ErlIOVec *ev);
static void ready_async(ErlDrvData drv_data, ErlDrvThreadData async_data);


static ErlDrvEntry async_driver_entry = {
    NULL,                             /* init */
    start,
    stop,
    NULL,                             /* output */
    NULL,                             /* ready_input */
    NULL,                             /* ready_output */
    "async_drv",                      /* the name of the driver */
    NULL,                             /* finish */
    NULL,                             /* handle */
    NULL,                             /* control */
    NULL,                             /* timeout */
    process_async,                    /* outputv */
    ready_async,                             /* ready_async */
    NULL,                             /* flush */
    NULL,                             /* call */
    NULL,                             /* event */
    ERL_DRV_EXTENDED_MARKER,          /* ERL_DRV_EXTENDED_MARKER */
    ERL_DRV_EXTENDED_MAJOR_VERSION,   /* ERL_DRV_EXTENDED_MAJOR_VERSION */
    ERL_DRV_EXTENDED_MINOR_VERSION,   /* ERL_DRV_EXTENDED_MINOR_VERSION */
    ERL_DRV_FLAG_USE_PORT_LOCKING,    /* ERL_DRV_FLAGs */
    NULL,
    NULL,
    NULL
};


static ErlDrvData start(ErlDrvPort port, char *cmd) {
	cmd = cmd;
	async_drv_t *async_drv = (async_drv_t*)driver_alloc(sizeof(async_drv_t));
	async_drv->port = port;

	return (ErlDrvData) async_drv;
}

static void stop(ErlDrvData handle) {
	async_drv_t *async_drv = (async_drv_t *)handle;

	driver_free(async_drv);
}

static void process(void *data) {
	process_data_t *process_data = data;

	switch(process_data->command) {
	case PUT: 
		printf("put\n");
		break;
	case PROCESS: // Reduces data
		printf ("process\n");
		break;
	}
}

static void process_async(ErlDrvData handle, ErlIOVec *ev) {
	async_drv_t* driver_data = (async_drv_t*) handle;
	process_data_t *process_data = malloc(sizeof(process_data_t));

	//ErlDrvBinary* data = ev->binv[1];
	SysIOVec *iov = &ev->iov[1];;
	const char *buf = &iov->iov_base[1];
	int index = 0;

	ei_decode_version(buf, &index, NULL);
	/* Our marshalling spec is that we are expecting a tuple {Command, Arg1, Arg2} */
	ei_decode_tuple_header(buf, &index, NULL);

	// command
	ei_decode_char(buf, &index, &process_data->command);
	// key
	ei_decode_string(buf, &index, process_data->dbkey);
	// value
	ei_decode_string(buf, &index, process_data->value);

	driver_async(driver_data->port, NULL, process, process_data, NULL);
}

static void ready_async(ErlDrvData handle, ErlDrvThreadData async_data) {
	ErlDrvTermData spec[] = {ERL_DRV_ATOM, driver_mk_atom((char *) "ok"),
							 ERL_DRV_TUPLE, 1};

	async_drv_t* driver_data = (async_drv_t*) handle;

	// TODO: free process_t from async_data
	driver_output_term(driver_data->port, spec, sizeof(spec) / sizeof(spec[0]));
}

DRIVER_INIT(async_driver) {
	return &async_driver_entry;
}
