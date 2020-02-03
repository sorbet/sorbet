# typed: __STDLIB_INTERNAL

module T
  def self.cast(value, type, checked: true); value; end;
  def self.unsafe(value); value; end
  def self.any(type_a, type_b, *types); end
  def self.all(type_a, type_b, *types); end
  def self.must(arg); arg; end
  def self.let(value, type, checked: true); value; end
  def self.untyped(); end
  def self.class_of(klass); end
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

module T::Hash
  def self.[](arg1, arg2); end;
end


class Sorbet; end
module Sorbet::Private; end
module Sorbet::Private::Static
  def self.keep_method_def(this, fun); fun; end
  def self.keep_class_method_def(this, fun); fun; end
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
