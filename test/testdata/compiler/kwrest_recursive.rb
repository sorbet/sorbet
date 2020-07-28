# frozen_string_literal: true
# typed: true
# compiled: true

class Main
  def self.main(n, **kwargs)
    if n == 0
      puts 'values after inner call:'
      kwargs[:foo] = 1
    else
      T.unsafe(Main).main(n-1, **kwargs)
      puts 'values after outer call:'
    end
    p n
    p kwargs
  end
end

T.unsafe(Main).main(1, bar: :qux)
