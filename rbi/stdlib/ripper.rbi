# typed: __STDLIB_INTERNAL

# [`Ripper`](https://docs.ruby-lang.org/en/2.6.0/Ripper.html) is a Ruby script
# parser.
#
# You can get information from the parser with event-based style. Information
# such as abstract syntax trees or simple lexical analysis of the Ruby program.
#
# ## Usage
#
# [`Ripper`](https://docs.ruby-lang.org/en/2.6.0/Ripper.html) provides an easy
# interface for parsing your program into a symbolic expression tree (or
# S-expression).
#
# Understanding the output of the parser may come as a challenge, it's
# recommended you use [`PP`](https://docs.ruby-lang.org/en/2.6.0/PP.html) to
# format the output for legibility.
#
# ```
# require 'ripper'
# require 'pp'
#
# pp Ripper.sexp('def hello(world) "Hello, #{world}!"; end')
#   #=> [:program,
#        [[:def,
#          [:@ident, "hello", [1, 4]],
#          [:paren,
#           [:params, [[:@ident, "world", [1, 10]]], nil, nil, nil, nil, nil, nil]],
#          [:bodystmt,
#           [[:string_literal,
#             [:string_content,
#              [:@tstring_content, "Hello, ", [1, 18]],
#              [:string_embexpr, [[:var_ref, [:@ident, "world", [1, 27]]]]],
#              [:@tstring_content, "!", [1, 33]]]]],
#           nil,
#           nil,
#           nil]]]]
# ```
#
# You can see in the example above, the expression starts with `:program`.
#
# From here, a method definition at `:def`, followed by the method's identifier
# `:@ident`. After the method's identifier comes the parentheses `:paren` and
# the method parameters under `:params`.
#
# Next is the method body, starting at `:bodystmt` (`stmt` meaning statement),
# which contains the full definition of the method.
#
# In our case, we're simply returning a
# [`String`](https://docs.ruby-lang.org/en/2.6.0/String.html), so next we have
# the `:string_literal` expression.
#
# Within our `:string_literal` you'll notice two `@tstring_content`, this is the
# literal part for `Hello, ` and `!`. Between the two `@tstring_content`
# statements is a `:string_embexpr`, where *embexpr* is an embedded expression.
# Our expression consists of a local variable, or `var_ref`, with the identifier
# (`@ident`) of `world`.
#
# ## Resources
#
# *   [Ruby
#     Inside](http://www.rubyinside.com/using-ripper-to-see-how-ruby-is-parsing-your-code-5270.html)
#
#
# ## Requirements
#
# *   ruby 1.9 (support CVS HEAD only)
# *   bison 1.28 or later (Other yaccs do not work)
#
#
# ## License
#
# Ruby License.
#
# *   Minero Aoki
# *   aamine@loveruby.net
# *   http://i.loveruby.net
class Ripper
  # EXPERIMENTAL
  # :   Parses `src` and create S-exp tree. Returns more readable tree rather
  #     than
  #     [`Ripper.sexp_raw`](https://docs.ruby-lang.org/en/2.6.0/Ripper.html#method-c-sexp_raw).
  #     This method is mainly for developer use.
  #
  # ```
  # require 'ripper'
  # require 'pp'
  #
  # pp Ripper.sexp("def m(a) nil end")
  #   #=> [:program,
  #        [[:def,
  #         [:@ident, "m", [1, 4]],
  #         [:paren, [:params, [[:@ident, "a", [1, 6]]], nil, nil, nil, nil]],
  #         [:bodystmt, [[:var_ref, [:@kw, "nil", [1, 9]]]], nil, nil, nil]]]]
  # ```
  sig {params(src: String, filename: String, lineno: Integer).returns(T::Array[T.untyped])}
  def self.sexp(src, filename = "-", lineno = 1); end

  # EXPERIMENTAL
  # :   Parses `src` and create S-exp tree. This method is mainly for developer
  #     use.
  #
  # ```
  # require 'ripper'
  # require 'pp'
  #
  # pp Ripper.sexp_raw("def m(a) nil end")
  #   #=> [:program,
  #        [:stmts_add,
  #         [:stmts_new],
  #         [:def,
  #          [:@ident, "m", [1, 4]],
  #          [:paren, [:params, [[:@ident, "a", [1, 6]]], nil, nil, nil]],
  #          [:bodystmt,
  #           [:stmts_add, [:stmts_new], [:var_ref, [:@kw, "nil", [1, 9]]]],
  #           nil,
  #           nil,
  #           nil]]]]
  # ```
  sig {params(src: String, filename: String, lineno: Integer).returns(T::Array[T.untyped])}
  def self.sexp_raw(src, filename = "-", lineno = 1); end

  # EXPERIMENTAL
  # :   Parses `src` and return a string which was matched to `pattern`.
  #     `pattern` should be described as
  #     [`Regexp`](https://docs.ruby-lang.org/en/2.6.0/Regexp.html).
  #
  # ```ruby
  # require 'ripper'
  #
  # p Ripper.slice('def m(a) nil end', 'ident')                   #=> "m"
  # p Ripper.slice('def m(a) nil end', '[ident lparen rparen]+')  #=> "m(a)"
  # p Ripper.slice("<<EOS\nstring\nEOS",
  #                'heredoc_beg nl $(tstring_content*) heredoc_end', 1)
  #     #=> "string\n"
  # ```
  sig {params(src: String, pattern: String, n: Integer).returns(String)}
  def self.slice(src, pattern, n = 0); end

  # Tokenizes the Ruby program and returns an array of strings.
  #
  # ```ruby
  # p Ripper.tokenize("def m(a) nil end")
  #    # => ["def", " ", "m", "(", "a", ")", " ", "nil", " ", "end"]
  # ```
  sig {params(src: String, filename: String, lineno: Integer).returns(T::Array[String])}
  def self.tokenize(src, filename = "-", lineno = 1); end
end
