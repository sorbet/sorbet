#!/usr/bin/env ruby

require_relative 'parse'
require 'fileutils'
require 'digest'

Dir.glob(File.join(ARGV.first, '**/*.rb')) do |file|
  begin
    code = File.read(file)
    sexp_rb = parse_ruby(code, file).to_s.strip
    next if code.empty? or sexp_rb.empty?

    hash = Digest::MD5.hexdigest(code)

    dirname = File.join("library", hash[0..1])
    filename = File.join(dirname, hash[2..-1])

    FileUtils.mkdir_p(dirname)
    File.write("#{filename}.rb", code)
    File.write("#{filename}.rbast", sexp_rb)
  rescue => err
    puts err
    next
  end
end
