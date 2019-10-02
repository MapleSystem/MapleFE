// Check and see whether all 10s add up to be 30
// return true if yes, if not, false
import java.util.*;
import java.io.*;

public class ArrCheck2 {
  public static void main(String[] args) {
    int[] array1 = {10, 2, 50, 10, -1, 8, 10};
    int[] array2 = {3, 58, 14, 43, 10};

    int num1 = 10;
    int total = 30;

    System.out.println("Array 1 : " + Arrays.toString(array1));
    System.out.println("Does all 10s add up to be 30? : " + check(array1, num1, total));

    System.out.println("Array 2 : " + Arrays.toString(array2));
    System.out.println("Does all 10s add up to be 30? " + check(array2, num1, total));
  }
  
  public static boolean check(int[] numbers, int num1, int total) {
    int temp = 0;
    for (int number : numbers) {
      if (number == num1) {
        temp += num1;
      }
   
      if (temp > total) {
        break;
      }
    }
    return temp == total;
  }
}
