# typed: true
module Mixin1
  def say_hello
  end
end

module Mixin2
  def say_goodbye
  end
end

class C
  extend Mixin1
  extend Mixin2
end

C.say_hello
C.say_goodbye
C.new.say_hello # error: Method `say_hello` does not exist
