// Check how many elements in an array
// Show the number of elements after deduplication
import java.util.*;

public class ArrRemoveDup2 {
  public static void main(String[] args) {
    int array1[] = {10, 10, 20, 30, 40, 40, 40};

    System.out.println("Array 1 : " + Arrays.toString(array1));
    System.out.println("How many elements in Array 1? " + array1.length);
    System.out.println("How many elements after deduplication? " + sorted(array1));
  }

  public static int sorted(int[] nums) {
    int idx = 1;
    for (int x = 1; x < nums.length; x++) {
      if (nums[x] != nums[idx - 1]) {
        nums[idx++] = nums[x];
      }
    }
    return idx;
  }
}
