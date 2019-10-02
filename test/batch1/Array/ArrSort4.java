// Soft the elements into two groups.
// Negative numbers on the left and positive numbers on the right
import java.util.Arrays;

public class ArrSort4 {
  public static void main(String[] args) {
    int[] array1 = {-1, 2, 5, -7, 9, -10, 3, -4};
    System.out.println("Array 1 : " + Arrays.toString(array1));
    sorted(array1);
    System.out.println("Sorted Array 1 : " + Arrays.toString(array1));
  }
  
  public static void sorted(int[] array) {
    int left = 0;  // negative numbers
    int right = 0;  // positive numbers
    int x, y;
    int max = Integer.MIN_VALUE;

    for (x = 0; x < array.length; x++) {
      if (array[x] < 0) {
        left++;
      } else {
        right++;
      }

      if (array[x] > max)
        max = array[x];
    }

    max++;
   
    if (left == 0 || right == 0)
      return;

    x = 0;
    y = 1;

    while (true) {
      while (x <= left && array[x] < 0)
        x++;

      while (y < array.length && array[y] >= 0)
        y++;

      if (x > left || y >= array.length)
        break;

      array[x]=max*(x+1);
      swap(array, x, y);
    }

    x = array.length - 1;
    while (x >= left) {
      int div = array[x] / max;
      if (div == 0) {
        x--;
      } else {
        array[x]%=max;
        swap(array, x, left+div-2);
      }  
    }
  }

  private static void swap(int[] array, int x, int y) {
    int temp = array[x];
    array[x] = array[y];
    array[y] = temp;
  }
}
