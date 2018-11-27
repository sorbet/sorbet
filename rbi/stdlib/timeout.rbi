# typed: true

module Timeout
  class Error < ::RuntimeError
    def thread()
    end

    def exception(*_)
    end
  end
end
