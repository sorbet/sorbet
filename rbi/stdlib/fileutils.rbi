# typed: true

module FileUtils
  include(FileUtils::StreamUtils_)
  extend(FileUtils::StreamUtils_)

  LOW_METHODS = T.let(T.unsafe(nil), Array)

  module LowMethods

  end

  METHODS = T.let(T.unsafe(nil), Array)

  module StreamUtils_

  end

  module Verbose
    include(FileUtils)
    include(FileUtils::StreamUtils_)
    extend(FileUtils::Verbose)
    extend(FileUtils)
    extend(FileUtils::StreamUtils_)
  end

  module NoWrite
    include(FileUtils::LowMethods)
    include(FileUtils)
    include(FileUtils::StreamUtils_)
    extend(FileUtils::NoWrite)
    extend(FileUtils::LowMethods)
    extend(FileUtils)
    extend(FileUtils::StreamUtils_)
  end

  module DryRun
    include(FileUtils::LowMethods)
    include(FileUtils)
    include(FileUtils::StreamUtils_)
    extend(FileUtils::DryRun)
    extend(FileUtils::LowMethods)
    extend(FileUtils)
    extend(FileUtils::StreamUtils_)
  end

  OPT_TABLE = T.let(T.unsafe(nil), Hash)

  class Entry_
    include(FileUtils::StreamUtils_)

    SYSCASE = T.let(T.unsafe(nil), String)

    DIRECTORY_TERM = T.let(T.unsafe(nil), String)

    S_IF_DOOR = T.let(T.unsafe(nil), Integer)

    def path()
    end

    def remove()
    end

    def copy_file(dest)
    end

    def copy(dest)
    end

    def file?()
    end

    def entries()
    end

    def pipe?()
    end

    def symlink?()
    end

    def socket?()
    end

    def blockdev?()
    end

    def chardev?()
    end

    def dereference?()
    end

    def lstat!()
    end

    def remove_file()
    end

    def door?()
    end

    def stat()
    end

    def stat!()
    end

    def prefix()
    end

    def copy_metadata(path)
    end

    def lstat()
    end

    def remove_dir1()
    end

    def platform_support()
    end

    def rel()
    end

    def chown(uid, gid)
    end

    def chmod(mode)
    end

    def preorder_traverse()
    end

    def postorder_traverse()
    end

    def wrap_traverse(pre, post)
    end

    def inspect()
    end

    def traverse()
    end

    def directory?()
    end

    def exist?()
    end
  end

  VERSION = T.let(T.unsafe(nil), String)
end
