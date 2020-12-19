abstract class AbstractStringBuilder implements Appendable, CharSequence {
    char[] value;

    int count;

    AbstractStringBuilder() {
    }

    AbstractStringBuilder(int capacity) {
    }

    public int length() {
    }

    public int capacity() {
    }
    public void ensureCapacity(int minimumCapacity) {
    }
    private void ensureCapacityInternal(int minimumCapacity) {
    }
    private static final int MAX_ARRAY_SIZE = Integer.MAX_VALUE - 8;

    private int newCapacity(int minCapacity) {
    }

    private int hugeCapacity(int minCapacity) {
    }

    public void trimToSize() {
    }

    public void setLength(int newLength) {
    }
    public char charAt(int index) {
    }

    public int codePointAt(int index) {
    }

    public int codePointBefore(int index) {
    }
    public int codePointCount(int beginIndex, int endIndex) {
    }
    public int offsetByCodePoints(int index, int codePointOffset) {
    }
    public void getChars(int srcBegin, int srcEnd, char[] dst, int dstBegin)
    {
    }
    public void setCharAt(int index, char ch) {
    }
    public AbstractStringBuilder append(Object obj) {
    }
    public AbstractStringBuilder append(String str) {
    }

    public AbstractStringBuilder append(StringBuffer sb) {
    }

    AbstractStringBuilder append(AbstractStringBuilder asb) {
    }

    public AbstractStringBuilder append(CharSequence s) {
    }

    private AbstractStringBuilder appendNull() {
    }
    public AbstractStringBuilder append(CharSequence s, int start, int end) {
    }
    public AbstractStringBuilder append(char[] str) {
    }
    public AbstractStringBuilder append(char str[], int offset, int len) {
    }
    public AbstractStringBuilder append(boolean b) {
    }
    public AbstractStringBuilder append(char c) {
    }
    public AbstractStringBuilder append(int i) {
    }
    public AbstractStringBuilder append(long l) {
    }
    public AbstractStringBuilder append(float f) {
    }
}
