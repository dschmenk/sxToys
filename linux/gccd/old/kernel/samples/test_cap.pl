#!/usr/bin/perl

# Get exposure value from command line.

if ($ARGV[0] == 0) {
	$msec = 1000;
} else {
	$msec = $ARGV[0];
}

open (CCD, "+</dev/ccdA") or die "Can't open CCD: $!";

# Capture full frame

print CCD "<EXPOSE MSEC=$msec/>";

if (($header = <CCD>) =~ /<IMAGE/i) {
	$header =~ /HEIGHT\s*=\s*(\d+)/i;
	$height =  $1;
	$header =~ /WIDTH\s*=\s*(\d+)/i;
	$width =  $1;
	until (($pixel = <CCD>) =~ /IMAGE>/) {
		chomp $pixel;
		push @image, $pixel;
	}
}
close CCD;

# Write PGM file

print "P2 $width $height 65535\n";
for ($j=0; $j < $height; $j++) {
	for ($i = 0; $i < $width; $i++) {
		print $image[$j * $width + $i], " ";
	}
	print "\n";
}
