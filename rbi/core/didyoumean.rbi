# typed: true
module DidYouMean
  IGNORED_CALLERS = T.let(T.unsafe(nil), Array)
  SPELL_CHECKERS = T.let(T.unsafe(nil), Hash)
  VERSION = T.let(T.unsafe(nil), String)
end

class DidYouMean::ClassNameChecker::ClassName < SimpleDelegator
  RUBYGEMS_ACTIVATION_MONITOR = T.let(T.unsafe(nil), Monitor)
end

module DidYouMean::JaroWinkler
  THRESHOLD = T.let(T.unsafe(nil), Float)
  WEIGHT = T.let(T.unsafe(nil), Float)
end

class DidYouMean::MethodNameChecker < Object
  NAMES_TO_EXCLUDE = T.let(T.unsafe(nil), Hash)
end

class DidYouMean::VariableNameChecker < Object
  NAMES_TO_EXCLUDE = T.let(T.unsafe(nil), Hash)
end
