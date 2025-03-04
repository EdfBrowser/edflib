#pragma once
#include <fstream>
#include <functional>
#include <string>
#include <vector>

namespace edf {
using str = std::string;
using ushort = unsigned short;
using uint = unsigned int;

struct SignalInfo {
  str label;
  str transducer;
  str physical_dim;
  double physical_min;
  double physical_max;
  int digital_min;
  int digital_max;
  str prefiltering;
  uint samples;
  str reserved;
};

struct Header {
  str version;
  str patient_id;
  str recording_id;
  str start_date;
  str start_time;
  uint header_size;
  str reserved;
  uint data_record_count;
  double record_duration;
  ushort signal_count;

  std::vector<SignalInfo> signals;
};

class File {
 public:
  explicit File(const str& path);
  ~File();

  const Header& header() const noexcept { return header_; }

  int read_signal_data(char* const buf, uint signal_index,
                       uint start_record = 0, uint record_count = 0);

 private:
  struct FieldDescriptor {
    uint offset;
    uint length;
    std::function<void(const char*, Header&)> parser;
  };

  struct SignalDescriptor {
    uint offset;
    uint length;
    std::function<void(const char*, SignalInfo&)> parser;
  };

  void parse_header();
  void parse_signal_headers();

  std::ifstream file_;
  Header header_;

  static void register_fields();
  static std::vector<FieldDescriptor> header_descriptors_;
  static std::vector<SignalDescriptor> signal_descriptors_;
};
};  // namespace edf
