#!/usr/bin/perl -w

use strict;

BEGIN:
my $input  = shift or die("Please supply template filename as 1st argument");
my $prefix = shift or die("Please supply tinyaml install prefix as 2nd argument");
my $version = shift or die("Please supply tinyalk version number as 3rd argument");


sub check_dir() {
	while ($_=shift) { return $_ if -d $_; }
	return undef;
}

my $pkgconfigdir = &check_dir("/usr/local/lib/pkgconfig", "/usr/share/pkgconfig", "/usr/lib/pkgconfig") or die("Can't locate pkg-config install directory");

open my $fd, "<$input";
my $pcfile = "$pkgconfigdir/$input";
$pcfile =~ s/\.in$//;
open my $out, ">$pcfile" or die("Couldn't open file $pcfile for writing.\n");

print "Installing pkg-config file as $pcfile.\n";

while(my $line=<$fd>) {
	$line =~ s/#PREFIX#/$prefix/g;
	$line =~ s/#VERSION#/$version/g;
	print $out $line;
}



