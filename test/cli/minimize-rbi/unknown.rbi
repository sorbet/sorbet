# typed: true

class CommonToBoth
  def method_common_to_both; end

  def method_only_in_second; end
end

class OnlyInSecond
  include ModuleCommonToBoth
  extend ModuleOnlyInSecond

  def foo(x1, x2=nil, *x5, x3:, x4: nil, **x6, &x7); end

  def fwd_args(...); end

  def self.self_method(x); end

  def <<(other); end
end

module ModuleCommonToBoth; end
module ModuleOnlyInSecond; end
