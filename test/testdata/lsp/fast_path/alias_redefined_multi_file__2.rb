# typed: true
# spacer for exclude-from-file-update

class A
  alias_method :from_method, :to_method
end

A.new.to_method
A.new.to_method_new # error: Method `to_method_new` does not exist on `A`
A.new.from_method
A.new.from_method_new # error: Method `from_method_new` does not exist on `A`
