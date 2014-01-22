#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <new>
#include <ei.h>
#include <erl_driver.h>
#include <erl_interface.h>
#ifdef HAVE_AL_ALC_H
#  include <AL/alc.h>
#  include <AL/al.h>
#else
#  include <OpenAL/alc.h>
#  include <OpenAL/al.h>
#endif


#define OA_CHECK_ERRORS(msg) do {                           \
  ALenum _error;                                            \
  if ((_error = alGetError()) != AL_NO_ERROR)               \
  {                                                         \
    printf("OpenAL error: %u (%s)", _error, (msg));         \
  }                                                         \
} while(0)

#define CMD_PLAY 2

typedef struct {
  ErlDrvPort port;
} playerlang_data;

static ErlDrvData start_playerlang(ErlDrvPort, char *);
static void stop_playerlang(ErlDrvData);

static ErlDrvSSizeT control(ErlDrvData drv_data, unsigned int command, char *buf,
                       ErlDrvSizeT len, char **rbuf, ErlDrvSizeT rlen);
static int play(playerlang_data* driver_data);

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
  control, /* control */
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

static ErlDrvSSizeT control(ErlDrvData drv_data, unsigned int command, char *buf, ErlDrvSizeT len, char **rbuf, ErlDrvSizeT rlen) {
  playerlang_data* driver_data = (playerlang_data*) drv_data;
  switch (command) {
    case CMD_PLAY:
      play(driver_data);
      break;
  }
}

static inline ALenum formatForWav(short channels, short bits) {
  printf("Channels: %i, Bits: %i\n", channels, bits);
  if(bits == 16) {
    if (channels == 1)
      return AL_FORMAT_MONO16;
    return AL_FORMAT_STEREO16;
  }else if(bits == 8){
    if (channels == 1)
      return AL_FORMAT_MONO8;
    return AL_FORMAT_STEREO8;
  }else{
    if (channels == 1)
      return AL_FORMAT_MONO16;
    return AL_FORMAT_STEREO16;
  }
}

static void loadWAVfile(char *filename, ALuint buffer)
{
  char xbuffer[5];
  char smallbuffer[3];
  int size1;
  short channels;
  int samplerate;
  int byterate;
  short bitspersample;
  short extrasize;
  int datachunksize;
  float duration;
  unsigned char *bufferData;
  FILE *f = fopen(filename, "rb");
  if (!f)
  {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
      fprintf(stdout, "Current working dir: %s\n", cwd);

    printf("Couldn't open file %s\n",filename);
    return;
  }
  xbuffer[4] = '\0';
  smallbuffer[2]  ='\0';
  if ((fread(xbuffer, sizeof(char), 4, f) != 4) || strcmp(xbuffer, "RIFF") != 0)
  {
    printf("file: %s not a WAV file\n",filename);
    return;
  }
  fseek(f, 4, SEEK_CUR);
  if ((fread(xbuffer, sizeof(char), 4, f) != 4) || strcmp(xbuffer, "WAVE") != 0)
  {
    printf("file: %s not a WAV file\n",filename);
    return;
  }
  if ((fread(xbuffer, sizeof(char), 4, f) != 4) || strcmp(xbuffer, "fmt ") != 0)
  {
    printf("file: %s not a WAV file\n",filename);
    return;
  }
  fread(xbuffer, sizeof(char), 4, f);
  size1 = *(short*)&xbuffer[0];
  fseek(f, 2, SEEK_CUR);
  fread(smallbuffer, sizeof(char), 2, f);
  channels = *(short*)&smallbuffer[0];
  fread(xbuffer, sizeof(char), 4, f);
  samplerate = *(int*)&xbuffer[0];
  fread(xbuffer, sizeof(char), 4, f);
  byterate = *(int*)&xbuffer[0];
  fseek(f, 2, SEEK_CUR);
  fread(smallbuffer, sizeof(char), 2, f);
  bitspersample = *(short*)&smallbuffer[0];

  if (size1 != 16)
  {
    // Non PCM
    fread(smallbuffer, sizeof(char), 2, f);
    extrasize = *(short*)&smallbuffer[0];
    fseek(f, extrasize, SEEK_CUR);
  }

  if (fread(xbuffer, sizeof(char), 4, f)!=4 || strcmp(xbuffer, "data")!=0)
  {
    printf("Invalid WAV file: %s\n",filename);
    return;
  }
  fread(xbuffer, sizeof(char), 4, f);
  datachunksize = *(int*)&xbuffer[0];
  bufferData = (unsigned char*) malloc(sizeof(unsigned char)*datachunksize);
  fread(bufferData, sizeof(char), datachunksize, f);
  duration = (float)datachunksize / byterate;
  alBufferData(buffer, formatForWav(channels, bitspersample), bufferData, datachunksize, samplerate);
  fclose(f);
  free(bufferData);
}

static int play(playerlang_data *driver_data) {
  printf("\n\nPlay!!!\n\n");

  ALCdevice *device;

  device = alcOpenDevice(NULL);
  if (!device){
    printf("Error opening device\n");
  }
  printf("Device\n");

  ALCcontext *context;
  context = alcCreateContext(device, NULL);
  if (!alcMakeContextCurrent(context)) {
    printf("Error getting context\n");
  }
  printf("Context\n");

  ALuint buffer;
  alGenBuffers((ALuint)1, &buffer);
  printf("Buffer\n");
  OA_CHECK_ERRORS("generate buffers");

  ALuint source;
  alGenSources((ALuint)1, &source);
  printf("Source\n");
  OA_CHECK_ERRORS("generate buffers");
  OA_CHECK_ERRORS("gen sources");

  printf("Load wav\n");
  loadWAVfile("../thermo.wav", buffer);

  printf("Bind source to buffer\n");
  alSourcei(source, AL_BUFFER, buffer);

  printf("Play wav\n");
  alSourcePlay(source);
  for(;;){

  }

  printf("Clean up\n");
  context = alcGetCurrentContext();
  device = alcGetContextsDevice(context);
  alcMakeContextCurrent(NULL);
  alcDestroyContext(context);
  alcCloseDevice(device);

  return 0;
}


