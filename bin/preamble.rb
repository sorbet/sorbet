# typed: __STDLIB_INTERNAL

module T
  def self.unsafe(value); value; end
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
end

module Sorbet::Private::Static::StubModule
end

class Sorbet::Private::Static::ImplicitModuleSuperclass < BasicObject
end

def pry
  require 'pry-byebug'
  binding.pry
end
