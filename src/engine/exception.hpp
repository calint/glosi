#pragma once
// reviewed: 2024-07-08

namespace glos {

class exception final : public std::exception {
public:
  std::string message{};

  inline explicit exception(
      std::string const &msg = "",
      std::source_location const &src_loc = std::source_location::current())
      : message{std::format("{}:{} - {}: {}", src_loc.file_name(),
                            src_loc.line(), src_loc.function_name(), msg)} {}

  inline char const *what() const noexcept override { return message.c_str(); }
};
} // namespace glos