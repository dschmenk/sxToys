#!/usr/bin/perl

open (CCD, "+</dev/ccdA") or die "Can't open CCD: $!";

print CCD "<QUERY/>";
print <CCD>;
close CCD;
