#pragma once
#include <fstream>
#include <functional>
#include <string>
#include <vector>

namespace edf {
using str = std::string;
using ushort = unsigned short;
using uint = unsigned int;

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

  struct SignalInfo {
    str label;
    str transducer;
    str physical_dim;
    double physical_min;
    double physical_max;
    short digital_min;
    short digital_max;
    str prefiltering;
    uint samples_per_count;
    str reserved;
  };

  std::vector<SignalInfo> signals;
};

class File {
 public:
  explicit File(const str& path);
  ~File();

  const Header& header() const noexcept { return header_; }

  // std::vector<short> read_signal_data(ushort signal_index,
  //                                    ushort start_record = 0,
  //                                    ushort record_count = 0);

 private:
  struct FieldDescriptor {
    uint offset;
    uint length;
    std::function<void(const char*, Header&)> parser;
  };

  void parse_header();
  void register_header_fields();
  void parse_signal_headers();

  std::ifstream file_;
  Header header_;
  std::vector<FieldDescriptor> header_descriptors_;
};
};  // namespace edf