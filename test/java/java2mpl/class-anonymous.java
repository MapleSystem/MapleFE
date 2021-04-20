public class ChatTest {

    private static void performDontReceiveMessageInNameState() throws Exception {

        Thread client2 = new Thread(new ChatConnection() {
            @Override
            public void run(Socket socket, BufferedReader reader, Writer writer) throws Exception {
                writer.write("testClient2\n");
                //barrier3.await();
            }
        });
    }
}
