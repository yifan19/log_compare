package test;
import java.util.*;
import java.io.*;
import java.lang.Math;

class chooseRandom{
    
    public static void main(String[] args){
        System.out.println(testMethod(3, 5));
    }
    public static boolean testMethod(int init, int numLoops){ // L1
        boolean succeed = true;
        StringBuffer logBuffer = new StringBuffer();
        int val = init; // L2
        logBuffer.append( "L2 " + "val=" + val + " "); // L2 instrument: val
        
        for(int i=0; i<numLoops; i++){ // L3: loop condition
            logBuffer.append("L3 ").append(i<numLoops).append(" "); // L3 instrument: loop condition
            double ran = Math.random();
            logBuffer.append("L4 ").append(ran<0.5).append(" "); // L4 instrument: if condition
            if(ran<0.5){ // L4: if condition
                val--; // L5 
                logBuffer.append("L5 val=").append(val).append(" "); // L5 instrument: val change
            }
        }
        logBuffer.append("L6 ").append(val>0).append(" "); // L6 instrument: if condition
        if(val > 0){ // L6: if condition
            logBuffer.append("L7 fail"); // L7 instrument: throw Exception
            succeed = false;
        }

        File file = new File("test/chooseRandom_log.txt");
        try {
            BufferedWriter writer = new BufferedWriter(new FileWriter(file));
            file.createNewFile();
            writer.write(logBuffer.toString());
            writer.flush();
            writer.close();
            System.out.println(logBuffer.toString());
        } catch (IOException e) {
            System.out.println("write exception");
            e.printStackTrace();
        }

        return succeed;
    }
}