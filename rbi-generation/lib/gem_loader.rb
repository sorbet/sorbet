# frozen_string_literal: true
# typed: true

class SorbetRBIGeneration::GemLoader
  NO_GEM = "_unknown"

  # A map defining the code to load a gem. By default any gem mentioned by
  # Gemfile is loaded by its name, here are either overrides or things that
  # Bundler misses.
  GEM_LOADER = {
    'Ascii85' => proc do
      my_require 'ascii85'
    end,
    'aws-sdk-v1' => proc do
      my_require 'aws-sdk-v1'
      AWS.eager_autoload!
      [
        AWS::DynamoDB::Errors::InternalServerError,
        AWS::DynamoDB::Errors::ProvisionedThroughputExceededException,
        AWS::DynamoDB::Errors::ResourceInUseException,
        AWS::DynamoDB::Errors::ResourceNotFoundException,
        AWS::EC2::Errors::RequestLimitExceeded,
        AWS::S3::Errors::AccessDenied,
        AWS::S3::Errors::NoSuchBucket,
        AWS::S3::Errors::NotFound,
      ]
    end,
    'aws-sdk-core' => proc do
      my_require 'aws-sdk'
      [
        Aws::AssumeRoleCredentials,
        Aws::Athena,
        Aws::AutoScaling::Client,
        Aws::AutoScaling::Errors::AlreadyExists,
        Aws::AutoScaling::Errors::Throttling,
        Aws::AutoScaling::Errors::ValidationError,
        Aws::CloudFormation::Client,
        Aws::CloudFormation::Errors::ValidationError,
        Aws::CloudWatchLogs::Client,
        Aws::Credentials,
        Aws::DynamoDB::Client,
        Aws::DynamoDB::Errors::ProvisionedThroughputExceededException,
        Aws::EC2::Errors::RequestLimitExceeded,
        Aws::ElasticLoadBalancing::Client,
        Aws::Errors::ServiceError,
        Aws::IAM::Client,
        Aws::IAM::Errors::NoSuchEntity,
        Aws::IAM::Resource,
        Aws::InstanceProfileCredentials,
        Aws::Lambda::Client,
        Aws::Query::ParamList,
        Aws::S3::Bucket,
        Aws::S3::Client,
        Aws::S3::Encryption::Client,
        Aws::S3::Errors::InvalidRange,
        Aws::S3::Errors::NoSuchKey,
        Aws::S3::Errors::NotFound,
        Aws::S3::Object,
        Aws::S3::Resource,
        Aws::SES::Client,
        Aws::SES::Errors,
        Aws::SES::Errors::AccessDenied,
        Aws::SES::Errors::MessageRejected,
        Aws::SES::Errors::ServiceError,
        Aws::SES::Types,
        Aws::SES::Types::SendEmailResponse,
        Aws::SNS::Client,
        Aws::SNS::MessageVerifier,
        Aws::SQS::QueuePoller,
        Aws::STS::Client,
        Aws::STS::Errors::AccessDenied,
        Seahorse::Client::NetworkingError,
        Seahorse::Client::Response,
      ]
    end,
    'bloomfilter-rb' => proc do
      my_require 'bloomfilter-rb'
    end,
    'hashie' => proc do
      my_require 'hashie'
      [
        Hashie::Mash,
      ]
    end,
    'hike' => proc do
      my_require 'hike'
      [
        Hike::Trail,
      ]
    end,
    'http' => proc do
      my_require 'http'
    end,
    'http-form_data' => proc do
      my_require 'http/form_data'
    end,
    'http_parser.rb' => proc do
      my_require 'http/parser'
    end,
    'minitest' => proc do
      my_require 'minitest'
      my_require 'minitest/mock'
      my_require 'minitest/spec'
      my_require 'minitest/reporters'
    end,
    'rack-test' => proc do
      my_require 'rack/test'
    end,
    'pagerduty-full' => proc do
      my_require 'pagerduty/full'
    end,
    'puma' => proc do
      my_require 'puma'
      my_require 'puma/configuration'
      my_require 'puma/launcher'
      my_require 'puma/server'
    end,
    'term-ansicolor' => proc do
      my_require 'term/ansicolor'
    end,
    'rexml' => proc do
      my_require "rexml/document"
      my_require "rexml/streamlistener"
    end,
    'rubyzip' => proc do
      my_require "zip"
      my_require 'zip/filesystem'
    end,
    'nsq-ruby' => proc do
      my_require 'nsq'
    end,
    'mongo-ruby-driver' => proc do
      my_require 'mongo'
    end,
    'presto-client-ruby' => proc do
      my_require 'presto-client'
    end,
    'bcrypt-ruby' => proc do
      my_require 'bcrypt'
    end,
    'xml-simple' => proc do
      my_require 'xmlsimple'
    end,
    'sinatra-contrib' => proc do
      # We can't my_require all of 'sinatra/contrib' since we put `raise` in them
      my_require 'sinatra/content_for'
      my_require 'sinatra/capture'
      my_require 'sinatra/multi_route'
      my_require 'sinatra/contrib/version'
    end,
    'thin-attach_socket' => proc do
      my_require 'thin'
      my_require 'thin/attach_socket'
    end,
    'sinatra-partial' => proc do
      my_require 'sinatra/partial'
    end,
    'rack_csrf' => proc do
      my_require 'rack/csrf'
    end,
    'rack-flash3' => proc do
      my_require 'rack-flash'
    end,
    'google-api-client' => proc do
      # There are lots more but this is all we use for now
      my_require 'google/apis/calendar_v3'
      my_require 'google/apis/drive_v3'
      my_require 'google/apis/gmail_v1'
    end,
    'concurrent-ruby' => proc do
      my_require 'concurrent'
    end,
    'cld2' => proc do
      my_require 'cld'
    end,
    'twitter_cldr' => proc do
      my_require 'twitter_cldr'
      [
        TwitterCldr::Shared::Territories,
      ]
    end,
    'stackprof' => proc do
      my_require 'stackprof'
      [
        StackProf::Report,
      ]
    end,
    'sprockets' => proc do
      my_require 'sprockets'
      [
        Sprockets::Cache::FileStore,
        Sprockets::Environment,
        Sprockets::Manifest,
      ]
    end,
    'signet' => proc do
      my_require 'signet'
      my_require 'signet/oauth_2/client'
    end,
    'roo' => proc do
      my_require 'roo'
      [
        Roo::Excel,
        Roo::Spreadsheet,
      ]
    end,
    'rack-protection' => proc do
      my_require 'rack-protection'
      [
        Rack::Protection::FrameOptions,
      ]
    end,
    'rack' => proc do
      my_require 'rack'
      [
        Rack::Auth::Basic::Request,
        Rack::Builder,
        Rack::Deflater,
        Rack::File,
        Rack::Mime,
        Rack::MockRequest,
        Rack::MockResponse,
        Rack::Session::Cookie,
        Rack::Static,
      ]
    end,
    'poncho' => proc do
      [
        Poncho::ClientError,
        Poncho::JSONMethod,
        Poncho::Resource,
        Poncho::ValidationError,
      ]
    end,
    'parser' => proc do
      my_require 'parser'
      my_require 'parser/ruby24'
    end,
    'net' => proc do
      my_require 'net/dns'
      my_require 'net/ftp'
      my_require 'net/http'
      my_require 'net/http/digest_auth'
      my_require 'net/http/persistent'
      my_require 'net/imap'
      my_require 'net/protocol'
      my_require 'net/sftp'
      my_require 'net/smtp'
      my_require 'net/ssh'
      my_require 'net/ssh/proxy/http'
      my_require 'rubyntlm'
    end,
    'openssl' => proc do
      my_require 'openssl'
      [
        OpenSSL::X509::Store,
      ]
    end,
    'mail' => proc do
      my_require 'mail'
      [
        Mail::Address,
        Mail::AddressList,
        Mail::Parsers::AddressListsParser,
        Mail::Parsers::ContentDispositionParser,
        Mail::Parsers::ContentLocationParser,
        Mail::Parsers::ContentTransferEncodingParser,
        Mail::Parsers::ContentTypeParser,
        Mail::Parsers::DateTimeParser,
        Mail::Parsers::EnvelopeFromParser,
        Mail::Parsers::MessageIdsParser,
        Mail::Parsers::MimeVersionParser,
        Mail::Parsers::PhraseListsParser,
        Mail::Parsers::ReceivedParser,
      ]
    end,
    'kramdown' => proc do
      my_require 'kramdown'
      [
        Kramdown::Converter::Html,
      ]
    end,
    'ice_cube' => proc do
      my_require 'ice_cube'
      [
        IceCube::DailyRule,
        IceCube::MonthlyRule,
        IceCube::WeeklyRule,
        IceCube::YearlyRule,
        IceCube::Schedule,
      ]
    end,
    'i18n' => proc do
      my_require 'i18n'
      [
        I18n::Locale::Tag::Rfc4646,
      ]
    end,
    'http-cookie' => proc do
      my_require 'http-cookie'
      my_require 'mechanize'
      [
        HTTP::CookieJar::AbstractSaver,
      ]
    end,
    'faraday' => proc do
      my_require 'faraday'
      [
        Faraday::Request::Multipart,
        Faraday::Request::UrlEncoded,
        Faraday::Response::RaiseError,
      ]
    end,
    'escort' => proc do
      my_require 'escort'
      [
        Escort::App,
        Escort::Setup::Dsl::Options,
        Escort::Trollop::Parser,
      ]
    end,
    'digest' => proc do
      my_require 'digest'
      [
        Digest::SHA2,
      ]
    end,
    'coderay' => proc do
      my_require 'coderay'
      [
        CodeRay::PluginHost,
      ]
    end,
    'byebug' => proc do
      my_require 'byebug'
      my_require 'byebug/core'
    end,
    'racc' => proc do
      my_require 'parser'
    end,
    'rbnacl' => proc do
      my_require 'rbnacl/libsodium'
    end,
    'double-bag-ftps' => proc do
      my_require 'double_bag_ftps'
    end,
    'livechat_client' => proc do
      my_require 'livechat'
    end,
    'nio4r' => proc do
      my_require 'nio'
    end,
    'rgl' => proc do
      my_require 'rgl/adjacency'
      my_require 'rgl/implicit'
      my_require 'rgl/traversal'
      my_require 'rgl/graph_iterator'
      my_require 'rgl/edmonds_karp'
    end,
    'redcarpet' => proc do
      my_require 'redcarpet'
      my_require 'redcarpet/render_strip'
    end,
    'sequel' => proc do
      my_require 'sequel'
      my_require 'sequel/sql'
    end,
    'will_paginate' => proc do
      my_require 'will_paginate'
      my_require 'will_paginate/collection'
    end,
    'yard' => proc do
      my_require 'yard'
      [
        YARD::CodeObjects::MethodObject,
        YARD::Docstring,
        YARD::Handlers::Ruby::Base,
        YARD::Registry,
        YARD::Tags::Library,
        YARD::Tags::Tag,
      ]
    end,
    'mocha' => proc do
      my_require 'minitest/spec' # mocha forces you to do this first
      my_require 'mocha/setup'
    end,
    'bundler-audit' => proc do
      my_require 'bundler/audit'
    end,
    'google-protobuf' => proc do
      my_require 'google/protobuf'
    end,
    'multipart-post' => proc do
      my_require 'net/http/post/multipart'
    end,
    'rdl' => proc do
      # needed because this isn't required by default in the Gemfile
      my_require 'rdl_disable'
    end,
    'rss' => proc do
      # needed because this isn't required our Gemfile but some of our gems use it
      my_require 'rss'
    end,
    'ruby-ole' => proc do
      my_require 'ole/storage'
    end,
    'ruby-rc4' => proc do
      my_require 'rc4'
    end,
    'ruby-prof' => proc do
      my_require 'ruby-prof'
      [
        RubyProf::AbstractPrinter,
      ]
    end,
    'stylus-source' => proc do
      my_require 'stylus'
    end,
    'time-utils' => proc do
      my_require 'time/utils'
      my_require 'date/utils'
    end,
    'thor' => proc do
      my_require 'thor'
      [
        Thor::Actions,
        Thor::Group,
      ]
    end,
    'unicode-display_width' => proc do
      my_require 'unicode/display_width'
    end,
    'simplecov-html' => proc do
      my_require 'simplecov'
    end,
    'thwait' => proc do
      my_require 'thwait'
    end,
    'matrix' => proc do
      my_require 'matrix'
    end,
    'zxcvbn-ruby' => proc do
      my_require 'zxcvbn'
    end,
    'elasticsearch-transport' => proc do
      my_require 'elasticsearch'
    end,
    'tzinfo' => proc do
      my_require 'tzinfo'
      my_require 'tzinfo/data'
      TZInfo::Timezone.all.map(&:canonical_zone)
    end,
    'pry-doc' => proc do
      my_require 'pry'
      my_require 'pry-doc'
    end,
    'taxjar-ruby' => proc do
      my_require 'taxjar'
    end,
    'nokogiri' => proc do
      my_require 'nokogiri'
      my_require 'webrobots'
      my_require 'html_truncator'
    end,
    'actionpack' => proc do
      [
        ActionController::Base,
        ActionDispatch::SystemTestCase,
      ]
    end,
    'actionmailer' => proc do
      [
        ActionMailer::Base,
        ActionMailer::MessageDelivery,
      ]
    end,
    'activejob' => proc do
      [
        ActiveJob::Base,
      ]
    end,
    'activerecord' => proc do
      [
        ActiveRecord::Schema,
        ActiveRecord::Migration::Current,
      ]
    end,
    'activesupport' => proc do
      [
        ClearanceMailer,
        ActionView::TestCase,
      ]
    end,
    'rdoc' => proc do
      [
        RDoc::Options,
      ]
    end,
    'paul_revere' => proc do
      [
        Announcement,
      ]
    end,
  }

  # This is so that the autoloader doesn't treat these as manditory requires
  # before loading this file
  def self.my_require(gem)
    require gem # rubocop:disable PrisonGuard/NoDynamicRequire
  end

  def self.require_gem(gem)
    if gem == NO_GEM
      require_all_gems
      return
    end
    loader = GEM_LOADER[gem]
    if loader
      loader.call
    else
      require gem # rubocop:disable PrisonGuard/NoDynamicRequire
    end
  end

  def self.require_all_gems
    require 'bundler/setup'
    Bundler.require

    Bundler.load.specs.sort_by(&:name).each do |gemspec|
      begin
        require_gem(gemspec.name)
      rescue LoadError
      end
    end
  end
end
# rubocop:enable PrisonGuard/AutogenLoaderPreamble
