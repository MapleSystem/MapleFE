// Copy the first array to a new array
import java.util.Arrays;

public class ArrCopy {
  public static void main(String[] args) {
    String[] array1 = {"Test1", "Test6", "Test3", "Test8",
                       "Test5", "Test2", "Test7", "Test4"};

    String[] array2 = new String[8];

    System.out.println("Original : " + Arrays.toString(array1));
    
    for (int x = 0; x < array1.length; x++) {
      array2[x] = array1[x];
    }
    
    System.out.println("New : " + Arrays.toString(array2));
  }
}
