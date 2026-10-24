# typed: true

class ClassRoot # error: `ClassRoot` can only be a `class` if package `ClassRoot` is marked `outer_package_with_namespace_behavior!`
  class Helper; end
end

ClassRoot::Helper.new

module ClassRoot::Extra
end
