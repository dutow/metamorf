
#pragma once

#include <string>
#include <vector>

struct source_pos {
  int line;
  int offset;
};

struct source_range {
  source_pos start;
  source_pos end;
};

enum class diagnostic_level { warning, error };

class diagnostic_message {
 public:
  virtual diagnostic_level level() const = 0;
  virtual std::string message() const = 0;
  virtual int diagnostic_code() const = 0;
  virtual ~diagnostic_message(){};
};

struct source_diagnostic {
  const diagnostic_message& diag;
  source_range where;
  std::string suggestion;
};

class diagnostic_reporter {
 public:
  using messages_t = std::vector<source_diagnostic>;

  void report(source_diagnostic diag);

  messages_t const& messages() const;

 private:
  messages_t messages_;
};
