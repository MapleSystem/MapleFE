// Segregate two given values into two groups.  3 on left and 8 on right
import java.util.*;
import java.lang.*;

public class ArrSeg {
  public static void main(String[] args) {
    int array1[] = {3, 8, 3, 8, 8, 8, 3, 8, 3, 3, 8, 3};
    int x;
    int left = 0, right = array1.length - 1;

    System.out.println("Array 1 : " + Arrays.toString(array1));
   
    while (left < right) {
      while (array1[left] == 0 && left < right) {
        left++;  
      }

      while (array1[right] == 1 && left < right) {
        right--;
      }

      if (left < right) {
        array1[left] = 3;
        array1[right] = 8;
        left++;
        right--;
      }
    }
   
    System.out.println("Array after re-arrangement is : " + Arrays.toString(array1));
  }
}
