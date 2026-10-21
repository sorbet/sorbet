# typed: true

class ClassRoot
  class Helper; end
end

ClassRoot::Helper.new

module ClassRoot::Extra
end
