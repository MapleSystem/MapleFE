//Create an array with 100 variables and print out with reverse order.

import java.util.Scanner;

class ReverseArr {
  public static void main(String[] args) {
    //Declare variables and create the array.
    int [] a;
    int i;
    a = new int [10];

    // read in 10 value and store them in the array.
    Scanner in = new Scanner(System.in);
    i = 0;
    while (i < 10) {
      a[i] = in.nextInt();
      i = i + 1;
   }

  // write out the 10 value in reverse order.
    i = 9;
    while (i >= 0) {
      System.out.println (a[i]);
      i = i -1;
    }
  }
}

