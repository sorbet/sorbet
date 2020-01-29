# typed: __STDLIB_INTERNAL

module T
  def self.cast(value, *opts, **kwopts); value; end;
  def self.unsafe(value); value; end
  def self.any(left, right); end
  def self.all(left, right); end
  def self.must(arg, msg=nil); arg; end
  def self.let(arg, type); arg; end
  def self.untyped(); end
end
module T::Sig
  def sig(arg=nil, &blk); end
end
module T::Sig::WithoutRuntime
  def self.sig(arg=nil, &blk); end
end

module T::Array
  def self.[](arg); end;
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

module T; end
module T::Helpers; end
module T::Types; end
module T::Private; end
module T::Private::Abstract; end
module T::Private::Types; end
class T::Types::Base; end
class T::Private::Types::Void < T::Types::Base
  VOID = Module.new.freeze
end
