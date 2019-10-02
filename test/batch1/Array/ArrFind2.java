// Show FOUR elements which sum of given target value 38
import java.util.*;
import java.lang.*;

public class ArrFind2 {
  public static void main(String[] args) {
    int array1[] = {3, 1, 5, 10, 25, 20, 7};
    int target = 38;

    System.out.println("Array 1 : " + Arrays.toString(array1));
    System.out.println("Target value : " + target);
    System.out.println("Which FOUR elements total sum is " + target);

    for (int w = 0; w < array1.length - 3; w++) {
      for (int x = w + 1; x < array1.length - 2; x++) {
        for (int y = x + 1; y < array1.length - 1; y++) {
          for (int z = y + 1; z < array1.length; z++) {
            if (array1[w] + array1[x] + array1[y] + array1[z] == target) {
              System.out.println(array1[w] + " " + array1[x] + " " + array1[y] + " " + array1[z]);
            }
          }
        }
      }
    } 
  }
}
