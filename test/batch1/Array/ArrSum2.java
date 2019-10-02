// Show the minimum sum of two values
import java.util.*;
import java.lang.*;

public class ArrSum2 {
  public static void main(String[] args) {
    int array1[] = {10, -5, 6, 1, -3, 4};
    int len = array1.length;
    int x, y, total_min, total, l_min, r_min;
    
    if (len < 2) {
      System.out.println("Invalid Input");
      return;
    }

    l_min = 0;
    r_min = 1;
    total_min = array1[0] + array1[1];

    for (x = 0; x < len - 1; x++) {
      for (y = x + 1; y < len; y++) {
        total = array1[x] + array1[y];
        if (Math.abs(total_min) > Math.abs(total)) {
          total_min = total;
          l_min = x;
          r_min = y;
        }
      }
    }
    System.out.println("Array 1 : " + Arrays.toString(array1));
    System.out.println("Two elements in Array 1 whose sum is minimum are " + array1[l_min] + " and " + array1[r_min]);
  }
}
