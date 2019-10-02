// Display duplicate value of the string array
import java.util.Arrays;

public class ArrDupFound2 {
  public static void main(String[] args) {
    String[] array1 = {"Test1", "Test6", "Test3", "Test8",
                       "Test5", "Test8", "Test7", "Test4",
                       "Test2", "Test1"};

    System.out.println(Arrays.toString(array1));

    for (int x = 0; x < array1.length - 1; x++) {
      for (int y = x + 1; y < array1.length; y++) {
        if ((array1[x].equals(array1[y])) && (x != y)) {
          System.out.println("Duplicate value : " + array1[y] + " found!" );
        }
      }
    }
  }
}
