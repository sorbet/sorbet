class Boom
  methods.each do |name|
    send(:define_method, name) do |*args|
      raise "boom"
    end
  end
  class << self
    methods.each do |name|
      send(:define_method, name) do |*args|
        raise "boom"
      end
    end
  end

  EXPOSION = 1
end

class A < Boom
end
class B
  B1 = Boom
end
class C
  C1 = Class.instance_method(:new).bind(Boom).call
end
