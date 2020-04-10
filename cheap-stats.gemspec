
lib = File.expand_path("../lib", __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require "cheap_stats/version"

Gem::Specification.new do |spec|
  spec.name          = "cheap-stats"
  spec.version       = CheapStats::VERSION
  spec.authors       = ["Hiroshi Kuwagata"]
  spec.email         = ["kgt9221@gmail.com"]

  spec.summary       = %q{A cheaper statistics library.}
  spec.description   = %q{A cheaper statistics library.}
  spec.homepage      = "https://github.com/kwgt/cheap-stats"
  spec.license       = "MIT"

  if spec.respond_to?(:metadata)
    spec.metadata["homepage_uri"] = spec.homepage
  else
    raise "RubyGems 2.0 or newer is required to protect against " \
      "public gem pushes."
  end

  spec.files         = Dir.chdir(File.expand_path('..', __FILE__)) do
    `git ls-files -z`.split("\x0").reject { |f|
      f.match(%r{^(test|spec|features)/})
    }
  end

  spec.bindir        = "bin"
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.extensions    = ["ext/cheap_stats/extconf.rb"]
  spec.require_paths = ["lib"]

  spec.required_ruby_version = ">= 2.4.0"

  spec.add_development_dependency "bundler", ">= 2.1"
  spec.add_development_dependency "rake", ">= 12.3.3"
  spec.add_development_dependency "rake-compiler", "~> 1.1.0"
end
