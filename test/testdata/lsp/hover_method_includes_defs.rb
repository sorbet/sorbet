# typed: true

# rubocop:disable all

module OuterModule
  extend T::Sig

  sig {params(name: String).returns(Integer)}
  def module_method(name)
    # ^ hover: def module_method(name); end
    name.length
  end

  sig {params(name: String).returns(Integer)}
  def self.module_self_method(name)
    # ^ hover: def self.module_self_method(name); end
    name.length
  end

  sig {returns T.untyped}
  def module_usages
    s = "Foo"
    [
      module_method(s),
      # ^ hover: def module_method(name); end
      OuterModule.module_self_method(s),
      #           ^ hover: def self.module_self_method(name); end
      OuterModule::module_self_method(s),
      #            ^ hover: def self.module_self_method(name); end
    ]
  end

  class InnerClass
    extend T::Sig
    sig {params(name: String).void}
    def initialize(name)
      # ^ hover: private def initialize(name); end
      @name = name
    end

    sig {params(name: String).returns(Integer)}
    def self.class_self_method(name)
      name.length
    end

    sig {void}
    def no_args_return_void
      # no args
    end

    sig {returns(Integer)}
    def no_args_return_value
      "no args".length
    end

    sig {params(fname: String, lname: String).returns(String)}
    def positional_args(fname, lname)
      "#{fname}:#{lname}"
    end

    sig {params(fname: String, lname: String).returns(String)}
    def positional_args_with_defaults(fname='Jane', lname='Doe')
      # ^ hover: def positional_args_with_defaults(fname=…, lname=…); end
      "#{fname}:#{lname}"
    end

    sig {params(fname: String, lname: String).returns(String)}
    def keyword_args_no_defaults(fname:, lname:)
      "#{fname}:#{lname}"
    end

    sig {params(fname: String, lname: String).returns(String)}
    def keyword_args_with_defaults(fname: "Jane", lname: "Doe")
      # ^ hover: def keyword_args_with_defaults(fname: …, lname: …); end
      "#{fname}:#{lname}"
    end

    sig {params(names: String).returns(String)}
    def splat_args(*names)
      names.join ':'
    end

    sig {params(kwargs: String).returns(String)}
    def double_splat_args(**kwargs)
      kwargs.keys.join ':'
    end

    sig {params(blk: T.proc.params(arg0: Integer).returns(String)).returns(String)}
    def block_args(&blk)
      blk.call(1)
    end

    sig {params(pos: String, splat: String, required_key: String, optional_key: String, kwargs: T.untyped, block_arg: T.proc.params(a: String).returns(String)).returns(String)}
    def multiple_arg_types(pos, *splat, required_key:, optional_key: "Jane", **kwargs, &block_arg)
      "#{pos} #{required_key} #{optional_key} #{splat.join ':'} #{kwargs.to_s} #{block_arg.call(pos)}"
    end

    sig {returns T.untyped}
    def class_usages
      s = "Foo"
      qualified = InnerClass.new(s)
      #                        ^ hover: private def initialize(name); end
      [
        no_args_return_void,
        # ^ hover: def no_args_return_void; end
        no_args_return_value,
        # ^ hover: def no_args_return_value; end
        positional_args(s, s),
        # ^ hover: def positional_args(fname, lname); end
        keyword_args_no_defaults(fname: s, lname: s),
        # ^ hover: def keyword_args_no_defaults(fname:, lname:); end
        keyword_args_with_defaults,
        # ^ hover: def keyword_args_with_defaults(fname: …, lname: …); end
        splat_args,
        # ^ hover: def splat_args(*names); end
        splat_args(s, s, s, s, s),
        # ^ hover: def splat_args(*names); end
        double_splat_args(a:s, b:s, c:s),
        # ^ hover: def double_splat_args(**kwargs); end
        block_args {|x| "blk#{x}"},
        # ^ hover: def block_args(&blk); end
        multiple_arg_types(s, s, s, s, required_key: s, dynamic_key: s) {|x| x},
        # ^ hover: def multiple_arg_types(
        # ^ hover:   pos,
        # ^ hover:   *splat,
        # ^ hover:   required_key:,
        # ^ hover:   optional_key: …,
        # ^ hover:   **kwargs,
        # ^ hover:   &block_arg
        # ^ hover: )
        # ^ hover: end
        InnerClass::class_self_method(s),
        #           ^ hover: def self.class_self_method(name); end
        qualified.no_args_return_void,
        #         ^ hover: def no_args_return_void; end
        qualified.no_args_return_value,
        #         ^ hover: def no_args_return_value; end
        qualified.positional_args(s, s),
        #         ^ hover: def positional_args(fname, lname); end
        qualified.keyword_args_no_defaults(fname: s, lname: s),
        #         ^ hover: def keyword_args_no_defaults(fname:, lname:); end
        qualified.keyword_args_with_defaults(fname: s, lname: s),
        #         ^ hover: def keyword_args_with_defaults(fname: …, lname: …); end
        qualified.splat_args(s, s, s, s, s),
        #         ^ hover: def splat_args(*names); end
        qualified.double_splat_args(a: s, b: s, c: s),
        #         ^ hover: def double_splat_args(**kwargs); end
        qualified.multiple_arg_types(s, s, s, s, required_key: s, dynamic_key: s) {|x| x},
        #         ^ hover: def multiple_arg_types(
        #         ^ hover:   pos,
        #         ^ hover:   *splat,
        #         ^ hover:   required_key:,
        #         ^ hover:   optional_key: …,
        #         ^ hover:   **kwargs,
        #         ^ hover:   &block_arg
        #         ^ hover: )
        #         ^ hover: end
      ]
    end
  end
end
