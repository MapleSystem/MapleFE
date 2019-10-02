// Using Merge Sort to sort the given array from low to high values
import java.util.Arrays;

class Sort4 {
   void merge(int nums[], int left, int l, int right) {
     int i1 = l - left + 1;
     int i2 = right - l;

     int left_arr1[] = new int [i1];
     int right_arr1[] = new int [i2];
   
     for (int x = 0; x < i1; ++x) 
       left_arr1[x] = nums[left + x];

     for (int y = 0; y < i2; ++y)
       right_arr1[y] = nums[l + 1 + y];


     int x = 0, y = 0;

     int j = left;
     while (x < i1 && y < i2) {
       if (left_arr1[x] <= right_arr1[y]) {
         nums[j] = left_arr1[x];
         x++;
       } else {
         nums[j] = right_arr1[y];
         y++;
       }
       j++;
     }

     while (x < i1) {
       nums[j] = left_arr1[x];
       x++;
       j++;
     }
   
     while (y < i2) {
       nums[j] = right_arr1[y];
       y++;
       j++;
     }
   }

   void sort(int nums[], int l, int r) {
     if (l < r) {
       int m = (l + r) / 2;
  
       sort(nums, l, m);

       sort(nums, m + 1, r);

       merge(nums, l, m, r);
     }
   }

   public static void main(String args[]) {
     Sort4 s = new Sort4();
     int nums[] = {6, -4, 2, 1, 0, 44};
     System.out.println("Array 1 : ");
     System.out.println(Arrays.toString(nums));
     s.sort(nums, 0, nums.length - 1);
     System.out.println("Sorted : " );
     System.out.println(Arrays.toString(nums));
   }
   
}
