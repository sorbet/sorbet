# typed: true

module Parent
end

class Child < Parent # error: does not derive from `Class`
end
