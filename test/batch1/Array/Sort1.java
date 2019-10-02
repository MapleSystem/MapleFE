// Sort the given array index from low to high
import java.util.Arrays;

public class Sort1 {
  private int tmp_arr1[];
  private int len;

  public void sort(int[] nums) {
    if (nums == null || nums.length == 0) {
      return;
    }
    this.tmp_arr1 = nums;
    len = nums.length;
    qSort(0, len - 1);
  }

  private void qSort(int left_idx, int right_idx) {
    int x = left_idx;
    int y = right_idx;
  
    // pivot number
    int p = tmp_arr1[left_idx + (right_idx - left_idx) / 2];

    // divide into two arrays
    while (x <= y) {
      while (tmp_arr1[x] < p) {
        x++;
      }

      while (tmp_arr1[y] > p) {
        y--;
      }

      if (x <= y) {
        xNums(x, y);
        // move index to next position
        x++;
        y--;
      }
    }

     // call quick sort recursively
    if (left_idx < y)
      qSort(left_idx, y);

    if (x < right_idx)
      qSort(x, right_idx);
  }
 
  private void xNums(int x, int y) {
    int tmp = tmp_arr1[x];
    tmp_arr1[x] = tmp_arr1[y];
    tmp_arr1[y] = tmp;
  }

  public static void main(String args[]) {
    Sort1 qs = new Sort1();
    int nums[] = {6, -4, 2, 1, 0, 44};
    System.out.println("Array 1 : ");
    System.out.println(Arrays.toString(nums));
    qs.sort(nums);
    System.out.println("Sorted : ");
    System.out.println(Arrays.toString(nums));
  }
}
