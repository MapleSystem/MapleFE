// Calculate the average of the sum
import java.util.Arrays; 

public class ArrAvg {
  public static void main(String[] args) {
    int[] x = new int[]{1, 8, 15, 5, 30, 60, 100, 9, 20};
    int sum = 0;

    // Calculate the sum of the array
    for (int i = 0; i < x.length; i++) {
      sum = sum + x[i];
    }

    System.out.println(Arrays.toString(x));
    System.out.println("Sum : " + sum);

    // Calculate the average of the sum
    double avgx = sum / x.length;
    System.out.println("Average : " + avgx);
  }
}
