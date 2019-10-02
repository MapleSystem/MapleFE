// Display the array in reverse order
import java.util.Arrays;

public class ArrReverse {
  public static void main(String[] args) {
    int[] array1 = {2019, 2014, 2015, 2010, 2016,
                    2012, 2017, 2011, 2013, 2018};
   
    System.out.println("Original : " + Arrays.toString(array1));

    for (int x = 0; x < array1.length / 2; x++) {
      int tmp = array1[x];
      array1[x] = array1[array1.length - x - 1];
      array1[array1.length - x - 1] = tmp;
    }

    System.out.println("Reversed : " + Arrays.toString(array1));
  }
}
