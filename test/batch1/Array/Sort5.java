// Using Heap sort to sort the given array from lower to higher values
import java.util.Arrays;

public class Sort5 {
  void HeapSort(int nums[]) {
    buildheap(nums);
    for (int x = nums.length - 1; x >= 0; x--) {
      xc(nums, x, 0);
      heap(nums, x, 0);
    }
  }
  
  private void xc(int[] nums, int x, int y) {
    int tmp = nums[x];
    nums[x] = nums[y];
    nums[y] = tmp;
  }

  private void heap(int[] nums, int s, int x) {
    int left = ((2 * x) + 1);
    int right = ((2 * x) + 2);
    int max = x;

    if ((left < s) && (nums[left] > nums[x])) {
      max = left;
    }

    if ((right < s) && (nums[right] > nums[max])) {
      max = right;
    }

    if (max != x) {
      xc(nums, x, max);
      heap(nums, s, max);
    }
  }

  private void buildheap(int[] nums) {
    for (int x = (nums.length / 2) - 1; x >= 0; x--) {
      heap(nums, (nums.length - 1), x);
    }
  }

  public static void main(String args[]) {
    Sort5 s = new Sort5();
    int nums[] = {6, -4, 2, 1, 0, -44};
    System.out.println("Array 1 : ");
    System.out.println(Arrays.toString(nums));
    s.HeapSort(nums);
    System.out.println("Sorted : ");
    System.out.println(Arrays.toString(nums));
 
  }
}
