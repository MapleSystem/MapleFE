// The sum of a numeric array
import java.util.Arrays;

public class ArrSum {
  public static void main(String[] args) {
    int array1[] = {1, 8, 15, 5, 30, 60, 100, 9, 20};
    int sum = 0;

    for (int x : array1) {
      sum += x;
    }
    System.out.println(Arrays.toString(array1));
    System.out.println("The total is : " + sum);
  }
}
