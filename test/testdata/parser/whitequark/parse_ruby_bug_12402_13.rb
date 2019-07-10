# typed: true

foo::C ||= raise bar rescue nil # error-with-dupes: Constant reassignment is not supported
