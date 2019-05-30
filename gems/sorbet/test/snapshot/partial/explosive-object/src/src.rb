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
end
