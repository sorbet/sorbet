# typed: true

class ClassRoot # error: `ClassRoot` cannot be a `class`, because packages are nested under `ClassRoot`
  class Helper; end
end

ClassRoot::Helper.new # error: Unable to resolve constant `Helper`

module ClassRoot::Extra
end
