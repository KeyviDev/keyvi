#!/usr/bin/perl -w
@ARGV > 0 or print STDERR "Usage: $0 count [maxrand]\n" and exit;
my $countint = $ARGV[0];
my $randint;
my $randmax = $ARGV[1] ? $ARGV[1]: 10000000;
my $i = 0;
my %numbers;
my $j = 0;
while ($i < $countint && $j < 2*$countint) {
  $randint = int(rand($randmax));
  if (!$numbers{$randint}) {
    $numbers{$randint} = 1;
    $i++;
    print STDOUT "$randint ";
  }
  $j++;
}
if ($j == 2*$countint && $i < $countint) {
  print STDERR "Warning: generated only $i DISTINCT values (range is too small).\n";
}
