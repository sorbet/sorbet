# typed: true
module RbConfig
  CONFIG = T.let(T.unsafe(nil), Hash)
  DESTDIR = T.let(T.unsafe(nil), String)
  MAKEFILE_CONFIG = T.let(T.unsafe(nil), Hash)
  TOPDIR = T.let(T.unsafe(nil), String)
end
