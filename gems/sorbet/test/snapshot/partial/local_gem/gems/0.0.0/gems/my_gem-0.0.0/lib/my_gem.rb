# frozen_string_literals: true

class MyGem
  class << self
    def method; end
  end

  def my_gem_thing
    puts 'hello, world!'
  end
end
