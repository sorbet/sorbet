# typed: true

class ClassRoot # error: `ClassRoot` cannot be a `class`, because packages are nested under `ClassRoot`
  class Helper; end
end

ClassRoot::Helper.new

module ClassRoot::Extra
end
