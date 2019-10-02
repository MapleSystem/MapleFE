// Using Bead sort to sort the given array from lower to higher values
import java.util.Arrays;

public class Sort8 {
  public static void main(String[] args) {
    Sort8 s = new Sort8();

    int[] arr1 = new int[(int)(Math.random() * 11) + 5];
    for (int x = 0; x < arr1.length; x++) {
      arr1[x] = (int)(Math.random() * 10);
    }
    System.out.print("Array 1 : ");
    s.display1D(arr1);

    int[] sort = s.beadSort(arr1);
    System.out.print("Sorted : ");
    s.display1D(sort);
  }

  int[] beadSort(int[] arr) {
    int max = 0;
    for (int x = 0; x < arr.length; x++) {
      if (arr[x] > max)
        max = arr[x];
    }

    char[][] grid = new char[arr.length][max];
    int[] count1 = new int[max];
    for (int x = 0; x < max; x++) {
      count1[x] = 0;
      for (int y = 0; y < arr.length; y++) {
        grid[y][x] = '_';
      }
    }

    for (int x = 0; x < arr.length; x++) {
      int num = arr[x];
      for (int y = 0; num > 0; y++) {
        grid[count1[y]++][y] = '*';
        num--;
      }
    }

    System.out.println();
    display2D(grid);
    int[] sorted = new int[arr.length];
    for (int x = 0; x < arr.length; x++) {
      int tmp = 0;
      for (int y = 0; y < max && grid[arr.length - 1 - x][y] == '*'; y++) {
        tmp++;
      }
      sorted[x] = tmp;
    }

    return sorted;
  }

  void display1D(int[] arr) {
    for (int x = 0; x < arr.length; x++)
      System.out.print(arr[x] + " ");
    System.out.println();
  }

  void display1D(char[] arr) {
    for (int x = 0; x < arr.length; x++) 
      System.out.print(arr[x] + " ");
    System.out.println();
  }

  void display2D(char[][] arr) {
    for (int x = 0; x < arr.length; x++)
      display1D(arr[x]);
    System.out.println();
  }
}
