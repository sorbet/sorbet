# frozen_string_literal: true
# typed: true
# compiled: true

module Change
  def self.applicable
    return @@applicable if defined?(@@applicable)
    @@applicable = {}
    @@applicable['foo'] = 'bar'
    @@applicable
  end
end

class Version
  def versions
    return @versions if defined?(@versions)
    @versions = {}
    @versions['baz'] = 'quux'
    @versions
  end
end

# Calling the function twice should return the cached value from the first call.
a = Change.applicable.object_id
b = Change.applicable.object_id
p a == b

v = Version.new
x = v.versions.object_id
y = v.versions.object_id
p x == y
