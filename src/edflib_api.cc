
#include "edflib_api.hh"

#include "edflib.hh"

using namespace edf;

EDFHANDLE edf_open(const char* path) {
  return reinterpret_cast<EDFHANDLE>(new File(path));
}

int edf_close(EDFHANDLE handle) {
  if (!handle) return -1;
  delete reinterpret_cast<File*>(handle);
  return 0;
}

int edf_read_header(EDFHANDLE handle, Header_C* header) {
  if (!handle || !header) return -1;
  auto* file = reinterpret_cast<File*>(handle);

  const auto& hdr = file->header();

#pragma warning(push)
#pragma warning(disable : 4996)

  // strcpy函数 会自动在末尾添加'\0'，strncpy函数则不会
  std::strcpy(header->patient_id, hdr.patient_id.c_str());
  std::strcpy(header->recording_id, hdr.recording_id.c_str());
  std::strcpy(header->start_date, hdr.start_date.c_str());
  std::strcpy(header->start_time, hdr.start_time.c_str());

  header->patient_id[80] = 0;
  header->recording_id[80] = 0;
  header->start_date[8] = 0;
  header->start_time[8] = 0;

  header->data_record_count = hdr.data_record_count;
  header->record_duration = hdr.record_duration;
  header->signal_count = hdr.signal_count;

  for (int i = 0; i < hdr.signal_count; i++) {
    std::strcpy(header->signals[i].label, hdr.signals[i].label.c_str());
    std::strcpy(header->signals[i].physical_dim,
                hdr.signals[i].physical_dim.c_str());
    header->signals[i].physical_min = hdr.signals[i].physical_min;
    header->signals[i].physical_max = hdr.signals[i].physical_max;
    header->signals[i].digital_min = hdr.signals[i].digital_min;
    header->signals[i].digital_max = hdr.signals[i].digital_max;
    std::strcpy(header->signals[i].prefiltering,
                hdr.signals[i].prefiltering.c_str());
    header->signals[i].samples = hdr.signals[i].samples;

    header->signals[i].label[16] = 0;
    header->signals[i].physical_dim[8] = 0;
    header->signals[i].prefiltering[80] = 0;
  }

#pragma warning(pop)

  return 0;
}