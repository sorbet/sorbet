# typed: true
class Module; include T::Sig; end

module Runnable
  extend T::Helpers
  interface!

  sig {abstract.void}
  def run; end
end

module Parent
  include Runnable

  sig {override.void}
  def run; end
end

class Child1
  include Parent
  
  sig {override.params(x: Integer).void}
  def run(x); end # 💥 note Parent#run vs Runnable#run in error
# ^^^^^^^^^^ error: Override of method `Parent#run` must accept no more than `0` required argument(s)
end

class Child2
  include Parent
  
  sig {params(x: Integer).void}
  def run(x); end
# ^^^^^^^^^^ error: Implementation of abstract method `Runnable#run` must accept no more than `0` required argument(s)
# ^^^^^^^^^^ error: Method `Child2#run` implements an abstract method `Runnable#run` but is not declared with `override.`
end
