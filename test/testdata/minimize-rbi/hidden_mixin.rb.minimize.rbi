# typed: true

module ::Mocha
end

module ::Mocha::ClassMethods
  def any_instance(); end
end

class ::Class < Module
  include ::Mocha::ClassMethods
end
