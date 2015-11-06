package edu.asu.sma.client;


import android.os.Bundle;
import android.os.Handler;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import edu.asu.sma.NeighborHelper;

public class NetworkFragment extends BaseFragment implements NativeConsumer {
  private List<String> neighbors = new ArrayList<>();
  private ArrayAdapter<String> neighborsAdapter;
  private ScheduledExecutorService executor = Executors.newSingleThreadScheduledExecutor();

  public NetworkFragment() {
    bind(this);
  }

  @Override
  public void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
  }

  @Override
  public View onCreateView(LayoutInflater inflater, ViewGroup container,
                           Bundle savedInstanceState) {
    return inflater.inflate(R.layout.fragment_network, container, false);
  }

  @Override
  public void onActivityCreated(Bundle savedInstanceState) {
    super.onActivityCreated(savedInstanceState);

    neighborsAdapter = new ArrayAdapter<>(activity,
                                          android.R.layout.simple_list_item_1,
                                          neighbors);
    ListView neighbors_list = (ListView) activity.findViewById(R.id.neighbors_list);
    neighbors_list.setAdapter(neighborsAdapter);

    update();
  }

  @Override
  public void onNativeHandler(Handler handler) {
    update();
  }

  private void update() {
    if (nativeHandler == null)
      return;

    nativeHandler.post(new Runnable() {
      @Override
      public void run() {
        final List<String> updated_neighbors = NeighborHelper.all();
        activity.runOnUiThread(new Runnable() {
          @Override
          public void run() {
            neighbors.clear();
            neighborsAdapter.addAll(updated_neighbors);
          }
        });
      }
    });
    executor.schedule(new Runnable() {
      @Override
      public void run() {
        update();
      }
    }, 5000, TimeUnit.MILLISECONDS);
  }
}
