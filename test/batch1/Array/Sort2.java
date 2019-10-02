// Using sinking or bublle sort to sort given array from low to high indexes
import java.util.Arrays;

class Sort2 {
  void BubbleSort(int nums[]) {
    int len = nums.length;
    for (int x = 0; x < len - 1; x++) {
      for (int y = 0; y < len - x - 1; y++) {
        if (nums[y] > nums[y + 1]) {
          // swap temp and nums[x]
          int tmp = nums[y];
          nums[y] = nums[y + 1];
          nums[y + 1] = tmp;
        }
      }
    } 
  }

  public static void main(String args[]) {
    Sort2 s = new Sort2();
    int nums[] = {6, -4, 2, 1, 0, 44};
    System.out.println("Array 1 : ");
    System.out.println(Arrays.toString(nums));
    s.BubbleSort(nums);
    System.out.println("Sorted : ");
    System.out.println(Arrays.toString(nums));
  }
}
