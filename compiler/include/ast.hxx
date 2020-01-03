

#pragma once

#include <vector>

#include "tokenizer.hxx"

class function {
  // (member parameter)
  // parameters
  // return types
};

class interface;       // nethod declarations
class storage_type;    // size + alignment, that's all
class implementation;  // implements some interface(s)

// usable type: interface or implementation, or parameter type (which is generic, but will be specified!)

// concrete type or what?
class type {
 public:
  std::string name_;
  // constructors
  // other methods
};

class ast_base {
 public:
  virtual ~ast_base() {}

  void set_start(source_pos start);
  void set_end(source_pos end);

 private:
  source_range loc_;
};

class ast_stmt;

class ast_block : public ast_base {
 public:
 private:
  std::vector<std::unique_ptr<ast_stmt>> stmts_;
};

class ast_stmt : public ast_base {
 public:
};

class ast_construct_stmt : public ast_stmt {
 public:
 private:
  // constructor ref
  // parameter list
};

class ast_decl_stmt : public ast_stmt {
 public:
 private:
  type const& t_;
  std::string_view name_;                            // as in the source
  std::unique_ptr<ast_construct_stmt> constructor_;  // or null
};

// ...
