
#include "tokenizer.hxx"

class contination_error_t : public diagnostic_message {
  virtual diagnostic_level level() const { return diagnostic_level::error; }
  virtual std::string message() const { return "Tokenizer error: not allowed continuation."; }
  virtual int diagnostic_code() const { return 1; }
};

contination_error_t contination_error;

namespace {
const auto is_numeric_start = [](char c) { return c >= '0' && c <= '9'; };
const auto is_whitespace_start = [](char c) { return c == ' ' || c == '\t' || c == '\n' || c == '\r'; };
const auto is_operator_start = [](char c) {
  switch (c) {
    case '-':
    case '+':
    case '?':
    case '!':
    case '*':
    case '/':
    case '\\':
    case '^':
    case '$':
    case '@':
    case '%':
    case '~':
    case '|':
    case '=':
    case '<':
    case '>':
    case ':':
      return true;
    default:
      return false;
  }
};
const auto is_identifier_start = [](char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; };
const auto is_identifier_inside = [](char c) { return (c >= '0' && c <= '9') || is_identifier_start(c); };
const auto is_semicolon = [](char c) { return c == ';'; };
const auto is_bracket = [](char c) {
  switch (c) {
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
      return true;
    default:
      return false;
  }
};
}  // namespace

tokenizer::tokenizer(std::string filename, std::string content, diagnostic_reporter& rep)

    : filename_(filename), content_(content), index_(0), position_{1, 0}, reporter_(rep) {}

token const& tokenizer::next_token(lookup_t is_operator_or_prefix) noexcept {
  if (!tokens_.empty()) {
    auto& curr = *(--tokens_.end());
    if (curr.type == token_type::eof) {
      return curr;
    }
  }

  tokens_.push_back(create_token(is_operator_or_prefix));

  return *(--tokens_.end());
}

const token* tokenizer::skip_whitespace() noexcept {
  if (index_ < content_.size() && is_whitespace_start(content_[index_])) {
    return &next_token([](std::string_view) { return false; });
  }
  return nullptr;
}

bool tokenizer::try_operator(lookup_t is_operator_or_prefix) noexcept {
  const char* start_pos = &content_[index_];
  std::size_t len = 1;

  char next = content_[index_];

  if (!is_operator_start(next)) {
    return false;
  }

  while (is_operator_start(next) && is_operator_or_prefix({start_pos, len}) && index_ < content_.size()) {
    // possible symbols at the end
    // no greedy checks here: func names can't be concatenated with operators directly
    index_++;
    next = content_[index_];
    position_.offset++;
    len++;
  }

  // after the symbols, there can be identifier characters
  while (is_identifier_inside(next) && is_operator_or_prefix({start_pos, len}) && index_ < content_.size()) {
    index_++;
    next = content_[index_];
    position_.offset++;
    len++;
  }

  // and this can be followed by operator symbols again
  while (is_operator_start(next) && is_operator_or_prefix({start_pos, len}) && index_ < content_.size()) {
    index_++;
    next = content_[index_];
    position_.offset++;
    len++;
  }

  return true;
}

bool tokenizer::try_whitespace() noexcept {
  const auto old_idx = index_;

  char next = content_[index_];

  while (is_whitespace_start(next) && index_ < content_.size()) {
    index_++;
    position_.offset++;
    if (next == '\n') {
      position_.offset = 0;
      position_.line++;
    }
    next = content_[index_];
  }

  return old_idx != index_;
}

bool tokenizer::try_numeric() noexcept {
  const auto old_idx = index_;

  char next = content_[index_];

  if (next == '-' && index_ + 1 < content_.size() && is_numeric_start(content_[index_ + 1])) {
    index_++;
    next = content_[index_];
    position_.offset++;
  }

  while (is_numeric_start(next) && index_ < content_.size()) {
    index_++;
    next = content_[index_];
    position_.offset++;
  }

  return old_idx != index_;
}

token_type tokenizer::try_identifier() noexcept {
  // identifiers start with alphabet or _, continue with alphabet+number+_, and also can end with symbols

  char next = content_[index_];

  if (!is_identifier_start(next)) {
    return token_type::unknown;
  }

  while (is_identifier_inside(next) && index_ < content_.size()) {
    index_++;
    next = content_[index_];
    position_.offset++;
  }

  const auto old_idx = index_;

  while (is_operator_start(next) && index_ < content_.size()) {
    // possible symbols at the end
    // no greedy checks here: func names can't be concatenated with operators directly
    index_++;
    next = content_[index_];
    position_.offset++;
  }

  return old_idx != index_ ? token_type::function_identifier : token_type::identifier;
}

token tokenizer::create_token(lookup_t is_operator_or_prefix) noexcept {
  // actually create the token...

  // Assumption:
  // 1. position_ points to the next character to be read
  // 2. so far the input was correct
  // 3. we don't try to read after EOF

  if (index_ == content_.size()) {
    return {token_type::eof, {position_, position_}, "", false};
  }

  const char next = content_[index_];

  const auto saved_index = index_;
  const auto saved_position = position_;

  if (is_semicolon(next)) {
    index_++;
    position_.offset++;
    return {
        token_type::semicolon, {saved_position, position_}, {&(content_[saved_index]), index_ - saved_index}, false};
  }

  if (is_bracket(next)) {
    index_++;
    position_.offset++;
    return {token_type::bracket, {saved_position, position_}, {&(content_[saved_index]), index_ - saved_index}, false};
  }

  if (try_whitespace()) {
    return {
        token_type::whitespace, {saved_position, position_}, {&(content_[saved_index]), index_ - saved_index}, false};
  }

  {
    const auto tt = try_identifier();
    if (tt != token_type::unknown) {
      const bool allowed_continuation = !check_continuation(is_whitespace_start, is_semicolon);
      const source_range range = {saved_position, position_};
      if (!allowed_continuation) {
        reporter_.report({contination_error, range, ""});
      }
      return {tt,
              range,
              {&(content_[saved_index]), index_ - saved_index},
              allowed_continuation};  // TODO: parenthesis also: (, [, ... ?
    }
  }

  if (try_numeric()) {
    return {token_type::numeric,
            {saved_position, position_},
            {&(content_[saved_index]), index_ - saved_index},
            !check_continuation(is_whitespace_start, is_operator_start, is_semicolon)};
  }

  if (try_operator(is_operator_or_prefix)) {
    return {token_type::oper,
            {saved_position, position_},
            {&(content_[saved_index]), index_ - saved_index},
            !check_continuation(is_whitespace_start, is_operator_start, is_numeric_start, is_identifier_start,
                                is_semicolon) ||
                index_ == saved_index};
  }

  return {};  // assert!
}

template <typename... T>
bool tokenizer::check_continuation(T... fs) noexcept {
  if (index_ == content_.size()) {
    return true;  // end of file is always allowed
  }
  char next = content_[index_];
  return (fs(next) || ...);
}

tokenizer::checkpointer::checkpointer(tokenizer& t) noexcept
    : t_(t), index_(t.index_), position_(t.position_), vec_size_(t.tokens_.size()) {}

void tokenizer::checkpointer::commit() noexcept { index_ = npos; }

tokenizer::checkpointer::~checkpointer() {
  if (index_ != npos) {
    t_.index_ = index_;
    t_.position_ = position_;
    t_.tokens_.resize(vec_size_);
  }
}
