# typed: __STDLIB_INTERNAL

module T
  def self.unsafe(value); end
end
module T::Sig
  def sig(arg=nil, &blk); end
end
module T::Sig::WithoutRuntime
  def self.sig(arg=nil, &blk); end
end

module Sorbet; end
module Sorbet::Private; end
module Sorbet::Private::Static
  def self.keep_for_ide(expr)
  end

  def self.keep_for_typechecking(expr)
  end
end

module Sorbet::Private::Static::StubModule
end

class Sorbet::Private::Static::ImplicitModuleSuperclass < BasicObject
end

# TODO This is very wrong but we need it until we fix something with resolution
module Static
  def self.keep_for_ide(expr)
  end

  def self.keep_for_typechecking(expr)
  end
end
