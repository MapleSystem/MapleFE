// Sort numbers from right to left, one element at a time
import java.util.Arrays;

public class ArrSort6 {
  static int[] sorted(int[] array, int len) {
    int temp[] = new int[len];
    int left = 0;
    int right = len - 1;
    boolean f = true;

    for (int x = 0; x < len; x++) {
      if (f) {
        temp[x] = array[right--];
      } else {
        temp[x] = array[left++];
      }

      f = !f;
    }

    return temp;
  }

  public static void main(String[] args) {
    int array1[] = new int[]{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int array2[] = new int[]{10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    int res1[];
    int res2[];

    System.out.println("Array 1 : " + Arrays.toString(array1));
    res1 = sorted(array1, array1.length);
    System.out.println("Sorted Array 1 : " + Arrays.toString(res1));

    System.out.println("Array 2 : " + Arrays.toString(array2));
    res2 = sorted(array2, array2.length);
    System.out.println("Sorted Array 2 : " + Arrays.toString(res2));
  }
}
