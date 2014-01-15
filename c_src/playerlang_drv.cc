#include <stdio.h>
#include <string.h>
#include <new>
#include <ei.h>
#include <erl_driver.h>
#include <erl_interface.h>

static ErlDrvData start_playerlang(ErlDrvPort, char *);
static void stop_playerlang(ErlDrvData);

static ErlDrvEntry playerlang_entry = {
  NULL, /* init */
  start_playerlang, /* startup (defined below) */
  stop_playerlang, /* shutdown (defined below) */
  NULL, /* output */
  NULL, /* ready_input */
  NULL, /* ready_output */
  "playerlang_drv", /* the name of the driver */
  NULL, /* finish */
  NULL, /* handle */
  NULL, /* control */
  NULL, /* timeout */
  NULL, /* outputv */
  NULL, /* ready_async (defined below) */
  NULL, /* flush */
  NULL, /* call */
  NULL, /* event */
  ERL_DRV_EXTENDED_MARKER, /* ERL_DRV_EXTENDED_MARKER */
  ERL_DRV_EXTENDED_MAJOR_VERSION, /* ERL_DRV_EXTENDED_MAJOR_VERSION */
  ERL_DRV_EXTENDED_MINOR_VERSION, /* ERL_DRV_EXTENDED_MINOR_VERSION */
  ERL_DRV_FLAG_USE_PORT_LOCKING, /* ERL_DRV_FLAGs */
  NULL /* handle2 */,
  NULL /* process_exit */,
  NULL /* stop_select */
};

typedef struct {
  ErlDrvPort port;
} playerlang_data;

static ErlDrvData start_playerlang(ErlDrvPort port, char *command)
{
  playerlang_data *data = (playerlang_data*) driver_alloc(sizeof(playerlang_data));
  data->port = port;
  return (ErlDrvData) data;
}

static void stop_playerlang(ErlDrvData drv_data)
{
  driver_free((char*) drv_data);
}

extern "C" DRIVER_INIT(playerlang_drv)
{
  return &playerlang_entry;
}
