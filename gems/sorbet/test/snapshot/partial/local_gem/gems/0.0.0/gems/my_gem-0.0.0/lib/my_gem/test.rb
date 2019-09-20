begin
  require 'a-missing-gem'
rescue LoadError
  raise 'a runtime error'
end

module MyGem
  class Test
  end
end
