// Move the last element to first element
import java.util.Arrays;

public class ArrSort2 {
  static int array1[] = new int[]{1, 2, 3, 4, 5, 6, 7, 8};

  static void Sort() {
    int x = array1[array1.length - 1], y;
  
    for (y = array1.length - 1; y > 0; y--) 
      array1[y] = array1[y - 1];
    array1[0] = x;
  }

  public static void main(String[] args) {
    System.out.println("Array 1 : " + Arrays.toString(array1));
    Sort();
    System.out.println("Sorted : " + Arrays.toString(array1));
  }
}
