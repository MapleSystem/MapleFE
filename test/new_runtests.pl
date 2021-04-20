#!/usr/bin/perl

use Cwd;
use warnings;
use experimental 'smartmatch';
use File::Find;
use File::Basename;

use File::Find qw(find);


if(!(defined $ARGV[0])) {
  print "------------------------------------------------\n";
  print "usage: runtests.pl java/typescript/javascript\n";
  print "------------------------------------------------\n";
  exit;
}

my $pwd = getcwd;
my $currdir = "$pwd";
my $outroot = "$currdir/../output/test";

system("rm -Rf report.txt $pwd/output $pwd/diff $pwd/notexists");

if(!(-e "$outroot")) {
  system("mkdir -p $outroot");
}

my @failed_file;
my @successed_file;

my $count = 0;
my $counttotal = 0;
my $countfailedjava = 0;
my $countsub = 0;

opendir (my $DIR, $ARGV[0]) || die "Error in opening $ARGV[0] directory\n";

my $pjava = 'java';
my $ptypescript = 'typescript';
my $pinput = '';
my $cmnd = '';
print "Running $ARGV[0]\n";
if ($ARGV[0] =~ /\Q$pjava\E/) {
  $pinput = "JAVA";
  $cmnd = "../output/java/java2mpl";
} elsif ($ARGV[0] =~ /\Q$ptypescript\E/) {
  $pinput = "TYPESCRIPT";
  $cmnd = "";
}

sub listdirs {
  my @dirs = @_;
  my @files;
  
  if ( $pinput eq "JAVA" ) {
    find({ wanted => sub { push @files, glob "\"$_/{*.java,*.java.result}\"" } , no_chdir => 1 }, @dirs);
  } else {
    find({ wanted => sub { push @files, $_ } , no_chdir => 1 }, @dirs);
  }
  
  return @files;
}

my @paths = listdirs($ARGV[0]) ;

foreach my $file (@paths) {
  my ($filename) = ( $file =~ /([^\\\/]+)$/s ) ;
  my ($pathname) = dirname($file);

  if ( $pinput eq "JAVA" ) {
    system("cp -rp --parents $file $outroot/");

    if ($filename =~ (/(.+)[.]java$/)) {
      my $origresult = "$pwd/$file.result";
      my $outresult = $file.'.result.java2mpl';
      my $diff_file = $file.'.result.diff'; 
      my $notexistsdir = "$outroot/notexists";
      my $diffdir = "$outroot/diff";

      $count ++;
      print ".";
      if ($count % 50 == 0) {
        print " $count\n";
      }

      my $res = system("$pwd/$cmnd $outroot/$file > $outroot/$outresult");

      if ($res > 0) {
        print "$pwd/$cmnd $outroot/$file\n";
        print " ==$pinput===> $file\n";
        $countfailedjava ++;
        push(@failed_file, $pinput.":  ".$file);
        #print "---------------------------\n";
        next;
      }

      if (!(-e $origresult) ) {
        if(!(-e "$notexistsdir")) {
	  system("mkdir -p $notexistsdir");
	}
        print "\nOriginal file $origresult does NOT exists!\n";
        system("mkdir -p $notexistsdir/$file && touch $notexistsdir/$file");
        $countfailedjava ++;
        push(@failed_file, $pinput." file not exists: ".$file);
      } else {
        if ((!(-e "$outroot/$outresult")) || (-z "$outroot/$outresult")) {
	  if(!(-e "$notexistsdir")) {
            system("mkdir -p $notexistsdir");
	  }

          print "\n$outroot/$outresult either empty or not exists!\n";
          system("mkdir -p $notexistsdir/$file && touch $notexistsdir/$file");
          $countfailedjava ++;
          push(@failed_file, $pinput." file empty or not exists: ".$file);
	} else {
	  my $res2 = system("diff $origresult $outroot/$outresult");
          if ($res2 > 0) {
            if(!(-e "$diffdir")) {
              system("mkdir -p $diffdir");
            }

            print "\n$origresult $outroot/$outresult are different!!!\n";
            system("mkdir -p $diffdir/$diff_file && touch $diffdir/$diff_file");
            $countfailedjava ++;
            push(@failed_file, $pinput." result files diff: ".$origresult);
          } else {
	    push(@successed_file, $file." ".$pinput);
	  }
	}
      }
    }
  } elsif ( $pinput eq "TYPESCRIPT" ) {
    print "Run ts2cpp\n";
  }

}

my $countFailed = $countfailedjava;
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
    print("=== failed : $countfailedjava tests\n");
    print $fh "$countfailedjava testcases failed\n";

    foreach $failed (@failed_file) {
      print $failed."\n";
      print $fh $failed."\n";
    }
    print "\n";
  }
  print "=========================\n";
  close $fh;
}

