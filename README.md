# cheap-stats
A cheap statistics library for Ruby

## Installation

```ruby
gem 'cheap-stats'
```

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install cheap-stats

## Examplw

```ruby
require 'chaep_stats'

SAMPLES = [7.0, 4.0, 1.0, 5.0, 3.0, 10.0, 6.0, 2.0, 8.0, 9.0]

stats = CheapStats.new(SMAPLES)

p stats.mean
p stats.std
p stats.skewness

```

## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).
