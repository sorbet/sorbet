# typed: true

module Readline
  VERSION = T.let(T.unsafe(nil), String)

  HISTORY = T.let(T.unsafe(nil), Object)

  FILENAME_COMPLETION_PROC = T.let(T.unsafe(nil), Object)

  USERNAME_COMPLETION_PROC = T.let(T.unsafe(nil), Object)
end
