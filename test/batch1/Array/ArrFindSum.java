// Find two values sum equal to the given number

public class ArrFindSum {
  static void Find_Sum(int arr[], int sum) {
    System.out.println("Sum : ");

    for (int x = 0; x < arr.length; x++) {
      for (int y = x + 1; y < arr.length; y++) {
        if (arr[x] + arr[y] == sum) {
          System.out.println(arr[x] + " + " + arr[y] + " = " + sum);
        }
      }
    }
  }

  public static void main(String[] args) {
    Find_Sum(new int[] {9, 4, -3, 18, 6, 10, 5}, 15);

    Find_Sum(new int[] {10, 20, 30, -10, 40, -20, 9, 2, 29}, 20);
  }
}
