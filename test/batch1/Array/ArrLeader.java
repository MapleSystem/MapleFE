// Show the values which is greater than all the values on the right
import java.util.HashMap;
import java.util.Map;
import java.util.Iterator;
import java.util.Arrays;

public class ArrLeader {
  public static void main(String[] args) {
    int array1[] = {20, 9, 8, 15, 1, 7};
    int len = array1.length;

    System.out.println("Array 1 : " + Arrays.toString(array1));
    System.out.println("Show the value which is greater than all the values on the right");
    
    for (int x = 0; x < len; x++) {
      int y;
      for (y = x + 1; y < len; y++) {
        if (array1[x] <= array1[y]) {
          break;
        }
      }
      if (y == len) {
         System.out.print(array1[x] + " \n");
      }
    } 
  }
}
