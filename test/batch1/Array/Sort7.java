// Using Insertion sort to sort the given array from lower to higher values
import java.util.Arrays;

public class Sort7 {
  void InsertionSort(int[] nums) {
    for (int x = 1; x < nums.length; x++) {
      int val = nums[x];
      int y = x - 1;
      while (y >= 0 && nums[y] > val) {
        nums[y + 1] = nums[y];
        y = y - 1; 
      }
      nums[y + 1] = val;
    }
  }

  public static void main(String args[]) {
    Sort7 s = new Sort7();
    int arr1[] = {6, -4, 2, 1, 0, 44};
    System.out.println("Array 1 : ");
    System.out.println(Arrays.toString(arr1));
    s.InsertionSort(arr1);
    System.out.println("Sorted : ");
    System.out.println(Arrays.toString(arr1));
  }
}
