# frozen_string_literals: true

module MyGem
  def my_gem_thing
    puts 'hello, world!'
  end

  autoload :Test, 'my_gem/test'
end
