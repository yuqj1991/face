package com.example.clusterface;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Build;
import android.os.Environment;
import android.provider.ContactsContract;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.RandomAccessFile;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.util.ArrayList;
import java.util.List;


public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("clusterFacelib");
    }
    public String feature_filepath = "/storage/emulated/0/facefeature/";
    public String feature_filename = "cluster_feature.txt";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        // Example of a call to a native method
        //makeFilePath(feature_filepath, feature_filename);
        Button button =null;
        button = (Button) findViewById(R.id.button);
        button.setX(300);
        button.setY(500);
        checkPermission();

        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String star_path = "/storage/emulated/0/clusterface/";
                List<String> subDirList = getFilesAllName(star_path);
                System.out.println("Folder list size is : "+subDirList.size());
                for(int ii =0; ii<subDirList.size(); ii++){
                    String subImageName = subDirList.get(ii);
                    Log.w("sub directory ", subImageName);
                    List<String> subImageList = getFilesAllName(subImageName);
                    for(int jj =0; jj<subImageList.size(); jj++){
                        int img_width , img_height ;
                        Log.w("image file name",subImageList.get(jj));
                        Bitmap img = BitmapFactory.decodeFile(subImageList.get(jj));
                        img_width = img.getWidth();
                        img_height = img.getHeight();
                        System.out.println("width: "+img_width+" height: "+img_height);
                        boolean res = regFaceInit(img_width, img_height);
                        if (!res) {
                            Log.w("faceinit and create faceid state", "failed");
                            return;
                        }
                        Log.w("faceinit and create faceid state", "succeed");
                        boolean breg = true;
                        float[] faceFeature =RegFaceDetect(img, breg);
                        byte[] byteFeature = floatArray2byteArray(faceFeature);
                        writeTxtToFile(byteFeature,feature_filepath, feature_filename);
                        RegFaceUninit();
                    }
                }
            }
        });
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native boolean regFaceInit(int img_width,int img_height);
    public native void RegFaceUninit();
    public native float[] RegFaceDetect(Object bitmap, boolean breg);

    public static List<String> getFilesAllName(String path) {

        File file=new File(path);
        File[] files=file.listFiles();
        if (files == null){
            Log.e("error","empty directry");
            return null;
        }
        List<String> s = new ArrayList<>();
        for(int i =0;i<files.length;i++){
            s.add(files[i].getAbsolutePath());
        }
        return s;
    }

    public static byte[] floatArray2byteArray(float[] v){
        ByteBuffer byteBuffer = ByteBuffer.allocate(v.length * 4);
        for(int i = 0; i < v.length; i++)
            byteBuffer.putFloat(v[i]);
        return byteBuffer.array();
    }


    private void writeTxtToFile(byte[] strContent, String filePath, String fileName) {
        String strFilePath = filePath + fileName;

        File file = new File(strFilePath);
        if(!file.exists()){
            try {
                file.createNewFile();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        try {
            BufferedWriter bufferedWriter = new BufferedWriter(new FileWriter(file));
            String s = new String(strContent);
            bufferedWriter.write(s, 0, s.length());
            bufferedWriter.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void checkPermission(){

        if(ContextCompat.checkSelfPermission(this, Manifest.permission.READ_EXTERNAL_STORAGE)!= PackageManager.PERMISSION_GRANTED){
            ActivityCompat.requestPermissions(this, new String[]{Manifest.permission.READ_EXTERNAL_STORAGE}, 0);
        }
        if(ContextCompat.checkSelfPermission(this,Manifest.permission.WRITE_EXTERNAL_STORAGE)!= PackageManager.PERMISSION_GRANTED){
            ActivityCompat.requestPermissions(this,new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},0);
        }

    }
    
    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
    }
}
