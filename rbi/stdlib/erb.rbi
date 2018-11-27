# typed: true

class ERB
  module DefMethod

  end

  module Util

  end

  Revision = T.let(T.unsafe(nil), String)

  class Compiler
    class ExplicitScanner < ::ERB::Compiler::Scanner
      def scan()
      end
    end

    class Buffer
      def script()
      end

      def push(cmd)
      end

      def cr()
      end

      def close()
      end
    end

    class PercentLine
      def to_s()
      end

      def value()
      end
    end

    class Scanner
      DEFAULT_STAGS = T.let(T.unsafe(nil), Array)

      DEFAULT_ETAGS = T.let(T.unsafe(nil), Array)

      def stags()
      end

      def etags()
      end

      def stag=(_)
      end

      def scan()
      end

      def stag()
      end
    end

    class TrimScanner < ::ERB::Compiler::Scanner
      ERB_STAG = T.let(T.unsafe(nil), Array)

      def trim_line2(line)
      end

      def explicit_trim_line(line)
      end

      def scan_line(line)
      end

      def percent_line(line, &block)
      end

      def trim_line1(line)
      end

      def is_erb_stag?(s)
      end

      def scan(&block)
      end
    end

    class SimpleScanner < ::ERB::Compiler::Scanner
      def scan()
      end
    end

    def compile_stag(stag, out, scanner)
    end

    def compile_etag(etag, out, scanner)
    end

    def compile_content(stag, out)
    end

    def prepare_trim_mode(mode)
    end

    def compile(s)
    end

    def trim_mode()
    end

    def percent()
    end

    def insert_cmd()
    end

    def put_cmd()
    end

    def make_scanner(src)
    end

    def put_cmd=(_)
    end

    def insert_cmd=(_)
    end

    def pre_cmd()
    end

    def pre_cmd=(_)
    end

    def post_cmd()
    end

    def add_put_cmd(out, content)
    end

    def post_cmd=(_)
    end

    def add_insert_cmd(out, content)
    end
  end

  def lineno()
  end

  def lineno=(_)
  end

  def src()
  end

  def def_class(superklass = _, methodname = _)
  end

  def def_module(methodname = _)
  end

  def make_compiler(trim_mode)
  end

  def filename()
  end

  def set_eoutvar(compiler, eoutvar = _)
  end

  def result(b = _)
  end

  def encoding()
  end

  def filename=(_)
  end

  def location=(_)
  end

  def result_with_hash(hash)
  end

  def def_method(mod, methodname, fname = _)
  end

  def run(b = _)
  end
end

class ERB::Compiler::SimpleScanner < ::ERB::Compiler::Scanner
  def scan()
  end
end
