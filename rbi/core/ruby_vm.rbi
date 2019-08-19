# typed: __STDLIB_INTERNAL

# The [RubyVM](RubyVM) module provides some access to
# Ruby internals. This module is for very limited purposes, such as
# debugging, prototyping, and research. Normal users must not use it.
class RubyVM < Object
  DEFAULT_PARAMS = T.let(T.unsafe(nil), Hash)
  INSTRUCTION_NAMES = T.let(T.unsafe(nil), Array)
  OPTS = T.let(T.unsafe(nil), Array)
end

class RubyVM::InstructionSequence < Object
end
