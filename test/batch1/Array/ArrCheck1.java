// Check and see whether the specific Array contains 0 or -1
// If it does not then return true, if not, false
import java.util.*;
import java.io.*;

public class ArrCheck1 {
  public static void main(String[] args) {
    int[] array1 = {10, 20, -30, -40, 50};
    int[] array2 = {0, 1, 2, 3, 4};

    System.out.println("Array 1 : " + Arrays.toString(array1));
    System.out.println("Array 1 does not contain 0 or -1  : " + check(array1));
  
    System.out.println("Array 2 : " + Arrays.toString(array2));
    System.out.println("Array 2 does not contain 0 or -1  : " + check(array2));
  }

  public static boolean check(int[] numbers) {
      for (int number : numbers) {
        if (number == 0 || number == -1) {
          return false;
        }
      }
      return true;
  }
}
