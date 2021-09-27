#!/usr/bin/perl

use Cwd;
use warnings;
use experimental 'smartmatch';
use File::Find;
use File::Basename;

use File::Find qw(find);

if(!(defined $ARGV[0])) {
  print "------------------------------------------------\n";
  print "usage: runtests.pl [ java | java/subdirectory | typescript | typescript/subdirectory ]\n";
  print "------------------------------------------------\n";
  exit;
}

my $lang = $ARGV[0];
my $pwd = getcwd;
my $currdir = "$pwd";

my @failed_file;
my @failed_file1;
my @failed_file2;
my @successed_file;

my $count = 0;
my $counttotal = 0;
my $countfailedcases = 0;
my $countfailedcases1 = 0;
my $countfailedcases2 = 0;
my $countsub = 0;

my $pinput = '';
my $cmnd = '';
my $outroot = '';
print "Running $lang\n";
if ($lang =~ /\Qjava\E/) {
  $pinput = "java";
  $cmnd = "../output/java/java/java2mpl";
  $cmnd1 = "echo";
  $outroot = "$currdir/../output/$pinput/test";
} elsif ($lang =~ /\Qtypescript\E/) {
  $pinput = "ts";
  $cmnd = "../output/typescript/bin/ts2ast";
  $cmnd1 = "../output/typescript/bin/ast2cpp";
  $outroot = "$currdir/../output/typescript/test";
} else {
  print "$lang is an invalid option\n";
  exit;
}

my $testdir = $lang;
# get specified directory to test if other than $lang
if(defined $ARGV[1]) {
  $testdir = $ARGV[1];
}

system("rm -Rf $outroot/report.txt $outroot/diff $outroot/notexists");

if(!(-e "$outroot")) {
  system("mkdir -p $outroot");
}

opendir (my $DIR, $testdir) || die "Error in opening $testdir directory\n";

sub listdirs {
  my @dirs = @_;
  my @files;

  if ( $pinput ~~ [qw( java ts )] ) {
    find({ wanted => sub { push @files, glob "\"$_/{*.$pinput,*.$pinput.result}\"" } , no_chdir => 1 }, @dirs);
  } else {
    find({ wanted => sub { push @files, $_ } , no_chdir => 1 }, @dirs);
  }

  return @files;
}

my @paths = listdirs($testdir) ;

foreach my $file (@paths) {
  my ($filename) = ( $file =~ /([^\\\/]+)$/s ) ;
  my ($pathname) = dirname($file);

  if ( $pinput ~~ [qw( java ts )] ) {
    system("rm -rf $outroot/$file; cp -rp --parents $file $outroot/");

    if ( ($filename =~ (/(.+)[.]java$/)) || ($filename =~ (/(.+)[.]ts$/)) ) {

      my $origresult = "$pwd/$file.result";
      my $outresult = $file.'.result.'.$pinput;
      my $diff_file = $file.'.result.diff'; 
      my $notexistsdir = "$outroot/notexists";
      my $diffdir = "$outroot/diff";

      $count ++;
      print ".";
      if ($count % 50 == 0) {
        print " $count\n";
      }

      #my $res = system("$pwd/$cmnd $outroot/$file > $outroot/$outresult");
      #my $res = system('$pwd/$cmnd $outroot/$file; $pwd/$cmnd1 $outroot/$file.ast > $outroot/$outresult');
      my $res = system("$pwd/$cmnd $outroot/$file > $outroot/$outresult");

      if ($res > 0) {
        print " ==$pinput===> $file\n";
        print "$pwd/$cmnd $outroot/$file\n";
        $countfailedcases ++;
        push(@failed_file, $pinput.":  ".$file);
        #print "---------------------------\n";
        next;
      } else {
        my $res1 = system("$cmnd1 $outroot/$file.ast --no-imported > $outroot/$outresult.1");

        if ($res1 > 0) {
          print " ==$pinput===> $file\n";
          print "$cmnd1 $outroot/$file.ast\n";
          $countfailedcases1 ++;
          push(@failed_file1, $pinput.":  ".$file);
          #print "---------------------------\n";
          next;
        }
      }

      if (!(-e $origresult) ) {
        if(!(-e "$notexistsdir")) {
          system("mkdir -p $notexistsdir");
        }
        print "\nOriginal file $origresult does NOT exists!\n";
        system("mkdir -p $notexistsdir/$file && touch $notexistsdir/$file");
        $countfailedcases2 ++;
        push(@failed_file2, $pinput.": result file not exists: ".$origresult);
      } else {
        if ((!(-e "$outroot/$outresult")) || (-z "$outroot/$outresult")) {
          if(!(-e "$notexistsdir")) {
            system("mkdir -p $notexistsdir");
          }

          print "\n$outroot/$outresult either empty or not exists!\n";
          system("mkdir -p $notexistsdir/$file && touch $notexistsdir/$file");
          $countfailedcases2 ++;
          push(@failed_file2, $pinput.": file empty or not exists: ".$file);
        } else {
          my $res2 = system("diff $origresult $outroot/$outresult");
          if ($res2 > 0) {
            if(!(-e "$diffdir")) {
              system("mkdir -p $diffdir");
            }

            print "\n$origresult $outroot/$outresult are different!!!\n";
            print "\ncp $outroot/$outresult $origresult\n";
            system("mkdir -p $diffdir/$diff_file && touch $diffdir/$diff_file");
            $countfailedcases2 ++;
            push(@failed_file2, $pinput.": result files diff: ".$origresult);
          } else {
            push(@successed_file, $file." ".$pinput);
          }
        }
      }
    } # if #2

  }  # if #1

}

my $countFailed = $countfailedcases + $countfailedcases1 + $countfailedcases2;
my $countPassed = $count - $countFailed;

my $reportFile = "$outroot/report.txt";
open(my $fh, '>', $reportFile) or die "Could not open file '$reportFile' $!";

if ($countFailed eq 0) {
  print "\n\n=====Scan Result=====\n\n";
  print("\n all $count tests passed\n");
  print("======================================================\n");
  print $fh "all $count tests passed\n";
  close $fh;
} else {
  print "\n\n=====Scan Result=====\n\n";
  print "Total Test Cases: $count\n";
  if(scalar(@successed_file) > 0) {
    print "\n=========================\npassed $countPassed tests:\n\n";
    #foreach $passed (@successed_file) {
    #  print $passed."\n";
    #}
    #print $fh "$countPassed testcases passed\n";
  }
  print "\n=========================\nfailed $countFailed tests:\n\n";
  if(scalar(@failed_file) > 0){
    print("=== failed : $countfailedcases tests - $cmnd\n");
    print $fh "$countfailedcases testcases failed\n";

    foreach $failed (@failed_file) {
      print $failed."\n";
      print $fh $failed."\n";
    }
    print "\n";
  }
  if(scalar(@failed_file1) > 0){
    print("=== failed : $countfailedcases1 tests - $cmnd1\n");
    print $fh "$countfailedcases1 testcases failed\n";

    foreach $failed (@failed_file1) {
      print $failed."\n";
      print $fh $failed."\n";
    }
    print "\n";
  }
  if(scalar(@failed_file2) > 0){
    print("=== failed : $countfailedcases2 tests - result\n");
    print $fh "$countfailedcases2 testcases failed\n";

    foreach $failed (@failed_file2) {
      print $failed."\n";
      print $fh $failed."\n";
    }
    print "\n";
  }
  print "=========================\n";
  close $fh;
}

