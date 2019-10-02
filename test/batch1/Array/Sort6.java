// Using Selection sort to sort the given array from lower to higher values
import java.util.Arrays;

public class Sort6 {
  public static void SelectionSort(int[] nums) {
    for (int x = 0; x < nums.length - 1; x++) {
      int left1 = Integer.MAX_VALUE;
      int left2 = x + 1;
      for (int y = x; y < nums.length; y++) {
        if (nums[y] < left1) {
          left2 = y;
          left1 = nums[y];
        }
      }
      int tmp = nums[x];
      nums[x] = nums[left2];
      nums[left2] = tmp;
    } 
  }

  public static void main(String args[]) {
    Sort6 s = new Sort6();
    int arr1[] = {6, -4, 2, 1, 0, 44};
    System.out.println("Array 1 : ");
    System.out.println(Arrays.toString(arr1));
    s.SelectionSort(arr1);
    System.out.println("Sorted : ");
    System.out.println(Arrays.toString(arr1));
  }
} 

