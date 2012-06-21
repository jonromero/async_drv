#include <stdlib.h>
#include "erl_driver.h"
#include "ei.h"

#define PUT       'p'
#define PROCESS   'r'

typedef struct _async_drv_t {
	ErlDrvPort port;
} async_drv_t;

static ErlDrvData start(ErlDrvPort port, char* cmd);
static void stop(ErlDrvData handle);
static void process(ErlDrvData handle, ErlIOVec *ev);

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
    process,                          /* outputv */
    NULL,                             /* ready_async */
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

static void process(ErlDrvData handle, ErlIOVec *ev) {
	char command;
	char dbkey[256];
	char value[256];
	async_drv_t* driver_data = (async_drv_t*) handle;
	//ErlDrvBinary* data = ev->binv[1];
	SysIOVec *iov = &ev->iov[1];;
	const char *buf = &iov->iov_base[1];
	int index = 0;
	char result[1024] = "\0";

	ei_decode_version(buf, &index, NULL);
	/* Our marshalling spec is that we are expecting a tuple {Command, Arg1, Arg2} */
	ei_decode_tuple_header(buf, &index, NULL);

	// command
	ei_decode_char(buf, &index, &command);
	// key
	ei_decode_string(buf, &index, dbkey);
	// value
	ei_decode_string(buf, &index, value);

	//printf("dbkey %s\n", dbkey);
	//printf("value %s\n", value);
	//printf("Command %c\n", command);

	switch(command) {
	case PUT: // puts data in calcMemory
		printf ("put\n");
		//put_data(driver_data->lethe, dbkey, value);
		return;
	case PROCESS: // Reduces data
		printf ("process\n");
		break;
	}

	ErlDrvTermData spec[] = {ERL_DRV_ATOM, driver_mk_atom((char *) "ok"),
							 ERL_DRV_STRING, (ErlDrvTermData)result, strlen(result),
							 ERL_DRV_TUPLE, 2};

	driver_output_term(driver_data->port, spec, sizeof(spec) / sizeof(spec[0]));
}

DRIVER_INIT(async_driver) {
	return &async_driver_entry;
}
