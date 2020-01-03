
#include "tokenizer.hxx"

#include "catch.hpp"

namespace {
const auto token_always_exist = [](std::string_view) { return true; };
const auto token_never_exist = [](std::string_view) { return false; };
const auto token_len_5 = [](std::string_view sv) { return sv.length() <= 5; };
}  // namespace

void single_token_test(std::string inp, std::string tokenized, token_type tt, bool success) {
  diagnostic_reporter rep;
  tokenizer t("<test>", inp, rep);
  auto& tok = t.next_token(token_always_exist);
  REQUIRE(tok.text == tokenized);
  REQUIRE(tok.type == tt);
  REQUIRE(!tok.error == success);
  REQUIRE((!success || (t.next_token(token_always_exist).type == token_type::eof)));
}

void two_token_test(std::string inp, std::string p1, token_type tt1, std::string p2, token_type tt2, bool success) {
  diagnostic_reporter rep;
  tokenizer t("<test>", inp, rep);
  auto& tok1 = t.next_token(token_always_exist);
  REQUIRE(tok1.text == p1);
  REQUIRE(tok1.type == tt1);
  REQUIRE(!tok1.error == success);

  auto& tok2 = t.next_token(token_always_exist);
  REQUIRE(tok2.text == p2);
  REQUIRE(tok2.type == tt2);

  REQUIRE(t.next_token(token_always_exist).type == token_type::eof);
}

TEST_CASE("A number is tokenized", "[tokenizer]") { single_token_test("42", "42", token_type::numeric, true); }

TEST_CASE("An operator is tokenized", "[tokenizer]") { single_token_test(":not:", ":not:", token_type::oper, true); }

TEST_CASE("An operator requires some length", "[tokenizer]") {
  diagnostic_reporter rep;
  tokenizer t("<test>", "::::", rep);
  auto& tok1 = t.next_token(token_never_exist);
  REQUIRE(tok1.text == "");
  REQUIRE(tok1.type == token_type::oper);
  REQUIRE(tok1.error);
}

TEST_CASE("Two operator without inner tokens are tokenized", "[tokenizer]") {
  diagnostic_reporter rep;
  tokenizer t("<test>", ":one:!two", rep);
  auto& tok1 = t.next_token(token_len_5);
  REQUIRE(tok1.text == ":one:");
  REQUIRE(tok1.type == token_type::oper);
  REQUIRE(!tok1.error);

  auto& tok2 = t.next_token(token_len_5);
  REQUIRE(tok2.text == "!two");
  REQUIRE(tok2.type == token_type::oper);
  REQUIRE(!tok2.error);

  REQUIRE(t.next_token(token_always_exist).type == token_type::eof);
}

TEST_CASE("A number and a whitespace is tokenized", "[tokenizer]") {
  two_token_test("42 \t", "42", token_type::numeric, " \t", token_type::whitespace, true);
}

TEST_CASE("A number is tokenized, but an error is encountered", "[tokenizer]") {
  two_token_test("42a", "42", token_type::numeric, "a", token_type::identifier, false);
}

TEST_CASE("A number and a semicolon is tokenized", "[tokenizer]") {
  two_token_test("42;", "42", token_type::numeric, ";", token_type::semicolon, true);
}

TEST_CASE("Brackets can be tokenized", "[tokenizer]") {
  two_token_test("[]", "[", token_type::bracket, "]", token_type::bracket, true);
}

TEST_CASE("Non matching brackets can be tokenized", "[tokenizer]") {
  two_token_test("(}", "(", token_type::bracket, "}", token_type::bracket, true);
}

TEST_CASE("A negative number is tokenized", "[tokenizer]") {
  single_token_test("-42", "-42", token_type::numeric, true);
}

TEST_CASE("A name is tokenized", "[tokenizer]") { single_token_test("commit", "commit", token_type::identifier, true); }

TEST_CASE("A name with a question mark is tokenized", "[tokenizer]") {
  single_token_test("commit!?", "commit!?", token_type::function_identifier, true);
}

TEST_CASE("The checkpointer without commit brings back the state", "[tokenizer]") {
  diagnostic_reporter rep;
  tokenizer t("<test>", "int \n\n\na? b 42", rep);
  t.next_token(token_always_exist);
  t.next_token(token_always_exist);
  t.next_token(token_always_exist);
  t.next_token(token_always_exist);
  {
    tokenizer::checkpointer c{t};
    t.next_token(token_always_exist);
    t.next_token(token_always_exist);
    t.next_token(token_always_exist);
    REQUIRE(t.next_token(token_always_exist).type == token_type::eof);
  }
  REQUIRE(t.next_token(token_always_exist).text == "b");
}

TEST_CASE("A committed chekpointer keeps the state", "[tokenizer]") {
  diagnostic_reporter rep;
  tokenizer t("<test>", "int \n\n\na? b 42", rep);
  t.next_token(token_always_exist);
  t.next_token(token_always_exist);
  t.next_token(token_always_exist);
  t.next_token(token_always_exist);
  {
    tokenizer::checkpointer c{t};
    t.next_token(token_always_exist);
    t.next_token(token_always_exist);
    t.next_token(token_always_exist);
    REQUIRE(t.next_token(token_always_exist).type == token_type::eof);
    c.commit();
  }
  REQUIRE(t.next_token(token_always_exist).type == token_type::eof);
}

TEST_CASE("Whitespace can be skipped", "[tokenizer]") {
  diagnostic_reporter rep;
  tokenizer t("<test>", "\n \na?", rep);
  t.skip_whitespace();
  t.skip_whitespace();
  REQUIRE(t.next_token(token_always_exist).type == token_type::function_identifier);
}
