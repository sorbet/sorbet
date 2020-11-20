# typed: true

class A < T::Struct
  private_class_method :new
end

A.new

# TODO(jez) This shows the whole ClassDef tree's loc in the error message...?
