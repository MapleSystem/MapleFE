// Check whether there is two elements adds up to be sum of 8.
// if yes, return true, else, false.
import java.util.*;

public class ArrSum3 {
  static boolean sum_found(int array1[], int len, int x) {
    int y;
    for (y = 0; y < len - 1; y++)
      if (array1[y] > array1[y + 1])
        break;

    int z = (y + 1) % len;

    int w = y;

    while (z != w) {
      if (array1[z] + array1[w] == x)
        return true;

      if (array1[z] + array1[w] < x) {
        z = (z + 1) % len;
      } else {
        w = (len + w - z) % len;
      }
    }
    return false;
  }

  public static void main(String[] args) {
    int array1[] = {1, 2, 3, 4, 5};
    int target = 8;
   
    System.out.println("Array 1 : " + Arrays.toString(array1));
    
    if (sum_found(array1, array1.length, target)) {
      System.out.println("Array has two elements add up to be " + target);
    } else {
      System.out.println("There is no sum of " + target + " in the Arra");
    }
  }
}
