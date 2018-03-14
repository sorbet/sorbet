# @typed
def loop_it
  f = T.let(nil,T.nilable(String))

  (1..10).each do |num|
    f ||= 'a'
  end

  puts "f is now #{f}"
end
