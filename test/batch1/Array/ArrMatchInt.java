// Display the same value of two integer arrays
import java.util.Arrays;

public class ArrMatchInt {
  public static void main(String[] args) {
    int[] array1 = {2019, 2014, 2015, 2010, 2016,
                    2012, 2017, 2011, 2013, 2018};

    int[] array2 = {2018, 2020, 2023, 2027, 2024,
                    2026, 2021, 2025, 2019, 2022};

    System.out.println("List 1 : " + Arrays.toString(array1));
    System.out.println("List 2 : " + Arrays.toString(array2));

    for (int x = 0; x < array1.length; x++) {
      for (int y = 0; y < array2.length; y++) {
        if (array1[x] == array2[y]) {
          System.out.println("Same value : " + array1[x]);
        }
      }
    }
  }
}
