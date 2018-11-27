# typed: true

class Gem::RemoteFetcher
  include(Gem::UserInteraction)
  include(Gem::DefaultUserInteraction)

  BASE64_URI_TRANSLATE = T.let(T.unsafe(nil), Hash)

  class UnknownHostError < ::Gem::RemoteFetcher::FetchError

  end

  class FetchError < ::Gem::Exception
    def uri=(_)
    end

    def to_s()
    end

    def uri()
    end
  end

  def fetch_file(uri, *_)
  end

  def fetch_http(uri, last_modified = _, head = _, depth = _)
  end

  def api_endpoint(uri)
  end

  def headers=(_)
  end

  def fetch_path(uri, mtime = _, head = _)
  end

  def request(uri, request_class, last_modified = _)
  end

  def fetch_https(uri, last_modified = _, head = _, depth = _)
  end

  def fetch_s3(uri, mtime = _, head = _)
  end

  def fetch_size(uri)
  end

  def download_to_cache(dependency)
  end

  def https?(uri)
  end

  def cache_update_path(uri, path = _, update = _)
  end

  def close_all()
  end

  def download(spec, source_uri, install_dir = _)
  end

  def correct_for_windows_path(path)
  end

  def headers()
  end
end
