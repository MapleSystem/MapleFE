public final class Class<T> {
    public Method[] getDeclaredMethods() {
        Method[] result = getDeclaredMethodsUnchecked(false);
        for (Method m : result) {
            m.getParameterTypes();
        }
        return result;
    }
}
