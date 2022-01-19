# typed: false

class A
  private def foo
  end
end

class B
  private_class_method def self.foo
  end
end

class C
  private_class_method def foo
  end
end

class D
  private def self.foo
  end
end

class E
  package_private def self.foo
  end
end

class F
  package_private_class_method def foo
  end
end

class G
  package_private def foo
  end
end

class H
  package_private_class_method def self.foo
  end
end

