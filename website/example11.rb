# typed: true
class ActivityBase
  extend T::Helpers
  extend T::Generic
  extend T::Sig

  sig {void}
  def instance_method_i_want_to_use; end
end

class Activity < ActivityBase

  abstract!

  InputType = type_template(upper: T::Struct)

  def initialize(token)
  end

  sig {params(args: InputType).void}
  def self.call(args:)  
    self.new(rand).do_it(args)
  end

  sig {abstract.params(args: InputType).void}
  def do_it(args); end
end

class MyActivity < Activity

  InputType = type_template(fixed: Args)

  class Args < T::Struct
    const :a, Integer
  end

  sig {override.params(args: Args).void}
  def do_it(args)
    instance_method_i_want_to_use
  end
  
end

def main
end
