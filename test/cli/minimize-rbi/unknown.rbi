# typed: true

class CommonToBoth
  def method_common_to_both; end

  def method_only_in_second; end
end

class OnlyInSecond
  include ModuleCommonToBoth
  extend ModuleOnlyInSecond

  # You might ask "why would this not exist"?
  # - Technically speaking this could be hand-written, and Sorbet should still
  #   do something sensible given a hand-written file with bad input.
  # - In practice, this file is autogenerated using reflection, and the file
  #   itself should represent a self-contained view of the codebase. So since
  #   the include exists, then autogenerated file should have had a definition
  #   of that module. But if there was an error serializing the class
  #   definition, sometimes the codegen script may simply catch it and move on
  #   (so the class would be defined at runtime, just not in the file). This is
  #   common if the error is some constant that fails to resolve inside a
  #   `T.type_alias`, or something like that.
  include ModuleDoesNotExist

  def foo(x1, x2=nil, *x5, x3:, x4: nil, **x6, &x7); end

  def fwd_args(...); end

  def self.self_method(x); end

  def <<(other); end
end

module ModuleCommonToBoth; end
module ModuleOnlyInSecond; end
