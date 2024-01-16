# typed: __STDLIB_INTERNAL

class Racc::Parser
  def _racc_do_parse_c(arg0, arg1); end
  def _racc_do_parse_rb(arg, in_debug); end
  def _racc_do_reduce(arg, act); end
  # common
  def _racc_evalact(act, arg); end
  def _racc_init_sysvars; end
  def _racc_setup; end
  def _racc_yyparse_c(arg0, arg1, arg2, arg3); end
  def _racc_yyparse_rb(recv, mid, arg, c_debug); end
  # The entry point of the parser. This method is used with
  # [`next_token`](https://docs.ruby-lang.org/en/2.6.0/Racc/Parser.html#method-i-next_token).
  # If [`Racc`](https://docs.ruby-lang.org/en/2.6.0/Racc.html) wants to get
  # token (and its value), calls next\_token.
  #
  # Example:
  #
  # ```ruby
  # def parse
  #   @q = [[1,1],
  #         [2,2],
  #         [3,3],
  #         [false, '$']]
  #   do_parse
  # end
  #
  # def next_token
  #   @q.shift
  # end
  # ```
  def do_parse; end
  # The method to fetch next token. If you use do\_parse method, you must
  # implement
  # [`next_token`](https://docs.ruby-lang.org/en/2.7.0/Racc/Parser.html#method-i-next_token).
  #
  # The format of return value is [TOKEN\_SYMBOL, VALUE]. `token-symbol` is
  # represented by Ruby's symbol by default, e.g. :IDENT for 'IDENT'. ";"
  # ([`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)) for ';'.
  #
  # The final symbol (End of file) must be false.
  def next_token; end
  # This method is called when a parse error is found.
  #
  # ERROR\_TOKEN\_ID is an internal ID of token which caused error. You can get
  # string representation of this ID by calling
  # [`token_to_str`](https://docs.ruby-lang.org/en/2.7.0/Racc/Parser.html#method-i-token_to_str).
  #
  # ERROR\_VALUE is a value of error token.
  #
  # value\_stack is a stack of symbol values. DO NOT MODIFY this object.
  #
  # This method raises ParseError by default.
  #
  # If this method returns, parsers enter "error recovering mode".
  def on_error(t, val, vstack); end
  def racc_accept; end
  def racc_e_pop(state, tstack, vstack); end
  def racc_next_state(curstate, state); end
  def racc_print_stacks(t, v); end
  def racc_print_states(s); end
  # For debugging output
  def racc_read_token(t, tok, val); end
  def racc_reduce(toks, sim, tstack, vstack); end
  def racc_shift(tok, tstack, vstack); end
  def racc_token2str(tok); end
  def self.racc_runtime_type; end
  # Convert internal ID of token symbol to the string.
  def token_to_str(t); end
  # Exit parser. Return value is [Symbol\_Value\_Stack](0).
  def yyaccept; end
  # Leave error recovering mode.
  def yyerrok; end
  # Enter error recovering mode. This method does not call
  # [`on_error`](https://docs.ruby-lang.org/en/2.7.0/Racc/Parser.html#method-i-on_error).
  def yyerror; end
  # Another entry point for the parser. If you use this method, you must
  # implement RECEIVER#METHOD\_ID method.
  #
  # RECEIVER#METHOD\_ID is a method to get next token. It must 'yield' the
  # token, which format is [TOKEN-SYMBOL, VALUE].
  def yyparse(recv, mid); end
end

# $Id: 74ff4369ce53c7f45cfc2644ce907785104ebf6e $
#
# [`Copyright`](https://docs.ruby-lang.org/en/2.7.0/Racc.html#Copyright) (c)
# 1999-2006 Minero Aoki
#
# This program is free software. You can distribute/modify this program under
# the terms of the GNU LGPL, Lesser General Public License version 2.1. For
# details of LGPL, see the file "COPYING".
# $Id: ebb9798ad0b211e031670a12a1ab154678c1c8f3 $
#
# [`Copyright`](https://docs.ruby-lang.org/en/2.7.0/Racc.html#Copyright) (c)
# 1999-2006 Minero Aoki
#
# This program is free software. You can distribute/modify this program under
# the same terms of ruby. see the file "COPYING".
# $Id: 8ab2cb5341529fe5e35956bb1a1f42ec9b9c6f5a $
#
# [`Copyright`](https://docs.ruby-lang.org/en/2.7.0/Racc.html#Copyright) (c)
# 1999-2006 Minero Aoki
#
# This program is free software. You can distribute/modify this program under
# the same terms of ruby. see the file "COPYING".
# $Id: 31aa4331c08dfd4609c06eb5f94b7ef38dc708e1 $
#
# [`Copyright`](https://docs.ruby-lang.org/en/2.7.0/Racc.html#Copyright) (c)
# 1999-2006 Minero Aoki
#
# This program is free software. You can distribute/modify this program under
# the terms of the GNU LGPL, Lesser General Public License version 2.1. For
# details of the GNU LGPL, see the file "COPYING".
# $Id: 5e9d0a01b5d56fd9cdc3d5cb078b1a3e1bbaf779 $
#
# [`Copyright`](https://docs.ruby-lang.org/en/2.7.0/Racc.html#Copyright) (c)
# 1999-2006 Minero Aoki
#
# This program is free software. You can distribute/modify this program under
# the terms of the GNU LGPL, Lesser General Public License version 2.1. For
# details of the GNU LGPL, see the file "COPYING".
# [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) is a LALR(1) parser
# generator. It is written in Ruby itself, and generates Ruby programs.
#
# ## Command-line Reference
#
# ```
# racc [-o<var>filename</var>] [--output-file=<var>filename</var>]
#      [-e<var>rubypath</var>] [--embedded=<var>rubypath</var>]
#      [-v] [--verbose]
#      [-O<var>filename</var>] [--log-file=<var>filename</var>]
#      [-g] [--debug]
#      [-E] [--embedded]
#      [-l] [--no-line-convert]
#      [-c] [--line-convert-all]
#      [-a] [--no-omit-actions]
#      [-C] [--check-only]
#      [-S] [--output-status]
#      [--version] [--copyright] [--help] <var>grammarfile</var>
# ```
#
# `filename`
# :   [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) grammar file. Any
#     extension is permitted.
# -o+outfile+, --output-file=`outfile`
# :   A filename for output. default is <`filename`>.tab.rb
# -O+filename+, --log-file=`filename`
# :   Place logging output in file `filename`. Default log file name is
#     <`filename`>.output.
# -e+rubypath+, --executable=`rubypath`
# :   output executable file(mode 755). where `path` is the Ruby interpreter.
# -v, --verbose
# :   verbose mode. create `filename`.output file, like yacc's y.output file.
# -g, --debug
# :   add debug code to parser class. To display debugging information, use this
#     '-g' option and set @yydebug true in parser class.
# -E, --embedded
# :   Output parser which doesn't need runtime files (racc/parser.rb).
# -C, --check-only
# :   Check syntax of racc grammar file and quit.
# -S, --output-status
# :   Print messages time to time while compiling.
# -l, --no-line-convert
# :   turns off line number converting.
# -c, --line-convert-all
# :   Convert line number of actions, inner, header and footer.
# -a, --no-omit-actions
# :   Call all actions, even if an action is empty.
# --version
# :   print [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) version and
#     quit.
# --copyright
# :   Print copyright and quit.
# --help
# :   Print usage and quit.
#
#
# ## Generating Parser Using [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html)
#
# To compile [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) grammar
# file, simply type:
#
# ```
# $ racc parse.y
# ```
#
# This creates Ruby script file "parse.tab.y". The -o option can change the
# output filename.
#
# ## Writing A [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) Grammar [`File`](https://docs.ruby-lang.org/en/2.7.0/File.html)
#
# If you want your own parser, you have to write a grammar file. A grammar file
# contains the name of your parser class, grammar for the parser, user code, and
# anything else. When writing a grammar file, yacc's knowledge is helpful. If
# you have not used yacc before,
# [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) is not too difficult.
#
# Here's an example [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html)
# grammar file.
#
# ```
# class Calcparser
# rule
#   target: exp { print val[0] }
#
#   exp: exp '+' exp
#      | exp '*' exp
#      | '(' exp ')'
#      | NUMBER
# end
# ```
#
# [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) grammar files resemble
# yacc files. But (of course), this is Ruby code. yacc's $$ is the 'result', $0,
# $1... is an array called 'val', and $-1, $-2... is an array called '\_values'.
#
# See the [Grammar File
# Reference](https://docs.ruby-lang.org/en/2.7.0/lib/racc/rdoc/grammar_en_rdoc.html)
# for more information on grammar files.
#
# ## Parser
#
# Then you must prepare the parse entry method. There are two types of parse
# methods in [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html),
# Racc::Parser#do\_parse and Racc::Parser#yyparse
#
# Racc::Parser#do\_parse is simple.
#
# It's yyparse() of yacc, and
# [`Racc::Parser#next_token`](https://docs.ruby-lang.org/en/2.7.0/Racc/Parser.html#method-i-next_token)
# is yylex(). This method must returns an array like [TOKENSYMBOL, ITS\_VALUE].
# EOF is [false, false]. (TOKENSYMBOL is a Ruby symbol (taken from
# [`String#intern`](https://docs.ruby-lang.org/en/2.7.0/String.html#method-i-intern))
# by default. If you want to change this, see the grammar reference.
#
# Racc::Parser#yyparse is little complicated, but useful. It does not use
# [`Racc::Parser#next_token`](https://docs.ruby-lang.org/en/2.7.0/Racc/Parser.html#method-i-next_token),
# instead it gets tokens from any iterator.
#
# For example, `yyparse(obj, :scan)` causes calling +obj#scan+, and you can
# return tokens by yielding them from +obj#scan+.
#
# ## Debugging
#
# When debugging, "-v" or/and the "-g" option is helpful.
#
# "-v" creates verbose log file (.output). "-g" creates a "Verbose Parser".
# Verbose Parser prints the internal status when parsing. But it's *not*
# automatic. You must use -g option and set +@yydebug+ to `true` in order to get
# output. -g option only creates the verbose parser.
#
# ### [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) reported syntax error.
#
# Isn't there too many "end"? grammar of racc file is changed in v0.10.
#
# [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) does not use '%' mark,
# while yacc uses huge number of '%' marks..
#
# ### [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) reported "XXXX conflicts".
#
# Try "racc -v xxxx.y". It causes producing racc's internal log file,
# xxxx.output.
#
# ### Generated parsers does not work correctly
#
# Try "racc -g xxxx.y". This command let racc generate "debugging parser". Then
# set @yydebug=true in your parser. It produces a working log of your parser.
#
# ## Re-distributing [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) runtime
#
# A parser, which is created by
# [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html), requires the
# [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) runtime module;
# racc/parser.rb.
#
# Ruby 1.8.x comes with [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html)
# runtime module, you need NOT distribute
# [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) runtime files.
#
# If you want to include the
# [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) runtime module with
# your parser. This can be done by using '-E' option:
#
# ```
# $ racc -E -omyparser.rb myparser.y
# ```
#
# This command creates myparser.rb which 'includes'
# [`Racc`](https://docs.ruby-lang.org/en/2.7.0/Racc.html) runtime. Only you must
# do is to distribute your parser file (myparser.rb).
#
# Note: parser.rb is ruby license, but your parser is not. Your own parser is
# completely yours.
# $Id: 3b2d89d9ada2f5fcb043837dcc5c9631856d5b70 $
#
# [`Copyright`](https://docs.ruby-lang.org/en/2.7.0/Racc.html#Copyright) (c)
# 1999-2006 Minero Aoki
#
# This program is free software. You can distribute/modify this program under
# the terms of the GNU LGPL, Lesser General Public License version 2.1. For
# details of LGPL, see the file "COPYING".
module Racc
end

class Racc::ParseError < StandardError
end
