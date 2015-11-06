package edu.asu.sma.client;


import android.os.Bundle;
import android.os.Environment;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class PostFragment extends BaseFragment {
  private static final String TAG = PostFragment.class.getSimpleName();
  private static String path = Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DCIM) + "/Camera";

  private List<String> items = new ArrayList<>();
  private ArrayAdapter<String> itemsAdapter;

  public PostFragment() {
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup container,
                           Bundle savedInstanceState) {
    return inflater.inflate(R.layout.fragment_post, container, false);
  }

  @Override
  public void onActivityCreated(Bundle savedInstanceState) {
    super.onActivityCreated(savedInstanceState);

    itemsAdapter = new ArrayAdapter<>(activity, android.R.layout.simple_list_item_1, items);
    final ListView itemsList = (ListView) activity.findViewById(R.id.postItemsList);
    itemsList.setAdapter(itemsAdapter);

    itemsList.setOnItemClickListener(new OnItemClickListener() {
      @Override
      public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
        TextView item = (TextView) adapterView.getChildAt(i - itemsList.getFirstVisiblePosition());
        String fileName = item.getText().toString();

        FragmentTransaction txn = getFragmentManager().beginTransaction();
        Fragment prev = getFragmentManager().findFragmentByTag("post_preview");
        if (prev != null)
          txn.remove(prev);
        txn.addToBackStack(null);

        PostPreviewDialog dialog = PostPreviewDialog.newInstance(path + "/" + fileName);
        dialog.setNativeHandler(nativeHandler);
        dialog.show(txn, "post_preview");

      }
    });

//    try {
//      String[] files = activity.getAssets().list("");
      File d = new File(path);
      File f[] = d.listFiles();
      String[] files = new String[f.length];
      for (int i=0; i<f.length; i++)
        files[i] = f[i].getName();
      items.clear();
      Collections.addAll(items, files);
      itemsAdapter.notifyDataSetChanged();
//    } catch (IOException e) {
//      throw new RuntimeException(e);
//    }
  }
}
