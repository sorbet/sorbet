
class A
  # This one tests hidden-definitions working
  define_method(:foo) do |_, _|
    puts 'hello, world!'
  end
end
