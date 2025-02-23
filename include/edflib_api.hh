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

struct EDFHandle_C {
  char patient_id[80];
  char recording_id[80];
};

EDF_API EDFHANDLE edf_open(const char* path);
EDF_API int edf_close(EDFHANDLE handle);
EDF_API int edf_read_header(EDFHANDLE handle, EDFHandle_C* header);

#ifdef __cplusplus
}
#endif