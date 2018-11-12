#!/usr/bin/perl

open (CCD, "+</dev/ccdA") or die "Can't open CCD: $!";

print CCD "<QUERY/>";
$ccd = <CCD>;
$ccd =~ /HEIGHT\s*=\s*(\d+)/i;
$ccd_height =  $1;
$ccd =~ /WIDTH\s*=\s*(\d+)/i;
$ccd_width =  $1;

while (1) {

	$width  = int rand $ccd_width;
	$height = int rand $ccd_height;
	if ($width == 0) {
		$width = 1;
	}
	if ($height == 0) {
		$height = 1;
	}
	$xoffset = int rand $ccd_width;
	$yoffset = int rand $ccd_height;
	if ($xoffset + $width > $ccd_width) {
		$xoffset = $ccd_width - $width;
	}
	if ($yoffset + $height > $ccd_height) {
		$yoffset = $ccd_height - $height;
	}
	print "<EXPOSE WIDTH=",   $width,
                      " HEIGHT=",  $height,
	              " XOFFSET=", $xoffset,
	              " YOFFSET=", $yoffset,
                      " MSEC=1000/>\n";
	print CCD "<EXPOSE WIDTH=",   $width,
                          " HEIGHT=",  $height,
			  " XOFFSET=", $xoffset,
			  " YOFFSET=", $yoffset,
			  " MSEC=1000/>";

	if (($header = <CCD>) =~ /<IMAGE/i) {

		$header =~ /HEIGHT\s*=\s*(\d+)/i;
		$height =  $1;
		$header =~ /WIDTH\s*=\s*(\d+)/i;
		$width =  $1;
		until (($pixel = <CCD>) =~ /IMAGE>/) {
			chomp $pixel;
#			push @image, $pixel;
		}
	}
}
close CCD;

#
# Write PGM file
#
#print "P2 ", $width, " ", $height, " 65535\n";
#for ($j=0; $j < $height; $j++) {
#	for ($i = 0; $i < $width; $i++) {
#		print $image[$j * $width + $i], " ";
#	}
#	print "\n";
#}
