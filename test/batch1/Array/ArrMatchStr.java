// Display the same value in two string arrays
import java.util.*;

public class ArrMatchStr {
  public static void main(String[] args) {
    String[] array1 = {"Test1", "Test6", "Test3", "Test8",
                       "Test5", "Test2", "Test7", "Test4"};

    String[] array2 = {"Test9", "Test1", "Test10", "Test13",
                       "Test11", "Test14", "Test8", "Test12"};

    System.out.println("List 1 : " + Arrays.toString(array1));
    System.out.println("List 2 : " + Arrays.toString(array2));

    HashSet<String> set = new HashSet<String>();

    for (int x = 0; x < array1.length; x++) {
      for (int y = 0; y < array2.length; y++) {
        if (array1[x].equals(array2[y])) {
          set.add(array1[x]);
        }
      }
    }

    System.out.println("Same value : " + (set));
  }
}
