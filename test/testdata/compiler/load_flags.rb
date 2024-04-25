# frozen_string_literal: true
# typed: true
# compiled: true

# TODO Whoever wants to fix this: I was pretty lazy trying to minimize this.
# TODO This test is flaky. When it fails, sometimes it fails for one reason, sometimes another.
# Try running with --cache_test_results=no

require 'json'

class A
  extend T::Sig

  sig {params(blk: T.proc.params(arg0: T::Hash[String, T.untyped]).void).void}
  def self.load_flags(&blk)
    cache_file_path = T.let("./foo.json", T.nilable(String))
    if cache_file_path
      load_flags_from_cache_file(cache_file_path, &blk)
    end
  end

  sig do
    params(
      path: String,
      blk: T.proc.params(arg0: T::Hash[String, T.untyped]).void
    )
      .void
  end
  def self.load_flags_from_cache_file(path, &blk)
    begin
      flags = hash_from_json_path(path)
    rescue
      puts "would have sent metric"
      raise
    end
    yield flags
  end

  sig {params(path: String).returns(Hash)}
  def self.hash_from_json_path(path)
    puts "Would have read: #{path}"
    JSON.parse('{}')
  end
end

A.load_flags do |flags|
  p flags
end
