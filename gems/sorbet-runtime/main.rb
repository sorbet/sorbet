# typed: false
require_relative './lib/sorbet-runtime'

class Module
  include T::Sig
end

original_method = T.let(nil, T.nilable(T::Private::ClassUtils::ReplacedMethod))
original_method = T::Private::ClassUtils.replace_method(Module, :define_method) do |symbol, &blk|
  T::Private::Methods.install_hooks(self)
  original_method.bind(self).call(symbol, &blk)
end

original_singleton_method = T.let(nil, T.nilable(T::Private::ClassUtils::ReplacedMethod))
original_singleton_method = T::Private::ClassUtils.replace_method(Kernel, :define_singleton_method) do |symbol, &blk|
  T::Private::Methods.install_hooks(self)
  original_singleton_method.bind(self).call(symbol, &blk)
end

T::Configuration.toggle_metaprogramming_location_tracking

module MyDSL
  def yikes
    define_method(:yikes) {}
  end

  def big_yikes
    define_singleton_method(:big_yikes) {}
  end
end

class A
  extend MyDSL

  define_method(:bad1) {}
  define_singleton_method(:bad2) {}

  def good1; end
  def self.good2; end

  yikes
  big_yikes

end

puts "Locs where DSL call triggered metaprogrammed methods:\n"
pp(T::Private::Methods.metaprogrammed_methods)
