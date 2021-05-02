/* @param array The array to search in.
 * @param value The value to search.
 * @return The index of the searched element in the sorted array, if one is found;
 * otherwise, a negative number that is the bitwise complement of the index of the next element that is large than the searched value or,
 * if there is no larger element(include the case that the array is empty), the bitwise complement of array's length.
 */
function binarySearch (array: number[], value: number) {
    return binarySearchEpsilon(array, value, 0);
}

/**
 * Searches the **sorted** number array for an element and returns the index of that element.
 * @param array The array to search in.
 * @param value The value to search.
 * @param EPSILON The epsilon to compare the numbers. Default to `1e-6`.
 * @return The index of the searched element in the sorted array, if one is found;
 * otherwise, a negative number that is the bitwise complement of the index of the next element that is large than the searched value or,
 * if there is no larger element(include the case that the array is empty), the bitwise complement of array's length.
 */
function binarySearchEpsilon (array: number[], value: number, EPSILON = 1e-6) {
    let low = 0;
    let high = array.length - 1;
    let middle = high >>> 1;
    for (; low <= high; middle = (low + high) >>> 1) {
        const test = array[middle];
        if (test > (value + EPSILON)) {
            high = middle - 1;
        } else if (test < (value - EPSILON)) {
            low = middle + 1;
        } else {
            return middle;
        }
    }
    return ~low;
}

var sequence: number[] = [13, 21, 34, 55, 89, 144];
console.log(binarySearch(sequence, 144));
