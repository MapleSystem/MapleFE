// Count the distinct value in the array
import java.util.*;
import java.math.*;

public class Distinctval {
  public static void main(String[] args) {
    int[] arr1 = new int[] {-3, -3, 0, 3, 2, 3, 0, 1, 7, 9};
    int cnt = 0;
    HashSet < Integer > set = new HashSet < Integer > ();

    for (int x = 0; x < arr1.length; x++) {
      int y = Math.abs(arr1[x]);
      if (!set.contains(y)) {
      }
    }
    System.out.println("Array : " + Arrays.toString(arr1));
    System.out.println("Count the distinct value of the array : " + cnt);
  }
}
