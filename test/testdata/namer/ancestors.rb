# @typed
module Mixin1; end
module Mixin2; end

class Parent
  include Mixin1
end

class Child < Parent
  include Mixin2
end
