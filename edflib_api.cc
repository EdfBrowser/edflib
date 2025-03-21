#include "pch.h"

using namespace edf;

EDFHANDLE edf_open(const char* path) {
  return reinterpret_cast<EDFHANDLE>(new File(path));
}

int edf_close(EDFHANDLE handle) {
  if (!handle) return -1;
  delete reinterpret_cast<File*>(handle);
  return 0;
}

int edf_read_header_info(EDFHANDLE handle, Header_C* header) {
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

#pragma warning(pop)

  return 0;
}

int edf_read_signal_info(EDFHANDLE handle, SignalInfo_C* signal) {
  if (!handle || !signal) return -1;
  auto* file = reinterpret_cast<File*>(handle);

  const auto& hdr = file->header();

#pragma warning(push)
#pragma warning(disable : 4996)

  for (int i = 0; i < hdr.signal_count; i++) {
    std::strcpy(signal[i].label, hdr.signals[i].label.c_str());
    std::strcpy(signal[i].physical_dim,
                hdr.signals[i].physical_dim.c_str());
    signal[i].physical_min = hdr.signals[i].physical_min;
    signal[i].physical_max = hdr.signals[i].physical_max;
    signal[i].digital_min = hdr.signals[i].digital_min;
    signal[i].digital_max = hdr.signals[i].digital_max;
    std::strcpy(signal[i].prefiltering,
                hdr.signals[i].prefiltering.c_str());
    signal[i].samples = hdr.signals[i].samples;

    signal[i].label[16] = 0;
    signal[i].physical_dim[8] = 0;
    signal[i].prefiltering[80] = 0;
  }

#pragma warning(pop)

  return 0;
}

int edf_read_signal_data(EDFHANDLE handle, char* const buf,
                         unsigned int signal_index, unsigned int start_record,
                         unsigned int record_count) {
  if (!handle) return -1;
  auto* file = reinterpret_cast<File*>(handle);

  return file->read_signal_data(buf, signal_index, start_record, record_count);
}
