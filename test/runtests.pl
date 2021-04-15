#!/usr/bin/perl

use Cwd;
use warnings;

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
  print "Run java2mpl others openjdk\n";
} elsif ($ARGV[0] eq 'allall') {
  @dirname = qw(java2mpl errtest others openjdk syntaxonly);
  print "Run java2mpl errtest others openjdk\n";
} elsif (($ARGV[0] eq 'java2mpl')) {
  @dirname = "$ARGV[0]";
  print "Run $ARGV[0]\n";
} elsif (($ARGV[0] eq 'errtest')) {
  @dirname = "$ARGV[0]";
  print "Run $ARGV[0]\n";
} elsif (($ARGV[0] eq 'others')) {
  @dirname = "$ARGV[0]";
  print "Run $ARGV[0]\n";
} elsif (($ARGV[0] eq 'openjdk')) {
  @dirname = "$ARGV[0]";
  print "Run $ARGV[0]\n";
} elsif (($ARGV[0] eq 'syntaxonly')) {
  @dirname = "$ARGV[0]";
  print "Run $ARGV[0]\n";
} else {
  print "$ARGV[0] is an invalid option\n";
  exit;
}

my @failed_file;
my @successed_file;

my $count = 0;
my $countfailedjava = 0;
my $countsub = 0;

system("rm -Rf report.txt $pwd/output $pwd/diff $pwd/notexists");

my $currdir = "$pwd";
my $outroot = "$currdir/../output/java/test";
if(!(-e "$outroot")) {
  system("mkdir -p $outroot");
}

foreach my $dir (@dirname) {
#  print "here2 dir $dir\n";
  $dirname1 = "./$dir";
#  print "here3 dirname1 $dirname1\n";
  
  system("cp -rp $dir $outroot/");

  my $outdir = "$outroot/$dir";
  my $diffdir = "$pwd/diff/$dir";
  my $notexistsdir = "$outroot/notexists/$dir";

  chdir $dirname1;
#  print "here4 dirname1 $dirname1\n";
  $dirname1 = "./";
#  print "here5 dirname1 $dirname1\n";
  opendir (DIR, $dirname1) || die "Error in opening dir $dirname1\n";

  print("\n====================== BEGIN tests: $dir =====================\n");

  while( ($srcdir = readdir(DIR))){
    if ( -d $srcdir and $srcdir ne ".." and $srcdir ne "temp"  ) {
#print "here13 srcdir $srcdir\n";
      my $predir = getcwd;
      chdir $srcdir;
      my @javafiles;
      @javafiles = <*.java>;
#print "here14 predir $predir\n";

      my @allfiles = (@javafiles);
      foreach $fullname (@allfiles) {
        $count ++;
        $countsub ++;
#print "here144 fullname $fullname\n";
        (my $file = $fullname) =~ s/\.[^.]+$//;
        print ".";
        if ($count % 50 == 0) {
          print " $count\n";
        }
        my $flag = 0;
        my $src_file = $fullname;
        my $oresult_file = $file.'.java.result';
        my $log_file = $file.'.log';
        my $result_file = $file.'.java.'.$dir.'.result';
        my $err_file = $file.$dir.'.err';
        my $diff_file = $file.'.java.'.$dir.'.diff';

#print "here15 dir $dir\n";
#print "here16 src_file $src_file\n";
        if ($dir eq "java2mpl") {
          $res = system("$pwd/../output/java/java/java2mpl $outdir/$src_file > $outdir/$result_file");
        }
        if ($dir eq "errtest") {
          $res = system("$pwd/../output/java/java/java2mpl $outdir/$src_file > $outdir/$result_file");
        }
        if ($dir eq "others") {
          $res = system("$pwd/../output/java/java/java2mpl $outdir/$src_file > $outdir/$result_file");
        }
        if ($dir eq "openjdk") {
          $res = system("$pwd/../output/java/java/java2mpl $outdir/$src_file > $outdir/$result_file");
        }
        if ($dir eq "syntaxonly") {
          $res = system("$pwd/../output/java/java/java2mpl $outdir/$src_file > $outdir/$result_file");
        }
        
        if ($res > 0) {
#print "over here1...\n";
          if ($dir eq "java2mpl") { 
            print "$pwd/../output/java/java/java2mpl $outdir/$src_file\n";
          }
          if ($dir eq "errtest") { 
            print "$pwd/../output/java/java/java2mpl $outdir/$src_file\n";
          }
          if ($dir eq "others") {
            print "$pwd/../output/java/java/java2mpl $outdir/$src_file\n";
          }
          if ($dir eq "openjdk") {
            print "$pwd/../output/java/java/java2mpl $outdir/$src_file\n";
          }
          if ($dir eq "syntaxonly") {
            print "$pwd/../output/java/java/java2mpl $outdir/$src_file\n";
          }
          print " ==$dir===> $file\n";
          $countfailedjava ++;
          push(@failed_file, $dir.":  ".$file);
          #print "---------------------------\n";
          next;
        }

#print "here17 orig result file $oresult_file\n";
#print "here18 file $file\n";
        if (!(-e $oresult_file)) {
          if(!(-e "$notexistsdir")) {
            system("mkdir -p $notexistsdir");
          }
 
          print "\nOriginal file $oresult_file does NOT exists!\n";
          system("touch $notexistsdir/$oresult_file");
          $countfailedjava ++;
          push(@failed_file, $dir.": ".$file);
          #print "---------------------------\n";
        } else {
          if ((!(-e "$outdir/$result_file")) || (-z "$outdir/$result_file")) { 
            if(!(-e "$notexistsdir")) {
              system("mkdir -p $notexistsdir");
            }

            print "\n$result_file either empty or not exists!\n";
            system("touch $notexistsdir/$result_file");
            $countfailedjava ++;
            push(@failed_file, $dir.": ".$file);
            #print "---------------------------\n";
          } else {
#print "COMPARE file $pwd/$dir/$oresult_file and $outdir/$result_file\n";
            $res2 = system("diff $pwd/$dir/$oresult_file $outdir/$result_file");
            if ($res2 > 0) {
              if(!(-e "$diffdir")) {
                system("mkdir -p $diffdir");
              }


              print "\n$oresult_file $result_file are different!!!\n";
              system("touch $diffdir/$diff_file");
              $countfailedjava ++;
              push(@failed_file, $dir.": ".$file);
              #print "---------------------------\n";
            } else {
#print "GOOD files: $file\n";
              push(@successed_file, $file." ".$dir);
              #print "---------------------------\n";
            }
          }
        }
        next;

      }
      chdir $pwd;
    }
  }
  print("\n====================== END $dir test cases: $countsub ================\n");
  $countsub = 0;
}

closedir(DIR);
chdir $pwd;

my $countFailed = $countfailedjava ;
my $countPassed = $count - $countFailed;

my $reportFile = "$outroot/report.txt";
open(my $fh, '>', $reportFile) or die "Could not open file '$reportFile' $!";
if ($ARGV[0] eq "all") {
  print $fh "java2mpl and sharedfe report: \n";
} else {
  print $fh "$ARGV[0] report: \n";
}

if ($countFailed eq 0) {
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
    #print $fh "$countfailedjava testcases failed\n";

    foreach $failed (@failed_file) {
      print $failed."\n";
    }
    print "\n";
  }
  print "=========================\n";
  close $fh;
}
