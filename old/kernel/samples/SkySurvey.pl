#!/usr/bin/perl

#
# Sky Surveyor
#

use PDL;
use PDL::IO::Misc;
use PDL::Graphics::PGPLOT;

sub ccd_image {
   my $matrix;

   open (CCD, "+</dev/ccdA") or die "Can't open CCD: $!";
   #
   # Capture full frame
   #
   print CCD "<EXPOSE MSEC=$_/>";
   
   if (($header = <CCD>) =~ /<IMAGE/i) {
   	$header =~ /HEIGHT\s*=\s*(\d+)/i;
   	$height =  $1;
   	$header =~ /WIDTH\s*=\s*(\d+)/i;
   	$width  =  $1;
   	$header =~ /DEPTH\s*=\s*(\d+)/i;
   	$depth  =  $1;
      $matrix = zeroes($width, $height);
#      if ($depth == 32) {
#         $matrix->set_datatype($PDL_L);
#      } elsif ($depth == 16) {
         $matrix->set_datatype($PDL_US);
#      } else {
#         $matrix->set_datatype($PDL_B);
#      }
      for ($y = 0; $y < $height; $y++) {
         for ($x = 0; $x < $width; $x++) {
            if (($pixel = <CCD>) =~ /IMAGE>/) {
               close CCD;
               return $matrix;
            }
            print $pixel;
            chomp $pixel;
            set($matrix, $x, $y, $pixel);
         }
      }
   }
   close CCD;
   return $matrix;
}

$image = ccd_image(100);
#wfits $image, 'filename.fits', 16;
#print $image{BITPIX};
imag($image);


