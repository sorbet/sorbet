# typed: true
class ABD
end
class ABE
end

ABF = 123

ABC(1) # error: Method `ABC` does not exist on `T.class_of(<root>)`
ABD(1) # error: Method `ABD` does not exist on `T.class_of(<root>)`

class Container
  class Inner
  end

  def self.test
    Inner(1) # error: Method `Inner` does not exist on `T.class_of(Container)`
  end
end

class Parent
  class Inner
  end
end

class Child < Parent
  Inner(1) # error: Method `Inner` does not exist on `T.class_of(Child)`
end

Child::Inner(1) # error: Method `Inner` does not exist on `T.class_of(Child)`
