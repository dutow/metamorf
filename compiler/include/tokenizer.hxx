
#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

#include "source.hxx"

enum class token_type { unknown, bracket, semicolon, whitespace, oper, identifier, function_identifier, numeric, eof };

struct token {
  token_type type;
  source_range pos;
  std::string_view text;  // NOT null terminated!
  bool error;
};

class tokenizer {
 public:
  using lookup_t = std::function<bool(std::string_view)>;

  tokenizer(std::string filename, std::string content, diagnostic_reporter& reporter);

  token const& next_token(lookup_t is_operator_or_prefix) noexcept;

  const token* skip_whitespace() noexcept;

  class checkpointer {
   public:
    checkpointer(tokenizer& t) noexcept;
    ~checkpointer();

    void commit() noexcept;

   private:
    tokenizer& t_;
    std::size_t index_;
    source_pos position_;
    std::size_t vec_size_;

    static const constexpr std::size_t npos = std::string::npos;
  };

 private:
  std::string filename_;
  std::string content_;
  std::size_t index_;
  source_pos position_;
  std::vector<token> tokens_;
  diagnostic_reporter& reporter_;

  token create_token(lookup_t is_operator_or_prefix) noexcept;
  bool try_numeric() noexcept;
  bool try_whitespace() noexcept;
  bool try_operator(lookup_t is_operator_or_prefix) noexcept;
  token_type try_identifier() noexcept;

  template <typename... T>
  bool check_continuation(T... fs) noexcept;
};
