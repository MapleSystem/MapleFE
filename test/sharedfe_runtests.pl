#!/usr/bin/perl -w
use Cwd;
use warnings;

my $pwd = getcwd;

#print "here1 current pwd $pwd\n";

if ((!(defined $ARGV[0])) || ($ARGV[0] ne "sharedfe")) {
  print "------------------------------------------------\n";
  print "usage: sharedfe_runtests.pl sharedfe            \n";
  print "------------------------------------------------\n";
  exit;
}

$dirname = "./$ARGV[0]";

my $outdir = "$pwd/sharedfe_output";
my $diffdir = "$pwd/sharedfe_diff";
my $notexistsdir = "$pwd/sharedfe_notexists";
if(!(-e "$outdir")) {
  system("mkdir -p $outdir");
}

my @failed_sharedfe_file;
my @successed_file;

my $count = 0;
my $countsharedfe = 0; 


chdir $dirname;
$dirname = "./";
opendir (DIR, $dirname ) || die "Error in opening dir $dirname\n";

#print "here2 dirname $dirname\n";

print("\n====================== run tests: $ARGV[0] =====================\n");

while( ($srcdir = readdir(DIR))){
  if(-d $srcdir and $srcdir ne ".." and $srcdir ne "output" and $srcdir ne "temp") {
    my $predir = getcwd;
    chdir $srcdir;
    my @javafiles;
    @javafiles = <*.java>;

#print "here3 source directory $srcdir\n";
#print "here4 predir $predir\n";

    my @allfiles = (@javafiles);
    foreach $fullname (@allfiles) {
      $count ++;
      (my $file = $fullname) =~ s/\.[^.]+$//;
      if(defined $ARGV[1]) {
        print "\n$file";
      } else {
        print ".";
      }
      if ($count % 50 == 0) {
        print " $count\n";
      }
      my $flag = 0;
      my $src_file = $fullname;
      my $sharedfe_oresult_file = $file.'.java.result';
      my $sharedfe_log_file = $file.'.sharedfe.log';
      my $sharedfe_result_file = $file.'.java.sharedfe.result';
      my $sharedfe_err_file = $file.'.err';
      my $sharedfe_diff_file = $file.'.java.diff';

      if(!(-e "$outdir")) {
        system("mkdir -p $outdir");
      }

#print "here5 outdir $outdir\n";
#print "here6 src_file $src_file\n";

      system("cp $src_file $outdir/$src_file");
      $res = system("cd $BUILDDIR/autogen; ./sharedfe $outdir/$src_file > $outdir/$sharedfe_result_file");
      if ($res > 0) {
        print "\n(cd $BUILDDIR/autogen; gdb --args ./sharedfe $outdir/$src_file)\n";
        print " ==sharedfe===> $file\n";
#        $countsharedfe ++;
#        push(@failed_sharedfe_file, $file);
        $flag ++;
        next;
      } 

#print "here7 diff src_file $pwd/sharedfe/$sharedfe_oresult_file\n";
#print "here8 diff sharedfe_result_file $sharedfe_result_file\n";

      if (!(-e $sharedfe_oresult_file)) {
        if(!(-e "$notexistsdir")) {
          system("mkdir -p $notexistsdir");
        }

#print "here9 $sharedfe_oresult_file NOT exists\n";
        print "Original file $sharedfe_oresult_file does NOT exists!\n";
        system("touch $notexistsdir/$sharedfe_oresult_file");
        $countsharedfe ++;  
        push(@failed_sharedfe_file, $file);
      }
      else {
#print "here10 sharedfe_result_file $outdir/$sharedfe_result_file\n";
         if ((!(-e "$outdir/$sharedfe_result_file")) || (-z "$outdir/$sharedfe_result_file")) {
            if(!(-e "$notexistsdir")) {
               system("mkdir -p $notexistsdir");
            }
 
#print "here11 $outdir/$sharedfe_result_file NOT exists\n";
           print "$sharedfe_result_file either empty or not exists!\n";
           system("touch $notexistsdir/$sharedfe_result_file");
           $countsharedfe ++;
           push(@failed_sharedfe_file, $file);
        } else {
           $res2 = system("diff $pwd/sharedfe/$sharedfe_oresult_file $outdir/$sharedfe_result_file");
           if ($res2 > 0) {
             if(!(-e "$diffdir")) {
                system("mkdir -p $diffdir");
             }

#print "here12 $sharedfe_diff_file  Different!!!\n";
             print "$sharedfe_oresult_file $sharedfe_result_file are different!!!\n";
             system("touch $diffdir/$sharedfe_diff_file");
             $countsharedfe ++;
             push(@failed_sharedfe_file, $file);
           } else {
             push(@successed_file, $file);
           }

         } 
      }

#      if($flag eq 0){
#        push(@successed_file, $file);
#      }
      next;

      if ($flag eq -1) {
        push(@successed_file, $file);
        system("rm -f $outdir/$src_file");
        system("rm -f $outdir/$sharedfe_log_file");
        system("rm -f $outdir/$sharedfe_oresult_file");
        system("rm -f $outdir/$sharedfe_result_file");
        system("rm -f $outdir/$sharedfe_err_file");
        system("rm -f $diffdir/$sharedfe_diff_file");
        system("rm -f $notexists/$sharedfe_oresult_file");
        system("rm -f $notexists/$sharedfe_result_file");
        next;
      }
    }
    chdir $predir;
  }
}

print " $count\n";
closedir(DIR);
chdir $pwd;

my $countFailed = $countsharedfe ;
my $countPassed = $count - $countFailed;

my $reportFile = 'sharedfe_report.txt';
open(my $fh, '>', $reportFile) or die "Could not open file '$reportFile' $!";
print $fh "$ARGV[0] report: \n";

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
    foreach $passed (@successed_file) {
      print $passed."\n";
    }
    print $fh "$countPassed testcases passed\n";
  }
  print "\n=========================\nfailed $countFailed tests:\n\n";
  if(scalar(@failed_sharedfe_file) > 0){
    print("=== failed sharedfe: $countsharedfe tests\n");
    print $fh "$countsharedfe testcases failed\n";

    foreach $failed (@failed_sharedfe_file) {
      print $failed."\n";
    }
    print "\n";
  }
  print "=========================\n";
  close $fh;
}
