# typed: strict

class ERB
  Revision = ::T.let(nil, ::T.untyped)

  sig do
    params(
      superklass: ::T.untyped,
      methodname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def def_class(superklass=T.unsafe(nil), methodname=T.unsafe(nil)); end

  sig do
    params(
      mod: ::T.untyped,
      methodname: ::T.untyped,
      fname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def defMethod(mod, methodname, fname=T.unsafe(nil)); end

  sig do
    params(
      methodname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def defModule(methodname=T.unsafe(nil)); end

  sig {returns(::T.untyped)}
  def encoding(); end

  sig {returns(::T.untyped)}
  def filename(); end

  sig do
    params(
      filename: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def filename=(filename); end

  sig do
    params(
      str: ::T.untyped,
      safe_level: ::T.untyped,
      trim_mode: ::T.untyped,
      eoutvar: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(str, safe_level=T.unsafe(nil), trim_mode=T.unsafe(nil), eoutvar=T.unsafe(nil)); end

  sig {returns(::T.untyped)}
  def lineno(); end

  sig do
    params(
      lineno: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def lineno=(lineno); end

  sig do
    params(
      location: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def location=(location); end

  sig do
    params(
      trim_mode: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def make_compiler(trim_mode); end

  sig do
    params(
      b: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def result(b=T.unsafe(nil)); end

  sig do
    params(
      b: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def run(b=T.unsafe(nil)); end

  sig do
    params(
      compiler: ::T.untyped,
      eoutvar: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def set_eoutvar(compiler, eoutvar=T.unsafe(nil)); end

  sig {returns(::T.untyped)}
  def src(); end

  sig {returns(::T.untyped)}
  def self.version(); end
end

class ERB::Compiler
  sig do
    params(
      out: ::T.untyped,
      content: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_insert_cmd(out, content); end

  sig do
    params(
      out: ::T.untyped,
      content: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def add_put_cmd(out, content); end

  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def compile(s); end

  sig do
    params(
      stag: ::T.untyped,
      out: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def compile_content(stag, out); end

  sig do
    params(
      etag: ::T.untyped,
      out: ::T.untyped,
      scanner: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def compile_etag(etag, out, scanner); end

  sig do
    params(
      stag: ::T.untyped,
      out: ::T.untyped,
      scanner: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def compile_stag(stag, out, scanner); end

  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def content_dump(s); end

  sig do
    params(
      trim_mode: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(trim_mode); end

  sig {returns(::T.untyped)}
  def insert_cmd(); end

  sig do
    params(
      insert_cmd: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def insert_cmd=(insert_cmd); end

  sig do
    params(
      src: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def make_scanner(src); end

  sig {returns(::T.untyped)}
  def percent(); end

  sig {returns(::T.untyped)}
  def post_cmd(); end

  sig do
    params(
      post_cmd: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def post_cmd=(post_cmd); end

  sig {returns(::T.untyped)}
  def pre_cmd(); end

  sig do
    params(
      pre_cmd: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def pre_cmd=(pre_cmd); end

  sig do
    params(
      mode: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def prepare_trim_mode(mode); end

  sig {returns(::T.untyped)}
  def put_cmd(); end

  sig do
    params(
      put_cmd: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def put_cmd=(put_cmd); end

  sig {returns(::T.untyped)}
  def trim_mode(); end
end

class ERB::Compiler::Buffer
  sig {returns(::T.untyped)}
  def close(); end

  sig {returns(::T.untyped)}
  def cr(); end

  sig do
    params(
      compiler: ::T.untyped,
      enc: ::T.untyped,
      frozen: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(compiler, enc=T.unsafe(nil), frozen=T.unsafe(nil)); end

  sig do
    params(
      cmd: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def push(cmd); end

  sig {returns(::T.untyped)}
  def script(); end
end

class ERB::Compiler::ExplicitScanner < ERB::Compiler::Scanner
  sig {returns(::T.untyped)}
  def scan(); end
end

class ERB::Compiler::PercentLine
  sig {returns(::T.untyped)}
  def empty?(); end

  sig do
    params(
      str: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(str); end

  sig {returns(::T.untyped)}
  def to_s(); end

  sig {returns(::T.untyped)}
  def value(); end
end

class ERB::Compiler::Scanner
  sig {returns(::T.untyped)}
  def etags(); end

  sig do
    params(
      src: ::T.untyped,
      trim_mode: ::T.untyped,
      percent: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(src, trim_mode, percent); end

  sig {returns(::T.untyped)}
  def scan(); end

  sig {returns(::T.untyped)}
  def stag(); end

  sig do
    params(
      stag: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def stag=(stag); end

  sig {returns(::T.untyped)}
  def stags(); end

  sig do
    params(
      klass: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.default_scanner=(klass); end

  sig do
    params(
      src: ::T.untyped,
      trim_mode: ::T.untyped,
      percent: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.make_scanner(src, trim_mode, percent); end

  sig do
    params(
      klass: ::T.untyped,
      trim_mode: ::T.untyped,
      percent: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.regist_scanner(klass, trim_mode, percent); end

  sig do
    params(
      klass: ::T.untyped,
      trim_mode: ::T.untyped,
      percent: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.register_scanner(klass, trim_mode, percent); end
end

class ERB::Compiler::SimpleScanner < ERB::Compiler::Scanner
  sig {returns(::T.untyped)}
  def scan(); end
end

class ERB::Compiler::TrimScanner < ERB::Compiler::Scanner
  ERB_STAG = ::T.let(nil, ::T.untyped)

  sig do
    params(
      line: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def explicit_trim_line(line); end

  sig do
    params(
      src: ::T.untyped,
      trim_mode: ::T.untyped,
      percent: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def initialize(src, trim_mode, percent); end

  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def is_erb_stag?(s); end

  sig do
    params(
      line: ::T.untyped,
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def percent_line(line, &block); end

  sig do
    params(
      block: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def scan(&block); end

  sig do
    params(
      line: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def scan_line(line); end

  sig do
    params(
      line: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def trim_line1(line); end

  sig do
    params(
      line: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def trim_line2(line); end
end

module ERB::DefMethod
  sig do
    params(
      methodname: ::T.untyped,
      erb_or_fname: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.def_erb_method(methodname, erb_or_fname); end
end

module ERB::Util
  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.h(s); end

  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.html_escape(s); end

  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.u(s); end

  sig do
    params(
      s: ::T.untyped,
    )
    .returns(::T.untyped)
  end
  def self.url_encode(s); end
end
