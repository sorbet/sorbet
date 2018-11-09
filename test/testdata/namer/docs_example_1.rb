# typed: true

class A
  def method(method_arg)
    local = 1 # no name will be created

    @field = 2

    $global = 3
  end

  # singleton methods are just methods on the singleton class
  def self.singleton_method(singleton_method_arg)
    # singleton fields are just fields on the singleton class
    @singleton_field = 4
  end

  @@static_field = 5
end
