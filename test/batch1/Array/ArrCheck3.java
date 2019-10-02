import java.util.*;
import java.io.*;

public class ArrCheck3 {
  public static void main(String[] args) {
    int[] array1 = {0, 20, 65, 70, 43};
    int[] array2 = {43, 65};
    int number1 = 65;
    int number2 = 43;

    System.out.println("Array 1 : " + Arrays.toString(array1));
    System.out.println("Does all values contain 65 and 43?  " + check(array1, number1, number2));

    System.out.println("Array 2 : " + Arrays.toString(array2));
    System.out.println("Does all values contain 65 and 43?  " + check(array2, number1, number2));
  }
  
  public static boolean check(int[] array, int number1, int number2) {
    for (int number : array) {
      boolean c = number != number1 && number != number2;

      if (c) {
        return false;
      }
    }
    return true;
  }
}
