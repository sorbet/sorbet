# typed: __STDLIB_INTERNAL

module ErrorHighlight
  def self.formatter; end
  def self.formatter=(formatter); end

  # Identify the code fragment that seems associated with a given error
  #
  # Arguments:
  #
  # ```
  # node: RubyVM::AbstractSyntaxTree::Node (script_lines should be enabled)
  # point_type: :name | :args
  # name: The name associated with the NameError/NoMethodError
  # ```
  #
  # Returns:
  #
  # ```ruby
  # {
  #   first_lineno: Integer,
  #   first_column: Integer,
  #   last_lineno: Integer,
  #   last_column: Integer,
  #   snippet: String,
  # } | nil
  # ```
  def self.spot(*_arg0, **_arg1, &_arg2); end
end

module ErrorHighlight::CoreExt
  def to_s; end
end

# This is a marker to let `DidYouMean::Correctable#original\_message` skip the
# following method definition of `to\_s`. See
# https://github.com/ruby/did\_you\_mean/pull/152
ErrorHighlight::CoreExt::SKIP_TO_S_FOR_SUPER_LOOKUP = T.let(T.unsafe(nil), TrueClass)

class ErrorHighlight::DefaultFormatter
  def self.message_for(spot); end
end

class ErrorHighlight::Spotter
  def initialize(node, point_type: T.unsafe(nil), name: T.unsafe(nil)); end

  def spot; end

  private

  def fetch_line(lineno); end

  # Example:
  #
  # ```
  # x.foo = 1
  #         ^
  # x[42] = 1
  #   ^^^^^^^
  # x[] = 1
  #   ^^^^^
  # ```
  def spot_attrasgn_for_args; end

  # Example:
  #
  # ```
  # x.foo = 1
  #  ^^^^^^
  # x[42] = 1
  #  ^^^^^^
  # ```
  def spot_attrasgn_for_name; end

  # Example:
  #
  # ```
  # x.foo(42)
  #       ^^
  # x[42]
  #   ^^
  # x += 1
  #      ^
  # ```
  def spot_call_for_args; end

  # Example:
  #
  # ```
  # x.foo
  #  ^^^^
  # x.foo(42)
  #  ^^^^
  # x&.foo
  #  ^^^^^
  # x[42]
  #  ^^^^
  # x += 1
  #   ^
  # ```
  def spot_call_for_name; end

  # Example:
  #
  # ```
  # Foo::Bar
  #    ^^^^^
  # ```
  def spot_colon2; end

  # Example:
  #
  # ```
  # foo(42)
  #     ^^
  # foo 42
  #     ^^
  # ```
  def spot_fcall_for_args; end

  # Example:
  #
  # ```
  # foo(42)
  # ^^^
  # foo 42
  # ^^^
  # ```
  def spot_fcall_for_name; end

  # Example:
  #
  # ```
  # x[1] += 42
  #   ^^^^^^^^
  # ```
  def spot_op_asgn1_for_args; end

  # Example:
  #
  # ```
  # x[1] += 42
  #  ^^^    (for [])
  # x[1] += 42
  #      ^  (for +)
  # x[1] += 42
  #  ^^^^^^ (for []=)
  # ```
  def spot_op_asgn1_for_name; end

  # Example:
  #
  # ```
  # x.foo += 42
  #          ^^
  # ```
  def spot_op_asgn2_for_args; end

  # Example:
  #
  # ```
  # x.foo += 42
  #  ^^^     (for foo)
  # x.foo += 42
  #       ^  (for +)
  # x.foo += 42
  #  ^^^^^^^ (for foo=)
  # ```
  def spot_op_asgn2_for_name; end

  # Example:
  #
  # ```
  # Foo::Bar += 1
  #    ^^^^^^^^
  # ```
  def spot_op_cdecl; end

  # Example:
  #
  # ```
  # x + 1
  #     ^
  # ```
  def spot_opcall_for_args; end

  # Example:
  #
  # ```
  # x + 1
  #   ^
  # +x
  # ^
  # ```
  def spot_opcall_for_name; end

  # Example:
  #
  # ```
  # foo
  # ^^^
  # ```
  def spot_vcall; end
end

class ErrorHighlight::Spotter::NonAscii < ::Exception; end
ErrorHighlight::VERSION = T.let(T.unsafe(nil), String)
