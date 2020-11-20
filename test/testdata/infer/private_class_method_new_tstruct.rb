# typed: true

class A < T::Struct
  private_class_method :new
end

A.new
