// Show the smallest and the second smallest in the array
import java.util.*;
import java.lang.*;

public class ArrFind {
  public static void main(String[] args) {
    int array1[] = {8, 3, 9, -1, 1, 5};

    int first, last;

    if (array1.length < 2) {
      System.out.println("Array less than two values!");
      return;
    }
 
    first = last = Integer.MAX_VALUE;
 
    for (int x = 0; x < array1.length; x++) {
      if (array1[x] < first) {
        last = first;
        first = array1[x];
      }
      else if (array1[x] < last && array1[x] != first) {
        last = array1[x];
      }
    }

    System.out.println("Array 1 : " + Arrays.toString(array1));
    if (last == Integer.MAX_VALUE) {
      System.out.println("No last smallest value!");
    } else {
      System.out.println("The smallest value is " + first + " and the 2nd smallest is " + last + ".");
    }
  }
}
