# typed: true

module DidYouMean
  module Levenshtein

  end

  class DeprecatedIgnoredCallers < Array
    def []=(*_)
    end

    def insert(*_)
    end

    def push(*_)
    end

    def +(*_)
    end

    def unshift(*_)
    end

    def <<(*_)
    end
  end

  IGNORED_CALLERS = T.let(_, DidYouMean::DeprecatedIgnoredCallers)

  SPELL_CHECKERS = T.let(_, Hash)

  module Jaro

  end

  NameErrorCheckers = T.let(_, Object)

  class MethodNameChecker
    NAMES_TO_EXCLUDE = T.let(T.unsafe(nil), Hash)

    def receiver()
    end

    def method_name()
    end

    def method_names()
    end

    def corrections()
    end
  end

  class KeyErrorChecker
    def corrections()
    end
  end

  VERSION = T.let(T.unsafe(nil), String)

  module Correctable
    def to_s()
    end

    def corrections()
    end

    def original_message()
    end

    def spell_checker()
    end
  end

  class NullChecker
    def corrections()
    end
  end

  class PlainFormatter
    def message_for(corrections)
    end
  end

  class ClassNameChecker
    class ClassName < SimpleDelegator
      RUBYGEMS_ACTIVATION_MONITOR = T.let(T.unsafe(nil), Monitor)
    end

    def class_name()
    end

    def corrections()
    end

    def class_names()
    end

    def scopes()
    end
  end

  class VariableNameChecker
    RB_PREDEFINED_OBJECTS = T.let(_, Array)

    NAMES_TO_EXCLUDE = T.let(T.unsafe(nil), Hash)

    def method_names()
    end

    def name()
    end

    def lvar_names()
    end

    def ivar_names()
    end

    def cvar_names()
    end

    def corrections()
    end
  end

  class SpellChecker
    def correct(input)
    end
  end

  module JaroWinkler
    THRESHOLD = T.let(T.unsafe(nil), Float)
    WEIGHT = T.let(T.unsafe(nil), Float)
  end
end
