// Find common values in all the arrays
import java.util.*;

public class ArrSameVal {
  public static void main(String[] args) {
    ArrayList<Integer> c = new ArrayList<Integer>();

    int array1[] = {1, 3, 5, 7, 9, 11, 13, 15};
    int array2[] = {5, 10, 15, 20, 25, 30, 35};
    int array3[] = {5, 15, 25, 35};

    int x = 0;
    int y = 0;
    int z = 0;

    while (x < array1.length && y < array2.length && z < array3.length) {
      if (array1[x] == array2[y] && array2[y] == array3[z]) {
        c.add(array1[x]);
        x++;
        y++;
        z++;
      } else if (array1[x] < array2[y]) { 
        x++;
      } else if (array2[y] < array3[z]) {
        y++;
      } else {
        z++;
      }
    }

    System.out.println("Array 1 : " + Arrays.toString(array1));
    System.out.println("Array 2 : " + Arrays.toString(array2));
    System.out.println("Array 3 : " + Arrays.toString(array3));
    System.out.println("\n");
    System.out.println("The same elements among the three arrays are : ");
    System.out.println(c);
  }
}
