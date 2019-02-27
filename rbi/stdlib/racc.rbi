# typed: strict
class Racc::Parser
  def _racc_do_parse_c(arg0, arg1); end
  def _racc_do_parse_rb(arg, in_debug); end
  def _racc_do_reduce(arg, act); end
  def _racc_evalact(act, arg); end
  def _racc_init_sysvars; end
  def _racc_setup; end
  def _racc_yyparse_c(arg0, arg1, arg2, arg3); end
  def _racc_yyparse_rb(recv, mid, arg, c_debug); end
  def do_parse; end
  def next_token; end
  def on_error(t, val, vstack); end
  def racc_accept; end
  def racc_e_pop(state, tstack, vstack); end
  def racc_next_state(curstate, state); end
  def racc_print_stacks(t, v); end
  def racc_print_states(s); end
  def racc_read_token(t, tok, val); end
  def racc_reduce(toks, sim, tstack, vstack); end
  def racc_shift(tok, tstack, vstack); end
  def racc_token2str(tok); end
  def self.racc_runtime_type; end
  def token_to_str(t); end
  def yyaccept; end
  def yyerrok; end
  def yyerror; end
  def yyparse(recv, mid); end
end

module Racc
end

class Racc::ParseError < StandardError
end
