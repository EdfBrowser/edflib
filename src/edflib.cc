#include "edflib.hh"

#include <numeric>

using namespace edf;

std::vector<File::FieldDescriptor> File::header_descriptors_;
std::vector<File::SignalDescriptor> File::signal_descriptors_;

static str trim_edf_string(const char* data, ushort length) {
  str s(data, length);
  str::size_type end = s.find_last_not_of(' ');
  return end != str::npos ? s.substr(0, end + 1) : "";
}

template <class T>
static T parse_edf_num(const char* data, ushort length) {
  str s(data, length);
  s.erase(std::remove(s.begin(), s.end(), ' '), s.end());
  return s.empty() ? T(0) : static_cast<T>(std::stoll(s));
}

// TODO: 优化成函数指针
#define DEFINE_HEADER_STRING_PARSER(member, len) \
  [](const char* data, Header& h) { h.member = trim_edf_string(data, len); }

#define DEFINE_HEADER_NUMBER_PARSER(type, member, len) \
  [](const char* data, Header& h) { h.member = parse_edf_num<type>(data, len); }

#define DEFINE_SIGNAL_STRING_PARSER(member, len) \
  [](const char* data, SignalInfo& h) { h.member = trim_edf_string(data, len); }

#define DEFINE_SIGNAL_NUMBER_PARSER(type, member, len) \
  [](const char* data, SignalInfo& h) {                \
    h.member = parse_edf_num<type>(data, len);         \
  }

void File::register_fields() {
  header_descriptors_ = {
      // offset | length | parser
      {0, 8, DEFINE_HEADER_STRING_PARSER(Header::version, 8)},
      {8, 80, DEFINE_HEADER_STRING_PARSER(Header::patient_id, 80)},
      {88, 80, DEFINE_HEADER_STRING_PARSER(Header::recording_id, 80)},
      {168, 8, DEFINE_HEADER_STRING_PARSER(Header::start_date, 8)},
      {176, 8, DEFINE_HEADER_STRING_PARSER(Header::start_time, 8)},
      {184, 8, DEFINE_HEADER_NUMBER_PARSER(uint, Header::header_size, 8)},
      {192, 44, DEFINE_HEADER_STRING_PARSER(Header::reserved, 44)},
      {236, 8, DEFINE_HEADER_NUMBER_PARSER(uint, Header::data_record_count, 8)},
      {244, 8, DEFINE_HEADER_NUMBER_PARSER(uint, Header::record_duration, 8)},
      {252, 4, DEFINE_HEADER_NUMBER_PARSER(ushort, Header::signal_count, 4)},
  };

  signal_descriptors_ = {
      {0, 16, DEFINE_SIGNAL_STRING_PARSER(SignalInfo::label, 16)},
      {16, 80, DEFINE_SIGNAL_STRING_PARSER(SignalInfo::transducer, 80)},
      {96, 8, DEFINE_SIGNAL_STRING_PARSER(SignalInfo::physical_dim, 8)},
      {104, 8,
       DEFINE_SIGNAL_NUMBER_PARSER(double, SignalInfo::physical_min, 8)},
      {112, 8,
       DEFINE_SIGNAL_NUMBER_PARSER(double, SignalInfo::physical_max, 8)},
      {120, 8, DEFINE_SIGNAL_NUMBER_PARSER(int, SignalInfo::digital_min, 8)},
      {128, 8, DEFINE_SIGNAL_NUMBER_PARSER(int, SignalInfo::digital_max, 8)},
      {136, 80, DEFINE_SIGNAL_STRING_PARSER(SignalInfo::prefiltering, 80)},
      {216, 8, DEFINE_SIGNAL_NUMBER_PARSER(uint, SignalInfo::samples, 8)},
      {224, 32, DEFINE_SIGNAL_STRING_PARSER(SignalInfo::reserved, 32)},
  };
}

File::File(const str& path) : file_(path, std::ios::binary) {
  if (!file_) throw std::runtime_error("Failed to open file");
  parse_header();
}

File::~File() {
  if (file_.is_open()) {
    file_.close();
  }
}

void File::parse_header() {
  file_.seekg(0);

  char buf[256];
  file_.read(buf, 256);

  static bool initialized = []() {
    register_fields();
    return true;
  }();

  for (const FieldDescriptor& desc : header_descriptors_) {
    desc.parser(buf + desc.offset, header_);
  }

  parse_signal_headers();
}

void File::parse_signal_headers() {
  file_.seekg(256);

  const ushort ns = header_.signal_count;
  header_.signals.resize(ns);

  char* buf = new char[256 * ns];
  file_.read(buf, 256 * ns);

  for (const SignalDescriptor& desc : signal_descriptors_) {
    const char* buf_offset = buf + desc.offset * ns;
    for (int i = 0; i < ns; i++) {
      desc.parser(buf_offset + i * desc.length, header_.signals[i]);
    }
  }

  delete buf;
}

int File::read_signal_data(char* const buf, uint signal_index,
                           uint start_record, uint record_count) {
  ushort ns = header_.signal_count;

  if (signal_index >= ns) return -1;
  if (buf == nullptr) return -1;

  uint total_records = header_.data_record_count;
  // 参数校验
  start_record = std::min(start_record, total_records);
  if (record_count == 0U || start_record + record_count > total_records)
    record_count = total_records - start_record;

  //   // 计算偏移量
  uint record_size =
      std::accumulate(header_.signals.begin(), header_.signals.end(), 0U,
                      [](uint sum, const SignalInfo& sig) {
                        return sum + sig.samples * sizeof(short);
                      });

  // 定位到起始记录
  uint start_pos = 256U + 256U * ns + (start_record * record_size);

  // 计算信号在记录中内的偏移
  for (int i = 0; i < signal_index; i++) {
    start_pos += header_.signals[i].samples * sizeof(short);
  }

  file_.seekg(start_pos);

  //   跳过不需要读的字节
  const uint bytes_per_record =
      header_.signals[signal_index].samples * sizeof(short);
  const uint stride = record_size - bytes_per_record;

  for (int rec = 0; rec < record_count; rec++) {
    file_.read(buf + rec * header_.signals[signal_index].samples,
               bytes_per_record);

    file_.seekg(stride, std::ios::cur);
  }

  return 0;
}