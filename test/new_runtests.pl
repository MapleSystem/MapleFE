#!/usr/bin/perl

use Cwd;
use warnings;
use experimental 'smartmatch';
use File::Find;
use File::Basename;

my $StartTime= localtime;

my $pwd = getcwd;

#print "here1 $pwd\n";

if(!(defined $ARGV[0])) {
  print "------------------------------------------------\n";
  print "usage: runtests.pl all or java2mpl or errtest or others or openjdk or syntaxonly\n";
  print "------------------------------------------------\n";
  exit;
}

if ($ARGV[0] eq 'all') {
  @dirname = qw(java2mpl others openjdk syntaxonly);
  print "Run java2mpl others openjdk syntaxonly\n";
} elsif ($ARGV[0] eq 'allall') {
  @dirname = qw(java2mpl errtest others openjdk syntaxonly);
  print "Run java2mpl errtest others openjdk\n";
} elsif ($ARGV[0] ~~ [qw( java2mpl errtest others openjdk syntaxonly )]) {
  @dirname = "$ARGV[0]";
  print "Run $ARGV[0]\n";
} else {
  print "$ARGV[0] is an invalid option\n";
  exit;
}

my @failed_file;
my @successed_file;

my $count2 = 0;
my $counttotal2 = 0;
my $countfailedjava2 = 0;
my $countsub2 = 0;

system("rm -Rf report.txt $pwd/output $pwd/diff $pwd/notexists");

my $currdir = "$pwd";
my $outroot = "$currdir/../output/test";

if(!(-e "$outroot")) {
  system("mkdir -p $outroot");
}

my $dir2;

sub process_file {
    if (($_ ne "..") && ($_ ne "temp")) {
      my $predir2 = getcwd;
      if (-d && $_ eq 'fp'){
        $File::Find::prune = 1;
        return;
      }
      if ( -f and (/(.+)[.]java$/) ) {
	 $file2 = $_;
         $filebase2 = basename($predir2);
	 my $outdir2 = "$outroot/$dir2";
         my $diffdir2 = "$pwd/diff/$dir2";

	 $count2 ++;
	 $counttotal2 ++;
	 $countsub2 ++;
	 print ".";
	 if ($count2 % 50 == 0) {
           print " $count2\n";
	 }
         my $flag2 = 0;

	 $oresult_filename2 = "$predir2/$file2".'.result';
	 my $diff_file2 = $file2.'.'.$filebase2.'.diff';

	 if ( $dir2 ne $filebase2 ) {
	   $src_file2 = "$filebase2/$file2";  
	   $fulldir2 = "$dir2/$filebase2";
	   $oresult_file2 = $file2.'.result';
	   $result_file2 = "$filebase2/".$file2.'.'.$filebase2.'.result';
	   $notexistsdir2 = "$outroot/notexists/$dir2/$filebase2";
	 } else {
	   $src_file2 = $file2;
	   $fulldir2 = $dir2;
	   $oresult_file2 = $file2.'.result';
           $result_file2 = $file2.'.'.$filebase2.'.result';
	   $notexistsdir2 = "$outroot/notexists/$dir2";
	 }

         if ($dir2 ~~ [qw( java2mpl errtest others openjdk syntaxonly )]) {
	   $res2 = system("$pwd/../output/java/java2mpl $outdir2/$src_file2 > $outdir2/$result_file2");
         }

	 if ($res2 > 0) {
	   if ($dir2 ~~ [qw( java2mpl errtest others openjdk syntaxonly )]) {
	     print "$pwd/../output/java/java2mpl $outdir2/$src_file2\n";
           }
	   print " ==$fulldir2===> $file2\n";
	   $countfailedjava2 ++;
	   push(@failed_file, $fulldir2.":  ".$file2);
	   #print "---------------------------\n";
	   print "countfailedjava2 $countfailedjava2\n";
         }

	 if (!(-e $oresult_filename2)) {
           if(!(-e "$notexistsdir2")) {
             system("mkdir -p $notexistsdir2");
           }

           print "\nOriginal file $oresult_filename2 does NOT exists!\n";
           system("touch $notexistsdir2/$oresult_file2");
           $countfailedjava2 ++;
           push(@failed_file, $fulldir2.": ".$file2);
           #print "---------------------------\n";

	 } else {

           if ((!(-e "$outdir2/$result_file2")) || (-z "$outdir2/$result_file2")) {
             if(!(-e "$notexistsdir2")) {
               system("mkdir -p $notexistsdir2");
             }

             print "\n$outdir2/$result_file2 either empty or not exists!\n";
             system("touch $notexistsdir2/$result_file2");
             $countfailedjava2 ++;
             push(@failed_file, $fulldir2.": ".$file2);
             #print "---------------------------\n";
           } else {
#print "COMPARE file $pwd/$dir/$oresult_file and $outdir/$result_file\n";
             $res22 = system("diff $pwd/$fulldir2/$oresult_file2 $outdir2/$result_file2");
             if ($res22 > 0) {
               if(!(-e "$diffdir2")) {
                 system("mkdir -p $diffdir2");
               }

               print "\n$fulldir2/$oresult_file2 $outdir2/$result_file2 are different!!!\n";
               system("touch $diffdir2/$diff_file2");
               $countfailedjava2 ++;
               push(@failed_file, $fulldir2.": ".$file2);
               #print "---------------------------\n";
             } else {
#print "GOOD files: $file\n";
               push(@successed_file, $file2." ".$fulldir2);
               #print "---------------------------\n";
             }
           }

	 }
      }

    }
}

foreach my $dir1 (@dirname) {
  my $predir1 = getcwd;
  $dirname1 = "$predir1/$dir1";

  system("cp -rp $dir1 $outroot/");

  chdir $dirname1;
  opendir (DIR, $dirname1) || die "Error in opening dir $dirname1\n";

  print("\n====================== BEGIN tests: $dir1 =====================\n");

  $dir2 = $dir1;
  find(\&process_file, $dirname1);
  chdir $pwd;
  print("\n====================== END $dir1 test cases: $countsub2 ================\n");
  $countsub2 = 0;


}
closedir(DIR);

my $countFailed = $countfailedjava2;
my $countPassed = $count2 - $countFailed;

my $reportFile = "$outroot/report.txt";
open(my $fh, '>', $reportFile) or die "Could not open file '$reportFile' $!";
#if ($ARGV[0] eq "all") {
#  print $fh "Report: \n";
#} else {
#  print $fh "$ARGV[0] report: \n";
#}
print $fh "$ARGV[0] report: \n";


if ($countFailed eq 0) {
  print("\n all $count2 tests passed\n");
  print("======================================================\n");
  print $fh "all $count2 tests passed\n";
  close $fh;
} else {
  print "\n\n=====Scan Result=====\n\n";
  print "Total Test Cases: $count2\n";
  if(scalar(@successed_file) > 0) {
    print "\n=========================\npassed $countPassed tests:\n\n";
    #foreach $passed (@successed_file) {
    #  print $passed."\n";
    #}
    #print $fh "$countPassed testcases passed\n";
  }
  print "\n=========================\nfailed $countFailed tests:\n\n";
  if(scalar(@failed_file) > 0){
    print("=== failed : $countfailedjava2 tests\n");
    print $fh "$countfailedjava2 testcases failed\n";

    foreach $failed (@failed_file) {
      print $failed."\n";
      print $fh $failed."\n";
    }
    print "\n";
  }
  print "=========================\n";
  close $fh;
}

