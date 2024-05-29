#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "koopa.h"

class RiscV {
  class Environment {
    std::map<koopa_raw_value_t, int> address_map;
  public:
    bool has_call = false;
    int total_stack_size = 0;
    int current_offset = 0;
    void initialize(int size, bool call);
    int get_address(koopa_raw_value_t value);
  };

  Environment env;
  std::ofstream output_file;

  static int calculate_function_size(koopa_raw_function_t func, bool &call);
  static int calculate_bb_size(koopa_raw_basic_block_t bb, bool &call, int &max_arg);
  static int calculate_inst_size(koopa_raw_value_t value);
  static int calculate_type_size(koopa_raw_type_t ty);
  static int calculate_array_size(koopa_raw_type_t ty);

  void load_to_register(koopa_raw_value_t value, const std::string& reg);
  void store_to_stack(int addr, const std::string& reg);
  void visit_raw_program(const koopa_raw_program_t &raw);
  void visit_raw_slice(const koopa_raw_slice_t &slice);
  void visit_raw_function(const koopa_raw_function_t &func);
  void visit_raw_basic_block(const koopa_raw_basic_block_t &bb);
  void visit_raw_value(const koopa_raw_value_t &value);
  void visit_return(const koopa_raw_return_t &return_value);
  void visit_binary(const koopa_raw_binary_t &binary_value, int addr);
  void visit_load(const koopa_raw_load_t &load_value, int addr);
  void visit_store(const koopa_raw_store_t &store_value);
  void visit_branch(const koopa_raw_branch_t &branch_value);
  void visit_jump(const koopa_raw_jump_t &jump_value);
  void visit_call(const koopa_raw_call_t &call_value, int addr);
  void handle_global_alloc(const koopa_raw_value_t &global_alloc_value);
  void visit_aggregate(const koopa_raw_aggregate_t &aggregate_value);
  void visit_get_elem_ptr(const koopa_raw_get_elem_ptr_t &get_elem_ptr_value, int addr);
  void visit_get_ptr(const koopa_raw_get_ptr_t &get_ptr_value, int addr);

public:
  RiscV(const char *path) {
    output_file.open(path);
  }
  void build(const std::string& ir);
};
