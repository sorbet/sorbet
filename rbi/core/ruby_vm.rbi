# typed: __STDLIB_INTERNAL

# The [`RubyVM`](https://docs.ruby-lang.org/en/2.6.0/RubyVM.html) module
# provides some access to Ruby internals. This module is for very limited
# purposes, such as debugging, prototyping, and research. Normal users must not
# use it.
class RubyVM < Object
  # [`DEFAULT_PARAMS`](https://docs.ruby-lang.org/en/2.6.0/RubyVM.html#DEFAULT_PARAMS)
  # This constant variable shows VM's default parameters. Note that changing
  # these values does not affect VM execution. Specification is not stable and
  # you should not depend on this value. Of course, this constant is MRI
  # specific.
  DEFAULT_PARAMS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  # [`INSTRUCTION_NAMES`](https://docs.ruby-lang.org/en/2.6.0/RubyVM.html#INSTRUCTION_NAMES)
  INSTRUCTION_NAMES = T.let(T.unsafe(nil), T::Array[T.untyped])
  # [`OPTS`](https://docs.ruby-lang.org/en/2.6.0/RubyVM.html#OPTS), which shows
  # vm build options
  OPTS = T.let(T.unsafe(nil), T::Array[T.untyped])
end

# The
# [`InstructionSequence`](https://docs.ruby-lang.org/en/2.6.0/RubyVM/InstructionSequence.html)
# class represents a compiled sequence of instructions for the Ruby Virtual
# Machine.
#
# With it, you can get a handle to the instructions that make up a method or a
# proc, compile strings of Ruby code down to VM instructions, and disassemble
# instruction sequences to strings for easy inspection. It is mostly useful if
# you want to learn how the Ruby VM works, but it also lets you control various
# settings for the Ruby iseq compiler.
#
# You can find the source for the VM instructions in `insns.def` in the Ruby
# source.
#
# The instruction sequence results will almost certainly change as Ruby changes,
# so example output in this documentation may be different from what you see.
class RubyVM::InstructionSequence < Object
end
