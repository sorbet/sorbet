# typed: true

class A
  private def my_private_method; end
end

# this flags the method as not existing but does not suggest the private method
A.new.my_private_metho # error: Method `my_private_metho` does not exist on `A`
