// Move specified elements or values to the end of the array
import java.util.*;

public class ArrMoving {
  public static void main(String[] args) throws Exception {
    int[] array1 = {0, 2, 0, 3, 0, 4, 5, 6, 0, 8};
 
    System.out.println("Original : " + Arrays.toString(array1));

    int y = 0;
    for (int x = 0; x < array1.length; x++) {
      if (y != x && array1[x] != 0) {
         array1[y] = array1[x];
         array1[x] = 0;
         y++;
      }
    }

    System.out.println(Arrays.toString(array1));
  }
}

