function binarySearch(array: number[], value: number): number {
  var low: number = 0;
  var high: number = array.length - 1;
  var mid: number = high >>> 1;
  for (; low <= high; mid = (low + high) >>> 1) {
    const test = array[mid];
    if (test > value) {
      high = mid - 1;
    } else if (test < value) {
      low = mid + 1;
    } else {
      return mid;
    }
  }
  return ~low;
}

var sequence: number[] = [13, 21, 34, 55, 89, 144];
console.log(binarySearch(sequence, 144));
