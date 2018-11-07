# typed: true

class A
  def method(method_arg)
    local = 1
    @field = 2
    $global = 3
  end

  def self.singleton_method(singleton_method_arg)
    @singleton_field = 4
  end

  @@static_field = 5
end
