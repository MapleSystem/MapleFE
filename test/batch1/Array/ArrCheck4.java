// Display how many elements in Array 1
// Display the longest consecutive numbers
import java.util.HashSet;
import java.util.*;

public class ArrCheck4 {
  public static void main(String[] args) {
    int array1[] = {6, 4, 100, 3000, 3, 80, 2, 9, 1, 15, 5};

    System.out.println("Array 1 : " + Arrays.toString(array1));
    System.out.println("How many elements in Array 1? " + array1.length);
    for (int x = 0; x < array1.length; x++) {
      System.out.print(array1[x] + " ");
    }
    System.out.println("\nThe longest consecutive sequence is : " + longest_seq(array1));
  }
 
  public static int longest_seq(int[] numbers) {
    final HashSet<Integer> hset = new HashSet<Integer>();
    for (int x : numbers) hset.add(x);

    int longest_seq_len = 0;
    for (int x : numbers) {
      int len = 1;
      for (int y = x - 1; hset.contains(y); --y) {
        hset.remove(y);
        ++len;
      }
  
      for (int y = x + 1; hset.contains(y); --y) {
        hset.remove(y);
        ++len;
      }

      longest_seq_len = Math.max(longest_seq_len, len);
    }
    return longest_seq_len; 
  }
}
