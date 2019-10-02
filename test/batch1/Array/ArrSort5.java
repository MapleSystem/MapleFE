// Sort all positive numbers on the left and negative numbers on the right
import java.util.Arrays;

public class ArrSort5 {
  public static void main(String[] args) {
    int array1[] = {-1, 2, 4, -6, 8, -9, 10, 11, 12, -15};
    int y, temp, len;

    System.out.println("Array 1 : " + Arrays.toString(array1));
    len = 8;
    for (int x = 0; x < array1.length; x++) {
      y = x;

      while ((y > 0) && (array1[y] > 0) && (array1[y - 1] < 0)) {
        temp = array1[y];
        array1[y] = array1[y - 1];
        array1[y - 1] = temp;
        y--;
      }
    }
    System.out.println("Sorted Array 1 : " + Arrays.toString(array1));
  }
}

