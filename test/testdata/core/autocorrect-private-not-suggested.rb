# typed: true

class A
  private def my_private_method; end
end

A.new.my_private_metho # error: Method `my_private_metho` does not exist on `A`
