# typed: true
module One
  FOO = One
end
module Two
  FOO = Two
end
module Three
  FOO = Three
end
class Base
  include Three
  FOO = Base
end


class OneTwo < Base
  include One
  include Two
  puts("FOO=#{FOO}") # Should resolve to Two::FOO
  T.reveal_type(FOO) # error: `<Class:Two>`
end
