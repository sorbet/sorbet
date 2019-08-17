# typed: true

foo::C ||= raise bar rescue nil # error: Constant reassignment is not supported
