#include "edflib.hh"

using namespace edf;

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

#define DEFINE_STRING_PARSER(member, len) \
  [](const char* data, Header& h) { h.member = trim_edf_string(data, len); }

#define DEFINE_NUMBER_PARSER(type, member, len) \
  [](const char* data, Header& h) { h.member = parse_edf_num<type>(data, len); }

void File::register_header_fields() {
  static bool initialized = false;
  if (initialized) return;

  header_descriptors_ = {
      // offset | length | parser
      {0, 8, DEFINE_STRING_PARSER(Header::version, 8)},
      {8, 80, DEFINE_STRING_PARSER(Header::patient_id, 80)},
      {88, 80, DEFINE_STRING_PARSER(Header::recording_id, 80)},
      {168, 8, DEFINE_STRING_PARSER(Header::start_date, 8)},
      {176, 8, DEFINE_STRING_PARSER(Header::start_time, 8)},
      {184, 8, DEFINE_NUMBER_PARSER(uint, Header::header_size, 8)},
      {192, 44, DEFINE_STRING_PARSER(Header::reserved, 44)},
      {236, 8, DEFINE_NUMBER_PARSER(uint, Header::data_record_count, 8)},
      {244, 8, DEFINE_NUMBER_PARSER(uint, Header::record_duration, 8)},
      {252, 4, DEFINE_NUMBER_PARSER(ushort, Header::signal_count, 4)},
  };

  initialized = true;
}

File::File(const str& path) : file_(path, std::ios::binary) {
  if (!file_) throw std::runtime_error("Failed to open file");
  parse_header();
}

File::~File() {
  if (file_.is_open()) file_.close();
}

void File::parse_header() {
  file_.seekg(0);

  char buf[256];
  file_.read(buf, sizeof(buf));

  register_header_fields();
  for (const FieldDescriptor& desc : header_descriptors_) {
    desc.parser(buf + desc.offset, header_);
  }
}

void File::parse_signal_headers() {}