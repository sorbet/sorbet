# typed: __STDLIB_INTERNAL

class Object < BasicObject
  include Kernel

  sig {returns(Integer)}
  def object_id(); end
end

class TrueClass
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def &(obj)
  end
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def ^(obj)
  end
  sig {params(obj: BasicObject).returns(TrueClass)}
  def |(obj)
  end
  sig {returns(FalseClass)}
  def !
  end
end

class FalseClass
  sig {params(obj: BasicObject).returns(FalseClass)}
  def &(obj)
  end
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def ^(obj)
  end
  sig {params(obj: BasicObject).returns(T::Boolean)}
  def |(obj)
  end
  sig {returns(TrueClass)}
  def !
  end
end

::ARGF = T.let(T.unsafe(nil), Object)
::ARGV = T.let(T.unsafe(nil), Array)
::CROSS_COMPILING = T.let(T.unsafe(nil), NilClass)
::FALSE = T.let(T.unsafe(nil), FalseClass)
::NIL = T.let(T.unsafe(nil), NilClass)
::RUBY_COPYRIGHT = T.let(T.unsafe(nil), String)
::RUBY_DESCRIPTION = T.let(T.unsafe(nil), String)
::RUBY_ENGINE = T.let(T.unsafe(nil), String)
::RUBY_ENGINE_VERSION = T.let(T.unsafe(nil), String)
::RUBY_PATCHLEVEL = T.let(T.unsafe(nil), Integer)
::RUBY_PLATFORM = T.let(T.unsafe(nil), String)
::RUBY_RELEASE_DATE = T.let(T.unsafe(nil), String)
::RUBY_REVISION = T.let(T.unsafe(nil), Integer)
::RUBY_VERSION = T.let(T.unsafe(nil), String)
::STDERR = T.let(T.unsafe(nil), IO)
::STDIN = T.let(T.unsafe(nil), IO)
::STDOUT = T.let(T.unsafe(nil), IO)
::TOPLEVEL_BINDING = T.let(T.unsafe(nil), Binding)
::TRUE = T.let(T.unsafe(nil), TrueClass)
