# frozen_string_literal: true
# typed: true
# compiled: true

class Session
  MAX_OBJS = 849201
  MAX_OBJS_MSG = <<~MSG
    There are more than #{MAX_OBJS} objects!  Fire the missiles!
  MSG
end

p Session::MAX_OBJS_MSG
