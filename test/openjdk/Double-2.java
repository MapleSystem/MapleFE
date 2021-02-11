public final class Double extends Number implements Comparable<Double> {
    public boolean equals(Object obj) {
        return doubleToLongBits(((Double)obj).value) == doubleToLongBits(value);
    }
}
