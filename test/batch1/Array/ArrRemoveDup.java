// Remove duplicate item
import java.util.Arrays;

public class ArrRemoveDup {
  static void unique_values(int[] arr) {
    System.out.println("Original : ");

    for (int x = 0; x < arr.length; x++) {
      System.out.println(arr[x] + "\t");
    }
  
    int how_many_uniq = arr.length;

    for (int x = 0; x < how_many_uniq; x++) {
      for (int y = x + 1; y < how_many_uniq; y++) {
        if (arr[x] == arr[y]) {
          arr[y] = arr[how_many_uniq - 1];
          how_many_uniq--;
          y--;
        }
      }
    }
   
    int[] arr1 = Arrays.copyOf(arr, how_many_uniq);

    System.out.println();
    System.out.println("Unique values : ");

    for (int x = 0; x < arr1.length; x++) {
      System.out.print(arr1[x] + "\t");
    }
    
    System.out.println();
    System.out.println("------------------------------------------------------------------");
  }

  public static void main(String[] args) {
    unique_values(new int[] {1, 2, 3, 4, 3, 2, 1});
    
    unique_values(new int[] {100, 200, 300, 15, 20, 100, 1, 20});
  }
}
