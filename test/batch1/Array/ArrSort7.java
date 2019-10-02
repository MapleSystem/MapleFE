// Sort 0s to the left and 1s to the right
import java.util.Arrays;

public class ArrSort7 {
  public static void main(String[] args) {
    int array1[] = new int[]{0, 1, 0, 1, 0, 1, 1, 0, 1, 0};
    int res1[];
    int len = array1.length;

    System.out.println("Array 1 : " + Arrays.toString(array1));
    res1 = Sorted(array1, len);
    System.out.println("Sorted Array 1 : " + Arrays.toString(res1));
  }

  static int [] Sorted(int array[], int len) {
    int cnt = 0;

    for (int x = 0; x < len; x++) {
      if (array[x] == 0) {
        cnt++;
      }
    }

    for (int x = 0; x < cnt; x++) {
      array[x] = 0;
    }

    for (int x = cnt; x < len; x++) {
      array[x] = 1;
    }

    return array;
  }  
}
