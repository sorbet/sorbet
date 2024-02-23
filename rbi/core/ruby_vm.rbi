# typed: __STDLIB_INTERNAL

# for ast.c
# The [`RubyVM`](https://docs.ruby-lang.org/en/2.7.0/RubyVM.html) module only
# exists on MRI. `RubyVM` is not defined in other Ruby implementations such as
# JRuby and TruffleRuby.
#
# The [`RubyVM`](https://docs.ruby-lang.org/en/2.7.0/RubyVM.html) module
# provides some access to MRI internals. This module is for very limited
# purposes, such as debugging, prototyping, and research. Normal users must not
# use it. This module is not portable between Ruby implementations.
class RubyVM < Object
  # [`DEFAULT_PARAMS`](https://docs.ruby-lang.org/en/2.7.0/RubyVM.html#DEFAULT_PARAMS)
  # This constant exposes the VM's default parameters. Note that changing these
  # values does not affect VM execution. Specification is not stable and you
  # should not depend on this value. Of course, this constant is MRI specific.
  DEFAULT_PARAMS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  # [`INSTRUCTION_NAMES`](https://docs.ruby-lang.org/en/2.7.0/RubyVM.html#INSTRUCTION_NAMES)
  # A list of bytecode instruction names in MRI. This constant is MRI specific.
  INSTRUCTION_NAMES = T.let(T.unsafe(nil), T::Array[T.untyped])
  # [`OPTS`](https://docs.ruby-lang.org/en/2.7.0/RubyVM.html#OPTS) An
  # [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) of VM build
  # options. This constant is MRI specific.
  OPTS = T.let(T.unsafe(nil), T::Array[T.untyped])

  # Returns a [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) containing
  # implementation-dependent counters inside the VM.
  #
  # This hash includes information about method/constant cache serials:
  #
  # ```ruby
  # {
  #   :global_method_state=>251,
  #   :global_constant_state=>481,
  #   :class_serial=>9029
  # }
  # ```
  #
  # The contents of the hash are implementation specific and may be changed in
  # the future.
  #
  # This method is only expected to work on C Ruby.
  def self.stat(*_); end
end

# [`AbstractSyntaxTree`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/AbstractSyntaxTree.html)
# provides methods to parse Ruby code into abstract syntax trees. The nodes in
# the tree are instances of
# [`RubyVM::AbstractSyntaxTree::Node`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/AbstractSyntaxTree/Node.html).
#
# This class is MRI specific as it exposes implementation details of the MRI
# abstract syntax tree.
#
# This class is experimental and its API is not stable, therefore it might
# change without notice. As examples, the order of children nodes is not
# guaranteed, the number of children nodes might change, there is no way to
# access children nodes by name, etc.
#
# If you are looking for a stable API or an API working under multiple Ruby
# implementations, consider using the *parser* gem or
# [`Ripper`](https://docs.ruby-lang.org/en/2.7.0/Ripper.html). If you would like
# to make
# [`RubyVM::AbstractSyntaxTree`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/AbstractSyntaxTree.html)
# stable, please join the discussion at https://bugs.ruby-lang.org/issues/14844.
module RubyVM::AbstractSyntaxTree
  # Returns AST nodes of the given *proc* or *method*.
  #
  # ```ruby
  # RubyVM::AbstractSyntaxTree.of(proc {1 + 2})
  # # => #<RubyVM::AbstractSyntaxTree::Node:SCOPE@1:35-1:42>
  #
  # def hello
  #   puts "hello, world"
  # end
  #
  # RubyVM::AbstractSyntaxTree.of(method(:hello))
  # # => #<RubyVM::AbstractSyntaxTree::Node:SCOPE@1:0-3:3>
  # ```
  #
  # See [::parse](https://docs.ruby-lang.org/en/3.2/RubyVM/AbstractSyntaxTree.html#method-c-parse)
  # for explanation of keyword argument meaning and usage.
  sig do
    params(
      arg: T.any(T::proc.void, Method, Thread::Backtrace::Location),
      keep_script_lines: T::Boolean,
      error_tolerant: T::Boolean,
      keep_tokens: T::Boolean,
    )
    .returns(T.nilable(RubyVM::AbstractSyntaxTree::Node))
  end
  def self.of(arg, keep_script_lines: false, error_tolerant: false, keep_tokens: false); end

  # Parses the given *string* into an abstract syntax tree, returning the root
  # node of that tree.
  #
  # ```ruby
  # RubyVM::AbstractSyntaxTree.parse("x = 1 + 2")
  # # => #<RubyVM::AbstractSyntaxTree::Node:SCOPE@1:0-1:9>
  # ```
  # If `keep_script_lines: true` option is provided, the text of the parsed
  # source is associated with nodes and is available via
  # [Node#script_lines](https://docs.ruby-lang.org/en/3.2/RubyVM/AbstractSyntaxTree/Node.html#method-i-script_lines).
  #
  # If `keep_tokens: true` option is provided,
  # [Node#tokens](https://docs.ruby-lang.org/en/3.2/RubyVM/AbstractSyntaxTree/Node.html#method-i-tokens)
  # are populated.
  #
  # [`SyntaxError`](https://docs.ruby-lang.org/en/2.7.0/SyntaxError.html) is
  # raised if the given *string* is invalid syntax. To overwrite this behavior,
  # `error_tolerant: true`` can be provided. In this case, the parser will
  # produce a tree where expressions with syntax errors would be represented by
  # [Node](https://docs.ruby-lang.org/en/3.2/RubyVM/AbstractSyntaxTree/Node.html)
  # with `type=:ERROR`.
  #
  # ```ruby
  # root = RubyVM::AbstractSyntaxTree.parse("x = 1; p(x; y=2")
  # # <internal:ast>:33:in `parse': syntax error, unexpected ';', expecting ')' (SyntaxError)
  # # x = 1; p(x; y=2
  # #           ^
  #
  # root = RubyVM::AbstractSyntaxTree.parse("x = 1; p(x; y=2", error_tolerant: true)
  # # (SCOPE@1:0-1:15
  # #  tbl: [:x, :y]
  # #  args: nil
  # #  body: (BLOCK@1:0-1:15 (LASGN@1:0-1:5 :x (LIT@1:4-1:5 1)) (ERROR@1:7-1:11) (LASGN@1:12-1:15 :y (LIT@1:14-1:15 2))))
  # root.children.last.children
  # # [(LASGN@1:0-1:5 :x (LIT@1:4-1:5 1)),
  # #  (ERROR@1:7-1:11),
  # #  (LASGN@1:12-1:15 :y (LIT@1:14-1:15 2))]
  # ```
  sig do
    params(
      string: String,
      keep_script_lines: T::Boolean,
      error_tolerant: T::Boolean,
      keep_tokens: T::Boolean,
    )
    .returns(RubyVM::AbstractSyntaxTree::Node)
  end
  def self.parse(string, keep_script_lines: false, error_tolerant: false, keep_tokens: false); end

  # Reads the file from *pathname*, then parses it like
  # [`::parse`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/AbstractSyntaxTree.html#method-c-parse),
  # returning the root node of the abstract syntax tree.
  #
  # [`SyntaxError`](https://docs.ruby-lang.org/en/2.7.0/SyntaxError.html) is
  # raised if *pathname*'s contents are not valid Ruby syntax.
  #
  # ```ruby
  # RubyVM::AbstractSyntaxTree.parse_file("my-app/app.rb")
  # # => #<RubyVM::AbstractSyntaxTree::Node:SCOPE@1:0-31:3>
  # ```
  #
  # See [::parse](https://docs.ruby-lang.org/en/3.2/RubyVM/AbstractSyntaxTree.html#method-c-parse)
  # for explanation of keyword argument meaning and usage.
  sig do
    params(
      pathname: String,
      keep_script_lines: T::Boolean,
      error_tolerant: T::Boolean,
      keep_tokens: T::Boolean,
    )
    .returns(RubyVM::AbstractSyntaxTree::Node)
  end
  def self.parse_file(pathname, keep_script_lines: false, error_tolerant: false, keep_tokens: false); end
end

# [`RubyVM::AbstractSyntaxTree::Node`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/AbstractSyntaxTree/Node.html)
# instances are created by parse methods in
# [`RubyVM::AbstractSyntaxTree`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/AbstractSyntaxTree.html).
#
# This class is MRI specific.
class RubyVM::AbstractSyntaxTree::Node
  # Returns AST nodes under this one. Each kind of node has different children,
  # depending on what kind of node it is.
  #
  # The returned array may contain other nodes or `nil`.
  sig { returns(T::Array[T.nilable(RubyVM::AbstractSyntaxTree::Node)]) }
  def children; end

  # The column number in the source code where this AST's text began.
  sig { returns(Integer) }
  def first_column; end

  # The line number in the source code where this AST's text began.
  sig { returns(Integer) }
  def first_lineno; end

  # The column number in the source code where this AST's text ended.
  sig { returns(Integer) }
  def last_column; end

  # The line number in the source code where this AST's text ended.
  sig { returns(Integer) }
  def last_lineno; end

  # Returns the type of this node as a symbol.
  #
  # ```ruby
  # root = RubyVM::AbstractSyntaxTree.parse("x = 1 + 2")
  # root.type # => :SCOPE
  # call = root.children[2]
  # call.type # => :OPCALL
  # ```
  sig { returns(Symbol) }
  def type; end
end

# The
# [`InstructionSequence`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html)
# class represents a compiled sequence of instructions for the Virtual Machine
# used in MRI. Not all implementations of Ruby may implement this class, and for
# the implementations that implement it, the methods defined and behavior of the
# methods can change in any version.
#
# With it, you can get a handle to the instructions that make up a method or a
# proc, compile strings of Ruby code down to VM instructions, and disassemble
# instruction sequences to strings for easy inspection. It is mostly useful if
# you want to learn how YARV works, but it also lets you control various
# settings for the Ruby iseq compiler.
#
# You can find the source for the VM instructions in `insns.def` in the Ruby
# source.
#
# The instruction sequence results will almost certainly change as Ruby changes,
# so example output in this documentation may be different from what you see.
#
# Of course, this class is MRI specific.
class RubyVM::InstructionSequence < Object
  # Returns the absolute path of this instruction sequence.
  #
  # `nil` if the iseq was evaluated from a string.
  #
  # For example, using
  # [`::compile_file`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-compile_file):
  #
  # ```
  # # /tmp/method.rb
  # def hello
  #   puts "hello, world"
  # end
  #
  # # in irb
  # > iseq = RubyVM::InstructionSequence.compile_file('/tmp/method.rb')
  # > iseq.absolute_path #=> /tmp/method.rb
  # ```
  def absolute_path; end

  # Returns the base label of this instruction sequence.
  #
  # For example, using irb:
  #
  # ```ruby
  # iseq = RubyVM::InstructionSequence.compile('num = 1 + 2')
  # #=> <RubyVM::InstructionSequence:<compiled>@<compiled>>
  # iseq.base_label
  # #=> "<compiled>"
  # ```
  #
  # Using
  # [`::compile_file`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-compile_file):
  #
  # ```
  # # /tmp/method.rb
  # def hello
  #   puts "hello, world"
  # end
  #
  # # in irb
  # > iseq = RubyVM::InstructionSequence.compile_file('/tmp/method.rb')
  # > iseq.base_label #=> <main>
  # ```
  def base_label; end

  # Returns the instruction sequence as a `String` in human readable form.
  #
  # ```ruby
  # puts RubyVM::InstructionSequence.compile('1 + 2').disasm
  # ```
  #
  # Produces:
  #
  # ```
  # == disasm: <RubyVM::InstructionSequence:<compiled>@<compiled>>==========
  # 0000 trace            1                                               (   1)
  # 0002 putobject        1
  # 0004 putobject        2
  # 0006 opt_plus         <ic:1>
  # 0008 leave
  # ```
  def disasm; end

  # Returns the instruction sequence as a `String` in human readable form.
  #
  # ```ruby
  # puts RubyVM::InstructionSequence.compile('1 + 2').disasm
  # ```
  #
  # Produces:
  #
  # ```
  # == disasm: <RubyVM::InstructionSequence:<compiled>@<compiled>>==========
  # 0000 trace            1                                               (   1)
  # 0002 putobject        1
  # 0004 putobject        2
  # 0006 opt_plus         <ic:1>
  # 0008 leave
  # ```
  def disassemble; end

  # Iterate all direct child instruction sequences. Iteration order is
  # implementation/version defined so that people should not rely on the order.
  def each_child; end

  # Evaluates the instruction sequence and returns the result.
  #
  # ```ruby
  # RubyVM::InstructionSequence.compile("1 + 2").eval #=> 3
  # ```
  def eval; end

  # Returns the number of the first source line where the instruction sequence
  # was loaded from.
  #
  # For example, using irb:
  #
  # ```ruby
  # iseq = RubyVM::InstructionSequence.compile('num = 1 + 2')
  # #=> <RubyVM::InstructionSequence:<compiled>@<compiled>>
  # iseq.first_lineno
  # #=> 1
  # ```
  def first_lineno; end

  # Returns a human-readable string representation of this instruction sequence,
  # including the
  # [`label`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-i-label)
  # and
  # [`path`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-i-path).
  def inspect; end

  # Returns the label of this instruction sequence.
  #
  # `<main>` if it's at the top level, `<compiled>` if it was evaluated from a
  # string.
  #
  # For example, using irb:
  #
  # ```ruby
  # iseq = RubyVM::InstructionSequence.compile('num = 1 + 2')
  # #=> <RubyVM::InstructionSequence:<compiled>@<compiled>>
  # iseq.label
  # #=> "<compiled>"
  # ```
  #
  # Using
  # [`::compile_file`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-compile_file):
  #
  # ```
  # # /tmp/method.rb
  # def hello
  #   puts "hello, world"
  # end
  #
  # # in irb
  # > iseq = RubyVM::InstructionSequence.compile_file('/tmp/method.rb')
  # > iseq.label #=> <main>
  # ```
  def label; end

  # Returns the path of this instruction sequence.
  #
  # `<compiled>` if the iseq was evaluated from a string.
  #
  # For example, using irb:
  #
  # ```ruby
  # iseq = RubyVM::InstructionSequence.compile('num = 1 + 2')
  # #=> <RubyVM::InstructionSequence:<compiled>@<compiled>>
  # iseq.path
  # #=> "<compiled>"
  # ```
  #
  # Using
  # [`::compile_file`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-compile_file):
  #
  # ```
  # # /tmp/method.rb
  # def hello
  #   puts "hello, world"
  # end
  #
  # # in irb
  # > iseq = RubyVM::InstructionSequence.compile_file('/tmp/method.rb')
  # > iseq.path #=> /tmp/method.rb
  # ```
  def path; end

  # Returns an [`Array`](https://docs.ruby-lang.org/en/2.7.0/Array.html) with 14
  # elements representing the instruction sequence with the following data:
  #
  # magic
  # :   A string identifying the data format. **Always
  #     `YARVInstructionSequence/SimpleDataFormat`.**
  #
  # major\_version
  # :   The major version of the instruction sequence.
  #
  # minor\_version
  # :   The minor version of the instruction sequence.
  #
  # format\_type
  # :   A number identifying the data format. **Always 1**.
  #
  # misc
  # :   A hash containing:
  #
  #     `:arg_size`
  # :       the total number of arguments taken by the method or the block (0 if
  #         *iseq* doesn't represent a method or block)
  #     `:local_size`
  # :       the number of local variables + 1
  #     `:stack_max`
  # :       used in calculating the stack depth at which a
  #         [`SystemStackError`](https://docs.ruby-lang.org/en/2.7.0/SystemStackError.html)
  #         is thrown.
  #
  #
  # [`label`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-i-label)
  # :   The name of the context (block, method, class, module, etc.) that this
  #     instruction sequence belongs to.
  #
  #     `<main>` if it's at the top level, `<compiled>` if it was evaluated from
  #     a string.
  #
  # [`path`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-i-path)
  # :   The relative path to the Ruby file where the instruction sequence was
  #     loaded from.
  #
  #     `<compiled>` if the iseq was evaluated from a string.
  #
  # [`absolute_path`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-i-absolute_path)
  # :   The absolute path to the Ruby file where the instruction sequence was
  #     loaded from.
  #
  #     `nil` if the iseq was evaluated from a string.
  #
  # [`first_lineno`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-i-first_lineno)
  # :   The number of the first source line where the instruction sequence was
  #     loaded from.
  #
  # type
  # :   The type of the instruction sequence.
  #
  #     Valid values are `:top`, `:method`, `:block`, `:class`, `:rescue`,
  #     `:ensure`, `:eval`, `:main`, and `plain`.
  #
  # locals
  # :   An array containing the names of all arguments and local variables as
  #     symbols.
  #
  # params
  # :   An [`Hash`](https://docs.ruby-lang.org/en/2.7.0/Hash.html) object
  #     containing parameter information.
  #
  #     More info about these values can be found in `vm_core.h`.
  #
  # catch\_table
  # :   A list of exceptions and control flow operators (rescue, next, redo,
  #     break, etc.).
  #
  # bytecode
  # :   An array of arrays containing the instruction names and operands that
  #     make up the body of the instruction sequence.
  #
  #
  # Note that this format is MRI specific and version dependent.
  def to_a; end

  # Returns serialized iseq binary format data as a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) object. A
  # corresponding iseq object is created by
  # [`RubyVM::InstructionSequence.load_from_binary()`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-load_from_binary)
  # method.
  #
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) extra\_data will
  # be saved with binary data. You can access this data with
  # [`RubyVM::InstructionSequence.load_from_binary_extra_data(binary)`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-load_from_binary_extra_data).
  #
  # Note that the translated binary data is not portable. You can not move this
  # binary data to another machine. You can not use the binary data which is
  # created by another version/another architecture of Ruby.
  def to_binary(*_); end

  # Return trace points in the instruction sequence. Return an array of [line,
  # event\_symbol] pair.
  def trace_points; end

  # Takes `source`, a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) of Ruby code and
  # compiles it to an
  # [`InstructionSequence`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html).
  #
  # Optionally takes `file`, `path`, and `line` which describe the file path,
  # real path and first line number of the ruby code in `source` which are
  # metadata attached to the returned `iseq`.
  #
  # `file` is used for `\_\_FILE\_\_` and exception backtrace. `path` is used
  # for `require_relative` base. It is recommended these should be the same full
  # path.
  #
  # `options`, which can be `true`, `false` or a `Hash`, is used to modify the
  # default behavior of the Ruby iseq compiler.
  #
  # For details regarding valid compile options see
  # [`::compile_option=`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-compile_option-3D).
  #
  # ```ruby
  # RubyVM::InstructionSequence.compile("a = 1 + 2")
  # #=> <RubyVM::InstructionSequence:<compiled>@<compiled>>
  #
  # path = "test.rb"
  # RubyVM::InstructionSequence.compile(File.read(path), path, File.expand_path(path))
  # #=> <RubyVM::InstructionSequence:<compiled>@test.rb:1>
  #
  # path = File.expand_path("test.rb")
  # RubyVM::InstructionSequence.compile(File.read(path), path, path)
  # #=> <RubyVM::InstructionSequence:<compiled>@/absolute/path/to/test.rb:1>
  # ```
  def self.compile(*_); end

  # Takes `file`, a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html)
  # with the location of a Ruby source file, reads, parses and compiles the
  # file, and returns `iseq`, the compiled
  # [`InstructionSequence`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html)
  # with source location metadata set.
  #
  # Optionally takes `options`, which can be `true`, `false` or a `Hash`, to
  # modify the default behavior of the Ruby iseq compiler.
  #
  # For details regarding valid compile options see
  # [`::compile_option=`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-compile_option-3D).
  #
  # ```ruby
  # # /tmp/hello.rb
  # puts "Hello, world!"
  #
  # # elsewhere
  # RubyVM::InstructionSequence.compile_file("/tmp/hello.rb")
  # #=> <RubyVM::InstructionSequence:<main>@/tmp/hello.rb>
  # ```
  def self.compile_file(*_); end

  # Returns a hash of default options used by the Ruby iseq compiler.
  #
  # For details, see
  # [`InstructionSequence.compile_option=`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-compile_option-3D).
  def self.compile_option; end

  # Sets the default values for various optimizations in the Ruby iseq compiler.
  #
  # Possible values for `options` include `true`, which enables all options,
  # `false` which disables all options, and `nil` which leaves all options
  # unchanged.
  #
  # You can also pass a `Hash` of `options` that you want to change, any options
  # not present in the hash will be left unchanged.
  #
  # Possible option names (which are keys in `options`) which can be set to
  # `true` or `false` include:
  #
  # *   `:inline_const_cache`
  # *   `:instructions_unification`
  # *   `:operands_unification`
  # *   `:peephole_optimization`
  # *   `:specialized_instruction`
  # *   `:stack_caching`
  # *   `:tailcall_optimization`
  #
  #
  # Additionally, `:debug_level` can be set to an integer.
  #
  # These default options can be overwritten for a single run of the iseq
  # compiler by passing any of the above values as the `options` parameter to
  # [`::new`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-new),
  # [`::compile`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-compile)
  # and
  # [`::compile_file`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-compile_file).
  def self.compile_option=(_); end

  # Takes `body`, a [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html)
  # or [`Proc`](https://docs.ruby-lang.org/en/2.7.0/Proc.html) object, and
  # returns a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) with
  # the human readable instructions for `body`.
  #
  # For a [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) object:
  #
  # ```ruby
  # # /tmp/method.rb
  # def hello
  #   puts "hello, world"
  # end
  #
  # puts RubyVM::InstructionSequence.disasm(method(:hello))
  # ```
  #
  # Produces:
  #
  # ```
  # == disasm: <RubyVM::InstructionSequence:hello@/tmp/method.rb>============
  # 0000 trace            8                                               (   1)
  # 0002 trace            1                                               (   2)
  # 0004 putself
  # 0005 putstring        "hello, world"
  # 0007 send             :puts, 1, nil, 8, <ic:0>
  # 0013 trace            16                                              (   3)
  # 0015 leave                                                            (   2)
  # ```
  #
  # For a Proc:
  #
  # ```ruby
  # # /tmp/proc.rb
  # p = proc { num = 1 + 2 }
  # puts RubyVM::InstructionSequence.disasm(p)
  # ```
  #
  # Produces:
  #
  # ```
  # == disasm: <RubyVM::InstructionSequence:block in <main>@/tmp/proc.rb>===
  # == catch table
  # | catch type: redo   st: 0000 ed: 0012 sp: 0000 cont: 0000
  # | catch type: next   st: 0000 ed: 0012 sp: 0000 cont: 0012
  # |------------------------------------------------------------------------
  # local table (size: 2, argc: 0 [opts: 0, rest: -1, post: 0, block: -1] s1)
  # [ 2] num
  # 0000 trace            1                                               (   1)
  # 0002 putobject        1
  # 0004 putobject        2
  # 0006 opt_plus         <ic:1>
  # 0008 dup
  # 0009 setlocal         num, 0
  # 0012 leave
  # ```
  def self.disasm(_); end

  # Takes `body`, a [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html)
  # or [`Proc`](https://docs.ruby-lang.org/en/2.7.0/Proc.html) object, and
  # returns a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) with
  # the human readable instructions for `body`.
  #
  # For a [`Method`](https://docs.ruby-lang.org/en/2.7.0/Method.html) object:
  #
  # ```ruby
  # # /tmp/method.rb
  # def hello
  #   puts "hello, world"
  # end
  #
  # puts RubyVM::InstructionSequence.disasm(method(:hello))
  # ```
  #
  # Produces:
  #
  # ```
  # == disasm: <RubyVM::InstructionSequence:hello@/tmp/method.rb>============
  # 0000 trace            8                                               (   1)
  # 0002 trace            1                                               (   2)
  # 0004 putself
  # 0005 putstring        "hello, world"
  # 0007 send             :puts, 1, nil, 8, <ic:0>
  # 0013 trace            16                                              (   3)
  # 0015 leave                                                            (   2)
  # ```
  #
  # For a Proc:
  #
  # ```ruby
  # # /tmp/proc.rb
  # p = proc { num = 1 + 2 }
  # puts RubyVM::InstructionSequence.disasm(p)
  # ```
  #
  # Produces:
  #
  # ```
  # == disasm: <RubyVM::InstructionSequence:block in <main>@/tmp/proc.rb>===
  # == catch table
  # | catch type: redo   st: 0000 ed: 0012 sp: 0000 cont: 0000
  # | catch type: next   st: 0000 ed: 0012 sp: 0000 cont: 0012
  # |------------------------------------------------------------------------
  # local table (size: 2, argc: 0 [opts: 0, rest: -1, post: 0, block: -1] s1)
  # [ 2] num
  # 0000 trace            1                                               (   1)
  # 0002 putobject        1
  # 0004 putobject        2
  # 0006 opt_plus         <ic:1>
  # 0008 dup
  # 0009 setlocal         num, 0
  # 0012 leave
  # ```
  def self.disassemble(_); end

  # Load an iseq object from binary format
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) object created
  # by
  # [`RubyVM::InstructionSequence.to_binary`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-i-to_binary).
  #
  # This loader does not have a verifier, so that loading broken/modified binary
  # causes critical problem.
  #
  # You should not load binary data provided by others. You should use binary
  # data translated by yourself.
  def self.load_from_binary(_); end

  # Load extra data embed into binary format
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) object.
  def self.load_from_binary_extra_data(_); end

  # Takes `source`, a
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) of Ruby code and
  # compiles it to an
  # [`InstructionSequence`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html).
  #
  # Optionally takes `file`, `path`, and `line` which describe the file path,
  # real path and first line number of the ruby code in `source` which are
  # metadata attached to the returned `iseq`.
  #
  # `file` is used for `\_\_FILE\_\_` and exception backtrace. `path` is used
  # for `require_relative` base. It is recommended these should be the same full
  # path.
  #
  # `options`, which can be `true`, `false` or a `Hash`, is used to modify the
  # default behavior of the Ruby iseq compiler.
  #
  # For details regarding valid compile options see
  # [`::compile_option=`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-compile_option-3D).
  #
  # ```ruby
  # RubyVM::InstructionSequence.compile("a = 1 + 2")
  # #=> <RubyVM::InstructionSequence:<compiled>@<compiled>>
  #
  # path = "test.rb"
  # RubyVM::InstructionSequence.compile(File.read(path), path, File.expand_path(path))
  # #=> <RubyVM::InstructionSequence:<compiled>@test.rb:1>
  #
  # path = File.expand_path("test.rb")
  # RubyVM::InstructionSequence.compile(File.read(path), path, path)
  # #=> <RubyVM::InstructionSequence:<compiled>@/absolute/path/to/test.rb:1>
  # ```
  def self.new(*_); end

  # Returns the instruction sequence containing the given proc or method.
  #
  # For example, using irb:
  #
  # ```
  # # a proc
  # > p = proc { num = 1 + 2 }
  # > RubyVM::InstructionSequence.of(p)
  # > #=> <RubyVM::InstructionSequence:block in irb_binding@(irb)>
  #
  # # for a method
  # > def foo(bar); puts bar; end
  # > RubyVM::InstructionSequence.of(method(:foo))
  # > #=> <RubyVM::InstructionSequence:foo@(irb)>
  # ```
  #
  # Using
  # [`::compile_file`](https://docs.ruby-lang.org/en/2.7.0/RubyVM/InstructionSequence.html#method-c-compile_file):
  #
  # ```
  # # /tmp/iseq_of.rb
  # def hello
  #   puts "hello, world"
  # end
  #
  # $a_global_proc = proc { str = 'a' + 'b' }
  #
  # # in irb
  # > require '/tmp/iseq_of.rb'
  #
  # # first the method hello
  # > RubyVM::InstructionSequence.of(method(:hello))
  # > #=> #<RubyVM::InstructionSequence:0x007fb73d7cb1d0>
  #
  # # then the global proc
  # > RubyVM::InstructionSequence.of($a_global_proc)
  # > #=> #<RubyVM::InstructionSequence:0x007fb73d7caf78>
  # ```
  def self.of(_); end
end

# This module allows for introspection of YJIT, CRuby's just-in-time compiler.
# Everything in the module is highly implementation specific and the API might
# be less stable compared to the standard library.
#
# This module may not exist if YJIT does not support the particular platform
# for which CRuby is built.
module RubyVM::YJIT
  # Check if YJIT is enabled.
  sig { returns(T::Boolean) }
  def self.enabled?; end

  # Check if `--yjit-stats` is used.
  sig { returns(T::Boolean) }
  def self.stats_enabled?; end

  # Discard statistics collected for `--yjit-stats`.
  sig { void }
  def self.reset_stats!; end

  # Enable YJIT compilation. `stats` option decides whether to enable YJIT stats or not.
  #
  # * `false`: Disable stats.
  # * `true`: Enable stats. Print stats at exit.
  # * `:quiet`: Enable stats. Do not print stats at exit.
  sig { params(stats: T.any(T::Boolean, Symbol)).returns(T::Boolean) }
  def self.enable(stats: false); end

  # [`Marshal`](https://docs.ruby-lang.org/en/2.7.0/Marshal.html) dumps exit locations to the given filename.
  #
  # Usage:
  #
  # If `--yjit-exit-locations` is passed, a file named
  # "yjit_exit_locations.dump" will automatically be generated.
  #
  # If you want to collect traces manually, call `dump_exit_locations`
  # directly.
  #
  # Note that calling this in a script will generate stats after the
  # dump is created, so the stats data may include exits from the
  # dump itself.
  #
  # In a script call:
  #
  # ```
  # at_exit do
  #   RubyVM::YJIT.dump_exit_locations("my_file.dump")
  # end
  # ````
  #
  # Then run the file with the following options:
  #
  # ```
  # ruby --yjit --yjit-trace-exits test.rb
  # ```
  #
  # Once the code is done running, use Stackprof to read the dump file.
  # See Stackprof documentation for options.
  sig { params(filename: T.any(String, Pathname)).returns(Integer) }
  def self.dump_exit_locations(filename); end

  # Return a hash for statistics generated for the `--yjit-stats` command line option.
  # Return `nil` when option is not passed or unavailable.
  sig { params(context: T::Boolean).returns(T.nilable(T::Hash[Symbol, T.untyped])) }
  def self.runtime_stats(context: false); end

  # Format and print out counters as a [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html).
  # This returns a non-empty content only when `--yjit-stats` is enabled.
  sig { returns(String) }
  def self.stats_string; end

  # Discard existing compiled code to reclaim memory
  # and allow for recompilations in the future.
  sig { void }
  def self.code_gc; end
end
