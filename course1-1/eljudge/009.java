import java.util.Scanner;
import java.math.BigInteger;
import java.lang.Object;
class Main
{
    public static void main(String[] args)
    {
        Scanner s = new Scanner(System.in);
        int n = s.nextInt() + 1;
        BigInteger a = BigInteger.ONE;
        BigInteger b = BigInteger.ONE;
        BigInteger c = BigInteger.ONE;
        BigInteger d = BigInteger.ZERO;
        BigInteger rc = BigInteger.ZERO;
        BigInteger rd = BigInteger.ONE;
        BigInteger ta;
        BigInteger tb;
        BigInteger tc;
        while(n > 0)
        {
            if(n % 2 == 1)
            {
                tc = rc;
                rc = rc.multiply(a).add(rd.multiply(c));
                rd = tc.multiply(b).add(rd.multiply(d));
            }
            ta = a;
            tb = b;
            tc = c;
            a = a.multiply(a).add(b.multiply(c));
            b = ta.multiply(b).add(b.multiply(d));
            c = c.multiply(ta).add(d.multiply(c));
            d = tc.multiply(tb).add(d.multiply(d));
            n = n / 2;
        }
        System.out.println(rc.toString());
    }
}