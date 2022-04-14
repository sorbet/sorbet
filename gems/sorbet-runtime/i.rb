module Opus::Repro
  class I
    module FH
      include Kernel
      def make(i, s)
        m = F.new
      end
    end

    extend FH

    CA = make(16, [])
  end
end
