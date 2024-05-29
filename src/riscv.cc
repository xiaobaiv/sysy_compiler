#include "riscv.hh"
#include <cassert>
#include <cstring>
#include <iostream>
#include <random>
#include <algorithm>

int RiscV::calculate_function_size(koopa_raw_function_t func, bool &call) {
  int size = 0;
  int max_arg = 0;
  for (size_t i = 0; i < func->bbs.len; ++i) {
    auto bb_ptr = func->bbs.buffer[i];
    size += calculate_bb_size(reinterpret_cast<koopa_raw_basic_block_t>(bb_ptr), call, max_arg);
  }
  size += max_arg * 4;
  size += (func->params.len > 8 ? func->params.len - 8 : 0) * 4;
  size += call ? 4 : 0;
  return size;
}

int RiscV::calculate_bb_size(koopa_raw_basic_block_t bb, bool &call, int &max_arg) {
  int size = 0;
  for (size_t i = 0; i < bb->insts.len; ++i) {
    auto inst_ptr = bb->insts.buffer[i];
    if (reinterpret_cast<koopa_raw_value_t>(inst_ptr)->kind.tag == KOOPA_RVT_CALL) {
      call = true;
      max_arg = std::max(max_arg, static_cast<int>(reinterpret_cast<koopa_raw_value_t>(inst_ptr)->kind.data.call.args.len));
    }
    size += calculate_inst_size(reinterpret_cast<koopa_raw_value_t>(inst_ptr));
  }
  return size;
}

int RiscV::calculate_inst_size(koopa_raw_value_t inst) {
  if (inst->kind.tag == KOOPA_RVT_ALLOC) {
    return calculate_type_size(inst->ty->data.pointer.base);
  }
  return calculate_type_size(inst->ty);
}

int RiscV::calculate_type_size(koopa_raw_type_t ty) {
  switch (ty->tag) {
    case KOOPA_RTT_INT32: return 4;
    case KOOPA_RTT_UNIT: return 0;
    case KOOPA_RTT_POINTER: return 4;
    case KOOPA_RTT_ARRAY: return calculate_array_size(ty);
    default: return 0;
  }
}

int RiscV::calculate_array_size(koopa_raw_type_t ty) {
  if (ty->tag == KOOPA_RTT_ARRAY) {
    return calculate_array_size(ty->data.array.base) * ty->data.array.len;
  }
  return 4;
}

void RiscV::Environment::initialize(int size, bool call) {
  total_stack_size = size;
  has_call = call;
  current_offset = 0;
  address_map.clear();
}

int RiscV::Environment::get_address(koopa_raw_value_t raw) {
  if (address_map.find(raw) != address_map.end()) {
    return address_map[raw];
  } else {
    int size = calculate_inst_size(raw);
    if (size == 0) return -1;
    address_map[raw] = current_offset;
    current_offset += size;
    return address_map[raw];
  }
}

void RiscV::load_to_register(koopa_raw_value_t value, const std::string& reg) {
  if (value->kind.tag == KOOPA_RVT_INTEGER) {
    output_file << "  li " + reg + ", " + std::to_string(value->kind.data.integer.value) + "\n";
  } else {
    int addr = env.get_address(value);
    if (addr != -1) {
      if (addr < 2048 && addr >= -2048) {
        output_file << "  lw " + reg + ", " + std::to_string(addr) + "(sp)\n";
      } else {
        output_file << "  li t3, " + std::to_string(addr) + "\n";
        output_file << "  add t3, sp, t3\n";
        output_file << "  lw " + reg + ", 0(t3)\n";
      }
    } else {
      assert(false);
    }
  }
}

void RiscV::store_to_stack(int addr, const std::string& reg) {
  if (addr != -1) {
    if (addr < 2048 && addr >= -2048) {
      output_file << "  sw " + reg + ", " + std::to_string(addr) + "(sp)\n";
    } else {
      output_file << "  li t3, " + std::to_string(addr) + "\n";
      output_file << "  add t3, sp, t3\n";
      output_file << "  sw " + reg + ", 0(t3)\n";
    }
  } else {
    assert(false);
  }
}

void RiscV::visit_raw_program(const koopa_raw_program_t &raw) {
  if (raw.values.len != 0) {
    output_file << "  .data\n";
    visit_raw_slice(raw.values);
  }
  output_file << "\n  .text\n";
  visit_raw_slice(raw.funcs);
}

void RiscV::visit_raw_slice(const koopa_raw_slice_t &slice) {
  for (size_t i = 0; i < slice.len; ++i) {
    auto ptr = slice.buffer[i];
    switch (slice.kind) {
      case KOOPA_RSIK_FUNCTION: visit_raw_function(reinterpret_cast<const koopa_raw_function_t>(ptr)); break;
      case KOOPA_RSIK_BASIC_BLOCK: visit_raw_basic_block(reinterpret_cast<koopa_raw_basic_block_t>(ptr)); break;
      case KOOPA_RSIK_VALUE: visit_raw_value(reinterpret_cast<koopa_raw_value_t>(ptr)); break;
      default: assert(false);
    }
  }
}

void RiscV::visit_raw_function(const koopa_raw_function_t &func) {
  if (func->bbs.len == 0) return;
  output_file << "\n  .globl  " << std::string(func->name + 1) + "\n";
  output_file << std::string(func->name + 1) + ":\n";
  bool call = false;
  int size = calculate_function_size(func, call);
  size = (size + 15) / 16 * 16 * 2;
  if (size < 2048 && size >= -2048) {
    output_file << "  addi sp, sp, -" + std::to_string(size) + "\n";
  } else if (size > 0) {
    output_file << "  li t0, -" + std::to_string(size) + "\n";
    output_file << "  add sp, sp, t0\n";
  }
  if (call) {
    if (size - 4 < 2048 && size - 4 >= -2048)
      output_file << "  sw ra, " + std::to_string(size - 4) + "(sp)\n";
    else {
      output_file << "  li t0, " + std::to_string(size - 4) + "\n";
      output_file << "  add t0, sp, t0\n";
      output_file << "  sw ra, 0(t0)\n";
    }
  }
  env.initialize(size, call);
  env.total_stack_size -= call ? 4 : 0;
  env.current_offset += (func->params.len > 8 ? func->params.len - 8 : 0) * 4;
  visit_raw_slice(func->bbs);
}

void RiscV::visit_raw_basic_block(const koopa_raw_basic_block_t &bb) {
  std::string name = bb->name + 1;
  if (name != "entry")
    output_file << name << ":\n";
  visit_raw_slice(bb->insts);
}

void RiscV::visit_raw_value(const koopa_raw_value_t &value) {
  const auto &kind = value->kind;
  int addr = env.get_address(value);
  switch (kind.tag) {
    case KOOPA_RVT_RETURN: visit_return(kind.data.ret); break;
    case KOOPA_RVT_INTEGER: break;
    case KOOPA_RVT_ALLOC: break;
    case KOOPA_RVT_LOAD: visit_load(kind.data.load, addr); break;
    case KOOPA_RVT_STORE: visit_store(kind.data.store); break;
    case KOOPA_RVT_BINARY: visit_binary(kind.data.binary, addr); break;
    case KOOPA_RVT_BRANCH: visit_branch(kind.data.branch); break;
    case KOOPA_RVT_JUMP: visit_jump(kind.data.jump); break;
    case KOOPA_RVT_CALL: visit_call(kind.data.call, addr); break;
    case KOOPA_RVT_GLOBAL_ALLOC: handle_global_alloc(value); break;
    case KOOPA_RVT_GET_ELEM_PTR: visit_get_elem_ptr(kind.data.get_elem_ptr, addr); break;
    case KOOPA_RVT_GET_PTR: visit_get_ptr(kind.data.get_ptr, addr); break;
    default: assert(false);
  }
}

void RiscV::visit_return(const koopa_raw_return_t &ret_value) {
  if (ret_value.value != nullptr) {
    load_to_register(ret_value.value, "a0");
  }
  if (env.has_call) {
    if (env.total_stack_size < 2048 && env.total_stack_size >= -2048)
      output_file << "  lw ra, " + std::to_string(env.total_stack_size) + "(sp)\n";
    else {
      output_file << "  li t0, " + std::to_string(env.total_stack_size) + "\n";
      output_file << "  add t0, sp, t0\n";
      output_file << "  lw ra, 0(t0)\n";
    }
  }
  int size = env.total_stack_size;
  size += env.has_call ? 4 : 0;
  if (size < 2048 && size >= -2048) {
    output_file << "  addi sp, sp, " + std::to_string(size) + "\n";
  } else if (size > 0) {
    output_file << "  li t0, " + std::to_string(size) + "\n";
    output_file << "  add sp, sp, t0\n";
  }
  output_file << "  ret\n";
}

void RiscV::visit_binary(const koopa_raw_binary_t &binary_value, int addr) {
  std::string rd = "t0";
  std::string rs1 = "t0";
  std::string rs2 = "t1";
  load_to_register(binary_value.lhs, rs1);
  load_to_register(binary_value.rhs, rs2);
  switch (binary_value.op) {
    case KOOPA_RBO_ADD: output_file << "  add " + rd + ", " + rs1 + ", " + rs2 + "\n"; break;
    case KOOPA_RBO_SUB: output_file << "  sub " + rd + ", " + rs1 + ", " + rs2 + "\n"; break;
    case KOOPA_RBO_MUL: output_file << "  mul " + rd + ", " + rs1 + ", " + rs2 + "\n"; break;
    case KOOPA_RBO_DIV: output_file << "  div " + rd + ", " + rs1 + ", " + rs2 + "\n"; break;
    case KOOPA_RBO_MOD: output_file << "  rem " + rd + ", " + rs1 + ", " + rs2 + "\n"; break;
    case KOOPA_RBO_AND: output_file << "  and " + rd + ", " + rs1 + ", " + rs2 + "\n"; break;
    case KOOPA_RBO_OR: output_file << "  or " + rd + ", " + rs1 + ", " + rs2 + "\n"; break;
    case KOOPA_RBO_EQ: output_file << "  xor " + rd + ", " + rs1 + ", " + rs2 + "\n"; output_file << "  seqz " + rd + ", " + rd + "\n"; break;
    case KOOPA_RBO_NOT_EQ: output_file << "  xor " + rd + ", " + rs1 + ", " + rs2 + "\n"; output_file << "  snez " + rd + ", " + rd + "\n"; break;
    case KOOPA_RBO_GT: output_file << "  sgt " + rd + ", " + rs1 + ", " + rs2 + "\n"; break;
    case KOOPA_RBO_LT: output_file << "  slt " + rd + ", " + rs1 + ", " + rs2 + "\n"; break;
    case KOOPA_RBO_GE: output_file << "  slt " + rd + ", " + rs1 + ", " + rs2 + "\n"; output_file << "  seqz " + rd + ", " + rd + "\n"; break;
    case KOOPA_RBO_LE: output_file << "  sgt " + rd + ", " + rs1 + ", " + rs2 + "\n"; output_file << "  seqz " + rd + ", " + rd + "\n"; break;
    default: break;
  }
  store_to_stack(addr, rd);
}

void RiscV::visit_store(const koopa_raw_store_t &store_value) {
  if (store_value.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    output_file << "  la t1, " + std::string(store_value.dest->name + 1) + "\n";
    load_to_register(store_value.value, "t0");
    output_file << "  sw t0, 0(t1)\n";
  } else if (store_value.dest->kind.tag == KOOPA_RVT_GET_ELEM_PTR || store_value.dest->kind.tag == KOOPA_RVT_GET_PTR) {
    load_to_register(store_value.dest, "t1");
    load_to_register(store_value.value, "t0");
    output_file << "  sw t0, 0(t1)\n";
  } else {
    int addr = env.get_address(store_value.dest);
    if (store_value.value->kind.tag == KOOPA_RVT_FUNC_ARG_REF) {
      if (store_value.value->kind.data.func_arg_ref.index < 8) {
        store_to_stack(addr, "a" + std::to_string(store_value.value->kind.data.func_arg_ref.index));
      } else {
        if ((store_value.value->kind.data.func_arg_ref.index - 8) * 4 < 2048 && (store_value.value->kind.data.func_arg_ref.index - 8) * 4 >= -2048)
          output_file << "  lw t0, " << (store_value.value->kind.data.func_arg_ref.index - 8) * 4 << "(sp)\n";
        else {
          output_file << "  li t3, " << (store_value.value->kind.data.func_arg_ref.index - 8) * 4 << "\n";
          output_file << "  add t3, sp, t3\n";
          output_file << "  lw t0, 0(t3)\n";
        }
        store_to_stack(addr, "t0");
      }
    } else {
      load_to_register(store_value.value, "t0");
      store_to_stack(addr, "t0");
    }
  }
}

void RiscV::visit_load(const koopa_raw_load_t &load_value, int addr) {
  std::string rs1 = "t0";
  if (load_value.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    output_file << "  la " + rs1 + ", " + std::string(load_value.src->name + 1) + "\n";
    output_file << "  lw " + rs1 + ", 0(" + rs1 + ")\n";
  } else if (load_value.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR || load_value.src->kind.tag == KOOPA_RVT_GET_PTR) {
    load_to_register(load_value.src, rs1);
    output_file << "  lw " + rs1 + ", 0(" + rs1 + ")\n";
  } else {
    load_to_register(load_value.src, rs1);
  }
  store_to_stack(addr, rs1);
}

void RiscV::visit_branch(const koopa_raw_branch_t &branch_value) {
  load_to_register(branch_value.cond, "t0");
  output_file << "  bnez t0, " + std::string(branch_value.true_bb->name + 1) + "_tmp" + "\n";
  output_file << "  j " + std::string(branch_value.false_bb->name + 1) + "\n";
  output_file << std::string(branch_value.true_bb->name + 1) + "_tmp:\n";
  output_file << "  j " + std::string(branch_value.true_bb->name + 1) + "\n";
}

void RiscV::visit_jump(const koopa_raw_jump_t &jump_value) {
  output_file << "  j " + std::string(jump_value.target->name + 1) + "\n";
}

void RiscV::visit_call(const koopa_raw_call_t &call_value, int addr) {
  for (int i = 0; i < call_value.args.len && i < 8; ++i) {
    auto arg_ptr = call_value.args.buffer[i];
    load_to_register(reinterpret_cast<koopa_raw_value_t>(arg_ptr), "a" + std::to_string(i));
  }
  bool call = false;
  int size = calculate_function_size(call_value.callee, call);
  size = (size + 15) / 16 * 16 * 2;
  for (int i = 8; i < call_value.args.len; ++i) {
    auto arg_ptr = call_value.args.buffer[i];
    load_to_register(reinterpret_cast<koopa_raw_value_t>(arg_ptr), "t0");
    store_to_stack((i - 8) * 4 - size, "t0");
  }
  output_file << "  call " + std::string(call_value.callee->name + 1) + "\n";
  if (addr != -1) store_to_stack(addr, "a0");
}

void RiscV::handle_global_alloc(const koopa_raw_value_t &global_alloc_value) {
  output_file << "\n  .global " + std::string(global_alloc_value->name + 1) + "\n";
  output_file << std::string(global_alloc_value->name + 1) + ":\n";
  if (global_alloc_value->kind.data.global_alloc.init->kind.tag == KOOPA_RVT_INTEGER) {
    output_file << "  .word " + std::to_string(global_alloc_value->kind.data.global_alloc.init->kind.data.integer.value) + "\n";
  } else if (global_alloc_value->kind.data.global_alloc.init->kind.tag == KOOPA_RVT_ZERO_INIT) {
    output_file << "  .zero " + std::to_string(calculate_type_size(global_alloc_value->ty->data.pointer.base)) + "\n";
  } else if (global_alloc_value->kind.data.global_alloc.init->kind.tag == KOOPA_RVT_AGGREGATE) {
    visit_aggregate(global_alloc_value->kind.data.global_alloc.init->kind.data.aggregate);
  }
}

void RiscV::visit_aggregate(const koopa_raw_aggregate_t &aggregate_value) {
  for (size_t i = 0; i < aggregate_value.elems.len; ++i) {
    auto elem_ptr = aggregate_value.elems.buffer[i];
    koopa_raw_value_t value = reinterpret_cast<koopa_raw_value_t>(elem_ptr);
    if (value->kind.tag == KOOPA_RVT_INTEGER) {
      output_file << "  .word " + std::to_string(value->kind.data.integer.value) + "\n";
    } else if (value->kind.tag == KOOPA_RVT_AGGREGATE) {
      visit_aggregate(value->kind.data.aggregate);
    } else {
      assert(false);
    }
  }
}

void RiscV::visit_get_elem_ptr(const koopa_raw_get_elem_ptr_t &gep_value, int addr) {
  if (gep_value.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC) {
    output_file << "  la t0, " + std::string(gep_value.src->name + 1) + "\n";
  } else {
    int src_addr = env.get_address(gep_value.src);
    if (src_addr != -1) {
      if (src_addr < 2048 && src_addr >= -2048) {
        output_file << "  addi t0, sp, " + std::to_string(src_addr) + "\n";
      } else {
        output_file << "  li t3, " + std::to_string(src_addr) + "\n";
        output_file << "  add t0, sp, t3\n";
      }
    } else {
      assert(false);
    }
    if (gep_value.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR || gep_value.src->kind.tag == KOOPA_RVT_GET_PTR) {
      output_file << "  lw t0, 0(t0)\n";
    }
  }
  load_to_register(gep_value.index, "t1");
  int size = calculate_array_size(gep_value.src->ty->data.pointer.base->data.array.base);
  output_file << "  li t2, " + std::to_string(size) + "\n";
  output_file << "  mul t1, t1, t2\n";
  output_file << "  add t0, t0, t1\n";
  store_to_stack(addr, "t0");
}

void RiscV::visit_get_ptr(const koopa_raw_get_ptr_t &gp_value, int addr) {
  int src_addr = env.get_address(gp_value.src);
  if (src_addr != -1) {
    if (src_addr < 2048 && src_addr >= -2048) {
      output_file << "  addi t0, sp, " + std::to_string(src_addr) + "\n";
    } else {
      output_file << "  li t3, " + std::to_string(src_addr) + "\n";
      output_file << "  add t0, sp, t3\n";
    }
  } else {
    assert(false);
  }
  output_file << "  lw t0, 0(t0)\n";
  load_to_register(gp_value.index, "t1");
  int size = calculate_array_size(gp_value.src->ty->data.pointer.base);
  output_file << "  li t2, " + std::to_string(size) + "\n";
  output_file << "  mul t1, t1, t2\n";
  output_file << "  add t0, t0, t1\n";
  store_to_stack(addr, "t0");
}

void RiscV::build(const std::string& ir) {
  koopa_program_t program;
  koopa_error_code_t ret = koopa_parse_from_string(ir.data(), &program);
  assert(ret == KOOPA_EC_SUCCESS);
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  koopa_delete_program(program);
  visit_raw_program(raw);
  output_file.close();
  koopa_delete_raw_program_builder(builder);
}
