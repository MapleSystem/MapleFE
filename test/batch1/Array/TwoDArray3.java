/*
  Two-dimensional array Example 3
  Three ways to print out values
*/
import java.util.Arrays;

public class TwoDArray3 {
  public static void main(String args[]) {
    int i, j;

    String[][] names = {
                        {"Ryder", "Marshall"},
                        {"Rubble", "Chase"},
                        {"Zuma", "Sky"},
                       };

    System.out.println("***** Method1 ******");
    for (i = 0; i < names.length; i++) {
      for (j = 0; j < names[i].length; j++) {
        System.out.println(names[i][j]);
      }
      System.out.println();
    }

    System.out.println("***** Method2 *****");
    for (String[] x : names) {
      for (String y : x) {
        System.out.println(y);
      }
      System.out.println();
    }

    System.out.println("***** Method3 *****");
    System.out.println(Arrays.deepToString(names));
  }
}
