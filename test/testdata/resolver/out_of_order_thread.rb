# check-out-of-order-constant-references: true
# frozen_string_literal: true
# typed: false


class ::Thread
  Thread.current[:opus_thread_is_root_fiber] = true
end
