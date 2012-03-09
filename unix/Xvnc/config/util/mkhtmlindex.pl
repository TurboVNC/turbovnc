#!/usr/bin/perl
#
# $XFree86: xc/config/util/mkhtmlindex.pl,v 1.3 2001/03/18 14:49:01 dawes Exp $
#
# Copyright © 2000,2001 by VA Linux Systems, Inc.
#
# Generate index files for HTML man pages.
#
# Author:	David Dawes <dawes@xfree86.org>
#

#
# Best viewed with tabs set to 4
#

if ($#ARGV ne 0) {
	print STDERR "Usage: mkhtmlindex.pl htmlmandir\n";
	exit 1;
}

$dir = $ARGV[0];

if (! -d $dir) {
	print STDERR "$dir is not a directory\n";
	exit 1;
}

@vollist = ("1", "2", "3", "4", "5", "6", "7", "8", "9", "o", "l", "n", "p");

$indexprefix = "manindex";

foreach $vol (@vollist) {
	$empty = "yes";
	$indexname="$dir/$indexprefix$vol.html";

	# print "Processing volume $vol\n";

	open(mindex, ">$indexname") || die "Can't create $indexname";
	opendir(dir, "$dir") || die "Can't open $dir";

	print mindex <<EOF;
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">
<HTML>
<HEAD>
<META HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=iso-8859-1">
<TITLE>XFree86[tm] Manual pages: Section $vol</TITLE>
</HEAD>
<BODY BGCOLOR="#efefef" TEXT="black" LINK="blue" VLINK="#551A8B" ALINK="red">

<H1>XFree86[tm] Manual pages: Section $vol</H1>
<P>
<UL>
EOF

	foreach $file (sort readdir dir) {
		if ($file =~ "\.$vol\.html") {
			open(file, "<$dir/$file") || die "Can't open $dir/$file";
			while (<file>) {
				chop;
				if (/^<H2>/) {
					if (! /<\/H2>$/) {
						while (<file> && ! /<\/H2>$/) {
							;
						}
					}
					$heading = "";
					while (<file>) {
						if (/^<H2>/) {
							last;
						}
						$heading = "$heading" . "$_";
					}
					if ($heading) {
						undef $empty;
						$heading =~ s/--/-/;
						($name, $descr) = split(/-/, $heading, 2);
						$file =~ /(.*)\.$vol\.html/;
						$fname = $1;
						$descr =~ s/<[P]>//g;
						print mindex
							"<LI><A href=\"$file\">$fname</A> - $descr</LI>";
					}
					last;
				}
			}
			close file;
		}
	}

	print mindex <<EOF;
</UL>
<P>
</BODY>
</HTML>
EOF

	close mindex;
	closedir dir;
	if (defined $empty) {
		# print "Removing empty $indexname\n";
		unlink $indexname;
	}
}
