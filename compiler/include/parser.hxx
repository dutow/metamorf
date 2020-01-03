
#pragma once

#include <algorithm>
#include <set>
#include <unordered_set>

#include "ast.hxx"
#include "tokenizer.hxx"

namespace {
bool has_newline(token const& t) {
  return t.type == token_type::whitespace && t.text.find('\n') != std::string_view::npos;
}
}  // namespace

class parser_context {
 public:
  parser_context() {
    operators_.insert("=");
    types_.insert("u8");
    types_.insert("u16");
    types_.insert("u32");
    types_.insert("u64");
    types_.insert("s8");
    types_.insert("s16");
    types_.insert("s32");
    types_.insert("s64");
  }

  bool type_exists(std::string_view typen) const noexcept { return types_.find(std::string{typen}) != types_.end(); }

  bool operator_or_prefix(std::string_view name) const noexcept {
    auto it = std::lower_bound(operators_.begin(), operators_.end(), name);
    return (it != operators_.end() && it->substr(0, name.length()) == name);
  }

 private:
  parser_context* root_context_;
  std::unordered_set<std::string> types_;
  std::set<std::string> operators_;
  std::unordered_set<std::string> variables_;
};

class parser_base {
 public:
  // descendant destructors will either commit or throw
  parser_base(parser_context& pc, tokenizer& t) noexcept : tokenizer_(t), checkpoint_(tokenizer_), pc_(pc) {}

 protected:
  tokenizer& tokenizer_;
  tokenizer::checkpointer checkpoint_;
  parser_context& pc_;

  token const& next_token() noexcept {
    return tokenizer_.next_token([this](std::string_view sv) { return pc_.operator_or_prefix(sv); });
  }

  token const& require_token_allow_ws(token_type tt) {
    tokenizer_.skip_whitespace();
    auto const& tok = next_token();
    if (tok.type != tt) {
      throw 2;
    }
    if (tok.error) {
      throw 3;
    }
    return tok;
  }

  token const& require_token_allow_ws(token_type tt, std::string_view text) {
    auto const& tok = require_token_allow_ws(tt);
    if (tok.text != text) {
      throw 1;
    }
    return tok;
  }
};

class parser_block : private parser_context, public parser_base {
 public:
  parser_block(tokenizer& t) : parser_base(*this, t) {
    // assert: current token is a block opener

    // if (something) {}
    // for (something) {}
    // while {}
    // <type> <name> [= <constructor>] ;
    // <func-name> <paramblock> ;

    // u8 i = 8
    // s8 b
    // s16 c \ // ignore newline
    //   = 16 // error!
    // tt v(a, b, c)
    // tt v(a, b,
    //      c)

    ast_block a_b;

    bool block_end = false;
    while (!block_end) {
      t.skip_whitespace();
      auto const& tok = next_token();
      switch (tok.type) {
        case token_type::bracket:
          if (tok.text == "}") {
            block_end = true;
            break;
          }
          break;
        case token_type::function_identifier:
          // start of a function call
        case token_type::identifier: {
          // has to be a type, keyword, or function call

          // if it is a type, this has to be a variable declaration statement
          if (pc_.type_exists(tok.text)) {
            // assume a variable declaration: <type> <name> = <integer_value>
            // require_token(token_type::identifier);
            require_token_allow_ws(token_type::identifier);
            require_token_allow_ws(token_type::oper, "=");
            require_token_allow_ws(token_type::numeric);
            const auto maybe_ws = t.skip_whitespace();
            if (maybe_ws && has_newline(*maybe_ws)) {
              // found a statement end!
              break;
            } else {
              require_token_allow_ws(token_type::semicolon);
              break;
            }
          }
          throw 4;
          break;
        }
        default:
          // error :(
          break;
      }
    }
  }

 private:
};
