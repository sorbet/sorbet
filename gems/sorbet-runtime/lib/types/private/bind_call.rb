# typed: true

module T::Private::BindCall
  if Gem::Version.new(RUBY_VERSION) >= Gem::Version.new('2.7')
    # TODO(jez) Move all the pry message comments to this file
    def self.bind_call0(method, recv, &blk)
      method.bind_call(recv, &blk)
    end

    def self.bind_call1(method, recv, arg0, &blk)
      method.bind_call(recv, arg0, &blk)
    end

    def self.bind_call2(method, recv, arg0, arg1, &blk)
      method.bind_call(recv, arg0, arg1, &blk)
    end

    def self.bind_call3(method, recv, arg0, arg1, arg2, &blk)
      method.bind_call(recv, arg0, arg1, arg2, &blk)
    end

    def self.bind_call4(method, recv, arg0, arg1, arg2, arg3, &blk)
      method.bind_call(recv, arg0, arg1, arg2, arg3, &blk)
    end

    def self.bind_call_argv(method, recv, argv, &blk)
      method.bind_call(recv, *argv, &blk)
    end
  else
    def self.bind_call0(method, recv, &blk)
      method.bind(recv).call(&blk)
    end

    def self.bind_call1(method, recv, arg0, &blk)
      method.bind(recv).call(arg0, &blk)
    end

    def self.bind_call2(method, recv, arg0, arg1, &blk)
      method.bind(recv).call(arg0, arg1, &blk)
    end

    def self.bind_call3(method, recv, arg0, arg1, arg2, &blk)
      method.bind(recv).call(arg0, arg1, arg2, &blk)
    end

    def self.bind_call4(method, recv, arg0, arg1, arg2, arg3, &blk)
      method.bind(recv).call(arg0, arg1, arg2, arg3, &blk)
    end

    def self.bind_call_argv(method, recv, argv, &blk)
      method.bind(recv).call(*argv, &blk)
    end
  end
end
