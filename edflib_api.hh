#pragma once

#ifdef _WIN32
#ifdef EDFLIB_EXPORTS
#define EDF_API __declspec(dllexport)
#else
#define EDF_API __declspec(dllimport)
#endif
#else
#define EDF_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void* EDFHANDLE;

struct SignalInfo_C {
  char label[17];
  char physical_dim[9];
  double physical_min;
  double physical_max;
  int digital_min;
  int digital_max;
  char prefiltering[81];
  unsigned int samples;
};

struct Header_C {
  char patient_id[81];
  char recording_id[81];
  char start_date[9];
  char start_time[9];
  unsigned int data_record_count;
  double record_duration;
  unsigned int signal_count;
  SignalInfo_C signals[4096];
};

EDF_API EDFHANDLE edf_open(const char* path);
EDF_API int edf_close(EDFHANDLE handle);
EDF_API int edf_read_header(EDFHANDLE handle, Header_C* header);
EDF_API int edf_read_signal_data(EDFHANDLE handle, char* const buf,
                                 unsigned int signal_index,
                                 unsigned int start_record = 0,
                                 unsigned int record_count = 0);

#ifdef __cplusplus
}
#endif