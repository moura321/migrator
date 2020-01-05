import java.util.Hashtable;
import helma.xmlrpc.*;

public class JavaServer {

    public JavaServer () {}

    public Hashtable sumAndDifference (int x, int y) {
        Hashtable result = new Hashtable();
        result.put("sum", x + y);
        result.put("difference", x - y);
        System.err.println("JavaServer");
        return result;
    }

    public static void main (String [] args) {
        try {
            WebServer server = new WebServer(8080);
            server.addHandler("sample", new JavaServer());

        } catch (Exception exception) {
            System.err.println("JavaServer: " + exception.toString());
        }
    }
}
