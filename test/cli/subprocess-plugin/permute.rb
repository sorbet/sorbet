# typed: true

def hook(*)
end

# class module singleton
class CMS
  hook 1, 'cms'
  module M
    hook 2
    class << self
      hook 3
    end
    hook 4
  end
  hook 5
end

# class singleton module
class CSM
  hook 1, "CSM"
  class << self
    hook 2
    module M
      hook 3
    end
    hook 4
  end
  hook 5
end

# module class singleton
module MCS
  hook 1
  class C
    hook 2
    class << self
      hook 3
    end
    hook 4
  end
  hook(5) do |mcs|
    # very involved code here
    # comment should be sent to subprocess
  end
end

# module singleton class
module MSC
  hook 1
  class << self
    hook 2
    class C
      hook 3
    end
    hook 4
  end
  hook 5
end

# singleton module class
class << self
  hook 1
  module M
    hook 2
    class C
      hook 3
    end
    hook 4
  end
  hook 5
end

# singleton class module
class << self
  hook 1
  module M
    hook 2
    class C
      hook 3
    end
    hook 4
  end
  hook 5
end

class NestedReopen
  class MCS::C
    hook 'no ::'
  end
end

class NestedReopen
  class ::MCS::C
    hook "nested with :: prefix"
  end
end

module CMS::M
  hook "CMS::M at top level"
  class ::MCS::C
    hook "nested with :: prefix"
  end
end

module ::CMS::M
  hook "::CMS::M at top level"
  class C
    hook 'nested C'
  end
end
