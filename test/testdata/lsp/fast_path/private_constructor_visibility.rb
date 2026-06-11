# typed: true

class FastPathPrivateCtorParent
  private_class_method :new
end

class FastPathPrivateCtorChild < FastPathPrivateCtorParent; end

FastPathPrivateCtorChild.new # error: Non-private call to private method `new`
