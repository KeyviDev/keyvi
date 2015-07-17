#!perl

# Copyright 2011, The TPIE development team
# 
# This file is part of TPIE.
# 
# TPIE is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
# 
# TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with TPIE.  If not, see <http://www.gnu.org/licenses/>

use warnings;
use strict;

my $pq = 0;

while (@ARGV) {
    if ($ARGV[0] eq '--pq') {
	shift @ARGV;
	$pq = 1;
    }
}

my %tests = (
    sort => qr/test_ami_sort\D*(\d+)/,
    pq => qr/speed_regression\/pq_speed_test \d+ (\d+)/,
);
my %dirs = (
    filestream128 => qr/tpie\/Release/,
    filestream2MB => qr/tpie\/Release-BS2MB/,
    memory128 => qr/tpie-memory\/Release/
);

INPUT: while (1) {

    # Seek to first test
    while (<>) {
        last if /BEGIN TEST/;
    }
    last INPUT unless defined $_;

    my $test = ['', undef];
    my $dir = '';

    if ($pq) {
	# First line contains block size
	my $bs = <>;
	if ($bs =~ /Block size: (\d+(?:\.\d+)?)/) {
	    $dir = $1;
	} else {
	    $dir = 'unknown';
	}
	$test = ['pq', ''];

    } else {

	# First line contains the test name
	my $testline = <>;
	for (keys %tests) {
	    if ($testline =~ $tests{$_}) {
		$test = [$_, $1];
		last;
	    }
	}
	$test = ['', undef] unless defined $test;

	# Second line contains the build name
	my $dirline = <>;
	for (keys %dirs) {
	    if ($dirline =~ $dirs{$_}) {
		$dir = $_;
		last;
	    }
	}
    }

    # Do some test-specific input parsing
    if ($test->[0] eq 'pq') {
	# Priority queue test. Get the average push and pop times.
        my $pushx = 0;
        my $pushn = 0;
        my $popx = 0;
        my $popn = 0;

	# Seek to result header
        while (<>) {
	    if ($pq && /\d+ times, (\d+) elements/) {
		$test->[1] = $1;
	    }
            last if /Elems Push Pop Total/;
        }

	# Read times
        while (<>) {
            last unless /\S+ (\d+) (\d+) (\d+) (\d+)/;
            $pushx += $2;
            ++$pushn;
            $popx += $3;
            ++$popn;
        }

	# Output a line for pushing and one for popping
        print "pq-push,$dir,$test->[1],", $pushx/$pushn/1000000, "\n",
	    "pq-pop,$dir,$test->[1],", $popx/$popn/1000000, "\n" if $pushn > 0;

    } elsif ($test->[0] eq 'sort') {
	# Sort test. Output the wall clock time reported by /usr/bin/time.
        my $elapsed;
        while (<>) {
            if (/(\d+):(\d+\.\d+)elapsed/) {
		$elapsed = $1*60+$2;
		last;
            }
        }
        print "sort,$dir,$test->[1],$elapsed\n";
    }

    # Seek to footer
    while (<>) {
        last if /END TEST/;
    }
    last INPUT unless defined $_;
}
# vim:ts=8 sts=4 sw=4 noet:
