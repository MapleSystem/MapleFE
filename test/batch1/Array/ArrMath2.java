// Find the average of all values excluding the MIN and MAX
import java.util.*;
import java.io.*;

public class ArrMath2 {
  public static void main(String[] args) {
    int[] array1 = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    System.out.println("Original : " + Arrays.toString(array1));

    int min = 0;
    int max = 0;
    float sum = 0;

    for (int x = 1; x < array1.length; x++) {
      sum += array1[x];
    
      if (array1[x] > max) {
        max = array1[x];
      } else if (array1[x] < min) {
        min = array1[x];
      }
    }
    float y = ((sum - min - max) / (array1.length - 2));
    System.out.printf("The average of all numbers except MIN and MAX are : " + y + "\n");
  }
}

