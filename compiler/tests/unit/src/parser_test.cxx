
#include "parser.hxx"

namespace {
const auto token_always_exist = [](std::string_view) { return true; };
}

#include "catch.hpp"

TEST_CASE("A variable declaration can be parsed, ending with a newilne", "[parser]") {
  diagnostic_reporter rep;
  tokenizer t("<test>", "{ u8 v = 42\n }", rep);
  t.next_token(token_always_exist);  // skip the first bracket, parser_block expect it to be parsed
  parser_block p{t};
}

TEST_CASE("A variable declaration can be parsed, ending with a semicolon", "[parser]") {
  diagnostic_reporter rep;
  tokenizer t("<test>", "{ u8 v = 2; }", rep);
  t.next_token(token_always_exist);  // skip the first bracket, parser_block expect it to be parsed
  parser_block p{t};
}

TEST_CASE("Two variable declarations can be parsed", "[parser]") {
  diagnostic_reporter rep;
  tokenizer t("<test>", "{ u8 a = 42; u16 b = -5\n }", rep);
  t.next_token(token_always_exist);  // skip the first bracket, parser_block expect it to be parsed
  parser_block p{t};
}
