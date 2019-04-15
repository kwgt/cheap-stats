#! /usr/bin/env ruby
# coding: utf-8

require 'test/unit'
require 'cheap_stats'

class TestCheapStats < Test::Unit::TestCase
  SAMPLES = [7.0, 4.0, 1.0, 5.0, 3.0, 10.0, 6.0, 2.0, 8.0, 9.0]

  test "initialize" do
    stats = assert_nothing_raised {
      CheapStats.new(SAMPLES)
    }

    assert_kind_of(CheapStats, stats)
  end

  test "total" do 
    stats = assert_nothing_raised {
      CheapStats.new(SAMPLES)
    }

    assert_equal(55.0, stats.total)
  end

  test "mean" do 
    stats = assert_nothing_raised {
      CheapStats.new(SAMPLES)
    }

    assert_equal(5.5, stats.mean)
  end

  test "min,max" do 
    stats = assert_nothing_raised {
      CheapStats.new(SAMPLES)
    }

    assert_equal(1.0, stats.min)
    assert_equal(10.0, stats.max)
  end

  test "q1,q3" do
    stats = assert_nothing_raised {
      CheapStats.new(SAMPLES)
    }

    assert_equal(3.0, stats.q1)
    assert_equal(8.0, stats.q3)
  end

  test "std" do
    stats = assert_nothing_raised {
      CheapStats.new(SAMPLES)
    }

    assert_in_delta(2.87228132327, stats.std, 10e-6)
  end

  test "cdf" do
    stats = assert_nothing_raised {
      CheapStats.new(SAMPLES)
    }

    assert_equal(0.0, stats.cdf(1.0))
    assert_equal(0.4, stats.cdf(5.5))
    assert_equal(0.9, stats.cdf(10.0))
    assert_equal(1.0, stats.cdf(10.1))
  end
end
