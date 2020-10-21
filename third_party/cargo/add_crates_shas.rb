require 'uri'
require 'net/http'
require 'digest'

def main
  data = ""
  File.open("./crates.bzl").each_line do |line|
    data << line
    if line.include?("url = ")
      url = line.split("\"")[1].split("\"")[0]
      uri = URI(url)
      res = Net::HTTP.get(uri)
      hash = Digest::SHA256.hexdigest(res)
      data << "        sha256 = \"#{hash}\",\n"
    end
  end

  File.open("./crates.bzl", "w") do |fp|
    fp.write(data)
  end
end

if __FILE__ == $0
  main
end
