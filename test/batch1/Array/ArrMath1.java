// Math = Max value - Min value
import java.util.Arrays;

public class ArrMath1 {
  public static void main(String[] args) {
    int[] array1 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    System.out.println("Original : " + Arrays.toString(array1));
    int max = array1[0];
    int min = array1[0];
    for (int x = 1; x < array1.length; x++) {
      if (array1[x] > max) {
        max = array1[x];
      } else if (array1[x] < min) {
        min = array1[x];
      }
    }
    System.out.println("Max value - Min value = " + (max - min));
  }
}
