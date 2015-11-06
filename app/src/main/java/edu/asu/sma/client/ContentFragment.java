package edu.asu.sma.client;


import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import edu.asu.sma.ContentHelper;
import edu.asu.sma.InterestHelper;

public class ContentFragment extends BaseFragment implements NativeConsumer {
  private static final String TAG = ContentFragment.class.getSimpleName();

  private EditText newInterestText;

  private List<String> interests = new ArrayList<>();
  private List<String> metas = new ArrayList<>();

  private ArrayAdapter<String> interestsAdapter;
  private ArrayAdapter<String> metadataAdapter;
  private ContentListAdapter contentAdapter;

  private ScheduledExecutorService executor = Executors.newSingleThreadScheduledExecutor();


  public ContentFragment() {
    bind(this);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup container,
                           Bundle savedInstanceState) {
    return inflater.inflate(R.layout.fragment_content, container, false);
  }

  @Override
  public void onActivityCreated(Bundle savedInstanceState) {
    super.onActivityCreated(savedInstanceState);

    newInterestText = (EditText) activity.findViewById(R.id.newInterestText);

    interestsAdapter = new ArrayAdapter<>(activity,
                                          android.R.layout.simple_list_item_1,
                                          interests);
    final ListView interestsList = (ListView) activity.findViewById(R.id.interestsList);
    interestsList.setAdapter(interestsAdapter);

    metadataAdapter = new ArrayAdapter<>(activity,
                                      android.R.layout.simple_list_item_1,
                                      metas);

    final ListView metaList = (ListView) activity.findViewById(R.id.metasList);
    metaList.setAdapter(metadataAdapter);

    metaList.setOnItemClickListener(new OnItemClickListener() {
      @Override
      public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        TextView item = (TextView) parent.getChildAt(
                position-metaList.getFirstVisiblePosition());
        String meta = item.getText().toString();
        if (meta.endsWith(" (remote)")) {
          meta = meta.split("\\s+")[1];
          ContentHelper.fetch(meta);
          String log_str = "fetch content " + meta;
          Log.i(TAG, log_str);
        } else {
          meta = meta.split("\\s+")[1];
          String log_str = "openning content " + meta;
          Log.i(TAG, log_str);
        }
      }
    });

    interestsList.setOnItemClickListener(new OnItemClickListener() {
      @Override
      public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
        TextView item = (TextView) adapterView.getChildAt(
                i - interestsList.getFirstVisiblePosition());
        String interest = item.getText().toString();
        if (interest.endsWith(" (remote)"))
          InterestHelper.create(interest.substring(0, interest.length() - " (remote)".length()));
      }
    });

    activity.findViewById(R.id.addInterestButton).setOnClickListener(new OnClickListener() {
      @Override
      public void onClick(View view) {
        String interest = newInterestText.getText().toString();
        if (interest.length() == 0)
          return;

        InterestHelper.create(interest);

        newInterestText.setText("");
      }
    });

//   contentAdapter = new ContentListAdapter(activity);
//    final ListView contentList = (ListView) activity.findViewById(R.id.storedContentList);
//    contentList.setAdapter(contentAdapter);

    update();
    updateMeta();
//    updateContentList();
  }

  private void update() {

    executor.schedule(new Runnable() {
      @Override
      public void run() {
        update();
      }
    }, 500, TimeUnit.MILLISECONDS);

    if (nativeHandler == null)
      return;

    nativeHandler.post(new Runnable() {
      @Override
      public void run() {
        final List<String> local_interests = InterestHelper.local();
        final List<String> remote_interests = InterestHelper.remote();
        java.util.Collections.sort(local_interests);
        java.util.Collections.sort(remote_interests);

        activity.runOnUiThread(new Runnable() {
          @Override
          public void run() {
            interests.clear();
            interestsAdapter.addAll(local_interests);
            for (String remote : remote_interests)
              interestsAdapter.add(remote + " (remote)");
          }
        });
      }
    });


  }

  public void contentCompleted(String content_bame) {

  }

  private void updateMeta() {

    executor.schedule(new Runnable() {
      @Override
      public void run() {
        updateMeta();
      }
    }, 5000, TimeUnit.MILLISECONDS);


    if (nativeHandler == null)
      return;

    nativeHandler.post(new Runnable() {
      @Override
      public void run() {

        final List<String> local_metas = ContentHelper.local();
        final List<String> remote_metas = ContentHelper.remote();

        activity.runOnUiThread(new Runnable() {
          @Override
          public void run() {
            metas.clear();
            metadataAdapter.addAll(local_metas);
            for (String remote : remote_metas)
              metadataAdapter.add(remote + " (remote)");
          }
        });
      }
    });

  }

  private void updateContentList() {
//    String path = "/storage/emulated/legacy/Download";
//    File d = new File(path);
    String path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM) + "/Camera/";
    File d = new File(path);
    File files[] = d.listFiles();
    Log.d(TAG, "Content files: " + files.length);
    if (files.length != contentAdapter.getCount()) {
      contentAdapter.clear();
      for (File file : files) {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        try {
          InputStream in = new FileInputStream(file);
          byte[] buf = new byte[8192];
          int read = 0;
          while ((read = in.read(buf)) != -1)
            baos.write(buf, 0, read);
          in.close();
          contentAdapter.add(baos.toByteArray());

        } catch (java.io.IOException e) {
          Log.e(TAG, "Exception reading content file", e);
        }
      }
    }

    executor.schedule(new Runnable() {
      @Override
      public void run() {
        updateContentList();
      }
    }, 1000, TimeUnit.MILLISECONDS);
  }

  @Override
  public void onNativeHandler(Handler handler) {
    update();
  }
}
