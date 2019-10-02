// Insert a value in the specify index position
import java.util.Arrays;

public class ArrReplaceItem {
  public static void main(String[] args) {
    String[] array1 = {"Test1", "Test6", "Test3", "Test8",
                       "Test5", "Test2", "Test7", "Test4"};

    System.out.println("Original : " + Arrays.toString(array1));

    int idx_pos = 6;
    String str = "Test9";

    for (int x = array1.length - 1; x > idx_pos; x--) {
      array1[x] = array1[x - 1];
    }  
    array1[idx_pos] = str;

    System.out.println("After insert Test9 into index 6");
    System.out.println("New : " + Arrays.toString(array1));
  }
}
