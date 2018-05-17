# typed: true
class RubyVM < Object
  DEFAULT_PARAMS = T.let(T.unsafe(nil), Hash)
  INSTRUCTION_NAMES = T.let(T.unsafe(nil), Array)
  OPTS = T.let(T.unsafe(nil), Array)
end
