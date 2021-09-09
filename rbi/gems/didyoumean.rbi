# typed: __STDLIB_INTERNAL
module DidYouMean
  IGNORED_CALLERS = T.let(T.unsafe(nil), T::Array[T.untyped])
  SPELL_CHECKERS = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
  VERSION = T.let(T.unsafe(nil), String)
end

class DidYouMean::ClassNameChecker < Object
end

class DidYouMean::ClassNameChecker::ClassName < SimpleDelegator
  RUBYGEMS_ACTIVATION_MONITOR = T.let(T.unsafe(nil), Monitor)
end

module DidYouMean::Correctable
end

class DidYouMean::Formatter < Object
end

module DidYouMean::Jaro
end

module DidYouMean::JaroWinkler
  THRESHOLD = T.let(T.unsafe(nil), Float)
  WEIGHT = T.let(T.unsafe(nil), Float)
end

class DidYouMean::MethodNameChecker < Object
  NAMES_TO_EXCLUDE = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
end

class DidYouMean::VariableNameChecker < Object
  NAMES_TO_EXCLUDE = T.let(T.unsafe(nil), T::Hash[T.untyped, T.untyped])
end

module DidYouMean::Levenshtein
end

module DidYouMean::NameErrorCheckers
end

class DidYouMean::NullChecker < Object
end

class DidYouMean::SpellChecker < Object
  sig {params(dictionary: T::Enumerable[T.any(String, Symbol)]).void}
  def initialize(dictionary:)
  end

  sig {params(input: T.any(String, Symbol)).returns(T::Array[T.any(String, Symbol)])}
  def correct(input)
  end
end
