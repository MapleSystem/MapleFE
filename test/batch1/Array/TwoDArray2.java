/*
  Two-dimensional array Example 2
*/
import java.util.Arrays;

public class TwoDArray2 {
  public static void main(String args[]) {
    int i, j;
    int[][] group = new int[3][3];

    for (i = 0; i < group.length; i++) {
      for (j = 0; j < group[i].length; j++) {
        group[i][j] = i + j;
      }
    }

    for (int[] x : group) {
      for (int y : x) {
        System.out.println(y + "\t");
      }
      System.out.println();
    }

    System.out.println(Arrays.deepToString(group));
  }
}
