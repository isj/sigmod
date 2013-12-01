#!/usr/bin/perl
##use strict; use warnings;

open( INPUT_FILE, "<", "7mb.txt");
open ( OUTPUT_FILE, ">", "7mb_edit_distance.txt");

my @good_queries;
my @good_query_ids;
my @bad_queries;
my $good_query = 2;
=good_query match_types
	0 = exact_match
	1 = hamming_match
	2 = edit_match
=cut
while (my $line = <INPUT_FILE>){
     chomp $line;
     if ($line =~ /^s/) {
			if ($line =~ /^s \d+ $good_query/) {
				##exact match search query
				push (@good_queries, $line);
				(my $query_id = $line) =~ s/s (\d+).*$/\1/;
				push (@good_query_ids, $query_id);
				print OUTPUT_FILE "$line\n";
			}
     } 
	if ($line =~ /^e/) {
	   ($deletion = $line) =~ s/e (\d+).*$/\1/;
	   for ($i = 0, $i<$#good_queries+1,$i++) {
	       if ($i = $good_queries[$i]) {
	          splice(@good_queries,$i,1);
	       }
	   }
	   print OUTPUT_FILE "$line\n";

	}

    if ($line =~ /^m/) {
    	print OUTPUT_FILE "$line\n";
    }
     if ($line =~ /^r/) {
     		print "\nchecking line $line\n";
     		$good_query_ids = join(" ",@good_query_ids);

			@read_array = split(' ',$line);
			@results_array = @read_array[0..2];
			shift (@read_array);
			shift (@read_array);
			shift (@read_array);
			$read_array_length = scalar(@read_array);
			print "length: $read_array_length\n";
	        for ($i=0; $i<$read_array_length; $i++) {
				if (grep {$_ eq @read_array[$i]} @good_query_ids) {
					##print "found @read_array[$i]\n" ;
					push (@results_array,@read_array[$i]);
					print "building...".join(" ",@results_array)." \n";
				}
			}
			print join(" ",@results_array)."\n";
			$results_count =  scalar(@results_array)-3;
			print "results_count $results_count\n";
			if ($results_count<0) {
				$results_count = 0;
				}
			$results_array[2] = $results_count;
			$results_line = join(" ",@results_array)."\n";
			print "results_line..".$results_line;
			print OUTPUT_FILE $results_line ;
     }
}

close INPUT_FILE;
close OUTPUT_FILE;
