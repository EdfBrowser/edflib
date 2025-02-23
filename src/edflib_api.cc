#include "edflib_api.hh"

#include "edflib.hh"

using namespace edf;

EDFHANDLE edf_open(const char* path) {
  // printf("%s", "Open!!");
  return reinterpret_cast<EDFHANDLE>(new File(path));
}

int edf_close(EDFHANDLE handle) {
  if (!handle) return -1;
  delete reinterpret_cast<File*>(handle);
  return 0;
}

int edf_read_header(EDFHANDLE handle, EDFHandle_C* header) {
  if (!handle || !header) return -1;
  // printf("%s", "Read!");
  auto* file = reinterpret_cast<File*>(handle);

  const auto& hdr = file->header();
  //   printf("%s\n", hdr.patient_id.c_str());
  //   printf("%s\n", hdr.recording_id.c_str());

  std::strncpy(header->patient_id, hdr.patient_id.c_str(), 80);
  std::strncpy(header->recording_id, hdr.recording_id.c_str(), 80);

  return 0;
}